#include "class/xinput/xinput_device.h"

// 设备就绪标志
static bool _xinput_ready = false;

// 震动回调函数指针
static void (*_rumble_callback)(uint8_t itf, uint8_t left, uint8_t right) = NULL;

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

// 发送XInput报告到指定接口
bool xinput_device_send_report(uint8_t itf, xinput_report_t const* report) {
  if (!_xinput_ready || !report) return false;
  
  // 发送HID报告到指定接口
  return tud_hid_report(itf, report, sizeof(xinput_report_t));
}

// 发送相同的报告到两个接口（同步模式）
bool xinput_device_send_report_both(xinput_report_t const* report) {
  if (!_xinput_ready || !report) return false;
  
  bool success1 = tud_hid_report(0, report, sizeof(xinput_report_t));
  bool success2 = tud_hid_report(1, report, sizeof(xinput_report_t));
  
  return success1 && success2;
}

// 注册震动回调
void xinput_device_set_rumble_cb(void (*cb)(uint8_t itf, uint8_t left, uint8_t right)) {
  _rumble_callback = cb;
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
  // 处理震动指令
  if (report_type == HID_REPORT_TYPE_OUTPUT && _rumble_callback != NULL) {
    // XInput震动报告格式：8字节
    if (bufsize >= 8) {
      uint8_t left_motor = buffer[2];  // 左电机（低频震动）
      uint8_t right_motor = buffer[3]; // 右电机（高频震动）
      _rumble_callback(instance, left_motor, right_motor);
    }
  }
}
