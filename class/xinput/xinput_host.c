#include "class/xinput/xinput_host.h"
#include "tusb.h"
#include <string.h>

// 全局XInput主机实例
xinput_host_t xinput_host = {
  .dev_addr = 0,
  .inst = 0,
  .connected = false
};

// XInput主机专用常量
#define XINPUT_HOST_EP_IN    0x81
#define XINPUT_HOST_EP_OUT   0x01
#define XINPUT_RUMBLE_REPORT_SIZE 8

// 缓存最新的手柄报告
static xinput_report_t _last_report = {0};
static bool _report_updated = false;

// 检查设备是否为XInput手柄（匹配Vid/Pid）
static bool _is_xinput_device(uint16_t vid, uint16_t pid) {
  // Xbox 360手柄标准Vid/Pid（可扩展其他XInput设备）
  return (vid == 0x045E && pid == 0x028E) || 
         (vid == 0x045E && pid == 0x0291) ||
         (vid == 0x045E && pid == 0x02A1);
}

// tinyUSB主机HID匹配回调（识别XInput手柄）
bool tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
  (void) desc_len;
  
  // 获取设备Vid/Pid
  tusb_desc_device_t const* dev_desc = tuh_device_get_descriptor(dev_addr);
  if (!dev_desc || !_is_xinput_device(dev_desc->idVendor, dev_desc->idProduct)) {
    return false; // 非XInput设备，跳过
  }

  // 记录XInput手柄的设备地址和实例
  xinput_host.dev_addr = dev_addr;
  xinput_host.inst = instance;
  xinput_host.connected = true;
  _report_updated = false;
  memset(&_last_report, 0, sizeof(_last_report));

  // 注册HID报告接收回调
  tuh_hid_set_report_received_cb(dev_addr, instance, NULL);
  return true;
}

// tinyUSB主机HID断开回调
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
  if (xinput_host.dev_addr == dev_addr && xinput_host.inst == instance) {
    xinput_host.connected = false;
    xinput_host.dev_addr = 0;
    xinput_host.inst = 0;
  }
}

// 接收XInput手柄报告的回调
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
  if (xinput_host.dev_addr != dev_addr || xinput_host.inst != instance) return;
  if (len < sizeof(xinput_report_t)) return; // 报告长度不合法

  // 更新缓存的手柄报告
  memcpy(&_last_report, report, sizeof(xinput_report_t));
  _report_updated = true;
}

// 初始化XInput主机模式
void xinput_host_init(void) {
  // 确保tinyUSB主机模式已初始化
  if (!tuh_inited()) {
    tuh_init(BOARD_TUH_RHPORT); // BOARD_TUH_RHPORT为RP2350的USB Host端口（通常是0或1）
  }
  memset(&xinput_host, 0, sizeof(xinput_host_t));
  memset(&_last_report, 0, sizeof(xinput_report_t));
  _report_updated = false;
}

// 主循环轮询（处理USB主机事件）
void xinput_host_task(void) {
  tuh_task(); // 必须调用：处理tinyUSB主机核心逻辑

  if (!xinput_host.connected) return;

  // 主动请求XInput手柄报告（轮询模式）
  if (tuh_hid_receive_report(xinput_host.dev_addr, xinput_host.inst)) {
    // 成功发起报告请求，等待回调接收数据
  }
}

// 读取最新的XInput手柄报告
bool xinput_host_get_report(xinput_report_t* report) {
  if (!report || !xinput_host.connected || !_report_updated) return false;

  memcpy(report, &_last_report, sizeof(xinput_report_t));
  _report_updated = false; // 重置更新标志
  return true;
}

// 发送震动指令到XInput手柄
bool xinput_host_send_rumble(uint8_t left_motor, uint8_t right_motor) {
  if (!xinput_host.connected) return false;

  // XInput震动报告格式（微软标准）
  uint8_t rumble_report[XINPUT_RUMBLE_REPORT_SIZE] = {0};
  rumble_report[0] = 0x00; // 报告类型
  rumble_report[1] = 0x08; // 指令长度
  rumble_report[2] = left_motor;  // 左电机（低频震动）
  rumble_report[3] = right_motor; // 右电机（高频震动）

  // 发送震动报告到手柄的OUT端点
  return tuh_hid_set_report(
    xinput_host.dev_addr, 
    xinput_host.inst, 
    0, 
    HID_REPORT_TYPE_OUTPUT, 
    rumble_report, 
    XINPUT_RUMBLE_REPORT_SIZE
  );
}
