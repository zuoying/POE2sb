#ifndef _CLASS_XINPUT_XINPUT_HOST_H_
#define _CLASS_XINPUT_HOST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "tusb.h"
#include "xinput_device.h" // 复用XInput报告格式定义

// XInput主机设备句柄
typedef struct {
  uint8_t dev_addr;    // USB设备地址
  uint8_t inst;        // HID实例号
  bool connected;      // 手柄是否已连接
} xinput_host_t;

// 全局XInput主机实例（支持单手柄）
extern xinput_host_t xinput_host;

// 初始化XInput主机模式
void xinput_host_init(void);

// 轮询XInput手柄（需在主循环中调用）
void xinput_host_task(void);

// 读取XInput手柄的最新报告
// 返回值：true=读取成功，false=无数据/未连接
bool xinput_host_get_report(xinput_report_t* report);

// 发送震动指令到XInput手柄
// left_motor: 左电机（0-255）, right_motor: 右电机（0-255）
bool xinput_host_send_rumble(uint8_t left_motor, uint8_t right_motor);

#ifdef __cplusplus
}
#endif

#endif /* _CLASS_XINPUT_XINPUT_HOST_H_ */
