cmake_minimum_required(VERSION 3.13)

# 引入Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# 项目配置
project(rp2350_gaishichick_xinput C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# 初始化Pico SDK
pico_sdk_init()

# 添加可执行文件（新增xinput_host.c）
add_executable(${PROJECT_NAME}
    main.c
    class/xinput/xinput_device.c
    class/xinput/xinput_host.c  # 新增主机模式源码
)

# 链接依赖库（需添加tinyusb_host，主机模式核心库）
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    tinyusb_device  # 设备模式（若不需要可删除）
    tinyusb_host   # 主机模式（必须添加）
    hardware_gpio
    hardware_timer
)

# 头文件搜索路径（保持不变，已包含class/目录）
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/class
)

# 串口配置
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# 生成UF2文件
pico_add_extra_outputs(${PROJECT_NAME})
