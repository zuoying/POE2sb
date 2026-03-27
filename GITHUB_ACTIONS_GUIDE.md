# GitHub Actions 使用指南

## 已完成的修复

1. **CMakeLists.txt 修复**：
   - 修复了PIO-USB路径：`${TINYUSB_PATH}/hw/mcu/raspberrypi/pio_usb`
   - 解决了 `pio_usb.h: No such file or directory` 编译错误

2. **GitHub Actions 配置优化**：
   - 添加了PIO-USB编译选项：`-DCFG_TUH_RPI_PIO_USB=1`
   - 添加了PIO-USB引脚配置选项
   - 确保正确的CMake配置

## 使用步骤

### 1. 提交代码到GitHub
```bash
git add .
git commit -m "修复PIO-USB路径和GitHub Actions配置"
git push origin main
```

### 2. 触发GitHub Actions编译
1. 访问你的GitHub仓库页面
2. 点击顶部菜单栏的 **"Actions"** 标签
3. 在左侧边栏选择 **"Build Firmware"** 工作流
4. 点击 **"Run workflow"** 按钮（如果需要手动触发）
5. 选择分支（默认为main）并点击绿色按钮

### 3. 监控编译过程
- GitHub Actions会自动开始编译
- 编译过程大约需要5-10分钟
- 可以实时查看编译日志

### 4. 下载编译结果
编译成功后：
1. 在Actions页面找到成功的运行记录
2. 点击该记录进入详细页面
3. 在 **"Artifacts"** 部分找到 `poe2gamepad-firmware`
4. 点击下载按钮获取ZIP文件
5. 解压后包含 `poe2gamepad.uf2` 固件文件

## 故障排除

### 编译失败
1. **检查编译日志**：
   - 查看详细的错误信息
   - 重点关注CMake配置和头文件包含错误

2. **常见问题**：
   - **PIO-USB路径错误**：确保CMakeLists.txt第8行路径正确
   - **缺少头文件**：检查GitHub Actions中PICO_SDK_PATH设置
   - **编译定义缺失**：确保所有必要的-D选项都已设置

3. **重新触发编译**：
   - 如果修复了问题，重新push代码或手动触发工作流

### 固件下载问题
1. **找不到Artifacts**：
   - 确保编译成功完成（绿色勾号）
   - 等待几分钟让Artifacts生成

2. **下载失败**：
   - 尝试使用不同的浏览器
   - 检查网络连接

## 验证步骤

### 1. 编译验证
- 查看GitHub Actions日志，确认没有编译错误
- 确认所有目标文件（.uf2, .bin, .hex）已生成

### 2. 固件验证
1. 下载 `poe2gamepad.uf2` 文件
2. 按照烧录指南烧录到开发板
3. 观察LED状态指示
4. 连接手柄测试功能

## 自动化流程

每次向main分支推送代码时，GitHub Actions会自动：
1. 拉取最新代码和子模块
2. 安装ARM GCC交叉编译工具链
3. 下载并设置Pico SDK 2.2.0
4. 配置CMake构建系统
5. 编译固件
6. 生成可下载的UF2固件文件

## 技术支持

如果遇到问题，请提供：
1. GitHub Actions编译日志链接
2. 具体的错误信息
3. 你尝试的修复步骤

## 更新记录

### 2025-03-26
- 修复CMakeLists.txt中的PIO-USB路径
- 优化GitHub Actions配置
- 添加PIO-USB编译选项
- 创建详细的使用指南