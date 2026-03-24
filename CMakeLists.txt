cmake_minimum_required(VERSION 3.13)

# 引入Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# 项目配置
project(rp2350_gaishichick_xinput C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# 初始化Pico SDK
pico_sdk_init()

# 添加可执行文件（包含XInput源码）
add_executable(${PROJECT_NAME}
    main.c
    class/xinput/xinput_device.c  # 新增XInput实现文件
    # 其他源码文件...
)

# 链接依赖库
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    tinyusb_device  # tinyUSB设备库
    hardware_gpio   # 按需添加（如GPIO操作）
    hardware_timer  # 按需添加（如定时器）
)

# 关键：添加头文件搜索路径（让编译器找到class/xinput/目录）
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}          # 根目录（tusb_config.h）
    ${CMAKE_CURRENT_SOURCE_DIR}/class    # XInput头文件父目录
)

# 串口配置（USB串口启用，UART禁用）
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# 生成UF2烧录文件
pico_add_extra_outputs(${PROJECT_NAME})
