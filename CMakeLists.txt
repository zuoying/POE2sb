# 设置最低CMake版本
cmake_minimum_required(VERSION 3.13)

# 强制指定平台为RP2350
set(PICO_PLATFORM rp2350 CACHE STRING "Platform" FORCE)

# 引入Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# 项目名称
project(rp2350_gaishichick_xinput C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# 初始化Pico SDK
pico_sdk_init()

# 添加可执行文件
add_executable(${PROJECT_NAME}
    main.c
)

# 链接库（新增tinyusb_host）
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    tinyusb_device
    tinyusb_host
    tinyusb_board
)

# 启用USB（关闭串口USB，避免冲突）
pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

# 生成UF2文件
pico_add_extra_outputs(${PROJECT_NAME})

# 编译选项（指定RP2350，启用Host+Device）
target_compile_definitions(${PROJECT_NAME} PRIVATE
    CFG_TUSB_MCU=OPT_MCU_RP2350
    CFG_TUSB_DEBUG=0 # 关闭调试（如需调试改为1）
)
