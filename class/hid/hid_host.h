#ifndef _CLASS_HID_HID_HOST_H_
#define _CLASS_HID_HID_HOST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "tusb.h"
#include "usb_descriptors.h"

// HID主机设备句柄
typedef struct {
    uint8_t dev_addr;    // USB设备地址
    uint8_t inst;        // HID实例号
    bool connected;      // 手柄是否已连接
    uint16_t vid;        // 设备VID
    uint16_t pid;        // 设备PID
    uint8_t report_len;  // 原始报告长度
    uint8_t raw_report[64]; // 原始HID报告缓冲区
} hid_host_t;

// 全局HID主机实例
extern hid_host_t hid_host;

// 初始化HID主机模式
void hid_host_init(void);

// 轮询HID手柄（需在主循环中调用）
void hid_host_task(void);

// 读取HID手柄的最新原始报告
// 返回值：true=读取成功，false=无数据/未连接
bool hid_host_get_raw_report(uint8_t* buffer, uint16_t* len);

// HID到XInput转换函数
// 自动检测报告格式并转换为标准XInput报告
// 参数：hid_data - 原始HID数据，hid_len - 原始数据长度
// 返回：转换后的XInput报告
xinput_report_t hid_to_xinput_convert(const uint8_t* hid_data, uint16_t hid_len);

#ifdef __cplusplus
}
#endif

#endif /* _CLASS_HID_HID_HOST_H_ */