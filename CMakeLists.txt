cmake_minimum_required(VERSION 3.13)

set(PICO_PLATFORM rp2350 CACHE STRING "Platform")
set(PICO_BOARD pico2 CACHE STRING "Board")

# 设置PIO-USB路径（兼容GitHub Actions）
# 首先检查CMake变量，然后检查环境变量
if(PICO_PIO_USB_PATH)
    message(STATUS "Using PICO_PIO_USB_PATH from CMake variable: ${PICO_PIO_USB_PATH}")
elseif(DEFINED ENV{PICO_PIO_USB_PATH})
    set(PICO_PIO_USB_PATH $ENV{PICO_PIO_USB_PATH} CACHE PATH "Path to Pico-PIO-USB")
    message(STATUS "Using PICO_PIO_USB_PATH from environment: ${PICO_PIO_USB_PATH}")
else()
    set(PICO_PIO_USB_PATH "" CACHE PATH "Path to Pico-PIO-USB")
    message(STATUS "PICO_PIO_USB_PATH not set, will use TinyUSB built-in PIO-USB if available")
endif()

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
    class/xinput/xinput_host.c
    class/xinput/xinput_device.c
)

target_include_directories(poe2gamepad PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/class/xinput
)

# 如果有PICO_PIO_USB_PATH，添加包含路径
if(PICO_PIO_USB_PATH)
    target_include_directories(poe2gamepad PRIVATE
        ${PICO_PIO_USB_PATH}
    )
    message(STATUS "Added PICO_PIO_USB_PATH to include directories: ${PICO_PIO_USB_PATH}")
endif()

target_link_libraries(poe2gamepad
    pico_stdlib
    tinyusb_device
    tinyusb_host
    pico_multicore
    hardware_pio
    hardware_gpio
)

# 注释掉PIO-USB库链接，因为我们已经禁用了PIO-USB功能
# 如果需要重新启用PIO-USB，请取消注释并确保PICO_PIO_USB_PATH正确设置
#if(PICO_PIO_USB_PATH)
#    target_link_libraries(poe2gamepad
#        pico_pio_usb
#    )
#endif()

pico_enable_stdio_usb(poe2gamepad 0)
pico_enable_stdio_uart(poe2gamepad 1)

pico_add_extra_outputs(poe2gamepad)
