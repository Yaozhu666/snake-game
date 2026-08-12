# project 001 · 贪吃蛇

一个用 C 语言编写的 Windows 控制台贪吃蛇小游戏。

## 目录结构

```
project 001贪吃蛇/
├── Makefile          # 构建脚本(编译 / 运行 / 清理)
├── README.md         # 项目说明
├── src/              # 源代码
│   └── snake.c       # 贪吃蛇主程序(单文件实现)
├── include/          # 头文件目录(本项目为单文件,暂未使用;拆分为多模块时 .h 放这里)
├── build/            # 构建中间产物目录(如 .o 文件)
├── bin/              # 编译产物目录(可执行文件)
└── docs/             # 文档目录(设计说明、使用说明等)
```

## 编译与运行

```bash
make          # 编译,生成 bin/snake.exe
make run      # 编译并运行
make clean    # 清理产物
```

也可以手动编译:

```bash
gcc -Wall -Wextra -O2 -o bin/snake.exe src/snake.c
```

## 操作方式

| 按键 | 功能 |
|---|---|
| WASD / 方向键 | 控制蛇头移动 |
| 空格 Space | 暂停 / 继续 |
| Q 或 Esc | 退出游戏 |

## 游戏规则

- 蛇吃到食物 `*` 增长一节,得分 +10;
- 撞到墙壁(`#`)或撞到自己即游戏结束;
- 蛇越长移动速度越快(帧间隔从 130ms 递减至 50ms);
- 蛇身填满整个场地即获胜。

## 更新日志

每次代码修改后均需同步更新 [CHANGELOG.md](CHANGELOG.md),
记录新增功能与修复的 bug。

