# 答案之书

> Python GUI 课程练习项目

## 项目简介

“答案之书”是一个基于 Python 和 Tkinter 开发的桌面互动程序。用户输入问题后，程序通过抽签动画随机给出回答；针对不同类型的回答，界面会展示烟花或电子木鱼等动画效果。

## 功能特点

- 输入问题并随机获取回答；
- 抽签筒摇晃、抽签和粒子动画；
- 积极回答触发烟花特效；
- 消极回答触发电子木鱼与祈福动画；
- 支持窗口缩放与中文界面显示；
- 缺少音效文件时自动静音运行，不影响核心功能。

## 技术栈

- Python 3
- Tkinter（Python 标准库）
- Pygame（音效播放）

## 运行方式

1. 建议创建并激活虚拟环境：

   ```powershell
   python -m venv .venv
   .\.venv\Scripts\Activate.ps1
   ```

2. 安装依赖：

   ```powershell
   pip install -r requirements.txt
   ```

3. 启动程序：

   ```powershell
   python AnswerBook.py
   ```

## 可选音效

程序会在运行目录中尝试加载以下音效文件：

```text
shake.wav
draw.wav
reveal.wav
wooden_fish.wav
firework.wav
```

这些文件不是必需的。未提供时，程序会跳过音效播放并继续正常运行。

## 项目结构

```text
answer-book/
├─ AnswerBook.py      # 主程序
├─ requirements.txt   # Python 依赖
└─ README.md          # 项目说明
```

## 说明

本项目用于 GUI、事件处理、动画和随机逻辑的编程练习，回答内容仅供娱乐。
