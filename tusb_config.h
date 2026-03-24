#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

// 仅在未定义时指定MCU类型（避免重定义）
#ifndef CFG_TUSB_MCU
#define CFG_TUSB_MCU OPT_MCU_RP2350 // RP2350 专用宏
#endif

// USB工作模式：仅设备模式（按需改为 TUSB_MODE_HOST/TUSB_MODE_DEVICE|TUSB_MODE_HOST）
#ifndef CFG_TUSB_MODE
#define CFG_TUSB_MODE TUSB_MODE_DEVICE
#endif

// RP2350 内部USB PHY配置
#define BOARD_TUD_RHPORT 0
#define BOARD_TUD_RHPORT_SPEED OPT_MODE_FULL_SPEED

// USB端点0大小（默认64）
#define CFG_TUD_ENDPOINT0_SIZE 64

// 按需启用USB类（示例：启用HID（手柄/键鼠），关闭其他）
#define CFG_TUD_HID           1    // HID类（XInput手柄需开启）
#define CFG_TUD_CDC           0    // CDC串口
#define CFG_TUD_MSC           0    // 大容量存储
#define CFG_TUD_MIDI          0    // MIDI
#define CFG_TUD_VENDOR        0    // 自定义厂商类

// HID缓冲区大小（XInput手柄建议64）
#define CFG_TUD_HID_EP_BUFSIZE 64

#ifdef __cplusplus
}
#endif

#endif /* _TUSB_CONFIG_H_ */
