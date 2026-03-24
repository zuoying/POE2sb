#ifndef _CLASS_XINPUT_XINPUT_DEVICE_H_
#define _CLASS_XINPUT_XINPUT_DEVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "tusb.h"

// ===================== 新增/修改部分 =====================
// 兼容旧代码：定义xinput_state_t为xinput_report_t的别名
typedef struct xinput_report xinput_state_t;

// XInput手柄报告（状态）结构体（微软标准格式）
struct xinput_report {
  uint8_t  report_id;       // 报告ID（固定为0x00）
  uint16_t buttons;         // 按键状态（位掩码）
  int8_t   left_trigger;    // 左扳机（0~255）
  int8_t   right_trigger;   // 右扳机（0~255）
  int16_t  lx;              // 左摇杆X轴（-32768~32767）
  int16_t  ly;              // 左摇杆Y轴（-32768~32767）
  int16_t  rx;              // 右摇杆X轴（-32768~32767）
  int16_t  ry;              // 右摇杆Y轴（-32768~32767）
  uint8_t  reserved[6];     // 保留字段
};
// 简化类型名（核心类型）
typedef struct xinput_report xinput_report_t;
// ========================================================

// XInput设备模式初始化
void xinput_device_init(void);

// 发送XInput手柄报告到主机（电脑）
bool xinput_device_send_report(xinput_report_t const* report);

// 注册震动回调（主机发送震动指令时触发）
void xinput_device_set_rumble_cb(void (*cb)(uint8_t left, uint8_t right));

#ifdef __cplusplus
}
#endif

#endif /* _CLASS_XINPUT_XINPUT_DEVICE_H_ */
