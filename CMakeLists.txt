cmake_minimum_required(VERSION 3.13)

set(PICO_PLATFORM rp2040 CACHE STRING "Platform")
set(PICO_BOARD adafruit_feather_rp2040 CACHE STRING "Board")

# 获取TinyUSB内置的PIO-USB路径（用于头文件包含）
set(TINYUSB_PATH ${PICO_SDK_PATH}/lib/tinyusb)
set(PIO_USB_PATH ${TINYUSB_PATH}/hw/mcu/raspberrypi/pio_usb)

# 自动从GitHub获取SDK
set(PICO_SDK_FETCH_FROM_GIT ON)
set(PICO_SDK_FETCH_FROM_GIT_TAG "2.0.0")

include(pico_sdk_import.cmake)

project(poe2gamepad C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# Adafruit Feather RP2040 with USB Type A Host特定的引脚配置
# USB Type A Host端口（主机模式）使用GPIO26 (D+) 和 GPIO27 (D-) - PIO-USB
# 硬件USB控制器用于设备模式
add_compile_definitions(
    PICO_RP2040=1
    CFG_TUH_RPI_PIO_USB=1
    PIO_USB_HOST_PIN_DM=27
    PIO_USB_HOST_PIN_DP=26
    PIO_USB_HOST_PIO_INST=0
    PIO_USB_HOST_SM=0
    # 设备端使用硬件USB控制器，无PIO-USB设备端定义
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
    tinyusb_pico_pio_usb
    tinyusb_host
    tinyusb_board
    pico_multicore
    hardware_pio
    hardware_gpio
)

pico_enable_stdio_usb(poe2gamepad 0)
pico_enable_stdio_uart(poe2gamepad 1)

pico_add_extra_outputs(poe2gamepad)
