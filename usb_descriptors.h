#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

#include <stdint.h>
#include <stdbool.h>

// 使用通用HID游戏手柄的VID/PID
#define GAMEPAD_VID 0x1209  // 通用测试VID
#define GAMEPAD_PID 0x0001  // 通用HID游戏手柄PID

// HID端点号 - 双接口需要两个端点
#define EPNUM_HID1   0x81  // 接口0的IN端点
#define EPNUM_HID2   0x82  // 接口1的IN端点

// 外部引用的HID报告描述符
extern uint8_t const desc_hid_report[];
extern uint16_t const desc_hid_report_len;

// XInput报告结构体（与xinput_device.h保持一致）
typedef struct __attribute__((packed)) {
    uint8_t  report_id;       // 报告ID（固定为0x00）
    uint16_t buttons;         // 按键状态（位掩码）
    int8_t   left_trigger;    // 左扳机（0~255）
    int8_t   right_trigger;   // 右扳机（0~255）
    int16_t  lx;              // 左摇杆X轴（-32768~32767）
    int16_t  ly;              // 左摇杆Y轴（-32768~32767）
    int16_t  rx;              // 右摇杆X轴（-32768~32767）
    int16_t  ry;              // 右摇杆Y轴（-32768~32767）
    uint8_t  reserved[6];     // 保留字段
} xinput_report_t;

// 按钮定义（XInput标准）
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

// 工作模式定义
typedef enum {
    SYNC_MODE,    // 同步模式：两个虚拟控制器都输出
    MAIN_MODE,    // 仅主模式：只输出虚拟控制器1
    SUB_MODE      // 仅副模式：只输出虚拟控制器2
} work_mode_t;

#endif
