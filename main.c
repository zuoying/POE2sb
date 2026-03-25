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
#include "class/xinput/xinput_device.h"
#include "usb_descriptors.h"

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

// USB设备挂载回调函数
void tud_mount_cb(void) {
    printf("USB Device: Mounted successfully!\n");
    printf("Device is now visible to PC as HID gamepad\n");
    printf("Device Info: VID=0x%04X, PID=0x%04X\n", GAMEPAD_VID, GAMEPAD_PID);
}

void tud_umount_cb(void) {
    printf("USB Device: Unmounted\n");
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
    printf("USB Device Parameters:\n");
    printf("  VID: 0x1209, PID: 0x0001\n");
    printf("  Product: Xbox 360 Controller\n");
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
    
    // 初始化tinyusb设备栈
    printf("Calling tusb_init()...\n");
    tusb_init();
    printf("tusb_init() completed\n");
    
    // 添加USB设备挂载回调
    printf("Setting up USB device callbacks...\n");
    printf("  Device descriptor callback: %p\n", tud_descriptor_device_cb);
    printf("  Configuration descriptor callback: %p\n", tud_descriptor_configuration_cb);
    printf("  String descriptor callback: %p\n", tud_descriptor_string_cb);
    printf("  HID report descriptor callback: %p\n", tud_hid_descriptor_report_cb);
    
    printf("System initialization complete\n");
    printf("Waiting for gamepad connection...\n");
    set_led_color(0, 255, 255); // 青色：系统就绪
    
    // 主循环
    while (1) {
        tud_task();
        
        // 定期打印USB设备状态
        static absolute_time_t last_usb_status = {0};
        if (absolute_time_diff_us(last_usb_status, get_absolute_time()) > 5000000) { // 每5秒
            last_usb_status = get_absolute_time();
            
            printf("=== USB Device Status ===\n");
            printf("  tud_inited(): %s\n", tud_inited() ? "YES" : "NO");
            printf("  tud_mounted(): %s\n", tud_mounted() ? "YES" : "NO");
            printf("  tud_connected(): %s\n", tud_connected() ? "YES" : "NO");
            
            // 检查更详细的状态
            #if CFG_TUD_ENABLED
                printf("  CFG_TUD_ENABLED: 1 (device stack enabled)\n");
            #else
                printf("  CFG_TUD_ENABLED: 0 (device stack disabled)\n");
            #endif
            
            #if CFG_TUD_HID
                printf("  CFG_TUD_HID: %d (HID interfaces)\n", CFG_TUD_HID);
            #endif
            
            printf("===========================\n");
        }
        
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
                // 将手柄数据转发到USB设备端
                xinput_device_send_report(&report);
                
                // 调试输出（可选）
                // printf("Buttons: 0x%04X, LX: %d, LY: %d\n", 
                //        report.buttons, report.lx, report.ly);
            }
        }
        
        sleep_ms(10);
    }
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
