# 贪吃蛇游戏


## 项目简介

这是一个基于 C++ 和 EasyX 图形库实现的 Windows 桌面贪吃蛇小游戏。项目在基础贪吃蛇规则之上加入难度选择、随机障碍物、多类型食物、分数统计和游戏结束后重新开始等功能。

## 功能特点

- 支持简单、普通、困难、专家四个难度等级；
- 支持 `W`、`A`、`S`、`D` 和方向键控制移动；
- 每次吃到食物后蛇身增长，并随机生成新的食物；
- 包含普通、加速、减速和加分四类食物；
- 随游戏进程生成障碍物；
- 撞墙、撞到自身或撞到障碍物时结束游戏；
- 游戏结束后按 `R` 返回难度选择页面并重新开始。

## 技术栈

- C++
- Windows API
- EasyX 图形库
- Visual Studio 2022（或兼容版本）

## 运行环境

- Windows 10/11；
- Visual Studio，安装“使用 C++ 的桌面开发”工作负载；
- EasyX 图形库（项目使用 `graphics.h`）。

## 运行方式

1. 安装与 Visual Studio 版本兼容的 EasyX 图形库。
2. 使用 Visual Studio 打开 `HungrySnakeGame/HungrySnakeGame.sln`。
3. 推荐选择 `x64` 和 `Debug` 配置。
4. 生成并运行项目。
5. 在难度选择界面按数字键 `1` 至 `4` 选择难度，按空格键开始游戏。

## 操作说明

| 操作 | 按键 |
| --- | --- |
| 向上移动 | `W` 或 `↑` |
| 向下移动 | `S` 或 `↓` |
| 向左移动 | `A` 或 `←` |
| 向右移动 | `D` 或 `→` |
| 选择难度 | `1` - `4` |
| 开始游戏 | 空格键 |
| 游戏结束后重新开始 | `R` |

## 项目结构

```text
Hungrysnake-game/
├─ README.md
└─ HungrySnakeGame/
   ├─ HungrySnakeGame.sln
   ├─ HungrySnakeGame.cpp  # 游戏主程序
   ├─ HungrySnakeGame.vcxproj
   └─ testx.cpp            # EasyX 图形测试文件，未加入主工程
```

## 说明

- 主工程目前只编译 `HungrySnakeGame.cpp`，因此 `testx.cpp` 不会参与主程序构建。
- 源码会尝试播放 `eat_food.wav` 和 `crash.wav`。当前项目未包含这两个音效文件，缺少时通常只会没有音效，不影响基本游戏逻辑。


## 后续改进方向

- 添加开始、暂停和退出按钮；
- 增加最高分保存与排行榜；
- 补齐可选音效资源并加入音量控制；
- 将全局状态进一步封装为游戏类，提高代码可维护性。
