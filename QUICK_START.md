# POE2sb 快速开始指南

## 使用GitHub Desktop推送代码

### 方法一：一键构建和推送（推荐）
1. 双击运行 `build_and_push.bat`
2. 脚本会自动：
   - 构建固件
   - 创建Git提交
   - 指导使用GitHub Desktop

### 方法二：PowerShell快速操作
1. 右键点击 `quick_push.ps1` → "用PowerShell运行"
2. 按照提示操作
3. 选择打开GitHub Desktop

### 方法三：手动步骤
1. 构建固件：运行 `build.bat`
2. 打开GitHub Desktop
3. 查看更改 → 提交 → 推送

## GitHub Desktop 基本操作

### 1. 打开GitHub Desktop
- 从开始菜单打开
- 或者运行 `github-desktop` 命令

### 2. 查看更改
- 左侧选择"POE2sb"仓库
- 修改的文件显示在"Changes"标签页

### 3. 提交更改
1. 填写提交信息
2. 点击"Commit to main"
3. 或者按 `Ctrl+Enter`

### 4. 推送到GitHub
1. 点击工具栏的"Push origin"按钮
2. 或者按 `Ctrl+P`

## 常见问题

### Q: GitHub Desktop找不到仓库
A: 点击"File" → "Add Local Repository" → 选择项目目录

### Q: 推送需要认证
A: 在GitHub Desktop中登录账号，或使用Personal Access Token

### Q: 如何查看推送历史
A: 点击"History"标签页查看所有提交

## 快捷方式

| 操作 | 命令/快捷键 |
|------|------------|
| 构建固件 | `build.bat` |
| 构建并准备推送 | `build_and_push.bat` |
| 快速推送 | `quick_push.ps1` |
| GitHub Desktop提交 | `Ctrl+Enter` |
| GitHub Desktop推送 | `Ctrl+P` |

## 下一步

1. ✅ 安装GitHub Desktop
2. ✅ 配置项目
3. ⚡ 构建固件
4. ⚡ 烧录到设备
5. ⚡ 测试功能