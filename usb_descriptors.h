#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

#include "tusb.h"

// Xbox 360 控制器 PID/VID
#define XBOX_VID 0x045E
#define XBOX_PID 0x028E

// XInput 接口类 (Vendor Specific)
#define XINPUT_CLASS 0xFF
#define XINPUT_SUBCLASS 0x5D
#define XINPUT_PROTOCOL 0x01

// 每个手柄的端点数量 (1 IN, 1 OUT)
#define EPNUM_XBOX1_IN  0x81
#define EPNUM_XBOX1_OUT 0x01
#define EPNUM_XBOX2_IN  0x82
#define EPNUM_XBOX2_OUT 0x02

// 每个手柄的报告大小
#define XBOX_REPORT_SIZE 20

// 手柄输入报告结构体 (Xbox 360 XInput)
typedef struct {
    uint8_t report_id;   // 总是 0x00
    uint8_t report_size; // 总是 0x14 (20 字节)
    uint16_t buttons;    // 按键位掩码
    uint8_t left_trigger;
    uint8_t right_trigger;
    int16_t left_stick_x;
    int16_t left_stick_y;
    int16_t right_stick_x;
    int16_t right_stick_y;
    uint8_t reserved[6];
} __attribute__((packed)) xbox_report_t;

// 按键位掩码定义 (微软 Xbox 360)
#define XBOX_BTN_UP     0x0001
#define XBOX_BTN_DOWN   0x0002
#define XBOX_BTN_LEFT   0x0004
#define XBOX_BTN_RIGHT  0x0008
#define XBOX_BTN_START  0x0010  // Menu
#define XBOX_BTN_BACK   0x0020  // View
#define XBOX_BTN_L3     0x0040
#define XBOX_BTN_R3     0x0080
#define XBOX_BTN_LB     0x0100
#define XBOX_BTN_RB     0x0200
#define XBOX_BTN_GUIDE  0x0400
#define XBOX_BTN_A      0x1000
#define XBOX_BTN_B      0x2000
#define XBOX_BTN_X      0x4000
#define XBOX_BTN_Y      0x8000

#endif
