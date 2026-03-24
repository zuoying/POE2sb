cmake_minimum_required(VERSION 3.13)

set(PICO_PLATFORM rp2350 CACHE STRING "Platform")
set(PICO_BOARD pico2 CACHE STRING "Board")

# 设置PIO-USB路径（兼容GitHub Actions）
if(DEFINED ENV{PICO_PIO_USB_PATH})
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
set(POE2GAMEPAD_SOURCES
    main.c
    ws2812.pio.h
)

# 如果PICO_PIO_USB_PATH已设置，使用外部的PIO-USB库
if(PICO_PIO_USB_PATH)
    message(STATUS "Using external Pico-PIO-USB library from: ${PICO_PIO_USB_PATH}")
    # 在这种情况下，TinyUSB应该能找到pio_usb.h
else()
    # 否则，尝试使用Pico SDK内置的PIO-USB
    # 检查hcd_pio_usb.c文件是否存在
    if(EXISTS "${PICO_SDK_PATH}/lib/tinyusb/src/portable/raspberrypi/pio_usb/hcd_pio_usb.c")
        message(STATUS "Adding built-in hcd_pio_usb.c from Pico SDK")
        list(APPEND POE2GAMEPAD_SOURCES 
            ${PICO_SDK_PATH}/lib/tinyusb/src/portable/raspberrypi/pio_usb/hcd_pio_usb.c
        )
    else()
        message(WARNING "hcd_pio_usb.c not found in Pico SDK. PIO-USB may not work.")
    endif()
endif()

add_executable(poe2gamepad ${POE2GAMEPAD_SOURCES})

target_include_directories(poe2gamepad PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# 如果有PICO_PIO_USB_PATH，添加包含路径
if(PICO_PIO_USB_PATH)
    target_include_directories(poe2gamepad PRIVATE
        ${PICO_PIO_USB_PATH}
    )
else()
    # 如果没有外部PIO-USB库，尝试添加Pico SDK中的PIO-USB包含路径
    if(EXISTS "${PICO_SDK_PATH}/lib/tinyusb/src/portable/raspberrypi/pio_usb")
        target_include_directories(poe2gamepad PRIVATE
            ${PICO_SDK_PATH}/lib/tinyusb/src/portable/raspberrypi/pio_usb
        )
    endif()
endif()

target_link_libraries(poe2gamepad
    pico_stdlib
    tinyusb_device
    tinyusb_host
    pico_multicore
    hardware_pio
    hardware_gpio
)

# 如果有PICO_PIO_USB_PATH，链接额外的库
if(PICO_PIO_USB_PATH)
    target_link_libraries(poe2gamepad
        pico_pio_usb
    )
endif()

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
