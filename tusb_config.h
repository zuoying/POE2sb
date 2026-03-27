#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#include <stdint.h>

// 强制指定MCU类型为RP2040（如果尚未定义）
#ifndef CFG_TUSB_MCU
#define CFG_TUSB_MCU 11  // RP2040
#endif

// 注释掉端点数量定义，使用Pico SDK默认值
// TUP_DCD_ENDPOINT_MAX已在Pico SDK中定义为16
// 不要重新定义，避免警告

#define CFG_TUSB_OS               OPT_OS_PICO
#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN        __attribute__((aligned(4)))
#define CFG_TUD_ENDPOINT0_SIZE    64

// USB设备配置 - 使用硬件USB (USB-C端口)
#define CFG_TUD_ENABLED           1
#define CFG_TUD_HID              2  // 支持两个虚拟手柄
#define CFG_TUD_HID_EP_BUFSIZE    64
// 强制使用自定义USB描述符，避免Pico SDK默认VID/PID
#define CFG_TUD_DESC_USE_CDC      0
#define CFG_TUD_DESC_USE_MSC      0
#define CFG_TUD_DESC_USE_HID      1
#define CFG_TUD_DESC_USE_AUDIO    0

// USB主机配置 - 使用PIO-USB (USB-A端口)
#define CFG_TUH_ENABLED           1
#define CFG_TUH_HID               1  // 支持HID设备
#define CFG_TUH_HUB               1  // 支持USB Hub
#define CFG_TUH_RPI_PIO_USB       1  // 启用PIO-USB以支持USB-A端口
#define CFG_TUH_MAX_DEVICE        2  // 最多支持2个设备
#define CFG_TUH_ENUMERATION_BUFSIZE 256

// PIO-USB配置 - 只用于主机端
#define BOARD_TUH_RHPORT          0  // RP2040 PIO-USB使用端口0 (GPIO26/27)

#endif
