# GitHub编译说明

## 快速开始

### 1. 上传代码到GitHub
将修改后的代码推送到GitHub仓库。

### 2. 触发GitHub Actions编译
- 进入你的GitHub仓库
- 点击"Actions"标签页
- 选择"Build Firmware"工作流程
- 点击"Run workflow"按钮（如果需要手动触发）

### 3. 下载编译好的固件
- 等待编译完成（约5-10分钟）
- 在"Artifacts"部分下载 `poe2gamepad-firmware.zip`
- 解压后包含以下文件：
  - `poe2gamepad.uf2` - 主程序固件
  - `test_led_power.uf2` - LED和电源测试固件
  - `test_gamepad_detect.uf2` - 手柄检测固件

## 固件说明

### 1. poe2gamepad.uf2（主程序）
- 完整功能：双虚拟XInput手柄模拟
- 支持盖世小鸡超新星手柄（VID: 0x3537, PID: 0x100E）
- 三种模式切换：同步模式、仅主号、仅副号
- LED状态指示

### 2. test_led_power.uf2（测试固件）
- 仅测试LED和电源控制功能
- 用于验证硬件连接是否正确

### 3. test_gamepad_detect.uf2（检测固件）
- 用于检测USB设备的VID/PID
- 输出详细的设备信息到串口

## 烧录步骤

### Windows用户
1. 按住开发板上的BOOTSEL按钮
2. 通过USB-C接口连接电脑
3. 释放BOOTSEL按钮
4. 电脑将识别为U盘设备
5. 将对应的.uf2文件拖入U盘
6. 开发板将自动重启并运行新固件

### Linux/macOS用户
```bash
# 安装picotool（可选）
# 使用picotool烧录
picotool load -x poe2gamepad.uf2

# 或者使用U盘方式（同上）
```

## 验证步骤

### 1. 基础功能验证
1. 烧录 `test_led_power.uf2`
2. 观察LED是否按以下顺序工作：
   - 蓝色：初始化
   - 白色三次闪烁：启动完成
   - 红→绿→蓝→白循环5次
   - 最终绿色常亮，电源开启

### 2. 手柄识别验证
1. 烧录 `poe2gamepad.uf2`
2. 通过串口监控观察输出（波特率115200）
3. 连接盖世小鸡手柄到USB-A端口
4. 观察LED状态变化：
   - 青色：系统就绪
   - 绿色：手柄已连接
   - 红色闪烁：手柄未连接

### 3. Windows设备验证
1. 按Win+R，输入 `joy.cpl` 打开游戏控制器设置
2. 应该能看到两个新的游戏控制器
3. 点击"属性"测试按钮和摇杆

## 故障排除

### 编译失败
- 检查GitHub Actions日志中的错误信息
- 确保所有文件已正确上传
- 检查CMakeLists.txt语法

### 固件无法烧录
- 确保按住BOOTSEL按钮再连接USB
- 检查USB-C线缆和数据口
- 尝试不同的USB端口

### 手柄无法识别
- 检查串口输出中的VID/PID信息
- 确保手柄已正确插入USB-A端口
- 验证电源指示灯是否亮起

### LED不工作
- 检查GPIO16连接
- 确保烧录了正确的固件
- 测试 `test_led_power.uf2` 验证硬件

## 技术支持

如果遇到问题，请提供：
1. GitHub Actions编译日志
2. 串口输出的完整日志（如可用）
3. 具体的错误现象描述
4. 硬件连接照片（如可能）

## 更新记录

### 2025-03-24
- 添加盖世小鸡超新星手柄支持（VID: 0x3537, PID: 0x100E）
- 修复PIO-USB配置
- 改进调试输出
- 更新GitHub Actions配置