#ifndef _CLASS_XINPUT_XINPUT_DEVICE_H_
#define _CLASS_XINPUT_XINPUT_DEVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "tusb.h"
#include "usb_descriptors.h"

// XInput设备模式初始化
void xinput_device_init(void);

// 发送XInput手柄报告到主机（电脑）- 双接口版本
// itf: 接口号 (0=第一个控制器, 1=第二个控制器)
bool xinput_device_send_report(uint8_t itf, xinput_report_t const* report);

// 发送相同的报告到两个接口（同步模式）
bool xinput_device_send_report_both(xinput_report_t const* report);

// 注册震动回调（主机发送震动指令时触发）
void xinput_device_set_rumble_cb(void (*cb)(uint8_t itf, uint8_t left, uint8_t right));

#ifdef __cplusplus
}
#endif

#endif /* _CLASS_XINPUT_XINPUT_DEVICE_H_ */
