#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#include <stdint.h>

// 仅在未定义时指定 MCU 为 RP2350 (111) 和 OS 为 PICO (4)
// 避免与命令行定义冲突
#if !defined(CFG_TUSB_MCU)
  #define CFG_TUSB_MCU                111
#endif

#if !defined(CFG_TUSB_OS)
  #define CFG_TUSB_OS                 4
#endif

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
#define CFG_TUD_VENDOR              0  // 禁用Vendor特定类
#define CFG_TUD_HID                 2  // 支持2个HID接口（Xbox 360控制器）
#define CFG_TUD_HID_EP_BUFSIZE      32  // 每个HID接口的端点缓冲区大小

// HOST 配置
#define CFG_TUH_ENABLED             1
#define CFG_TUH_RPI_PIO_USB         1 
#define CFG_TUH_HID                 1 
#define CFG_TUH_DEVICE_MAX          2
#define CFG_TUH_ENUMERATION_BUFSIZE 256

#endif
