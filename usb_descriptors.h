#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

#include "tusb.h"

// 使用通用HID游戏手柄的VID/PID
#define GAMEPAD_VID 0x1209  // 通用测试VID
#define GAMEPAD_PID 0x0001  // 通用HID游戏手柄PID

// HID端点号
#define EPNUM_HID   0x01

// 外部引用的HID报告描述符
extern uint8_t const desc_hid_report[];
extern uint16_t const desc_hid_report_len;

typedef struct {
    uint8_t report_id;
    uint16_t buttons;
    uint8_t hat;
    int8_t left_x;
    int8_t left_y;
    int8_t right_x;
    int8_t right_y;
    uint8_t left_trigger;
    uint8_t right_trigger;
} __attribute__((packed)) hid_report_t;

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
