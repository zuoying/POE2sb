#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#include <stdint.h>

// 不再手动定义 CFG_TUSB_MCU 和 CFG_TUSB_OS，让 Pico SDK 自动设置
// 这些宏在命令行中由 CMake 自动生成，避免重复定义警告

// 核心修复：定义 TinyUSB PIO-USB 必须的类型
#ifndef _PIPE_HANDLE_T_
#define _PIPE_HANDLE_T_
typedef void * pipe_handle_t;
#endif

// 端点0大小 (必须定义)
#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE       64
#endif

// 端口模式配置 - Port0是设备(连接PC), Port1是主机(连接手柄)
#define CFG_TUSB_RHPORT0_MODE       OPT_MODE_DEVICE
#define CFG_TUSB_RHPORT1_MODE       (OPT_MODE_HOST | OPT_MODE_FULL_SPEED)

// DEVICE 配置
#define CFG_TUD_ENABLED             1
#define CFG_TUD_VENDOR              1  // 先支持1个Vendor接口（Xbox 360控制器，使用XInput协议）
#define CFG_TUD_VENDOR_RX_BUFSIZE   32  // 接收缓冲区大小
#define CFG_TUD_VENDOR_TX_BUFSIZE   32  // 发送缓冲区大小
#define CFG_TUD_HID                 0  // 禁用标准HID类

// HOST 配置
#define CFG_TUH_ENABLED             1
#define CFG_TUH_RPI_PIO_USB         1 
#define CFG_TUH_HID                 1 
#define CFG_TUH_DEVICE_MAX          2
#define CFG_TUH_ENUMERATION_BUFSIZE 256

#endif
