#include "class/xinput/xinput_device.h"

// XInput设备端点定义（需与tusb_config.h中HID缓冲区匹配）
#define XINPUT_EP_IN 0x81
#define XINPUT_REPORT_SIZE 20 // XInput标准报告长度

// 设备就绪标志
static bool _xinput_ready = false;

// tinyUSB设备初始化回调（自动触发）
void tud_hid_mount_cb(void) {
  _xinput_ready = true;
}

// tinyUSB设备断开回调
void tud_hid_umount_cb(void) {
  _xinput_ready = false;
}

// 初始化XInput设备（调用tud_init后执行）
void xinput_device_init(void) {
  _xinput_ready = tud_ready(); // 同步tinyUSB就绪状态
}

// 发送XInput报告（封装tinyUSB的HID发送接口）
bool xinput_device_send_report(xinput_report_t const* report) {
  if (!_xinput_ready || !report) return false;
  
  // 发送HID报告（XInput复用HID端点）
  return tud_hid_report(0, report, sizeof(xinput_report_t));
}

// （可选）处理主机发来的XInput指令（如震动、LED）
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;
  return 0; // 极简实现：暂不处理主机指令
}

// （可选）接收主机写入的XInput报告（如震动控制）
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) bufsize;
  // 极简实现：暂不处理震动/LED等主机指令
}
