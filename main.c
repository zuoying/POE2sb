/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 * Copyright (c) 2021 Ha Thach (tinyusb.org)
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "tusb.h"
#include "class/xinput/xinput_device.h"
#include "class/xinput/xinput_host.h"
#include "class/hid/hid_host.h"

// RP2350 USB启动延迟
#define RP2350_USB_START_DELAY 500

// 盖世小鸡手柄相关定义
#define GAMEPAD_REPORT_SIZE 64                    // 适配盖世小鸡HID报告大小
#define XINPUT_REPORT_ID 0                        // XInput报告ID

// 全局变量：手柄状态缓存
static xinput_report_t gamepad_report =
{
  .type = XINPUT_REPORT_TYPE_GAMEPAD,
  .gamepad =
  {
    .buttons = 0,
    .left_trigger = 0,
    .right_trigger = 0,
    .lx = 0,
    .ly = 0,
    .rx = 0,
    .ry = 0,
    .reserved = 0
  }
};

static uint8_t hid_dev_addr = 0;                  // 盖世小鸡手柄的USB地址
static bool is_xinput_gamepad = false;            // 是否为XInput模式手柄
static uint8_t hid_report_buf[GAMEPAD_REPORT_SIZE];// HID报告缓冲区

// 盖世小鸡HID报告解析（适配通用HID游戏手柄）
static void parse_gaishi_hid_report(uint8_t const* report, uint16_t len)
{
  if (len < 8) return; // 最小有效报告长度

  // 适配盖世小鸡有线手柄HID报告格式（需根据实际手柄调整，以下为通用模板）
  // 按键映射（示例：需根据手柄实际HID描述符修改）
  gamepad_report.gamepad.buttons = 0;
  if (report[1] & 0x01) gamepad_report.gamepad.buttons |= XINPUT_GAMEPAD_A;
  if (report[1] & 0x02) gamepad_report.gamepad.buttons |= XINPUT_GAMEPAD_B;
  if (report[1] & 0x04) gamepad_report.gamepad.buttons |= XINPUT_GAMEPAD_X;
  if (report[1] & 0x08) gamepad_report.gamepad.buttons |= XINPUT_GAMEPAD_Y;
  if (report[1] & 0x10) gamepad_report.gamepad.buttons |= XINPUT_GAMEPAD_LB;
  if (report[1] & 0x20) gamepad_report.gamepad.buttons |= XINPUT_GAMEPAD_RB;
  if (report[1] & 0x40) gamepad_report.gamepad.buttons |= XINPUT_GAMEPAD_BACK;
  if (report[1] & 0x80) gamepad_report.gamepad.buttons |= XINPUT_GAMEPAD_START;

  if (report[2] & 0x01) gamepad_report.gamepad.buttons |= XINPUT_GAMEPAD_LS;
  if (report[2] & 0x02) gamepad_report.gamepad.buttons |= XINPUT_GAMEPAD_RS;
  if (report[2] & 0x04) gamepad_report.gamepad.buttons |= XINPUT_GAMEPAD_HOME;
  if (report[2] & 0x08) gamepad_report.gamepad.buttons |= XINPUT_GAMEPAD_DPAD_UP;
  if (report[2] & 0x10) gamepad_report.gamepad.buttons |= XINPUT_GAMEPAD_DPAD_DOWN;
  if (report[2] & 0x20) gamepad_report.gamepad.buttons |= XINPUT_GAMEPAD_DPAD_LEFT;
  if (report[2] & 0x40) gamepad_report.gamepad.buttons |= XINPUT_GAMEPAD_DPAD_RIGHT;

  // 摇杆映射（0-255 → -128~127）
  gamepad_report.gamepad.lx = (int8_t)(report[3] - 128);
  gamepad_report.gamepad.ly = (int8_t)(128 - report[4]); // Y轴反向（适配多数手柄）
  gamepad_report.gamepad.rx = (int8_t)(report[5] - 128);
  gamepad_report.gamepad.ry = (int8_t)(128 - report[6]);

  // 扳机映射（0-255）
  gamepad_report.gamepad.left_trigger = report[7];
  gamepad_report.gamepad.right_trigger = report[8];
}

// 处理XInput模式手柄数据（盖世小鸡支持XInput的型号）
static void process_xinput_gamepad(uint8_t dev_addr)
{
  xinput_report_t xinput_report;
  if (tuh_xinput_get_report(dev_addr, XINPUT_REPORT_ID, &xinput_report))
  {
    // 直接复用XInput报告
    memcpy(&gamepad_report, &xinput_report, sizeof(xinput_report_t));
  }
}

// TinyUSB Host回调：设备连接
void tuh_mount_cb(uint8_t dev_addr)
{
  printf("USB Device Attached: Address %d\n", dev_addr);

  // 枚举设备，判断是否为盖世小鸡手柄（XInput/HID）
  uint16_t vid, pid;
  tuh_vid_pid_get(dev_addr, &vid, &pid);
  printf("VID: 0x%04X, PID: 0x%04X\n", vid, pid);

  // 检测XInput设备（Xbox兼容，盖世小鸡部分型号VID/PID可自定义）
  if (tuh_xinput_check(dev_addr))
  {
    printf("Found XInput Gamepad (Gaishi Chick)\n");
    hid_dev_addr = dev_addr;
    is_xinput_gamepad = true;
    tuh_xinput_open(dev_addr);
  }
  // 检测HID游戏手柄
  else if (tuh_hid_check(dev_addr))
  {
    printf("Found HID Gamepad (Gaishi Chick)\n");
    hid_dev_addr = dev_addr;
    is_xinput_gamepad = false;
    // 打开HID接口，注册报告回调
    tuh_hid_open(dev_addr, 0, hid_report_buf, sizeof(hid_report_buf));
  }
}

// TinyUSB Host回调：设备断开
void tuh_umount_cb(uint8_t dev_addr)
{
  if (dev_addr == hid_dev_addr)
  {
    printf("Gaishi Chick Gamepad Disconnected\n");
    hid_dev_addr = 0;
    is_xinput_gamepad = false;
    // 重置手柄状态
    memset(&gamepad_report, 0, sizeof(xinput_report_t));
  }
}

// TinyUSB Host回调：HID报告接收（通用HID手柄）
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len)
{
  (void)instance;
  if (dev_addr == hid_dev_addr && !is_xinput_gamepad)
  {
    parse_gaishi_hid_report(report, len);
  }
}

// 主循环：处理USB事件+转发手柄数据
int main(void)
{
  // 初始化串口（GPIO0=TX，GPIO1=RX）
  stdio_init_all();

  // RP2350 USB启动延迟
  sleep_ms(RP2350_USB_START_DELAY);
  printf("RP2350 Gaishi Chick Gamepad (Wired) → XInput Device\n");

  // 初始化TinyUSB（Host+Device双模）
  tusb_init();

  while (1)
  {
    // 处理USB Host/Device事件
    tuh_task();
    tud_task();

    // 读取物理手柄数据
    if (hid_dev_addr != 0)
    {
      if (is_xinput_gamepad)
      {
        // XInput模式手柄
        process_xinput_gamepad(hid_dev_addr);
      }
      // HID模式已通过回调解析，无需额外处理
    }

    // 转发手柄数据到PC（XInput Device）
    if (tud_xinput_ready())
    {
      tud_xinput_report(0, &gamepad_report);
    }

    // 降低CPU占用
    sleep_ms(1);
  }

  return 0;
}

// TinyUSB Device回调：XInput配置完成
void tud_xinput_configured_cb(uint8_t itf, bool configured)
{
  (void) itf;
  if (configured)
  {
    printf("XInput Device Configured (RP2350 → PC)\n");
  }
  else
  {
    printf("XInput Device Unconfigured\n");
  }
}

// TinyUSB Device回调：LED状态请求
uint8_t tud_xinput_get_led_state_cb(void)
{
  // 同步盖世小鸡手柄LED（示例：常亮LED1）
  return XINPUT_LED_1;
}

// TinyUSB Device回调：震动请求（转发给物理手柄）
void tud_xinput_set_state_cb(xinput_state_t const* state)
{
  if (hid_dev_addr != 0 && is_xinput_gamepad)
  {
    // 转发震动指令给盖世小鸡XInput手柄
    tuh_xinput_set_state(hid_dev_addr, state);
    printf("Vibration Forwarded: Left=%d, Right=%d\n", state->motor_left, state->motor_right);
  }
}
