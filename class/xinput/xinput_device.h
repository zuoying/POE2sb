#ifndef _CLASS_XINPUT_XINPUT_DEVICE_H_
#define _CLASS_XINPUT_XINPUT_DEVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "tusb.h"

// XInput手柄报告格式（微软标准）
typedef struct {
  uint16_t wButtons;      // 按键位掩码
  uint8_t  bLeftTrigger;  // 左扳机（0-255）
  uint8_t  bRightTrigger; // 右扳机（0-255）
  int16_t  sThumbLX;      // 左摇杆X轴（-32768~32767）
  int16_t  sThumbLY;      // 左摇杆Y轴
  int16_t  sThumbRX;      // 右摇杆X轴
  int16_t  sThumbRY;      // 右摇杆Y轴
} xinput_report_t;

// XInput按键位定义（按需使用）
#define XINPUT_GAMEPAD_A        (1 << 0)
#define XINPUT_GAMEPAD_B        (1 << 1)
#define XINPUT_GAMEPAD_X        (1 << 2)
#define XINPUT_GAMEPAD_Y        (1 << 3)
#define XINPUT_GAMEPAD_LB       (1 << 4)
#define XINPUT_GAMEPAD_RB       (1 << 5)
#define XINPUT_GAMEPAD_BACK     (1 << 6)
#define XINPUT_GAMEPAD_START    (1 << 7)
#define XINPUT_GAMEPAD_L3       (1 << 8)
#define XINPUT_GAMEPAD_R3       (1 << 9)
#define XINPUT_GAMEPAD_UP       (1 << 10)
#define XINPUT_GAMEPAD_DOWN     (1 << 11)
#define XINPUT_GAMEPAD_LEFT     (1 << 12)
#define XINPUT_GAMEPAD_RIGHT    (1 << 13)
#define XINPUT_GAMEPAD_HOME     (1 << 14)

// 初始化XInput设备
void xinput_device_init(void);

// 发送XInput手柄报告到主机
bool xinput_device_send_report(xinput_report_t const* report);

#ifdef __cplusplus
}
#endif

#endif /* _CLASS_XINPUT_XINPUT_DEVICE_H_ */
