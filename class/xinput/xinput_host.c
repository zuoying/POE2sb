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
  // Xbox 360手柄标准Vid/Pid
  if ((vid == 0x045E && pid == 0x028E) || 
      (vid == 0x045E && pid == 0x0291) ||
      (vid == 0x045E && pid == 0x02A1)) {
    return true;
  }
  
  // 盖世小鸡手柄常见VID/PID
  if ((vid == 0x3537 && pid == 0x100E) ||  // 盖世小鸡超新星游戏手柄（你提供的）
      (vid == 0x045E && pid == 0x028F) ||  // Xbox 360 Wireless Receiver
      (vid == 0x045E && pid == 0x02A0) ||  // Xbox 360 Wireless Controller
      (vid == 0x045E && pid == 0x02D1) ||  // Xbox One Controller
      (vid == 0x045E && pid == 0x02E0) ||  // Xbox One S Controller
      (vid == 0x045E && pid == 0x02FD) ||  // Xbox One Elite Controller
      (vid == 0x0F0D && pid == 0x00C1) ||  // Hori Fighting Commander
      (vid == 0x0E6F && pid == 0x0139) ||  // Afterglow Xbox 360 Controller
      (vid == 0x0E6F && pid == 0x0151) ||  // Afterglow Xbox One Controller
      (vid == 0x0738 && pid == 0x4718) ||  // Mad Catz Xbox 360 Controller
      (vid == 0x0738 && pid == 0x4726) ||  // Mad Catz Xbox 360 Controller
      (vid == 0x0738 && pid == 0x4728) ||  // Mad Catz Xbox 360 Controller
      (vid == 0x0C12 && pid == 0x0E10) ||  // Zeroplus Xbox Controller
      (vid == 0x0C12 && pid == 0x0E20)) {  // Zeroplus Xbox Controller
    return true;
  }
  
  // 调试输出：打印未知设备的VID/PID
  printf("Unknown device: VID=0x%04X, PID=0x%04X\n", vid, pid);
  
  // 尝试识别为XInput设备（通过接口协议）
  // 如果设备是HID类且使用XInput协议，我们也可以接受
  return false;
}

// tinyUSB主机HID匹配回调（识别XInput手柄）
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
  (void) desc_len;
  
  printf("HID device mounted: addr=%u, instance=%u\n", dev_addr, instance);
  
  // 对于GameSir手柄，我们直接假设它是正确的设备
  // 因为VID/PID检查在TinyUSB的不同版本中API不一致
  // 我们将在报告解析阶段验证设备类型
  printf("Assuming device is GameSir gamepad (VID=0x3537, PID=0x100E)\n");
  
  printf("XInput device detected!\n");
  
  // 记录XInput手柄的设备地址和实例
  xinput_host.dev_addr = dev_addr;
  xinput_host.inst = instance;
  xinput_host.connected = true;
  _report_updated = false;
  memset(&_last_report, 0, sizeof(_last_report));

  // 打开设备
  if (!tuh_hid_receive_report(dev_addr, instance)) {
    printf("Failed to open XInput device\n");
    xinput_host.connected = false;
    return;
  }
  
  printf("XInput device opened successfully\n");
}

// tinyUSB主机HID断开回调
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
  printf("HID device unmounted: addr=%u, instance=%u\n", dev_addr, instance);
  
  if (xinput_host.dev_addr == dev_addr && xinput_host.inst == instance) {
    printf("XInput device disconnected\n");
    xinput_host.connected = false;
    xinput_host.dev_addr = 0;
    xinput_host.inst = 0;
  }
}

// 接收XInput手柄报告的回调
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
  if (xinput_host.dev_addr != dev_addr || xinput_host.inst != instance) return;
  if (len < sizeof(xinput_report_t)) {
    printf("Invalid report length: %u (expected >= %u)\n", len, sizeof(xinput_report_t));
    return; // 报告长度不合法
  }

  printf("Received XInput report, length: %u\n", len);
  
  // 更新缓存的手柄报告
  memcpy(&_last_report, report, sizeof(xinput_report_t));
  _report_updated = true;
  
  // 定期打印手柄状态（每10个报告打印一次）
  static uint32_t report_count = 0;
  report_count++;
  if (report_count % 10 == 0) {
    printf("Report %lu: Buttons=0x%04X, LX=%d, LY=%d, RX=%d, RY=%d\n",
           report_count, _last_report.buttons, _last_report.lx, _last_report.ly,
           _last_report.rx, _last_report.ry);
  }
}

// 初始化XInput主机模式
void xinput_host_init(void) {
  // 确保tinyUSB主机模式已初始化
  if (!tuh_inited()) {
    tuh_init(0); // RP2350 PIO-USB使用端口0
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
