#!/bin/bash
# POE2sb Gamepad Synchronizer 构建脚本

set -e  # 遇到错误时退出

echo "=== POE2sb Gamepad Synchronizer Build Script ==="
echo "Build Date: $(date)"
echo ""

# 检查构建目录
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

cd build

# 清理之前的构建
echo "Cleaning previous build..."
rm -rf *

# 配置项目
echo "Configuring project..."
cmake ..

# 编译固件
echo "Compiling firmware..."
make -j$(nproc)

echo ""
echo "=== Build Completed Successfully ==="
echo "Output files:"
echo "  - poe2gamepad.uf2: 主固件文件（用于烧录）"
echo "  - poe2gamepad.bin: 二进制格式"
echo "  - poe2gamepad.hex: Intel Hex格式"
echo ""
echo "To flash the firmware:"
echo "  1. Hold BOOTSEL button on RP2350 board"
echo "  2. Connect USB-C to computer"
echo "  3. Release BOOTSEL button"
echo "  4. Copy poe2gamepad.uf2 to RPI-RP2 drive"
echo "  5. Wait for automatic reboot"
echo ""
echo "To test the firmware:"
echo "  - Connect GameSir controller to USB-A port"
echo "  - Connect RP2350 USB-C to PC"
echo "  - Check LED status and serial output"
echo ""