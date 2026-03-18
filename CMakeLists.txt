cmake_minimum_required(VERSION 3.13)

# 强制指定 RP2350 平台
set(PICO_PLATFORM rp2350 CACHE STRING "Platform")
set(PICO_BOARD pico2 CACHE STRING "Board")

# 强制设置 TinyUSB 使用 PIO-USB 作为主机驱动 (HCD)
set(TINYUSB_HOST_HCD "pico_pio_usb" CACHE STRING "TinyUSB Host HCD")

# 包含 Pico SDK 导入脚本
include(pico_sdk_import.cmake)

project(poe2gamepad C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# 初始化 Pico SDK
pico_sdk_init()

# 增加全局定义，确保所有库都能识别 RP2350
add_compile_definitions(PICO_RP2350=1)

# 强制指定 RP2350 平台
# (已经作为缓存变量在 include 之前设置)

# 优先从环境变量读取 PICO_PIO_USB_PATH，否则使用本地默认路径
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

add_executable(poe2gamepad
    main.c
    usb_descriptors.c
)



# 包含头文件
include_directories(
    ${CMAKE_CURRENT_LIST_DIR}
    ${PICO_PIO_USB_PATH}/src
)

# 链接必要的库
target_link_libraries(poe2gamepad
    pico_stdlib
    pico_rand
    tinyusb_device
    tinyusb_host
    pico_pio_usb
    hardware_pio
    hardware_dma
)

# 启用 TinyUSB 设备模式和主机模式
# (PICO_RP2350=1 已经通过 add_compile_definitions 全局设置)

# 配置 PIO-USB 引脚 (微雪 RP2350-USB-A 的 USB-A 口使用 GPIO 12/13)
target_compile_definitions(poe2gamepad PRIVATE
    PIO_USB_HOST_D_MINUS_PIN=13
)

# 启用 USB 输出
pico_enable_stdio_usb(poe2gamepad 0)
pico_enable_stdio_uart(poe2gamepad 1)

# 生成 UF2 文件
pico_add_extra_outputs(poe2gamepad)
