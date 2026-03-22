#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/rand.h"
#include "pico/multicore.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"
#include "hardware/sync.h"

// 必须在 tusb.h 之前包含 pio_usb.h
#include "pio_usb.h"
#include "tusb.h"

// 定义Xbox 360控制器的XInput报告ID
#define XINPUT_REPORT_ID 0x00

#include "usb_descriptors.h"
#include "ws2812.pio.h"

// UART 调试输出
#define DEBUG_printf printf
#define DEBUG_LED 25

// ------------------------------------------------------------------
// 全局变量与枚举
// ------------------------------------------------------------------
typedef enum {
    MODE_SYNC = 0,    // 绿灯：同步模式
    MODE_MAIN_ONLY,   // 蓝灯：仅主角色 (手柄1)
    MODE_SUB_ONLY,    // 红灯：仅副角色 (手柄2)
    MODE_COUNT
} sync_mode_t;

static sync_mode_t current_mode = MODE_SYNC;
static bool last_toggle_state = false;
static xbox_report_t host_report;      // 从真实手柄读取的原始数据
static bool host_connected = false;

// 反作弊配置
#define ANTI_CHEAT_STRENGTH 30  // 0-100，默认30%
typedef struct {
    uint32_t last_update_ms;
    uint32_t delay_ms;
    xbox_report_t delayed_report;
} anti_detect_t;

static anti_detect_t ad_ctrl = {0, 1, {0}};

// 核心间同步信号
static volatile bool core1_ready = false;

// ------------------------------------------------------------------
// LED 定义
// ------------------------------------------------------------------
#define WS2812_PIN 16
#define WS2812_FREQ 800000

// ------------------------------------------------------------------
// WS2812 RGB LED 驱动 (官方方法)
// ------------------------------------------------------------------
static inline void put_pixel(uint32_t pixel_data) {
    // 对于RGB格式，ws2812_program_init会自动处理数据长度
    pio_sm_put_blocking(pio0, 0, pixel_data);
}

static inline void put_rgb(uint8_t r, uint8_t g, uint8_t b) {
    // RGB格式：构建24位GRB数据，ws2812_program_init会根据配置自动处理
    // 数据格式：GRB (Green, Red, Blue)
    uint32_t grb = ((uint32_t)g << 16) | ((uint32_t)r << 8) | ((uint32_t)b << 0);
    put_pixel(grb);
}

static inline void put_off(void) {
    put_rgb(0, 0, 0);
}

void init_ws2812(void) {
    uint offset = pio_add_program(pio0, &ws2812_program);
    ws2812_program_init(pio0, 0, offset, WS2812_PIN, WS2812_FREQ, false);  // 尝试使用RGB格式，可能LED硬件不支持RGBW
    printf("WS2812 initialized on GPIO %d (RGB format)\r\n", WS2812_PIN);
}

void update_led_indicator(void) {
    // 无论手柄是否连接，都显示当前模式的颜色
    // 但手柄未连接时亮度减半，以示区分
    uint8_t brightness = host_connected ? 255 : 64;
    
    DEBUG_printf("DEBUG: update_led_indicator - current_mode=%d, host_connected=%d, brightness=%d\r\n", current_mode, host_connected, brightness);
    
    switch (current_mode) {
        case MODE_SYNC:
            DEBUG_printf("DEBUG: Setting LED to GREEN\r\n");
            put_rgb(0, brightness, 0);
            break;
        case MODE_MAIN_ONLY:
            DEBUG_printf("DEBUG: Setting LED to BLUE\r\n");
            put_rgb(0, 0, brightness);
            break;
        case MODE_SUB_ONLY:
            DEBUG_printf("DEBUG: Setting LED to RED\r\n");
            put_rgb(brightness, 0, 0);
            break;
        default:
            DEBUG_printf("DEBUG: Setting LED to RED (DEFAULT)\r\n");
            put_rgb(brightness, 0, 0);
            break;
    }
}

// 模式切换时白闪3次
void flash_white_3times(void) {
    for (int i = 0; i < 3; i++) {
        put_rgb(255, 255, 255);  // 白色
        sleep_ms(100);
        put_off();
        sleep_ms(100);
    }
}

// ------------------------------------------------------------------
// 核心逻辑
// ------------------------------------------------------------------

// 计算基于强度的随机偏移量
static int16_t calculate_offset(int16_t base_range, uint8_t strength) {
    if (strength == 0) return 0;
    // 根据强度缩放偏移范围
    int16_t scaled_range = (base_range * strength) / 100;
    if (scaled_range < 1) scaled_range = 1;
    return (int16_t)((get_rand_32() % (scaled_range * 2 + 1)) - scaled_range);
}

static int16_t apply_stick_offset(int16_t value, uint8_t strength) {
    if (value == 0) return 0;
    
    // 摇杆偏移量：基准范围为±4，根据强度调整
    int16_t offset = calculate_offset(4, strength);
    int32_t new_val = (int32_t)value + offset;
    return (int16_t)(new_val > 32767 ? 32767 : (new_val < -32768 ? -32768 : new_val));
}

static uint8_t apply_trigger_offset(uint8_t value, uint8_t strength) {
    if (value == 0) return 0;
    
    // 扳机偏移量：基准范围为±2，根据强度调整
    int16_t offset = calculate_offset(2, strength);
    int16_t new_val = (int16_t)value + offset;
    return (uint8_t)(new_val > 255 ? 255 : (new_val < 0 ? 0 : new_val));
}

// 检查模式切换按钮组合
static bool check_mode_toggle(void) {
    static uint32_t last_toggle_time = 0;
    bool current_toggle = (host_report.buttons & (XBOX_BTN_BACK | XBOX_BTN_START)) == (XBOX_BTN_BACK | XBOX_BTN_START);
    uint32_t now = to_ms_since_boot(get_absolute_time());
    
    // 防长按重复触发：必须释放后再次按下
    if (current_toggle && !last_toggle_state) {
        // 添加最小按下间隔（300ms）
        if (now - last_toggle_time > 300) {
            last_toggle_time = now;
            return true;
        }
    }
    
    last_toggle_state = current_toggle;
    return false;
}

void process_and_send_reports(void) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    
    // 检查模式切换
    if (host_connected && check_mode_toggle()) {
        current_mode = (current_mode + 1) % MODE_COUNT;
        flash_white_3times();  // 模式切换时白闪3次
        update_led_indicator();
        printf("Mode changed to: %d\r\n", current_mode);
    }

    if (now - ad_ctrl.last_update_ms >= ad_ctrl.delay_ms) {
        // 始终发送报告，应用随机化强度
        ad_ctrl.delayed_report = host_report;
        
        uint8_t strength = ANTI_CHEAT_STRENGTH;
        
        ad_ctrl.delayed_report.left_stick_x = apply_stick_offset(host_report.left_stick_x, strength);
        ad_ctrl.delayed_report.left_stick_y = apply_stick_offset(host_report.left_stick_y, strength);
        ad_ctrl.delayed_report.right_stick_x = apply_stick_offset(host_report.right_stick_x, strength);
        ad_ctrl.delayed_report.right_stick_y = apply_stick_offset(host_report.right_stick_y, strength);
        ad_ctrl.delayed_report.left_trigger = apply_trigger_offset(host_report.left_trigger, strength);
        ad_ctrl.delayed_report.right_trigger = apply_trigger_offset(host_report.right_trigger, strength);
        
        // 确保报告ID和大小正确
        ad_ctrl.delayed_report.report_id = XINPUT_REPORT_ID;
        ad_ctrl.delayed_report.report_size = 0x14;  // 20字节，Xbox 360标准报告大小
        
        ad_ctrl.delay_ms = (get_rand_32() % 6) + 1;
        ad_ctrl.last_update_ms = now;
        if (tud_ready()) {
            // 使用Vendor特定设备函数发送控制器报告（接口0）
            tud_vendor_n_write(0, &ad_ctrl.delayed_report, sizeof(xbox_report_t));
            tud_vendor_n_flush(0);
        }
    }
}

// ------------------------------------------------------------------
// Core1 任务（处理 USB Host 和手柄读取）
// ------------------------------------------------------------------
void core1_main() {
    // 初始化 USB Host
    DEBUG_printf("Core1: Initializing USB Host...\r\n");
    pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
    tuh_configure(1, TUH_CFGID_RPI_PIO_USB_CONFIGURATION, &pio_cfg);
    tuh_init(1);
    
    core1_ready = true;
    DEBUG_printf("Core1: USB Host initialized\r\n");
    
    // Core1 主循环：仅处理 USB Host 任务
    while (1) {
        tuh_task();
    }
}

// ------------------------------------------------------------------
// USB Device 回调
// ------------------------------------------------------------------
void tud_mount_cb(void) {
    DEBUG_printf("TinyUSB: Device mounted\r\n");
}

void tud_umount_cb(void) {
    DEBUG_printf("TinyUSB: Device unmounted\r\n");
}

// ------------------------------------------------------------------
// USB Host 回调
// ------------------------------------------------------------------
void tuh_mount_cb(uint8_t dev_addr) {
    DEBUG_printf("TinyUSB: Host device mounted (addr=%d)\r\n", dev_addr);
    host_connected = true;
}

void tuh_umount_cb(uint8_t dev_addr) {
    DEBUG_printf("TinyUSB: Host device unmounted (addr=%d)\r\n", dev_addr);
    host_connected = false;
    memset(&host_report, 0, sizeof(xbox_report_t));
}

// HID 报告接收回调
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    (void)dev_addr;
    (void)instance;
    if (len >= sizeof(xbox_report_t)) {
        memcpy(&host_report, report, sizeof(xbox_report_t));
        process_and_send_reports();
    }
}

// ------------------------------------------------------------------
// 主函数
// ------------------------------------------------------------------
int main(void) {
    // 设置系统时钟为 120MHz (必须是 12MHz 的倍数, 否则 PIO-USB 无法正常工作)
    set_sys_clock_khz(120000, true);

    stdio_init_all();
    DEBUG_printf("\r\n=== POE2GamePad Starting ===\r\n");
    DEBUG_printf("RP2350 PIO-USB Xbox 360 Controller Sync\r\n");
    DEBUG_printf("System clock: %ld Hz\r\n", clock_get_hz(clk_sys));

    // 初始化 GPIO18 控制 5V 供电（开启）
    const uint32_t GPIO_5V_EN = 18;
    gpio_init(GPIO_5V_EN);
    gpio_set_dir(GPIO_5V_EN, GPIO_OUT);
    gpio_put(GPIO_5V_EN, 1);  // 开启5V供电
    DEBUG_printf("GPIO18: 5V power enabled\r\n");

    // 初始化 WS2812 RGB 灯
    init_ws2812();
    DEBUG_printf("WS2812 initialized\r\n");

    // 设置初始模式并更新LED
    current_mode = MODE_SYNC;
    DEBUG_printf("Initializing with MODE_SYNC\r\n");
    update_led_indicator();  // 初始时手柄未连接，LED显示低亮度绿色
    DEBUG_printf("Initial mode: SYNC (LED shows low-brightness green until controller connected)\r\n");

    // 初始化 USB Device（Core0 负责）
    DEBUG_printf("Core0: Initializing USB Device...\r\n");
    tusb_init();
    DEBUG_printf("Core0: USB Device initialized\r\n");

    // 启动 Core1 任务（处理 USB Host 和手柄读取）
    DEBUG_printf("Core0: Starting Core1 task...\r\n");
    multicore_launch_core1(core1_main);
    
    // 等待 Core1 初始化完成
    while (!core1_ready) {
        tight_loop_contents();
    }

    DEBUG_printf("Core0: Entering main loop...\r\n");

    // Core0 主循环：处理 USB Device 任务和核心逻辑
    while (1) {
        tud_task();
        
        // 定期更新LED状态（确保手柄未连接时灯灭）
        static uint32_t last_led_update = 0;
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_led_update >= 100) {  // 每100ms更新一次
            update_led_indicator();
            last_led_update = now;
        }
        
        // 处理报告（模式切换、反作弊、虚拟柄输出）
        if (host_connected) {
            process_and_send_reports();
        }
    }
    return 0;
}

// ------------------------------------------------------------------
// Vendor 特定类回调函数 - Xbox 360 控制器(XInput)
// ------------------------------------------------------------------

// 当从PC接收到数据时调用（如震动和LED控制命令）
void tud_vendor_rx_cb(uint8_t itf) {
    uint8_t buffer[32];
    uint16_t len = tud_vendor_n_read(itf, buffer, sizeof(buffer));
    DEBUG_printf("DEBUG: Vendor RX callback for interface %d, len=%d\r\n", itf, len);
    // 简单忽略震动和LED控制命令，我们不支持这些功能
}

// 当数据发送到PC完成时调用
void tud_vendor_tx_cb(uint8_t itf, uint32_t sent_bytes) {
    (void) itf;
    (void) sent_bytes;
    DEBUG_printf("DEBUG: Vendor TX callback for interface %d, sent=%d bytes\r\n", itf, sent_bytes);
    // 数据发送完成后不需要特别处理
}

// USB控制请求处理 - Xbox 360控制器(XInput)识别的关键
bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request) {
    // 忽略数据阶段
    if (stage != CONTROL_STAGE_SETUP) return true;
    
    DEBUG_printf("DEBUG: Vendor control request - bmRequestType=%02x, bRequest=%02x, wValue=%04x, wIndex=%04x, wLength=%04x\r\n", 
                request->bmRequestType, request->bRequest, request->wValue, request->wIndex, request->wLength);
    
    // 直接解析请求类型
    uint8_t direction = (request->bmRequestType >> 7) & 0x01;
    uint8_t type = (request->bmRequestType >> 5) & 0x03;
    uint8_t recipient = request->bmRequestType & 0x1F;
    
    // XInput协议使用的是Class + Interface类型的请求
    if (type == 0x01 && recipient == 0x01) { // Class + Interface
        switch (request->bRequest) {
            case 0x01: // SET_REPORT
                // 处理SET_REPORT请求（用于震动和LED控制）
                DEBUG_printf("DEBUG: XInput SET_REPORT request received\r\n");
                uint8_t buffer[32] = {0};
                if (request->wLength > sizeof(buffer)) return false;
                
                // 告诉TinyUSB我们准备好接收数据
                tud_control_xfer(rhport, request, buffer, request->wLength);
                return true;
                
            case 0x02: // GET_REPORT
                // 处理GET_REPORT请求（用于获取输入数据或功能报告）
                DEBUG_printf("DEBUG: XInput GET_REPORT request received\r\n");
                
                // 根据请求类型返回适当的数据
                uint8_t report_data[32] = {0};
                
                // 对于Xbox 360控制器，功能报告应该包含设备状态信息
                if ((request->wValue & 0xFF00) == 0x0200) { // Feature report
                    // 设置基本的设备信息
                    report_data[0] = 0x00;  // Report ID
                    report_data[1] = 0x14;  // Report size (20 bytes)
                    // 其他字节保持0
                }
                
                tud_control_xfer(rhport, request, report_data, request->wLength);
                return true;
                
            case 0x03: // SET_IDLE
                // 处理SET_IDLE请求
                DEBUG_printf("DEBUG: XInput SET_IDLE request received\r\n");
                tud_control_xfer(rhport, request, NULL, 0);
                return true;
                
            case 0x04: // GET_IDLE
                // 处理GET_IDLE请求
                DEBUG_printf("DEBUG: XInput GET_IDLE request received\r\n");
                uint8_t idle_response = 0;
                tud_control_xfer(rhport, request, &idle_response, 1);
                return true;
                
            case 0x05: // SET_PROTOCOL
                // 处理SET_PROTOCOL请求
                DEBUG_printf("DEBUG: XInput SET_PROTOCOL request received\r\n");
                tud_control_xfer(rhport, request, NULL, 0);
                return true;
                
            case 0x06: // GET_PROTOCOL
                // 处理GET_PROTOCOL请求
                DEBUG_printf("DEBUG: XInput GET_PROTOCOL request received\r\n");
                uint8_t protocol_response = 0;
                tud_control_xfer(rhport, request, &protocol_response, 1);
                return true;
        }
    }
    
    // 处理标准的USB控制请求
    if (type == 0x00) { // Standard request
        switch (request->bRequest) {
            case 0x06: // GET_DESCRIPTOR
                // 让TinyUSB处理描述符请求
                DEBUG_printf("DEBUG: Standard GET_DESCRIPTOR request received\r\n");
                return false;
                
            case 0x09: // SET_CONFIGURATION
                // 处理SET_CONFIGURATION请求
                DEBUG_printf("DEBUG: Standard SET_CONFIGURATION request received\r\n");
                tud_control_xfer(rhport, request, NULL, 0);
                return true;
                
            case 0x08: // GET_CONFIGURATION
                // 处理GET_CONFIGURATION请求
                DEBUG_printf("DEBUG: Standard GET_CONFIGURATION request received\r\n");
                uint8_t config_response = 0x01;
                tud_control_xfer(rhport, request, &config_response, 1);
                return true;
                
            case 0x0A: // GET_INTERFACE
                // 处理GET_INTERFACE请求
                DEBUG_printf("DEBUG: Standard GET_INTERFACE request received\r\n");
                uint8_t interface_response = 0x00;
                tud_control_xfer(rhport, request, &interface_response, 1);
                return true;
                
            case 0x0B: // SET_INTERFACE
                // 处理SET_INTERFACE请求
                DEBUG_printf("DEBUG: Standard SET_INTERFACE request received\r\n");
                tud_control_xfer(rhport, request, NULL, 0);
                return true;
        }
    }
    
    // 默认情况下，让TinyUSB处理未识别的请求
    DEBUG_printf("DEBUG: Unhandled request, passing to TinyUSB\r\n");
    return false;
}
