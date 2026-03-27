# POE2sb Gamepad Synchronizer

## 项目概述
本项目为adafruit-feather-rp2040-with-usb-type-a-host开发固件，读取GameSir T4 Kaleid有线HID手柄并输出双虚拟XInput控制器到PC。

## 主要特性
- 读取GameSir T4 Kaleid控制器（VID:0x3537, PID:0x100E）
- 输出两个虚拟XInput控制器
- 三种工作模式：SYNC（同步）、MAIN（仅主）、SUB（仅副）
- LED状态指示
- 反作弊随机输入偏移（摇杆±4，扳机±2，延迟1-6ms）仅应用于虚拟控制器2
- 通过Menu+View按钮组合切换模式

## 架构设计
- Core0：USB设备模式（双XInput接口）
- Core1：USB主机模式（HID手柄读取）
- HID→XInput转换，自动检测报告格式
- RP2040双核心操作

## 文件结构
```
POE2sb/
├── main.c                    # 主程序，整合所有模块
├── usb_descriptors.c        # USB描述符（双接口配置）
├── usb_descriptors.h        # USB描述符定义
├── tusb_config.h            # TinyUSB配置
├── led_manager.c            # LED状态管理
├── led_manager.h            # LED管理头文件
├── mode_manager.c          # 模式切换和反作弊偏移
├── mode_manager.h          # 模式管理头文件
├── class/hid/hid_host.c    # HID主机读取GameSir控制器
├── class/hid/hid_host.h    # HID主机头文件
├── class/hid/hid_xinput_converter.c # HID到XInput转换
├── class/xinput/xinput_device.c     # XInput设备虚拟化
├── class/xinput/xinput_device.h    # XInput设备头文件
├── CMakeLists.txt          # CMake构建配置
├── build.sh                # Linux构建脚本
├── build.bat               # Windows构建脚本
├── BUILD_AND_USAGE.md      # 构建和使用指南
└── README.md               # 项目说明
```

## 构建方法
### github远程仓库编译




## 使用方法
1. 将GameSir手柄连接到RP2040板的USB-A端口
2. 将RP2040板的USB-C端口连接到电脑
3. 同时按Menu+View按钮切换模式

## 反作弊特性
- 随机偏移仅应用于虚拟控制器2
- 摇杆偏移：±4单位
- 扳机偏移：±2单位
- 延迟偏移：1-6毫秒
- 仅在SYNC和SUB模式下生效



## 硬件要求
- GameSir T4 Kaleid有线手柄
- USB-C线（连接到PC）
- USB-A线（连接到手柄）

## GitHub Desktop 使用
为了更方便地推送代码，推荐使用GitHub Desktop：


## 项目状态
✅ 完成代码架构设计和实现  
✅ 完成HID主机模块  
✅ 完成XInput设备模块  
✅ 完成LED管理模块  
✅ 完成模式管理模块  
✅ 完成构建脚本和文档  
⚠️ 需要在实际硬件上测试验证
手柄连接开发板usb后仅显示充电指示灯，无连接指示灯（直连PC会亮连接指示灯）
## 参考项目
- [Pico-PIO-USB](https://github.com/sekigon-gonnoc/Pico-PIO-USB)
- [OGX-Mini](https://github.com/wiredopposite/OGX-Mini/)
