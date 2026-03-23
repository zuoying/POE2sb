#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

#include "tusb.h"

// 标准游戏手柄的VID/PID
#define GAMEPAD_VID 0x04D8  // Microchip Technology Inc.
#define GAMEPAD_PID 0x003F  // 标准游戏手柄

// 标准HID游戏手柄报告结构
typedef struct {
    uint8_t report_id;   // 报告ID
    uint16_t buttons;    // 按键位掩码
    uint8_t hat;         // 方向键
    int8_t left_x;       // 左摇杆X轴
    int8_t left_y;       // 左摇杆Y轴
    int8_t right_x;      // 右摇杆X轴
    int8_t right_y;      // 右摇杆Y轴
    uint8_t left_trigger;
    uint8_t right_trigger;
} __attribute__((packed)) hid_report_t;

// 按键位掩码定义 (标准游戏手柄)
#define BTN_UP     0x0001
#define BTN_DOWN   0x0002
#define BTN_LEFT   0x0004
#define BTN_RIGHT  0x0008
#define BTN_START  0x0010
#define BTN_BACK   0x0020
#define BTN_L3     0x0040
#define BTN_R3     0x0080
#define BTN_LB     0x0100
#define BTN_RB     0x0200
#define BTN_GUIDE  0x0400
#define BTN_A      0x1000
#define BTN_B      0x2000
#define BTN_X      0x4000
#define BTN_Y      0x8000

#endif
