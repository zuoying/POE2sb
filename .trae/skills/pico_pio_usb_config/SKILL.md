---
name: "pico_pio_usb_config"
description: "Configures pico_pio_usb library for Waveshare RP2350-USB-A board. Invoke when setting up PIO-USB functionality on RP2350 platform."
---

# pico_pio_usb_config Skill

This skill provides comprehensive guidance for configuring the pico_pio_usb library on the Waveshare RP2350-USB-A board, ensuring proper USB Host and Device functionality.

## Hardware Configuration

### RP2350-USB-A Pinout for PIO-USB
- **USB-A Port (Host)**: GPIO12 (D+), GPIO13 (D-)
- **Type-C Port (Device)**: Native USB controller
- **WS2812 RGB LED**: GPIO16
- **5V Power Control**: GPIO18

## CMakeLists.txt Configuration

### Required Definitions
```cmake
# 强制指定 RP2350 平台
set(PICO_PLATFORM rp2350 CACHE STRING "Platform")
set(PICO_BOARD pico2 CACHE STRING "Board")

# 强制设置 TinyUSB 使用 PIO-USB 作为主机驱动
target_compile_definitions(your_project PRIVATE
    PIO_USB_HOST_D_MINUS_PIN=13  # D- pin
)

# 链接必要的库
target_link_libraries(your_project
    pico_stdlib
    pico_rand
    pico_multicore
    tinyusb_device
    tinyusb_host
    pico_pio_usb
    hardware_pio
    hardware_dma
)
```

### Pico SDK and PIO-USB Setup
```cmake
# 优先从环境变量读取 PICO_PIO_USB_PATH
if (NOT PICO_PIO_USB_PATH)
    if (DEFINED ENV{PICO_PIO_USB_PATH})
        set(PICO_PIO_USB_PATH $ENV{PICO_PIO_USB_PATH})
    else()
        set(PICO_PIO_USB_PATH ${PROJECT_SOURCE_DIR}/pico-pio-usb)
    endif()
endif()

# 确保包含 pico-pio-usb
if (EXISTS ${PICO_PIO_USB_PATH}/CMakeLists.txt)
    add_subdirectory(${PICO_PIO_USB_PATH} pico_pio_usb)
endif()
```

## Code Initialization

### Dual-Core Setup
```c
#include "pico/multicore.h"

// Core1 任务：处理 USB Host
void core1_main() {
    // 初始化 USB Host
    pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
    tuh_configure(1, TUH_CFGID_RPI_PIO_USB_CONFIGURATION, &pio_cfg);
    tuh_init(1);
    
    // USB Host 主循环
    while (1) {
        tuh_task();
    }
}

int main() {
    // 设置系统时钟为 120MHz
    set_sys_clock_khz(120000, true);
    
    // 初始化 USB Device
    tusb_init();
    
    // 启动 Core1 处理 USB Host
    multicore_launch_core1(core1_main);
    
    // USB Device 主循环
    while (1) {
        tud_task();
        // 其他 Core0 任务
    }
}
```

### WS2812 LED Configuration
```c
// 使用 RGBW 格式初始化 WS2812
void init_ws2812(void) {
    uint offset = pio_add_program(pio0, &ws2812_program);
    ws2812_program_init(pio0, 0, offset, WS2812_PIN, WS2812_FREQ, true);  // true = RGBW format
}

// 发送 RGB 颜色数据
static inline void put_rgb(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t grb = ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;
    put_pixel(grb);
}

// put_pixel 函数会自动左移 8 位
static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}
```

### 5V Power Control
```c
// 初始化 GPIO18 控制 5V 供电
const uint32_t GPIO_5V_EN = 18;
gpio_init(GPIO_5V_EN);
gpio_set_dir(GPIO_5V_EN, GPIO_OUT);
gpio_put(GPIO_5V_EN, 1);  // 开启5V供电
```

## Common Issues and Solutions

### 1. PIO-USB Initialization Failed
**Solution**: Ensure correct pin configuration and system clock setting
```c
// 必须使用 12MHz 的倍数作为系统时钟
set_sys_clock_khz(120000, true);  // 120MHz
```

### 2. LED Colors Incorrect
**Solution**: Use RGBW format (last parameter = true) in ws2812_program_init
```c
ws2812_program_init(pio0, 0, offset, WS2812_PIN, WS2812_FREQ, true);
```

### 3. Dual-Core Communication Issues
**Solution**: Use proper synchronization between cores and ensure pico_multicore is linked
```cmake
target_link_libraries(your_project pico_multicore)
```

### 4. USB Host Not Detecting Devices
**Solution**: Verify 5V power is enabled and PIO_USB_HOST_D_MINUS_PIN is set correctly
```cmake
target_compile_definitions(your_project PRIVATE PIO_USB_HOST_D_MINUS_PIN=13)
```

## Best Practices

1. **System Clock**: Always use a multiple of 12MHz for PIO-USB functionality
2. **Dual-Core**: Separate USB Host (Core1) and Device (Core0) tasks for better performance
3. **GPIO Initialization**: Enable 5V power before initializing USB Host
4. **LED Configuration**: Use RGBW format for WS2812 LEDs on RP2350-USB-A board
5. **Error Handling**: Add appropriate error checking and logging for USB operations

## Example Projects

Refer to these official examples for reference:
- `C:\Users\dring\Documents\trae_projects\Pico-PIO-USB-main` - PIO-USB library examples
- `C:\Users\dring\Documents\trae_projects\RP2350-USB-A-RGB` - WS2812 RGB LED examples

This skill ensures proper configuration of the pico_pio_usb library on the Waveshare RP2350-USB-A board, enabling reliable USB Host and Device functionality for your projects.