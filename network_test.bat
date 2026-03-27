@echo off
REM ================================================
REM GitHub推送网络测试工具
REM ================================================

echo === GitHub推送网络诊断 ===
echo 测试时间: %date% %time%
echo.

echo [1/6] 测试网络连通性...
ping github.com -n 4

if %errorlevel% neq 0 (
    echo ❌ 无法连接到github.com
    echo 请检查网络连接和DNS设置
    goto :end
)

echo.
echo [2/6] 测试HTTPS连接...
curl -I https://github.com --connect-timeout 10

if %errorlevel% neq 0 (
    echo ⚠️  HTTPS连接可能有问题
) else (
    echo ✅ HTTPS连接正常
)

echo.
echo [3/6] 检查Git配置...
git remote -v
echo.
git config --local --list | findstr "user\|remote"

echo.
echo [4/6] 测试Git推送（dry-run）...
git push --dry-run origin main

if %errorlevel% neq 0 (
    echo.
echo ❌ 推送测试失败
    echo 可能的解决方案：
    echo   1. 使用GitHub Desktop
    echo   2. 切换SSH方式
    echo   3. 检查防火墙/代理
) else (
    echo.
echo ✅ 推送测试成功
)

echo.
echo [5/6] 推荐的解决方案：
echo.
echo 📱 方案一：使用GitHub Desktop
echo   1. 打开GitHub Desktop
echo   2. 选择仓库: C:\Users\dring\Documents\GitHub\POE2sb
echo   3. 提交和推送
echo.
echo 🔐 方案二：使用SSH方式
echo   1. 生成SSH密钥: ssh-keygen -t ed25519 -C "your-email@example.com"
echo   2. 添加到GitHub: cat ~/.ssh/id_ed25519.pub
echo   3. 修改远程仓库: git remote set-url origin git@github.com:zuoying/POE2sb.git
echo.
echo 🌐 方案三：检查网络
echo   1. 重启路由器
echo   2. 检查防火墙设置
echo   3. 尝试使用手机热点
echo.
:end
echo [6/6] 当前工作目录状态
git status
echo.
echo === 诊断完成 ===
echo 建议优先使用GitHub Desktop进行推送
pause