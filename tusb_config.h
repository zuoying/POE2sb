#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#define BOARD_TUH_RHPORT 0 // RP2350的USB Host端口（0=默认USB端口）
#define BOARD_TUH_MAX_SPEED OPT_MODE_DEFAULT

// 启用USB主机模式
#define CFG_TUH_ENABLED 1
#define CFG_TUH_HID 1       // 启用HID主机
#define CFG_TUH_CDC 0       // 禁用CDC（按需）
#define CFG_TUH_MSC 0       // 禁用MSC（按需）
#define CFG_TUH_VENDOR 0    // 禁用Vendor（按需）

// 主机模式最大设备数（至少1，支持1个XInput手柄）
#define CFG_TUH_DEVICE_MAX 1
// HID主机实例数（至少1）
#define CFG_TUH_HID_EP_BUFSIZE 64

// 设备模式配置（若不需要可删除）
#define CFG_TUD_ENABLED 1
#define CFG_TUD_HID 1
#define CFG_TUD_HID_EP_BUFSIZE 64

#endif /* _TUSB_CONFIG_H_ */
