# POE2sb
### 1. 项目目标与背景
项目名称 ：poe2gamepad (Xbox 360 手柄 1 拖 2 同步器) 核心目标 ：
使用 微雪 RP2350-USB-A 开发板，实现将一个真实的有线 Xbox 360 手柄(国产盖世小鸡)输入，同步模拟为两个独立的虚拟 Xbox 360 手柄输出给 PC。 功能特性 ：

- 三种模式切换 ：通过手柄组合按键（View + Menu）切换：同步模式（绿灯）、仅主号（蓝灯）、仅副号（红灯）。
- 防检测优化 ：
  - 随机抖动 ：摇杆 [ o bj ec tO bj ec t ] ± 4 、触发器 [ o bj ec tO bj ec t ] ± 2 的随机数值偏移。
  - 输入延迟 ：1-6ms 的随机指令延迟，模拟人类操作。
- 硬件反馈 ：利用板载 WS2812 RGB LED 显示当前工作状态。
### 2. 技术栈与架构
- 硬件平台 ：Raspberry Pi RP2350 (ARM Cortex-M33)。
- 开发框架 ：Pico SDK 2.0.0 (支持 RP2350 的最新版本)。
- USB 协议栈 ：
  - Device 模式 (TinyUSB) ：模拟两个 Vendor-specific XInput 设备。
  - Host 模式 (PIO-USB) ：利用 PIO 状态机在 GPIO 12/13 上实现软件 USB 主机，读取真实手柄数据。
- 构建系统 ：CMake + GitHub Actions (云端 Ubuntu 编译环境)。
### 3. 当前进度情况
已完成部分 ：

- main.c ：核心逻辑代码，包括随机抖动算法、模式切换状态机、WS2812 PIO 驱动。
- usb_descriptors.c ：双 XInput 设备描述符配置。
- tusb_config.h ：针对 SDK 2.0.0 的各种兼容性补丁（类型定义、宏冲突解决）。
- CMakeLists.txt ：多目标链接配置，强制指定 RP2350 平台及驱动。
待验证/最后一步 ：

- 链接修复 ：目前正处于解决 GitHub Actions 编译报错的最后阶段。主要问题在于云端环境有时无法自动识别 RP2350 平台，导致 TinyUSB 主机控制器驱动（HCD）符号链接失败。
- 最新改动 ：已在 CMakeLists.txt 中手动强制包含 hcd_pio_usb.c ，并修改了 CI 脚本以显式传递平台参数。
### 4. 迁移时的核心技术要点 (关键坑点)
如果你将此项目迁移到新的 AI 环境，请务必告知新 AI 以下内容：

1. SDK 版本敏感性 ：
   RP2350 必须使用 Pico SDK 2.0.0+ 。该版本中 TinyUSB 的 API 有变动（如 tuh_edpt_xfer 参数结构）。
2. 类型定义冲突 (The pipe_handle_t Fix) ：
   TinyUSB 和 PIO-USB 在 RP2350 上存在头文件循环引用。我们在 tusb_config.h 中手动定义了 typedef void * pipe_handle_t; 来强行切断依赖。
3. MCU 识别强制化 ：
   在云端编译时，必须强制设置宏 CFG_TUSB_MCU=111 (代表 RP2350) 和 CFG_TUSB_OS=4 (代表 PICO SDK)，否则编译器会按 RP2040 处理导致功能失效。
4. 引脚定义 ：
   微雪 RP2350-USB-A 的 USB-A 口硬件引脚是 GPIO 12 (D+) 和 13 (D-) ，而非标准 Pico 的引脚。
5. 驱动链接 ：
   如果出现 undefined reference to hcd_... ，需要在 CMakeLists.txt 中手动添加 ${PICO_SDK_PATH}/lib/tinyusb/src/portable/raspberrypi/pio_usb/hcd_pio_usb.c 到编译源文件列表。
### 总结
项目逻辑已闭环，目前的挑战完全集中在 针对 RP2350 新平台的交叉编译环境适配 。迁移后，只需确保 CMake 能正确找到并链接 tinyusb 和 pio_usb 的主机驱动，即可产出固件。

### 盖世小鸡手柄识别问题解决方案
根据你提供的设备信息，盖世小鸡超新星游戏手柄的硬件ID为：
- **VID**: `0x3537`
- **PID**: `0x100E`

已进行以下修复：

1. **已添加盖世小鸡手柄VID/PID支持**：在 `xinput_host.c` 的 `_is_xinput_device()` 函数中已添加 `(vid == 0x3537 && pid == 0x100E)`
2. **修复PIO-USB配置**：修正了USB主机初始化参数和RP2350特定配置
3. **添加调试工具**：创建了 `test_gamepad_detect.c` 程序用于检测手柄VID/PID
4. **完善回调函数**：改进了USB主机回调函数链和错误处理

**测试步骤**：
1. 在GitHub Actions中编译最新代码
2. 烧录 `poe2gamepad.uf2` 固件到开发板
3. 连接盖世小鸡手柄到USB-A端口
4. 观察LED状态和串口输出（如可用）

详细测试说明请参考 [TEST_INSTRUCTIONS.md](TEST_INSTRUCTIONS.md)

### 官方示例参考
https://github.com/sekigon-gonnoc/Pico-PIO-USB
https://github.com/wiredopposite/OGX-Mini/
C:\Users\dring\Documents\trae_projects\Pico-PIO-USB-main
C:\Users\dring\Documents\trae_projects\RP2350-USB-A-RGB
