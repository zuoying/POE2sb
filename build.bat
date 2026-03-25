@echo off
REM POE2sb Gamepad Synchronizer Windows构建脚本
REM 使用: build.bat

echo === POE2sb Gamepad Synchronizer Build Script ===
echo Build Date: %date% %time%
echo.

REM 检查构建目录
if not exist "build\" (
    echo Creating build directory...
    mkdir build
)

cd build

REM 清理之前的构建
echo Cleaning previous build...
if exist "CMakeCache.txt" del CMakeCache.txt
if exist "CMakeFiles\" rmdir /s /q CMakeFiles
if exist "*.uf2" del *.uf2
if exist "*.bin" del *.bin
if exist "*.hex" del *.hex

REM 配置项目
echo Configuring project...
cmake ..

REM 检查CMake是否成功
if %errorlevel% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b %errorlevel%
)

REM 编译固件
echo Compiling firmware...
make -j%NUMBER_OF_PROCESSORS%

REM 检查编译是否成功
if %errorlevel% neq 0 (
    echo Compilation failed!
    pause
    exit /b %errorlevel%
)

echo.
echo === Build Completed Successfully ===
echo Output files:
echo   - poe2gamepad.uf2: 主固件文件（用于烧录）
echo   - poe2gamepad.bin: 二进制格式
echo   - poe2gamepad.hex: Intel Hex格式
echo.
echo To flash the firmware:
echo   1. Hold BOOTSEL button on RP2350 board
echo   2. Connect USB-C to computer
echo   3. Release BOOTSEL button
echo   4. Copy poe2gamepad.uf2 to RPI-RP2 drive
echo   5. Wait for automatic reboot
echo.
echo To test the firmware:
echo   - Connect GameSir controller to USB-A port
echo   - Connect RP2350 USB-C to PC
echo   - Check LED status and serial output
echo.

pause