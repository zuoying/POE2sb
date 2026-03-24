/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
 extern "C" {
#endif

//--------------------------------------------------------------------+
// Board Specific Configuration
//--------------------------------------------------------------------+

// 确保MCU为RP2350
#ifndef CFG_TUSB_MCU
  #define CFG_TUSB_MCU OPT_MCU_RP2350
#endif

// 同时启用USB Device和Host模式（双模）
#define CFG_TUD_ENABLED 1
#define CFG_TUH_ENABLED 1

// USB速度：RP2350 Host仅支持Full Speed
#define CFG_TUSB_RHPORT0_MODE OPT_MODE_NONE       // 禁用默认端口（可选）
#define CFG_TUSB_RHPORT1_MODE OPT_MODE_HOST       // RP2350 USB1作为Host（接盖世小鸡手柄）
#define CFG_TUSB_RHPORT2_MODE OPT_MODE_DEVICE     // RP2350 USB2作为Device（连PC）

//--------------------------------------------------------------------+
// Host Configuration（适配盖世小鸡有线手柄）
//--------------------------------------------------------------------+

#define CFG_TUH_MAX_TRIES 3                       // 设备枚举重试次数
#define CFG_TUH_ENUMERATION_BUFSIZE 256           // 枚举缓冲区（兼容盖世小鸡报告描述符）
#define CFG_TUH_HUB 1                             // 启用Hub（单手柄可禁用，建议保留）
#define CFG_TUH_CDC 0                             // 禁用CDC（无需串口）
#define CFG_TUH_HID 1                             // 启用HID Host（核心：读取手柄）
#define CFG_TUH_MSC 0                             // 禁用MSC（无需存储）
#define CFG_TUH_VENDOR 1                          // 启用Vendor类（兼容盖世小鸡XInput模式）

// HID配置（适配盖世小鸡）
#define CFG_TUH_HID_MAX_DEVICES 1                 // 仅接1个手柄
#define CFG_TUH_HID_MAX_REQ_PER_DEV 2             // 最大HID请求数
#define CFG_TUH_HID_EPIN_BUFSIZE 64               // 输入缓冲区（匹配盖世小鸡报告大小）
#define CFG_TUH_HID_EPOUT_BUFSIZE 64              // 输出缓冲区（震动/灯效）

// XInput Host配置（盖世小鸡部分型号支持XInput）
#define CFG_TUH_XINPUT 1
#define CFG_TUH_XINPUT_ENDPOINT_SIZE 32

//--------------------------------------------------------------------+
// Device Configuration（保留XInput Device，转发手柄数据）
//--------------------------------------------------------------------+

#define CFG_TUD_ENDPOINT0_SIZE 64
#define CFG_TUD_XINPUT 1
#define CFG_TUD_XINPUT_ENDPOINT_SIZE 32

#ifdef __cplusplus
 }
#endif

#endif /* _TUSB_CONFIG_H_ */
