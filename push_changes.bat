@echo off
echo === Pushing Changes to GitHub ===
echo Date: %date% %time%
echo.

echo Checking Git status...
git status

echo.
echo Adding changed files...
git add usb_descriptors.c README.md

echo.
echo Committing changes...
git commit -m "修复USB设备描述符以让Windows正确识别为游戏手柄

1. 修改接口协议bInterfaceProtocol从0x00改为0x01
2. 更新HID报告描述符为更标准的XInput格式（66字节）
3. 更新配置描述符总长度从59字节到69字节
4. 移除重复的send_xinput_report函数
5. 清理README.md中的过时参考

这些修改应该解决joy.cpl找不到手柄的问题，让Windows正确识别设备为游戏手柄。"

echo.
echo Pushing to GitHub...
git push origin main

echo.
echo === Push Completed ===
echo.
echo If push fails due to authentication, you may need to:
echo 1. Use GitHub Personal Access Token
echo 2. Use SSH key
echo 3. Or use Git credential manager
echo.
pause