cmake_minimum_required(VERSION 3.13)

set(PICO_PLATFORM rp2350 CACHE STRING "Platform")
set(PICO_BOARD pico2 CACHE STRING "Board")

# 自动从GitHub获取SDK
set(PICO_SDK_FETCH_FROM_GIT ON)
set(PICO_SDK_FETCH_FROM_GIT_TAG "master")

include(pico_sdk_import.cmake)

project(poe2gamepad C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# 微雪RP2350-USB-A特定的引脚配置
# USB-A端口使用GPIO12 (D+) 和 GPIO13 (D-)
add_compile_definitions(
    PICO_RP2350=1
    PIO_USB_HOST_D_PLUS_PIN=12
    PIO_USB_HOST_D_MINUS_PIN=13
    PIO_USB_DEVICE_D_PLUS_PIN=2
    PIO_USB_DEVICE_D_MINUS_PIN=3
)

# 添加主程序目标（带USB功能）
add_executable(poe2gamepad
    main.c
    ws2812.pio.h
)

target_include_directories(poe2gamepad PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(poe2gamepad
    pico_stdlib
    tinyusb_device
    tinyusb_host
    pico_multicore
    hardware_pio
    hardware_gpio
)

pico_enable_stdio_usb(poe2gamepad 0)
pico_enable_stdio_uart(poe2gamepad 1)

pico_add_extra_outputs(poe2gamepad)

# 添加简化的LED和电源测试程序
add_executable(test_led_power
    test_led_power.c
    ws2812.pio.h
)

target_include_directories(test_led_power PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(test_led_power
    pico_stdlib
    hardware_pio
    hardware_gpio
)

pico_enable_stdio_usb(test_led_power 0)
pico_enable_stdio_uart(test_led_power 1)

pico_add_extra_outputs(test_led_power)

# 添加游戏手柄检测程序
add_executable(test_gamepad_detect
    test_gamepad_detect.c
)

target_include_directories(test_gamepad_detect PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(test_gamepad_detect
    pico_stdlib
    tinyusb_host
    hardware_gpio
)

pico_enable_stdio_usb(test_gamepad_detect 0)
pico_enable_stdio_uart(test_gamepad_detect 1)

pico_add_extra_outputs(test_gamepad_detect)
