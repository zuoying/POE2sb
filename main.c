#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "tusb.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/multicore.h"
#include "ws2812.pio.h"
#include "class/xinput/xinput_host.h"

// 硬件定义
#define LED_PIN 16
#define POWER_PIN 18
#define USB_HOST_DP_PIN 12
#define USB_HOST_DM_PIN 13

// WS2812定义
#define WS2812_PIN 16
#define WS2812_FREQ 800000

// 全局状态
static volatile bool gamepad_connected = false;

// WS2812 LED驱动函数
static inline void put_pixel(uint32_t pixel_data) {
    pio_sm_put_blocking(pio0, 0, pixel_data);
}

static inline void put_rgb(uint8_t r, uint8_t g, uint8_t b) {
    // RGB格式：GRB
    uint32_t grb = ((uint32_t)g << 16) | ((uint32_t)r << 8) | ((uint32_t)b << 0);
    put_pixel(grb);
}

void init_ws2812(void) {
    uint offset = pio_add_program(pio0, &ws2812_program);
    ws2812_program_init(pio0, 0, offset, WS2812_PIN, WS2812_FREQ, false);
    puts("WS2812 LED initialized");
}

// LED闪烁函数
void led_blink(int times, int delay_ms) {
    for (int i = 0; i < times; i++) {
        put_rgb(255, 255, 255); // 白色闪烁
        sleep_ms(delay_ms);
        put_rgb(0, 0, 0);       // 关闭
        sleep_ms(delay_ms);
    }
}

// 设置LED颜色
void set_led_color(uint8_t r, uint8_t g, uint8_t b) {
    put_rgb(r, g, b);
}

// 打印系统信息
void print_system_info(void) {
    printf("=== System Information ===\n");
    printf("Board: Waveshare RP2350-USB-A\n");
    printf("PIO-USB Host Pins: D+ GPIO12, D- GPIO13\n");
    printf("LED Pin: GPIO16\n");
    printf("Power Control Pin: GPIO18\n");
    printf("System Clock: %lu Hz\n", clock_get_hz(clk_sys));
    printf("===========================\n");
}

// 初始化硬件
void init_hardware(void) {
    printf("Initializing hardware...\n");
    
    // 初始化LED
    printf("Initializing WS2812 LED on GPIO16\n");
    init_ws2812();
    put_rgb(0, 0, 255); // 蓝色：初始化中
    
    // 初始化5V电源控制
    printf("Initializing power control on GPIO18\n");
    gpio_init(POWER_PIN);
    gpio_set_dir(POWER_PIN, GPIO_OUT);
    // 使用正确的驱动强度常量（根据Pico SDK版本）
    #ifdef GPIO_DRIVE_STRENGTH_16MA
        gpio_set_drive_strength(POWER_PIN, GPIO_DRIVE_STRENGTH_16MA);
    #elif defined(GPIO_DRIVE_STRENGTH_12MA)
        gpio_set_drive_strength(POWER_PIN, GPIO_DRIVE_STRENGTH_12MA);
    #else
        // 默认使用中等驱动强度
        gpio_set_drive_strength(POWER_PIN, GPIO_DRIVE_STRENGTH_8MA);
    #endif
    gpio_put(POWER_PIN, 0); // 先关闭电源
    
    printf("Hardware initialized\n");
}

// USB主机回调函数
void tuh_mount_cb(uint8_t dev_addr) {
    printf("USB device mounted, address = %d\n", dev_addr);
    
    // 获取设备描述符以识别设备类型
    // 注意：tuh_device_get_descriptor API在TinyUSB不同版本中可能有变化
    // 这里使用条件编译来处理不同版本
    #ifdef TINYUSB_VERSION_MAJOR
        #if TINYUSB_VERSION_MAJOR >= 1
            // 新版本TinyUSB API
            const tusb_desc_device_t* dev_desc = tuh_device_get_descriptor(dev_addr);
            if (dev_desc) {
                printf("Device VID: 0x%04X, PID: 0x%04X\n", dev_desc->idVendor, dev_desc->idProduct);
                printf("Device Class: %u, SubClass: %u, Protocol: %u\n", 
                       dev_desc->bDeviceClass, dev_desc->bDeviceSubClass, dev_desc->bDeviceProtocol);
            }
        #else
            // 旧版本API，暂时跳过设备描述符获取
            printf("Device connected (device descriptor API not available in this TinyUSB version)\n");
        #endif
    #else
        // 未知版本，使用简化处理
        printf("Device connected\n");
    #endif
    
    // 等待XInput主机模块检测手柄
    // LED状态将由主循环根据gamepad_connected状态更新
}

void tuh_umount_cb(uint8_t dev_addr) {
    printf("USB device unmounted, address = %d\n", dev_addr);
    gamepad_connected = false;
    set_led_color(255, 0, 0); // 红色：手柄未连接
    led_blink(2, 100);
}

// HID报告接收回调 - 已在xinput_host.c中定义
// 注释掉以避免重复定义错误
/*
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, 
                              uint8_t const* report, uint16_t len) {
    (void) dev_addr;
    (void) instance;
    (void) len;
    
    // 简单的报告处理示例
    printf("Received HID report, length: %d\n", len);
}
*/

// Core1: USB主机任务
void core1_main() {
    printf("Core1: Starting USB host initialization\n");
    
    // 初始化USB主机 - 使用正确的端口号
    // 对于RP2350 PIO-USB，通常使用端口0
    tuh_init(0);
    
    // 初始化XInput主机模块
    xinput_host_init();
    
    // 开启手柄电源
    printf("Core1: Enabling gamepad power\n");
    gpio_put(POWER_PIN, 1);
    sleep_ms(500); // 等待电源稳定
    
    printf("Core1: Entering USB host task loop\n");
    
    // USB主机任务循环
    while (1) {
        // 处理USB主机事件
        tuh_task();
        
        // 处理XInput主机任务
        xinput_host_task();
        
        // 定期检查电源状态
        static absolute_time_t last_power_check = {0};
        static bool power_check_initialized = false;
        
        if (!power_check_initialized) {
            last_power_check = get_absolute_time();
            power_check_initialized = true;
        }
        
        if (absolute_time_diff_us(get_absolute_time(), last_power_check) > 100000) {
            gpio_put(POWER_PIN, 1); // 确保电源开启
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
    
    printf("\n=== POE2sb Gamepad Sync ===\n");
    printf("Build Date: %s %s\n", __DATE__, __TIME__);
    
    // 打印系统信息
    print_system_info();
    
    // 初始化硬件
    init_hardware();
    led_blink(3, 200); // 启动闪烁
    
    // 设置系统时钟
    printf("Setting system clock to 120MHz\n");
    set_sys_clock_khz(120000, true);
    
    // 启动Core1处理USB主机功能
    printf("Starting Core1 USB host task\n");
    multicore_launch_core1(core1_main);
    
    // 初始化USB设备
    printf("Initializing USB device (Virtual XInput)\n");
    tusb_init();
    
    printf("System initialization complete\n");
    printf("Waiting for gamepad connection...\n");
    set_led_color(0, 255, 255); // 青色：系统就绪
    
    // 主循环
    while (1) {
        tud_task();
        
        // 检查XInput手柄连接状态
        static bool last_gamepad_state = false;
        bool current_gamepad_state = xinput_host.connected;
        
        if (current_gamepad_state != last_gamepad_state) {
            if (current_gamepad_state) {
                printf("XInput gamepad connected!\n");
                gamepad_connected = true;
                set_led_color(0, 255, 0); // 绿色：手柄已连接
                led_blink(1, 100);
            } else {
                printf("XInput gamepad disconnected\n");
                gamepad_connected = false;
                set_led_color(255, 0, 0); // 红色：手柄未连接
                led_blink(2, 100);
            }
            last_gamepad_state = current_gamepad_state;
        }
        
        // 根据手柄连接状态更新LED
        static absolute_time_t last_led_update = {0};
        static bool led_update_initialized = false;
        
        if (!led_update_initialized) {
            last_led_update = get_absolute_time();
            led_update_initialized = true;
        }
        
        if (absolute_time_diff_us(get_absolute_time(), last_led_update) > 1000000) {
            if (gamepad_connected) {
                set_led_color(0, 255, 0); // 绿色：手柄已连接
            } else {
                // 手柄未连接，LED缓慢闪烁
                static bool blink_state = false;
                blink_state = !blink_state;
                if (blink_state) {
                    set_led_color(255, 0, 0); // 红色
                } else {
                    set_led_color(0, 0, 0);   // 关闭
                }
            }
            last_led_update = get_absolute_time();
        }
        
        // 如果手柄已连接，可以读取输入数据
        if (gamepad_connected) {
            xinput_report_t report;
            if (xinput_host_get_report(&report)) {
                // 在这里处理手柄输入数据
                // printf("Buttons: 0x%04X, LX: %d, LY: %d\n", 
                //        report.buttons, report.lx, report.ly);
            }
        }
        
        sleep_ms(10);
    }
}

// USB设备描述符
uint8_t const* tud_descriptor_device_cb(void) {
    static tusb_desc_device_t desc_device = {
        .bLength            = sizeof(tusb_desc_device_t),
        .bDescriptorType    = TUSB_DESC_DEVICE,
        .bcdUSB             = 0x0200,
        .bDeviceClass       = 0x00,
        .bDeviceSubClass    = 0x00,
        .bDeviceProtocol    = 0x00,
        .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
        .idVendor           = 0x04D8,  // Microchip
        .idProduct          = 0x003F,  // 标准HID设备
        .bcdDevice          = 0x0100,
        .iManufacturer      = 0x01,
        .iProduct           = 0x02,
        .iSerialNumber      = 0x03,
        .bNumConfigurations = 0x01
    };
    return (uint8_t const*) &desc_device;
}

// HID报告描述符
uint8_t const desc_hid_report[] = {
    0x05, 0x01,                    // Usage Page (Generic Desktop)
    0x09, 0x05,                    // Usage (Game Pad)
    0xA1, 0x01,                    // Collection (Application)
    0x85, 0x01,                    //   Report ID (1)
    
    // 16个按钮
    0x05, 0x09,                    //   Usage Page (Button)
    0x19, 0x01,                    //   Usage Minimum (Button 1)
    0x29, 0x10,                    //   Usage Maximum (Button 16)
    0x15, 0x00,                    //   Logical Minimum (0)
    0x25, 0x01,                    //   Logical Maximum (1)
    0x75, 0x01,                    //   Report Size (1)
    0x95, 0x10,                    //   Report Count (16)
    0x81, 0x02,                    //   Input (Data,Var,Abs)
    
    // 方向键
    0x05, 0x01,                    //   Usage Page (Generic Desktop)
    0x09, 0x39,                    //   Usage (Hat switch)
    0x15, 0x00,                    //   Logical Minimum (0)
    0x25, 0x07,                    //   Logical Maximum (7)
    0x35, 0x00,                    //   Physical Minimum (0)
    0x46, 0x3B, 0x01,              //   Physical Maximum (315 degrees)
    0x65, 0x14,                    //   Unit (Degrees)
    0x75, 0x04,                    //   Report Size (4)
    0x95, 0x01,                    //   Report Count (1)
    0x81, 0x42,                    //   Input (Data,Var,Abs,Null)
    
    // 保留位
    0x75, 0x04,                    //   Report Size (4)
    0x95, 0x01,                    //   Report Count (1)
    0x81, 0x03,                    //   Input (Const,Var,Abs)
    
    // 4个模拟轴
    0x05, 0x01,                    //   Usage Page (Generic Desktop)
    0x09, 0x30,                    //   Usage (X)
    0x09, 0x31,                    //   Usage (Y)
    0x09, 0x32,                    //   Usage (Z)
    0x09, 0x35,                    //   Usage (Rz)
    0x15, 0x81,                    //   Logical Minimum (-127)
    0x25, 0x7F,                    //   Logical Maximum (127)
    0x75, 0x08,                    //   Report Size (8)
    0x95, 0x04,                    //   Report Count (4)
    0x81, 0x02,                    //   Input (Data,Var,Abs)
    
    0xC0                           // End Collection
};

// HID报告描述符回调
uint8_t const* tud_hid_descriptor_report_cb(uint8_t itf) {
    (void) itf;
    return desc_hid_report;
}

// USB配置描述符
uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
    (void) index;
    static uint8_t desc_configuration[] = {
        // 配置描述符
        9, TUSB_DESC_CONFIGURATION,
        U16_TO_U8S_LE(34),            // 配置总长度
        0x01,                         // 接口数量
        0x01,                         // 配置值
        0x00,                         // 字符串索引
        0x80,                         // 供电模式 (总线供电)
        0x32,                         // 最大电流 (100mA)
        
        // 接口描述符
        9, TUSB_DESC_INTERFACE,
        0x00,                         // 接口号
        0x00,                         // 备用设置
        0x01,                         // 端点数量
        0x03,                         // 接口类 (HID)
        0x00,                         // 接口子类
        0x00,                         // 接口协议
        0x00,                         // 字符串索引
        
        // HID描述符
        9, HID_DESC_TYPE_HID,
        0x11, 0x01,                   // HID版本 (1.11)
        0x00,                         // 国家代码
        0x01,                         // 描述符数量
        HID_DESC_TYPE_REPORT,         // 描述符类型
        U16_TO_U8S_LE(sizeof(desc_hid_report)), // 描述符长度
        
        // 端点描述符
        7, TUSB_DESC_ENDPOINT,
        0x81,                         // 端点地址 (IN 1)
        0x03,                         // 属性 (中断)
        U16_TO_U8S_LE(64),            // 最大包大小
        0x01                          // 轮询间隔 (1ms)
    };
    return desc_configuration;
}

// 字符串描述符
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void) langid;
    static uint16_t desc_str[32];
    
    if (index == 0) {
        // 语言ID (英语)
        desc_str[1] = 0x0409;
        desc_str[0] = (TUSB_DESC_STRING << 8) | 4;
    } else if (index == 1) {
        // 制造商
        const char* str = "POE2sb";
        uint8_t len = strlen(str);
        desc_str[0] = (TUSB_DESC_STRING << 8) | (2*len + 2);
        for (uint8_t i = 0; i < len; i++) {
            desc_str[i+1] = str[i];
        }
    } else if (index == 2) {
        // 产品名称
        const char* str = "Gamepad Sync";
        uint8_t len = strlen(str);
        desc_str[0] = (TUSB_DESC_STRING << 8) | (2*len + 2);
        for (uint8_t i = 0; i < len; i++) {
            desc_str[i+1] = str[i];
        }
    } else {
        return NULL;
    }
    
    return desc_str;
}

// HID设置报告回调 - 已在xinput_device.c中定义
// 注释掉以避免重复定义错误
/*
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, 
                          uint8_t const* buffer, uint16_t bufsize) {
    (void) itf;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) bufsize;
}
*/

// HID获取报告回调 - 已在xinput_device.c中定义
// 注释掉以避免重复定义错误
/*
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, 
                              uint8_t* buffer, uint16_t reqlen) {
    (void) itf;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;
    
    return 0;
}
*/
