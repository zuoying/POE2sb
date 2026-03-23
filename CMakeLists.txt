cmake_minimum_required(VERSION 3.13)

# 必须在 include(pico_sdk_import.cmake) 之前设置平台！
set(PICO_PLATFORM rp2350 CACHE STRING "Platform")
set(PICO_BOARD pico2 CACHE STRING "Board")

# 设置 TinyUSB 使用 PIO-USB 作为主机驱动
set(TINYUSB_HOST_HCD "pico_pio_usb" CACHE STRING "TinyUSB Host HCD")

# 必须在 pico_sdk_import.cmake 之前设置 PICO_PIO_USB_PATH
if (DEFINED ENV{PICO_PIO_USB_PATH})
    set(PICO_PIO_USB_PATH $ENV{PICO_PIO_USB_PATH})
else()
    set(PICO_PIO_USB_PATH ${PROJECT_SOURCE_DIR}/pico-pio-usb)
endif()

# 包含 Pico SDK 导入脚本 (必须在 platform 设置之后)
include(pico_sdk_import.cmake)

project(poe2gamepad C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# 初始化 Pico SDK
pico_sdk_init()

# 确保识别 RP2350
add_compile_definitions(PICO_RP2350=1)

# 添加 pico-pio-usb 子模块
if (EXISTS ${PICO_PIO_USB_PATH}/CMakeLists.txt)
    add_subdirectory(${PICO_PIO_USB_PATH} pico_pio_usb)
endif()

# 查找 hcd_pio_usb.c 源文件
if (EXISTS ${PICO_PIO_USB_PATH}/src/hcd_pio_usb.c)
    set(HCD_PIO_USB_SRC ${PICO_PIO_USB_PATH}/src/hcd_pio_usb.c)
elseif (EXISTS ${PICO_SDK_PATH}/lib/tinyusb/src/portable/raspberrypi/pio_usb/hcd_pio_usb.c)
    set(HCD_PIO_USB_SRC ${PICO_SDK_PATH}/lib/tinyusb/src/portable/raspberrypi/pio_usb/hcd_pio_usb.c)
endif()

add_executable(poe2gamepad
    main.c
    usb_descriptors.c
    ${HCD_PIO_USB_SRC}
)

# 包含头文件
target_include_directories(poe2gamepad PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${PICO_PIO_USB_PATH}/src
)

# 链接必要的库
target_link_libraries(poe2gamepad
    pico_stdlib
    pico_rand
    pico_multicore
    tinyusb_device
    tinyusb_host
    pico_pio_usb
    hardware_pio
    hardware_dma
)

# PIO-USB 引脚配置
target_compile_definitions(poe2gamepad PRIVATE
    PIO_USB_HOST_D_PLUS_PIN=12
    PIO_USB_HOST_D_MINUS_PIN=13
)

# 启用 UART 输出（禁用 USB CDC）
pico_enable_stdio_usb(poe2gamepad 0)
pico_enable_stdio_uart(poe2gamepad 1)

# 生成 UF2 文件
pico_add_extra_outputs(poe2gamepad)
