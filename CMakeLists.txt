cmake_minimum_required(VERSION 3.13)

set(PICO_PLATFORM rp2350 CACHE STRING "Platform")
set(PICO_BOARD pico2 CACHE STRING "Board")

# 获取TinyUSB内置的PIO-USB路径（用于头文件包含）
set(TINYUSB_PATH ${PICO_SDK_PATH}/lib/tinyusb)
set(PIO_USB_PATH ${TINYUSB_PATH}/hw/mcu/raspberrypi)

# 自动从GitHub获取SDK
set(PICO_SDK_FETCH_FROM_GIT ON)
set(PICO_SDK_FETCH_FROM_GIT_TAG "master")

include(pico_sdk_import.cmake)

project(poe2gamepad C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# 微雪RP2350-USB-A特定的引脚配置
# USB-A端口（主机模式）使用GPIO12 (D+) 和 GPIO13 (D-) - PIO-USB
# USB-C端口（设备模式）使用硬件USB控制器 (GPIO0/GPIO1)
add_compile_definitions(
    PICO_RP2350=1
    PIO_USB_HOST_D_PLUS_PIN=12
    PIO_USB_HOST_D_MINUS_PIN=13
    # 删除PIO_USB_DEVICE_*定义，设备端使用硬件USB
)

# 添加主程序目标（带USB功能）
add_executable(poe2gamepad
    main.c
    usb_descriptors.c
    led_manager.c
    mode_manager.c
    ws2812.pio.h
    class/hid/hid_host.c
    class/hid/hid_xinput_converter.c
    class/xinput/xinput_device.c
)

target_include_directories(poe2gamepad PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/class/hid
    ${CMAKE_CURRENT_LIST_DIR}/class/xinput
    ${PIO_USB_PATH}
)

target_link_libraries(poe2gamepad
    pico_stdlib
    tinyusb_device
    tinyusb_host
    tinyusb_pico_pio_usb
    pico_multicore
    hardware_pio
    hardware_gpio
)

pico_enable_stdio_usb(poe2gamepad 0)
pico_enable_stdio_uart(poe2gamepad 1)

pico_add_extra_outputs(poe2gamepad)
