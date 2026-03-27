#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "tusb.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include <stdio.h>
#include <string.h>
#include "class/hid/hid_host.h"
#include "class/xinput/xinput_device.h"
#include "usb_descriptors.h"
#include "led_manager.h"
#include "mode_manager.h"

// 硬件引脚定义 - Adafruit Feather RP2040 with USB Type A Host
// 根据官方PDF手册：
// USB Host D+ (Data Plus) - GPIO16
// USB Host D- (Data Minus) - GPIO17  
// USB Host 5V Power - GPIO18
// 5V LED (绿色) - 位于D6/GPIO6旁边，指示5V电源状态

#define USB_HOST_DP_PIN 16    // USB主机D+ (Data Plus)
#define USB_HOST_DM_PIN 17    // USB主机D- (Data Minus) 
#define USB_HOST_POWER_PIN 18 // USB主机5V电源控制
#define POWER_STATUS_PIN 6    // 5V电源状态LED引脚 (GPIO6)
#define BUILTIN_LED_PIN 25    // 板载LED引脚 (GPIO25)

// 全局状态
static volatile bool gamepad_connected = false;

// 打印系统信息
void print_system_info(void) {
    printf("=== System Information ===\n");
    printf("Project: GameSir Pad Synchronizer\n");
    printf("Board: Adafruit Feather RP2040 with USB Type A Host\n");
    printf("PIO-USB Host Pins: D+ GPIO16, D- GPIO17 (5V: GPIO18)\n");
    printf("WS2812 LED Pin: GPIO19\n");
    printf("USB Host 5V Power Control: GPIO18\n");
    printf("5V Power Status LED (Green): GPIO6\n");
    printf("Builtin LED Pin: GPIO25\n");
    printf("System Clock: %lu Hz\n", clock_get_hz(clk_sys));
    printf("===========================\n");
}

// 优化电源管理：根据手册控制TPS61023升压转换器
static void optimize_power_management(void) {
    printf("Optimizing TPS61023 power management (1A boost converter)...\n");
    
    // 确保电源干净关闭
    gpio_put(USB_HOST_POWER_PIN, 0);
    sleep_ms(200);
    
    // 高级软启动序列 - 避免电流冲击
    printf("  Advanced soft-start sequence for TPS61023:\n");
    for (int i = 0; i < 10; i++) {
        // PWM式软启动：逐渐增加占空比
        gpio_put(USB_HOST_POWER_PIN, 1);
        sleep_ms(5 + i * 2);  // 逐渐增加开启时间
        gpio_put(USB_HOST_POWER_PIN, 0);
        sleep_ms(10 - i);     // 逐渐减少关闭时间
        
        if (i % 2 == 0) {
            printf("    Step %d/10\n", i + 1);
        }
    }
    
    // 最终开启电源
    gpio_put(USB_HOST_POWER_PIN, 1);
    sleep_ms(300); // 等待升压转换器完全稳定
    
    printf("  TPS61023 fully enabled, 5V output should be stable\n");
}

// 检查5V电源状态
static bool check_5v_power_status(void) {
    // 读取GPIO6状态（5V电源状态检测）
    sleep_ms(10); // 等待稳定
    bool status = gpio_get(POWER_STATUS_PIN);
    
    // 如果状态异常，打印详细信息
    if (!status) {
        printf("  5V power status check: FAIL (LOW)\n");
        printf("  Possible causes:\n");
        printf("  1. TPS61023 not fully enabled\n");
        printf("  2. Overcurrent protection triggered\n");
        printf("  3. Power supply insufficient\n");
        printf("  4. GPIO6 configuration incorrect\n");
    } else {
        printf("  5V power status check: OK (HIGH)\n");
    }
    
    return status;
}

// 初始化硬件
void init_hardware(void) {
    printf("Initializing hardware...\n");
    
    // 初始化PIO-USB主机引脚（根据官方PDF手册）
    printf("Initializing PIO-USB host pins for Adafruit Feather RP2040 USB-A Host\n");
    
    // USB主机D+ (GPIO16) - 数据正极
    printf("  USB Host D+ (Data Plus): GPIO16\n");
    gpio_init(USB_HOST_DP_PIN);
    gpio_set_dir(USB_HOST_DP_PIN, GPIO_OUT);
    
    // USB主机D- (GPIO17) - 数据负极  
    printf("  USB Host D- (Data Minus): GPIO17\n");
    gpio_init(USB_HOST_DM_PIN);
    gpio_set_dir(USB_HOST_DM_PIN, GPIO_OUT);
    
    // USB主机5V电源控制 (GPIO18) - TPS61023升压转换器（最大1A输出）
    printf("  USB Host 5V Power control: GPIO18 (TPS61023 boost converter, 1A peak)\n");
    gpio_init(USB_HOST_POWER_PIN);
    gpio_set_dir(USB_HOST_POWER_PIN, GPIO_OUT);
    
    // 根据手册：TPS61023使能引脚连接到GPIO18，用于手动电源控制
    printf("    TPS61023 Boost Converter Initialization:\n");
    
    // 调用优化的电源管理函数
    optimize_power_management();
    
    // 检查5V电源状态
    bool power_ok = check_5v_power_status();
    
    printf("    USB 5V power: %s (TPS61023 %s)\n", 
           power_ok ? "ON" : "OFF/UNSTABLE",
           power_ok ? "fully enabled" : "may need reset");
    printf("    Note: Power management allows hard-reset via GPIO18 control\n");
    
    // 5V电源状态检测引脚 (GPIO6) - 用于检测5V电源状态
    // 根据手册：5V LED位于USB-A端口后面，应该由电源管理电路控制
    printf("  5V Power status detection: GPIO6\n");
    printf("  Note: This pin likely reads 5V power status, not directly controls LED\n");
    
    // 安全初始化：先设置为输入模式，上拉电阻
    gpio_init(POWER_STATUS_PIN);
    gpio_set_dir(POWER_STATUS_PIN, GPIO_IN);
    gpio_pull_up(POWER_STATUS_PIN);
    
    // 短暂延迟后读取状态
    sleep_ms(10);
    bool power_status = gpio_get(POWER_STATUS_PIN);
    printf("  Current 5V power status: %s\n", power_status ? "HIGH (OK)" : "LOW (FAIL)");
    
    // 初始化板载LED引脚 (GPIO25) - 用于指示电源状态
    printf("Initializing builtin LED on GPIO25 (power indicator)\n");
    gpio_init(BUILTIN_LED_PIN);
    gpio_set_dir(BUILTIN_LED_PIN, GPIO_OUT);
    gpio_put(BUILTIN_LED_PIN, 1); // 点亮板载LED表示系统运行
    printf("  Builtin LED: ON\n");
    
    // 初始化LED管理器（WS2812）- 使用GPIO19
    printf("Initializing WS2812 LED on GPIO19\n");
    led_init();
    led_set_state(LED_STATE_INIT);
    printf("  WS2812 LED initialized\n");
    
    printf("Hardware initialization complete\n");
    printf("USB Host 5V power: %s, Status: %s\n", 
           gpio_get(USB_HOST_POWER_PIN) ? "ON" : "OFF", 
           gpio_get(POWER_STATUS_PIN) ? "OK" : "FAIL");
}

// USB设备挂载回调函数
void tud_mount_cb(void) {
    printf("USB Device: Mounted successfully!\n");
    printf("Device is now visible to PC as HID gamepad\n");
    printf("Device Info: VID=0x%04X, PID=0x%04X\n", GAMEPAD_VID, GAMEPAD_PID);
    printf("Device will appear as XInput controller in Windows\n");
}

void tud_umount_cb(void) {
    printf("USB Device: Unmounted\n");
}

// Core1: USB主机任务
void core1_main() {
    printf("Core1: Starting USB host initialization\n");
    
    // 等待USB设备栈完全初始化
    printf("Core1: Waiting for USB device stack to stabilize...\n");
    sleep_ms(1000);
    
    // 初始化HID主机模块
    printf("Core1: Initializing HID host module...\n");
    hid_host_init();
    
    // 等待PIO-USB硬件完全启动
    printf("Core1: Waiting for PIO-USB hardware to initialize...\n");
    sleep_ms(1000);
    
    // 开启手柄电源（已经在init_hardware中开启，这里确保保持开启）
    printf("Core1: Ensuring USB host 5V power is enabled on GPIO18\n");
    gpio_put(USB_HOST_POWER_PIN, 1);
    sleep_ms(500); // 确保电源稳定
    
    printf("Core1: Entering USB host task loop\n");
    
    // USB主机任务循环
    while (1) {
        // 处理USB主机事件
        hid_host_task();
        
        // 定期检查电源状态和控制板载LED
        static absolute_time_t last_power_check = {0};
        static bool power_check_initialized = false;
        
        if (!power_check_initialized) {
            last_power_check = get_absolute_time();
            power_check_initialized = true;
        }
        
        if (absolute_time_diff_us(get_absolute_time(), last_power_check) > 100000) {
            // 确保USB主机5V电源开启
            gpio_put(USB_HOST_POWER_PIN, 1);
            
            // 读取5V电源状态 (GPIO6) - 根据手册，这是状态检测引脚
            bool power_ok = gpio_get(POWER_STATUS_PIN);
            
            // 控制板载LED (GPIO25) 作为系统运行指示灯
            static bool led_state = false;
            led_state = !led_state;
            gpio_put(BUILTIN_LED_PIN, led_state);
            
            // 定期打印电源状态
            static uint8_t status_counter = 0;
            status_counter++;
            if (status_counter >= 10) { // 每1秒打印一次
                printf("Core1: USB Host 5V power: ON, Status: %s\n", 
                       power_ok ? "OK (HIGH)" : "FAIL (LOW)");
                status_counter = 0;
                
                // 如果电源状态异常，尝试重置TPS61023
                if (!power_ok) {
                    printf("Core1: 5V power failure detected, resetting TPS61023...\n");
                    gpio_put(USB_HOST_POWER_PIN, 0);
                    sleep_ms(100);
                    gpio_put(USB_HOST_POWER_PIN, 1);
                    sleep_ms(200);
                }
            }
            
            last_power_check = get_absolute_time();
        }
        
        sleep_ms(10);
    }
}

// 主函数
int main(void) {
    stdio_init_all();
    
    // 等待串口连接（用于调试）
    sleep_ms(2000);
    
    printf("\n=== GameSir Pad Synchronizer ===\n");
    printf("Build Date: %s %s\n", __DATE__, __TIME__);
    printf("GameSir T4 Kaleid Controller Synchronizer\n");
    printf("Features: Dual Virtual XInput, Mode Switching, Anti-Cheat Offsets\n");
    
    // 打印系统信息
    print_system_info();
    
    // 初始化硬件
    init_hardware();
    led_blink(255, 255, 255, 3, 200); // 白色启动闪烁
    
    // 设置系统时钟
    printf("Setting system clock to 120MHz\n");
    set_sys_clock_khz(120000, true);
    
    // 初始化模式管理器
    mode_manager_init();
    
    // 初始化USB设备（虚拟XInput） - 先于Core1启动
    printf("Initializing USB device (Dual Virtual XInput)\n");
    printf("USB Device Parameters:\n");
    printf("  VID: 0x%04X, PID: 0x%04X\n", GAMEPAD_VID, GAMEPAD_PID);
    printf("  Interfaces: 2 (Dual XInput Controllers)\n");
    printf("  Manufacturer: GameSir\n");
    printf("  Product: GameSir T4 Kaleid Controller\n");
    printf("  Serial: SN240327001\n");
    printf("  String descriptors loaded\n");
    
    // 检查tinyusb配置
    #if CFG_TUD_ENABLED
        printf("  TinyUSB Device stack: ENABLED\n");
    #else
        printf("  TinyUSB Device stack: DISABLED (this is wrong!)\n");
    #endif
    
    #if CFG_TUD_HID >= 2
        printf("  HID device interfaces: %d (OK for dual virtual controllers)\n", CFG_TUD_HID);
    #else
        printf("  HID device interfaces: %d (needs at least 1)\n", CFG_TUD_HID);
    #endif
    
    // 初始化tinyusb设备栈 - 只初始化设备端（硬件USB）
    printf("Initializing USB device stack (tud_init)...\n");
    printf("Using custom USB descriptors with VID=0x%04X, PID=0x%04X\n", GAMEPAD_VID, GAMEPAD_PID);
    
    // 等待系统稳定
    sleep_ms(500);
    
    tud_init(0);  // 只初始化设备端，避免与Core1的主机端冲突
    printf("USB device stack initialized\n");
    
    // 初始化XInput设备
    printf("Initializing XInput device interface...\n");
    xinput_device_init();
    
    // 等待USB设备枚举完成
    sleep_ms(1000);
    
    // 启动Core1处理USB主机功能
    printf("Starting Core1 USB host task\n");
    multicore_launch_core1(core1_main);
    
    printf("System initialization complete\n");
    printf("Current mode: %s\n", mode_manager_get_name(current_mode));
    printf("Waiting for gamepad connection...\n");
    
    // 设置系统就绪状态
    led_set_state(LED_STATE_READY);
    
    // 等待USB设备被Windows完全识别
    printf("Waiting for Windows to fully recognize USB device...\n");
    sleep_ms(2000);
    
    // 发送初始XInput报告，让Windows立即识别设备
    printf("Sending initial XInput report to Windows...\n");
    xinput_report_t init_report = {0};
    init_report.report_id = 0x00;
    // 发送到两个接口
    xinput_device_send_report(0, &init_report);
    xinput_device_send_report(1, &init_report);
    printf("Initial XInput reports sent\n");
    
    // 报告缓冲区
    xinput_report_t main_report = {0};
    xinput_report_t sub_report = {0};
    uint8_t raw_hid_report[64];
    uint16_t raw_hid_len = 0;
    
    // 主循环
    while (1) {
        // 处理USB设备任务
        tud_task();
        
        // 检查手柄连接状态变化
        static bool last_connected_state = false;
        bool current_connected_state = hid_host.connected;
        
        if (current_connected_state != last_connected_state) {
            if (current_connected_state) {
                printf("GameSir gamepad connected! VID=0x%04X, PID=0x%04X\n", 
                       hid_host.vid, hid_host.pid);
                gamepad_connected = true;
                led_blink(0, 255, 0, 1, 100); // 绿色连接确认闪烁
            } else {
                printf("GameSir gamepad disconnected\n");
                gamepad_connected = false;
                led_blink(255, 0, 0, 2, 100); // 红色断开确认闪烁
            }
            last_connected_state = current_connected_state;
        }
        
        // 根据手柄连接状态更新LED
        led_update(gamepad_connected, current_mode);
        
        // 如果手柄已连接，处理输入数据
        if (gamepad_connected) {
            // 读取原始HID报告
            if (hid_host_get_raw_report(raw_hid_report, &raw_hid_len)) {
                // 转换为XInput格式
                xinput_report_t converted_report = hid_to_xinput_convert(raw_hid_report, raw_hid_len);
                
                // 检测模式切换（Menu+View长按）
                mode_manager_check_switch(converted_report.buttons);
                
                // 根据模式处理报告
                mode_manager_process_report(&converted_report, current_mode, &main_report, &sub_report);
                
                // 根据模式发送报告
                switch (current_mode) {
                    case SYNC_MODE:
                        // 同步模式：发送两个不同的报告
                        xinput_device_send_report(0, &main_report);
                        xinput_device_send_report(1, &sub_report);
                        break;
                        
                    case MAIN_MODE:
                        // 仅主模式：只发送主控制器报告
                        xinput_device_send_report(0, &main_report);
                        break;
                        
                    case SUB_MODE:
                        // 仅副模式：只发送副控制器报告
                        xinput_device_send_report(1, &sub_report);
                        break;
                        
                    default:
                        break;
                }
                
                // 应用反作弊延迟（仅副控制器）
                if (current_mode == SYNC_MODE || current_mode == SUB_MODE) {
                    uint16_t delay_ms = mode_manager_get_anticheat_delay();
                    sleep_ms(delay_ms);
                }
            }
        }
        
        // 定期状态报告（每30秒）
        static absolute_time_t last_status_report = {0};
        static bool status_report_initialized = false;
        
        if (!status_report_initialized) {
            last_status_report = get_absolute_time();
            status_report_initialized = true;
        }
        
        if (absolute_time_diff_us(get_absolute_time(), last_status_report) > 30000000) { // 30秒
            printf("=== System Status ===\n");
            printf("  Gamepad connected: %s\n", gamepad_connected ? "YES" : "NO");
            printf("  Current mode: %s\n", mode_manager_get_name(current_mode));
            printf("  USB Device mounted: %s\n", tud_mounted() ? "YES" : "NO");
            printf("  Time: %lu ms\n", to_ms_since_boot(get_absolute_time()));
            printf("======================\n");
            
            last_status_report = get_absolute_time();
        }
        
        sleep_ms(10);
    }
    
    return 0;
}
