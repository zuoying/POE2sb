@echo off
REM ================================================
REM POE2sb Gamepad Synchronizer - 构建与推送脚本
REM 使用GitHub Desktop进行便捷的代码推送
REM ================================================

echo === POE2sb Gamepad Synchronizer - 构建与推送 ===
echo Build Date: %date% %time%
echo.

setlocal enabledelayedexpansion

REM 设置颜色
for /F "tokens=1,2 delims=#" %%a in ('"prompt #$H#$E# & echo on & for %%b in (1) do rem"') do (
  set "DEL=%%a"
)

echo %DEL%[92m[STEP 1] 检查Git状态...%DEL%[0m
git status
echo.

echo %DEL%[92m[STEP 2] 构建固件...%DEL%[0m

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

if !errorlevel! neq 0 (
    echo %DEL%[91mCMake configuration failed!%DEL%[0m
    pause
    exit /b !errorlevel!
)

REM 编译固件
echo Compiling firmware...
make -j%NUMBER_OF_PROCESSORS%

if !errorlevel! neq 0 (
    echo %DEL%[91mCompilation failed!%DEL%[0m
    pause
    exit /b !errorlevel!
)

echo.
echo %DEL%[92m[STEP 3] 构建成功！检查输出文件...%DEL%[0m
dir *.uf2 *.bin *.hex

echo.
echo %DEL%[92m[STEP 4] 准备Git提交...%DEL%[0m
cd ..

REM 添加所有修改的文件
echo Adding changed files...
git add .

REM 显示将要提交的文件
echo.
echo Files to be committed:
git status --short

echo.
echo %DEL%[92m[STEP 5] 创建提交...%DEL%[0m
set "commit_msg=更新固件构建和修复USB主机时钟问题

1. 修复系统时钟从120MHz改为240MHz（PIO-USB要求）
2. 简化电源管理函数
3. 简化GPIO6配置
4. 更新构建脚本
5. 添加GitHub Desktop推送支持

这些修改应该解决USB主机无法识别手柄的问题。"

git commit -m "%commit_msg%"

if !errorlevel! neq 0 (
    echo %DEL%[91mCommit failed!%DEL%[0m
    echo You may need to set user.email and user.name:
    echo   git config --global user.email "your-email@example.com"
    echo   git config --global user.name "Your Name"
    pause
    exit /b !errorlevel!
)

echo.
echo %DEL%[92m[STEP 6] 打开GitHub Desktop...%DEL%[0m
echo 请执行以下操作：
echo 1. 打开GitHub Desktop
echo 2. 选择POE2sb仓库
echo 3. 点击"Push origin"按钮
echo 4. 或者使用快捷键 Ctrl+P
echo.

REM 尝试打开GitHub Desktop
where github-desktop >nul 2>nul
if !errorlevel! equ 0 (
    echo 正在启动GitHub Desktop...
    start "" "github-desktop://openRepo/c:\Users\dring\gamepad-poe2\POE2sb"
) else (
    echo GitHub Desktop未找到，请手动打开
)

echo.
echo %DEL%[94m[INFO] 可选的手动推送方式：%DEL%[0m
echo 1. 使用GitHub Desktop: 打开软件 -> 选择仓库 -> 点击推送
echo 2. 使用命令行: git push origin main
echo 3. 使用HTTPS token: git push https://<token>@github.com/username/repo.git main
echo.

echo %DEL%[92m[STEP 7] 烧录固件到设备...%DEL%[0m
echo.
echo 烧录步骤：
echo   1. 按住RP2350板上的BOOTSEL按钮
echo   2. 将USB-C线连接到电脑
echo   3. 松开BOOTSEL按钮
echo   4. 将build\poe2gamepad.uf2文件复制到RPI-RP2驱动器
echo   5. 等待设备自动重启
echo.
echo 测试步骤：
echo   - 将GameSir手柄连接到USB-A端口
echo   - 将RP2350 USB-C连接到PC
echo   - 检查LED状态和串口输出（115200波特率）
echo.

echo %DEL%[92m=== 构建与推送准备完成 ===%DEL%[0m
echo.
echo 总结：
echo   ✅ 固件已构建：build\poe2gamepad.uf2
echo   ✅ Git提交已创建
echo   ⚠️  请使用GitHub Desktop完成推送
echo   ⚠️  请手动烧录固件到设备
echo.

pause