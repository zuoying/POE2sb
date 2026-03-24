#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#include <stdint.h>

// 强制指定MCU类型为RP2350
#define CFG_TUSB_MCU 111  // RP2350

#define CFG_TUSB_OS               OPT_OS_PICO
#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN        __attribute__((aligned(4)))
#define CFG_TUD_ENDPOINT0_SIZE    64

// USB设备配置
#define CFG_TUD_ENABLED           1
#define CFG_TUD_HID              2  // 支持两个虚拟手柄
#define CFG_TUD_HID_EP_BUFSIZE    64

// USB主机配置
#define CFG_TUH_ENABLED           1
#define CFG_TUH_HID               1  // 支持HID设备
#define CFG_TUH_HUB               1  // 支持USB Hub
#define CFG_TUH_RPI_PIO_USB       1  // 启用PIO-USB
#define CFG_TUH_MAX_DEVICE        2  // 最多支持2个设备
#define CFG_TUH_ENUMERATION_BUFSIZE 256

// PIO-USB配置
#define BOARD_TUH_RHPORT          0  // RP2350 PIO-USB使用端口0

#endif
