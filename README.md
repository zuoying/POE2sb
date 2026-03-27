# POE2sb Gamepad Synchronizer

## 项目概述
本项目为Waveshare RP2350-USB-A板开发固件，读取GameSir T4 Kaleid有线HID手柄并输出双虚拟XInput控制器到PC。

## 主要特性
- 读取GameSir T4 Kaleid控制器（VID:0x3537, PID:0x100E）
- 输出两个虚拟XInput控制器
- 三种工作模式：SYNC（同步）、MAIN（仅主）、SUB（仅副）
- WS2812 LED状态指示和自动亮度调节
- 反作弊随机输入偏移（摇杆±4，扳机±2，延迟1-6ms）仅应用于虚拟控制器2
- 通过Menu+View按钮组合（长按1秒）切换模式

## 架构设计
- Core0：USB设备模式（双XInput接口）
- Core1：USB主机模式（HID手柄读取）
- HID→XInput转换，自动检测报告格式
- RP2350双核心操作

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
### Linux/MacOS
```bash
chmod +x build.sh
./build.sh
```

### Windows
```cmd
build.bat
```

## 使用方法
1. 将GameSir手柄连接到RP2350板的USB-A端口
2. 将RP2350板的USB-C端口连接到电脑
3. LED状态指示：
   - 蓝色：系统初始化中
   - 青色：系统就绪，等待手柄连接
   - 绿色：手柄已连接
   - 绿色：SYNC模式（同步）
   - 蓝色：MAIN模式（仅主控制器）
   - 红色：SUB模式（仅副控制器）
4. 长按Menu+View按钮1秒切换模式

## 反作弊特性
- 随机偏移仅应用于虚拟控制器2
- 摇杆偏移：±4单位
- 扳机偏移：±2单位
- 延迟偏移：1-6毫秒
- 仅在SYNC和SUB模式下生效

## 技术细节
- USB描述符配置为双HID接口（CFG_TUD_HID=2）
- 自动检测XInput（19-20字节）和DInput（8字节）报告格式
- PIO-USB支持同时的主机和设备操作
- WS2812 LED通过PIO控制

## 硬件要求
- Waveshare RP2350-USB-A板
- GameSir T4 Kaleid有线手柄
- USB-C线（连接到PC）
- USB-A线（连接到手柄）
- WS2812 LED（连接到GPIO16）

## 许可证和安全
本项目仅供学习和研究用途。使用时请遵守：
- 游戏服务条款和最终用户许可协议
- 反作弊系统政策
- 当地法律法规

## 支持与贡献
如有问题或建议，请：
1. 查看串口调试输出（115200波特率）
2. 检查硬件连接
3. 参考技术文档
4. 在项目仓库中提交问题

## 项目状态
✅ 完成代码架构设计和实现  
✅ 完成HID主机模块  
✅ 完成XInput设备模块  
✅ 完成LED管理模块  
✅ 完成模式管理模块  
✅ 完成构建脚本和文档  
⚠️ 需要在实际硬件上测试验证

## 参考项目
- [Pico-PIO-USB](https://github.com/sekigon-gonnoc/Pico-PIO-USB)
- [OGX-Mini](https://github.com/wiredopposite/OGX-Mini/)
