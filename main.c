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

// 简化电源管理：根据OGX-Mini配置和Adafruit Feather原理图
static void enable_usb_host_power(void) {
    printf("Enabling USB host 5V power via TPS61023 (GPIO18)...\n");
    printf("  According to OGX-Mini config and Adafruit schematic:\n");
    printf("  - VCC_EN_PIN = GPIO18 (Active HIGH)\n");
    printf("  - TPS61023 boost converter provides 5.2V output\n");
    printf("  - Max current: 1A peak\n");
    
    // 根据OGX-Mini和Adafruit设计，GPIO18高电平启用TPS61023
    // 简单直接启用，TPS61023内置软启动和过流保护
    gpio_put(USB_HOST_POWER_PIN, 1);
    
    // 等待升压转换器完全启动
    printf("  Waiting for TPS61023 to stabilize (50ms)...\n");
    sleep_ms(50);
    
    printf("  USB host 5V power should now be available\n");
}

// 检查5V电源状态 - 根据OGX-Mini配置
static bool check_5v_power_status(void) {
    // 根据OGX-Mini的Config.h，LED指示灯使用GPIO13
    // 但我们可能没有连接到正确的外部检测引脚
    // 这里我们简单检查GPIO18（VCC_EN）是否已设置为高电平
    
    printf("Checking USB host power status...\n");
    
    // 检查VCC使能引脚状态
    bool vcc_enabled = gpio_get(USB_HOST_POWER_PIN);
    printf("  VCC_EN pin (GPIO18): %s\n", vcc_enabled ? "HIGH" : "LOW");
    
    if (vcc_enabled) {
        printf("  Status: USB host 5V power SHOULD be available\n");
        printf("  Note: Actual power availability depends on TPS61023 and input voltage\n");
        return true;
    } else {
        printf("  Status: USB host 5V power is DISABLED (GPIO18 LOW)\n");
        return false;
    }
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
    
    // 启用USB主机5V电源
    enable_usb_host_power();
    
    // 检查5V电源状态
    bool power_ok = check_5v_power_status();
    
    printf("    USB 5V power: %s (TPS61023 %s)\n", 
           power_ok ? "ON" : "OFF/UNSTABLE",
           power_ok ? "fully enabled" : "may need reset");
    printf("    Note: Power management allows hard-reset via GPIO18 control\n");
    
    // GPIO6配置 - 根据OGX-Mini配置，他们使用GPIO13作为LED
    // GPIO6可能不是电源状态引脚，我们将其配置为输入用于诊断
    printf("  GPIO6 configuration (diagnostic only):\n");
    
    // 简单配置为输入，上拉
    gpio_init(POWER_STATUS_PIN);
    gpio_set_dir(POWER_STATUS_PIN, GPIO_IN);
    gpio_pull_up(POWER_STATUS_PIN);
    sleep_ms(10);
    
    bool gpio6_state = gpio_get(POWER_STATUS_PIN);
    printf("    GPIO6 state: %s\n", gpio6_state ? "HIGH" : "LOW");
    printf("    Note: GPIO6 may not be connected to power status indicator\n");
    
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
    printf("USB Host Configuration:\n");
    printf("  D+ pin: GPIO%d, D- pin: GPIO%d\n", USB_HOST_DP_PIN, USB_HOST_DM_PIN);
    printf("  5V power control: GPIO%d (TPS61023 EN) - %s\n", 
           USB_HOST_POWER_PIN, gpio_get(USB_HOST_POWER_PIN) ? "ENABLED" : "DISABLED");
    printf("  GPIO6 state: %s (diagnostic only)\n", gpio_get(POWER_STATUS_PIN) ? "HIGH" : "LOW");
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
    printf("Core1: PIO-USB Host pins: D+ GPIO%d, D- GPIO%d\n", 
           USB_HOST_DP_PIN, USB_HOST_DM_PIN);
    printf("Core1: VBUS power control: GPIO%d\n", USB_HOST_POWER_PIN);
    printf("Core1: Using rhport: %d (configured in tusb_config.h)\n", BOARD_TUH_RHPORT);
    
    // 等待USB设备栈完全初始化
    printf("Core1: Waiting for USB device stack to stabilize...\n");
    sleep_ms(1500);
    
    // 初始化HID主机模块
    printf("Core1: Initializing HID host module...\n");
    printf("Core1: This will initialize PIO-USB host controller\n");
    printf("Core1: CPU frequency check: %lu Hz\n", clock_get_hz(clk_sys));
    
    // 检查CPU频率是否符合PIO-USB要求
    uint32_t cpu_hz = clock_get_hz(clk_sys);
    if (cpu_hz != 120000000UL && cpu_hz != 240000000UL) {
        printf("Core1: ERROR: CPU frequency %lu Hz is not valid for PIO-USB!\n", cpu_hz);
        printf("Core1: PIO-USB requires CPU frequency to be 120MHz or 240MHz\n");
        printf("Core1: USB host may not work correctly!\n");
    } else {
        printf("Core1: CPU frequency OK for PIO-USB\n");
    }
    
    hid_host_init();
    
    // 等待PIO-USB硬件完全启动
    printf("Core1: Waiting for PIO-USB hardware to initialize...\n");
    printf("Core1: This may take a moment for USB host enumeration\n");
    sleep_ms(2000);
    
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
        
        if (absolute_time_diff_us(get_absolute_time(), last_power_check) > 1000000) { // 每1秒
            // 确保USB主机5V电源开启
            gpio_put(USB_HOST_POWER_PIN, 1);
            
            // 简单的状态报告
            static uint8_t heartbeat = 0;
            heartbeat++;
            
            if (heartbeat >= 30) { // 每30秒详细报告一次
                printf("Core1: USB Host Status Report\n");
                printf("  TPS61023 enabled: GPIO18 %s\n", gpio_get(USB_HOST_POWER_PIN) ? "HIGH" : "LOW");
                printf("  Board LED: GPIO25 %s\n", gpio_get(BUILTIN_LED_PIN) ? "ON" : "OFF");
                printf("  GPIO6 (诊断): %s\n", gpio_get(POWER_STATUS_PIN) ? "HIGH" : "LOW");
                printf("  System running for: %lu ms\n", to_ms_since_boot(get_absolute_time()));
                heartbeat = 0;
            }
            
            // 简单的心跳指示
            static bool led_toggle = false;
            led_toggle = !led_toggle;
            gpio_put(BUILTIN_LED_PIN, led_toggle);
            
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
    
    // 运行系统诊断
    printf("\n=== Running System Diagnostics ===\n");
    
    // 检查CPU时钟频率（PIO-USB要求120MHz或240MHz倍数）
    uint32_t cpu_hz = clock_get_hz(clk_sys);
    printf("CPU Clock Check:\n");
    printf("  Current CPU frequency: %lu Hz\n", cpu_hz);
    printf("  PIO-USB requirement: multiple of 120MHz\n");
    if (cpu_hz == 120000000UL || cpu_hz == 240000000UL) {
        printf("  Result: PASS (valid frequency for PIO-USB)\n");
    } else {
        printf("  Result: FAIL (invalid frequency for PIO-USB)\n");
        printf("  Warning: PIO-USB may not work correctly!\n");
    }
    
    // 测试GPIO状态
    printf("GPIO Status:\n");
    printf("  GPIO18 (TPS61023 EN - VCC控制): %s\n", gpio_get(USB_HOST_POWER_PIN) ? "HIGH" : "LOW");
    printf("  GPIO16 (USB Host D+): configured for PIO-USB\n");
    printf("  GPIO17 (USB Host D-): configured for PIO-USB\n");
    printf("  GPIO6 (诊断引脚): %s\n", gpio_get(POWER_STATUS_PIN) ? "HIGH" : "LOW");
    printf("  GPIO25 (板载LED): %s\n", gpio_get(BUILTIN_LED_PIN) ? "HIGH" : "LOW");
    
    // 测试电源管理
    printf("Power Management Status:\n");
    printf("  TPS61023 should be providing 5.2V output\n");
    printf("  Max current: 1A peak (depends on input voltage)\n");
    
    // 设置系统时钟 - 根据OGX-Mini配置，PIO-USB需要240MHz
    printf("\nSetting system clock to 240MHz (PIO-USB requirement)\n");
    set_sys_clock_khz(240000, true);
    
    // 初始化模式管理器
    mode_manager_init();
    
    // 初始化USB设备（虚拟XInput） - 先于Core1启动
    printf("\nInitializing USB device (Dual Virtual XInput)\n");
    printf("USB Device Parameters:\n");
    printf("  VID: 0x%04X, PID: 0x%04X\n", GAMEPAD_VID, GAMEPAD_PID);
    printf("  Interfaces: 2 (Dual XInput Controllers)\n");
    printf("  Manufacturer: GameSir\n");
    printf("  Product: GameSir T4 Kaleid Controller\n");
    printf("  Serial: SN240327001\n");
    printf("  String descriptors loaded\n");
    
    // 检查PIO-USB主机配置
    printf("\nPIO-USB Host Configuration:\n");
    printf("  D+ pin: GPIO%d\n", USB_HOST_DP_PIN);
    printf("  D- pin: GPIO%d\n", USB_HOST_DM_PIN);
    printf("  5V power control: GPIO%d (TPS61023 EN)\n", USB_HOST_POWER_PIN);
    printf("  PIO instance: 0, State machine: 0\n");
    
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
