cmake_minimum_required(VERSION 3.13)

# 引入Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# 项目配置
project(rp2350_gaishichick_xinput C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# 初始化Pico SDK
pico_sdk_init()

# 添加可执行文件（替换为你的源码文件）
add_executable(${PROJECT_NAME}
    main.c
    # 其他源码文件...
)

# 链接依赖库（核心：tinyusb_device）
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    tinyusb_device  # tinyUSB设备模式库
    # 其他库（如hardware_gpio、hardware_timer等）...
)

# 串口配置（按需调整：USB串口启用，UART串口禁用）
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# 确保tusb_config.h被编译器找到（源码根目录）
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 生成UF2烧录文件
pico_add_extra_outputs(${PROJECT_NAME})
