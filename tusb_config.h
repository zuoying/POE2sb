#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#include <stdint.h>

// 强制指定 MCU 为 RP2350 (111) 和 OS 为 PICO (4)
// 使用数值常量规避 SDK 2.0.0 云端环境识别错误及宏重定义警告
#ifndef CFG_TUSB_MCU
  #define CFG_TUSB_MCU                111
#endif

#ifndef CFG_TUSB_OS
  #define CFG_TUSB_OS                 4
#endif

// 核心修复：定义 TinyUSB PIO-USB 必须的类型
#ifndef _PIPE_HANDLE_T_
#define _PIPE_HANDLE_T_
typedef void * pipe_handle_t;
#endif

// 端口模式配置
#define CFG_TUSB_RHPORT0_MODE       OPT_MODE_DEVICE
#define CFG_TUSB_RHPORT1_MODE       (OPT_MODE_HOST | OPT_MODE_FULL_SPEED)

// DEVICE 配置
#define CFG_TUD_ENABLED             1
#define CFG_TUD_VENDOR              2 
#define CFG_TUD_VENDOR_RX_BUFSIZE   64
#define CFG_TUD_VENDOR_TX_BUFSIZE   64

// HOST 配置
#define CFG_TUH_ENABLED             1
#define CFG_TUH_RPI_PIO_USB         1 
#define CFG_TUH_HID                 1 
#define CFG_TUH_DEVICE_MAX          2
#define CFG_TUH_ENUMERATION_BUFSIZE 256

#endif
