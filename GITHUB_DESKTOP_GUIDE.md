# GitHub Desktop 使用指南

本文档详细介绍如何使用GitHub Desktop来更方便地管理和推送POE2sb项目代码。

## 安装和设置

### 1. 安装GitHub Desktop
如果你还没有安装，请从以下地址下载安装：
- 官网: https://desktop.github.com/
- 中文版: https://desktop.github.com/

### 2. 克隆仓库到GitHub Desktop
如果你已经在使用Git，可以这样操作：
1. 打开GitHub Desktop
2. 点击"File" → "Add Local Repository"
3. 选择项目目录: `c:\Users\dring\gamepad-poe2\POE2sb`
4. 点击"Add Repository"

或者直接从GitHub克隆：
1. 点击"File" → "Clone Repository"
2. 选择你的POE2sb仓库
3. 选择本地路径

## 自动化脚本使用

### build_and_push.bat
这是一个综合脚本，可以：
1. 自动构建固件
2. 创建Git提交
3. 指导使用GitHub Desktop推送

使用方法：
```cmd
cd c:\Users\dring\gamepad-poe2\POE2sb
build_and_push.bat
```

## GitHub Desktop 快捷操作

### 1. 快捷键
| 快捷键 | 功能 |
|--------|------|
| Ctrl+Shift+A | 添加所有更改 |
| Ctrl+Enter | 提交更改 |
| Ctrl+P | 推送到GitHub |
| Ctrl+Shift+H | 查看历史 |
| Ctrl+Shift+F | 搜索 |

### 2. 常规工作流程

#### 第1步：查看修改
1. 打开GitHub Desktop
2. 在左侧选择"POE2sb"仓库
3. 修改的文件会显示在"Changes"标签页

#### 第2步：提交修改
1. 在底部填写提交信息
2. 点击"Commit to main"
3. 或者使用快捷键 Ctrl+Enter

#### 第3步：推送到GitHub
1. 点击工具栏的"Push origin"按钮
2. 或者使用快捷键 Ctrl+P
3. 等待推送完成

## 常见问题解决

### 1. GitHub Desktop无法推送
**问题**: 推送失败，需要认证
**解决**:
1. 在GitHub Desktop中，点击"File" → "Options"
2. 选择"Accounts"标签页
3. 确保已登录GitHub账号
4. 或者使用GitHub Personal Access Token

### 2. 冲突处理
**问题**: 本地和远程有冲突
**解决**:
1. 在GitHub Desktop中点击"Fetch origin"
2. 如果有冲突，会显示"Pull"按钮
3. 点击"Pull"拉取远程更改
4. 解决冲突后重新提交

### 3. 分支管理
**创建新分支**:
1. 点击当前分支名称
2. 选择"New Branch"
3. 输入分支名，如 `feature-usb-fix`
4. 点击"Create Branch"

**切换分支**:
1. 点击当前分支名称
2. 从列表中选择要切换的分支

## 高级功能

### 1. 查看历史
点击"History"标签页可以查看：
- 所有提交记录
- 每个提交的详细修改
- 可以回滚到特定版本

### 2. 比较分支
1. 点击"Current Branch"按钮
2. 选择"Compare to branch..."
3. 选择要比较的分支

### 3. 创建Pull Request
1. 确保在功能分支上
2. 点击"Create Pull Request"
3. GitHub Desktop会打开浏览器到GitHub
4. 填写PR描述并创建

## 与命令行结合使用

### 同步使用
你可以在命令行和GitHub Desktop之间无缝切换：
```cmd
# 在命令行构建
build.bat

# 然后在GitHub Desktop中查看和提交
# GitHub Desktop会自动检测到更改
```

### 查看状态
```cmd
# 查看Git状态
git status

# 查看提交历史
git log --oneline --graph

# 查看远程仓库
git remote -v
```

## 推荐的Git配置

为了提高效率，建议配置：

### 1. 用户信息
```cmd
git config --global user.name "你的名字"
git config --global user.email "你的邮箱@example.com"
```

### 2. 别名（可选）
```cmd
git config --global alias.br branch
git config --global alias.co checkout
git config --global alias.ci commit
git config --global alias.st status
git config --global alias.lg "log --oneline --graph --all"
```

## 故障排除

### 1. GitHub Desktop显示空白
- 重启GitHub Desktop
- 检查网络连接
- 重新登录GitHub账号

### 2. 无法检测到仓库
- 确保仓库目录存在
- 检查是否有.git文件夹
- 尝试重新添加仓库

### 3. 推送慢
- 检查网络连接
- 尝试使用SSH而不是HTTPS
- 在GitHub Desktop设置中调整代理

## 联系方式

如有GitHub Desktop使用问题：
1. 查看GitHub Desktop帮助文档
2. 访问GitHub社区论坛
3. 或在本项目仓库中提交Issue

---
*最后更新: 2025-03-27*