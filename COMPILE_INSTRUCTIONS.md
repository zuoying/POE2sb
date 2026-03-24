# POE2sb 编译说明

## 文件修改概述

我已经对以下文件进行了修改，以解决LED灯未亮和手柄未通电的问题：

### 1. `tusb_config.h`
- 启用了USB主机功能 (`CFG_TUH_ENABLED = 1`)
- 添加了HID主机支持 (`CFG_TUH_HID = 1`)
- 启用了USB Hub支持 (`CFG_TUH_HUB = 1`)
- 启用了PIO-USB功能 (`CFG_TUH_RPI_PIO_USB = 1`)
- 配置了最大设备数量和枚举缓冲区大小

### 2. `CMakeLists.txt`
- 设置了正确的板级定义 (`waveshare_rp2350_usb_a`)
- 配置了PIO-USB引脚 (D+ = 12, D- = 13)
- 添加了`pico_multicore`和`hardware_pio`库支持
- 启用了自动从GitHub获取SDK的功能
- 添加了简化的LED和电源测试程序

### 3. `main.c`
- 重写了整个文件，实现了以下功能：
  - WS2812 LED驱动（RGB格式）
  - 5V电源控制（16mA驱动强度）
  - 双核心架构（Core0处理USB设备，Core1处理USB主机）
  - USB主机初始化和错误处理
  - 标准HID游戏手柄描述符
  - 手柄连接状态的LED指示

### 4. `test_led_power.c`（新增）
- 简化的测试程序，只关注LED和电源控制
- 用于快速验证硬件连接是否正常

## GitHub编译步骤

1. 将所有修改后的文件上传到GitHub仓库
2. 确保`.github/workflows/build.yml`文件存在且配置正确
3. GitHub Actions将自动开始编译过程
4. 编译成功后，可在Actions页面下载生成的固件文件（.uf2格式）

## 测试建议

### 第一步：测试LED和电源功能
1. 上传`test_led_power.uf2`固件到开发板
2. 观察LED是否按照以下顺序闪烁：
   - 蓝色：初始化
   - 白色三次闪烁：启动完成
   - 红色（电源关闭）→ 绿色（电源开启）→ 蓝色（电源开启）→ 白色（电源关闭）循环5次
   - 最终状态：绿色常亮，电源开启
3. 检查手柄是否在绿色常亮期间通电

### 第二步：测试完整功能
1. 上传`poe2gamepad.uf2`固件到开发板
2. 观察LED状态：
   - 蓝色：初始化中
   - 白色三次闪烁：启动完成
   - 青色：系统就绪
   - 绿色：手柄已连接
   - 红色闪烁：手柄未连接
3. 连接手柄，检查是否被识别
4. 使用`joy.cpl`命令检查Windows是否识别到虚拟手柄

## 故障排除

如果LED仍然不亮或手柄不通电：

1. 检查硬件连接是否正确
   - LED（GPIO16）
   - 电源控制（GPIO18）
   - USB主机D+（GPIO12）
   - USB主机D-（GPIO13）

2. 检查开发板供电是否正常

3. 尝试重新烧录固件

4. 如果问题仍然存在，可以尝试上传`test_led_power.uf2`进行基础功能测试
