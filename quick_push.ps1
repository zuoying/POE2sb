# POE2sb 快速推送脚本 (PowerShell)
# 使用方法: 右键点击 -> "用PowerShell运行"

Write-Host "=== POE2sb 快速推送工具 ===" -ForegroundColor Green
Write-Host "当前时间: $(Get-Date)" 
Write-Host ""

# 检查Git状态
Write-Host "[1/5] 检查Git状态..." -ForegroundColor Yellow
$gitStatus = git status
Write-Host $gitStatus -ForegroundColor Gray

# 检查是否有未提交的更改
$hasChanges = $gitStatus -match "Changes not staged for commit|Untracked files"
if ($hasChanges) {
    Write-Host "检测到未提交的更改" -ForegroundColor Cyan
    
    # 询问是否要提交
    $answer = Read-Host "是否要提交这些更改? (y/n)"
    if ($answer -eq 'y' -or $answer -eq 'Y') {
        # 添加所有更改
        Write-Host "[2/5] 添加所有更改..." -ForegroundColor Yellow
        git add .
        
        # 创建提交
        Write-Host "[3/5] 创建提交..." -ForegroundColor Yellow
        $commitMsg = "自动提交: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
        
        - 修复USB主机时钟问题
        - 更新构建脚本
        - 添加GitHub Desktop支持"
        
        git commit -m $commitMsg
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "✅ 提交成功!" -ForegroundColor Green
        } else {
            Write-Host "❌ 提交失败，请检查Git配置" -ForegroundColor Red
        }
    }
} else {
    Write-Host "没有需要提交的更改" -ForegroundColor Green
}

# 询问是否要打开GitHub Desktop
Write-Host ""
Write-Host "[4/5] GitHub Desktop..." -ForegroundColor Yellow
Write-Host "请选择操作:"
Write-Host "  1. 打开GitHub Desktop"
Write-Host "  2. 使用命令行推送"
Write-Host "  3. 仅查看状态"
Write-Host ""

$choice = Read-Host "请输入选项 (1-3)"

switch ($choice) {
    "1" {
        # 尝试打开GitHub Desktop
        Write-Host "正在打开GitHub Desktop..." -ForegroundColor Cyan
        
        # 尝试多种方式打开GitHub Desktop
        $paths = @(
            "$env:LOCALAPPDATA\GitHubDesktop\GitHubDesktop.exe",
            "C:\Program Files\GitHub Desktop\GitHubDesktop.exe",
            "$env:ProgramW6432\GitHub Desktop\GitHubDesktop.exe"
        )
        
        $opened = $false
        foreach ($path in $paths) {
            if (Test-Path $path) {
                try {
                    Start-Process $path -ArgumentList "--open-shell c:\Users\dring\gamepad-poe2\POE2sb"
                    Write-Host "✅ GitHub Desktop已打开" -ForegroundColor Green
                    $opened = $true
                    break
                } catch {
                    Write-Host "⚠️  无法打开: $path" -ForegroundColor Yellow
                }
            }
        }
        
        if (-not $opened) {
            Write-Host "❌ 未找到GitHub Desktop，请手动打开" -ForegroundColor Red
        }
    }
    
    "2" {
        # 使用命令行推送
        Write-Host "[5/5] 推送到GitHub..." -ForegroundColor Yellow
        
        $pushChoice = Read-Host "使用HTTPS还是SSH? (h/s)"
        if ($pushChoice -eq 'h' -or $pushChoice -eq 'H') {
            # HTTPS推送
            git push origin main
        } else {
            # SSH推送
            git push git@github.com:username/POE2sb.git main
        }
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "✅ 推送成功!" -ForegroundColor Green
        } else {
            Write-Host "❌ 推送失败，请检查网络或认证" -ForegroundColor Red
        }
    }
    
    "3" {
        Write-Host "当前状态:" -ForegroundColor Cyan
        git log --oneline -5
    }
}

# 完成
Write-Host ""
Write-Host "=== 操作完成 ===" -ForegroundColor Green
Write-Host ""
Write-Host "后续步骤:"
Write-Host "  1. 构建固件: 运行 build.bat"
Write-Host "  2. 烧录固件: 复制 build\poe2gamepad.uf2 到设备"
Write-Host "  3. 测试: 连接手柄并检查LED状态"
Write-Host ""
Write-Host "按任意键继续..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")