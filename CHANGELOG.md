# 更新日志 (Changelog)

本文件记录项目的所有重要变更,遵循 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.0.0/)
格式,版本号遵循 [语义化版本](https://semver.org/lang/zh-CN/)。

## [v0.2.0] - 2026-08-12

### 修复
- 修复控制台中文乱码:源码为 UTF-8,而 Windows 控制台默认 GBK(代码页 936),
  新增 `setupConsole()` 调用 `SetConsoleOutputCP(CP_UTF8)` 切换输出代码页。
- 修复游戏画面只占窗口一角、四周大片空白:在 `setupConsole()` 中用
  `SetConsoleScreenBufferSize` + `SetConsoleWindowInfo` 将窗口/缓冲区尺寸
  调整为与地图匹配(44×26),并关闭滚动条。
- 修复贴墙吃食物后墙面缺一块:`step()` 原先无条件擦除蛇尾,吃到食物时
  蛇身变长、旧尾巴本应保留,却被误打为空格;现改为仅在未吃到食物时擦尾。

### 变更
- `main()` 中新增 `setupConsole()` 调用。

## [v0.1.0] - 2026-08-12

### 新增
- Windows 控制台版贪吃蛇小游戏(单文件 C 实现,依赖 `windows.h` / `conio.h`)。
- 核心玩法:WASD 或方向键移动、空格暂停、Q/Esc 退出、吃食物增长并加分、
  撞墙或撞到自己游戏结束,蛇越长速度越快。
- 标准 C 项目目录结构:src / include / build / bin / docs,附 Makefile。
- 文档:README.md、docs/设计说明.md。
