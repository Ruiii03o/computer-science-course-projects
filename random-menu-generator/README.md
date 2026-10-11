# 随机菜谱生成器


## 项目简介

这是一个使用 Flask 开发的菜谱网站。用户可以浏览已有菜谱、随机选择菜谱、按标签筛选菜谱、提交自定义菜谱，并通过 DeepSeek API 根据关键词生成菜谱内容。

## 主要功能

- 首页展示菜谱列表；
- 随机推荐菜谱；
- 按标签筛选菜谱；
- 查看菜谱详情，包括食材、步骤、图片和视频；
- 提交自定义菜谱并上传图片或视频；
- 联系页面留言；
- 通过 DeepSeek API 按关键词生成菜谱。

## 技术栈

- Python 3
- Flask
- Jinja2 模板
- HTML / CSS / JavaScript
- Requests
- DeepSeek API（可选，用于 AI 菜谱生成）

## 项目结构

```text
random-menu-generator/
├─ README.md
└─ Recipe/
   ├─ app.py                 # Flask 应用入口
   ├─ config.py              # 本机配置，不上传
   ├─ config.example.py      # 配置示例，可上传
   ├─ requirements.txt       # Python 依赖
   ├─ data/                  # 菜谱数据
   ├─ static/                # 样式、图片和视频资源
   ├─ templates/             # HTML 模板
   └─ utils/                 # 菜谱查询与生成逻辑
```

## 本地运行

1. 进入项目源码目录：

   ```powershell
   cd Recipe
   ```

2. 建议创建并激活虚拟环境：

   ```powershell
   python -m venv .venv
   .\.venv\Scripts\Activate.ps1
   ```

3. 安装依赖：

   ```powershell
   pip install -r requirements.txt
   ```

4. 配置环境变量 `DEEPSEEK_API_KEY`。Windows PowerShell 示例：

   ```powershell
   $env:DEEPSEEK_API_KEY = "你的 DeepSeek API Key"
   ```

   不使用“AI 生成菜谱”功能时，可以跳过这一步。

5. 启动应用：

   ```powershell
   python app.py
   ```

6. 在浏览器打开 `http://127.0.0.1:5000`。


