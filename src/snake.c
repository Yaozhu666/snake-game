/* ============================================================
 * 贪吃蛇小游戏 (Windows 控制台版, C 语言)
 * 编译: gcc snake.c -o snake.exe
 * 运行: snake.exe
 * 操作: WASD 或 方向键 控制移动, 空格 暂停, 按 Q 或 Esc 退出
 * ============================================================ */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

#define WIDTH  30                 /* 地图宽度(格): 控制台字符竖长(高≈2倍宽), 用 30x20 视觉更协调 */
#define HEIGHT 20                 /* 地图高度(格) */
#define MAX_LEN (WIDTH * HEIGHT)  /* 蛇身最大长度 */

/* 蛇身坐标数组, snake[0] 为蛇头; +1 冗余槽: 吃食物前移时需写入 snake[snakeLen] */
static int snakeX[MAX_LEN + 1], snakeY[MAX_LEN + 1];
static int snakeLen = 3;
static int dirX = 1, dirY = 0;    /* 当前方向 */
static int foodX, foodY;
static int score = 0;
static int speed = 130;           /* 帧间隔毫秒, 随长度加快 */
static int paused = 0;

static HANDLE hOut;

/* 移动光标到 (x, y) */
static void gotoxy(int x, int y) {
    COORD c = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(hOut, c);
}

/* 隐藏光标, 避免闪烁 */
static void hideCursor(void) {
    CONSOLE_CURSOR_INFO ci;
    ci.dwSize = 1;
    ci.bVisible = FALSE;
    SetConsoleCursorInfo(hOut, &ci);
}

/* 设置控制台: ①UTF-8 输出修复中文乱码 ②尽力把窗口/缓冲区调成贴合地图(CMD/PowerShell 生效, Git Bash 等终端无效也无妨) */
static void setupConsole(void) {
    COORD bufSize, maxBuf;
    SMALL_RECT win;
    HWND hwnd;

    /* 源码是 UTF-8, 而 Windows 控制台默认 GBK(代码页 936), 需切换为 UTF-8 才不会乱码 */
    SetConsoleOutputCP(CP_UTF8);

    /* 目标: 缓冲区 = 窗口 = 地图(30x20) + 边框(2) + 状态栏与边距 */
    bufSize.X = WIDTH + 4;   /* 34 */
    bufSize.Y = HEIGHT + 6;  /* 26 */
    win.Left = 0;
    win.Top = 0;
    win.Right = (SHORT)(WIDTH + 3);   /* 33 */
    win.Bottom = (SHORT)(HEIGHT + 5); /* 25 */

    /* 窗口若处于最大化状态, 尺寸设置不会生效, 先还原 */
    hwnd = GetConsoleWindow();
    if (hwnd) ShowWindow(hwnd, SW_RESTORE);

    /* 顺序很关键: ①先把缓冲区扩到当前字体允许的最大值,
       ②再设置窗口尺寸, ③最后把缓冲区收缩到与窗口一致(消除滚动条)。
       在 Git Bash(mintty)等终端中这些 API 不生效, 但调用无害, 游戏照常可玩 */
    maxBuf = GetLargestConsoleWindowSize(hOut);
    SetConsoleScreenBufferSize(hOut, maxBuf);
    SetConsoleWindowInfo(hOut, TRUE, &win);
    SetConsoleScreenBufferSize(hOut, bufSize);
}

/* 校验窗口: 仅当窗口小到地图显示不全时才提示; 窗口更大(留白)则不打扰,
   兼容 CMD / PowerShell / Git Bash 等不同终端 */
static void checkWindow(void) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int w, h;
    if (!GetConsoleScreenBufferInfo(hOut, &csbi)) return;
    w = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    h = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    if (w >= WIDTH + 2 && h >= HEIGHT + 4) return;  /* 能完整放下地图+边框+状态栏即可 */

    gotoxy(0, HEIGHT + 4);
    printf("窗口过小(当前 %dx%d), 地图显示不全", w, h);
    gotoxy(0, HEIGHT + 5);
    printf("提示: 调大窗口或调小终端字体后再开始");
}

/* 绘制游戏边框 */
static void drawWall(void) {
    int i;
    gotoxy(0, 0);
    for (i = 0; i < WIDTH + 2; i++) putchar('#');
    for (i = 1; i <= HEIGHT; i++) {
        gotoxy(0, i);
        putchar('#');
        gotoxy(WIDTH + 1, i);
        putchar('#');
    }
    gotoxy(0, HEIGHT + 1);
    for (i = 0; i < WIDTH + 2; i++) putchar('#');
}

/* 在随机空位生成食物 */
static void spawnFood(void) {
    int i, ok;
    do {
        foodX = rand() % WIDTH + 1;
        foodY = rand() % HEIGHT + 1;
        ok = 1;
        for (i = 0; i < snakeLen; i++) {
            if (snakeX[i] == foodX && snakeY[i] == foodY) { ok = 0; break; }
        }
    } while (!ok);
    gotoxy(foodX, foodY);
    putchar('*');
}

/* 初始化: 蛇在中间, 朝右 */
static void initGame(void) {
    int i, cx, cy;
    snakeLen = 3;          /* 每局都从 3 节开始 */
    cx = WIDTH / 2;
    cy = HEIGHT / 2;
    for (i = 0; i < snakeLen; i++) {
        snakeX[i] = cx - i;
        snakeY[i] = cy;
    }
    dirX = 1; dirY = 0;
    score = 0;
    speed = 130;
    paused = 0;
    system("cls");
    hideCursor();
    drawWall();
    for (i = 0; i < snakeLen; i++) {
        gotoxy(snakeX[i], snakeY[i]);
        putchar(i == 0 ? '@' : 'o');
    }
    spawnFood();
    gotoxy(0, HEIGHT + 3);
    printf("Score: %d    WASD/方向键移动  Space/Q暂停  R重开", score);
}

/* 读取键盘输入(兼容方向键两字节扫描码) */
static void handleInput(void) {
    int c;
    while (kbhit()) {
        c = getch();
        if (c == 0 || c == 224) {           /* 方向键: 第一字节 0/224 */
            c = getch();
            switch (c) {
                case 72: if (!dirY) { dirX = 0;  dirY = -1; } break;  /* 上 */
                case 80: if (!dirY) { dirX = 0;  dirY = 1;  } break;  /* 下 */
                case 75: if (!dirX) { dirX = -1; dirY = 0;  } break;  /* 左 */
                case 77: if (!dirX) { dirX = 1;  dirY = 0;  } break;  /* 右 */
            }
        } else {
            switch (c) {
                case 'w': case 'W': if (!dirY) { dirX = 0;  dirY = -1; } break;
                case 's': case 'S': if (!dirY) { dirX = 0;  dirY = 1;  } break;
                case 'a': case 'A': if (!dirX) { dirX = -1; dirY = 0;  } break;
                case 'd': case 'D': if (!dirX) { dirX = 1;  dirY = 0;  } break;
                case ' ': case 'q': case 'Q': case 27: paused = !paused; break;
            }
        }
    }
}

/* 返回 1 存活, 0 死亡 */
static int step(void) {
    int nx, ny, i;

    if (paused) return 1;

    nx = snakeX[0] + dirX;
    ny = snakeY[0] + dirY;

    /* 撞墙 */
    if (nx < 1 || nx > WIDTH || ny < 1 || ny > HEIGHT) return 0;
    /* 撞到自己(新头不与旧蛇身重叠, 尾部即将移开时不算) */
    for (i = 0; i < snakeLen; i++) {
        if (nx == snakeX[i] && ny == snakeY[i]) return 0;
    }

    /* 只有没吃到食物时才擦掉旧尾巴(吃到时旧尾保留, 蛇身变长, 否则会缺一块) */
    if (!(nx == foodX && ny == foodY)) {
        gotoxy(snakeX[snakeLen - 1], snakeY[snakeLen - 1]);
        putchar(' ');
    }

    /* 蛇身前移: 从 snakeLen 开始整体移位, 吃食物时旧尾自动保留为新尾(存入冗余槽),
       避免数组尾部残留未初始化坐标导致误擦地图 */
    for (i = snakeLen; i > 0; i--) {
        snakeX[i] = snakeX[i - 1];
        snakeY[i] = snakeY[i - 1];
    }
    snakeX[0] = nx;
    snakeY[0] = ny;

    /* 吃到食物: 长度+1 */
    if (nx == foodX && ny == foodY) {
        snakeLen++;
        score += 10;
        spawnFood();
        if (snakeLen > 5 && speed > 50) speed -= 5;   /* 越长越快 */
        gotoxy(0, HEIGHT + 3);
        printf("Score: %d    WASD/方向键移动  Space/Q暂停  R重开", score);
    }

    /* 画新头(保持蛇身连贯) */
    gotoxy(nx, ny);
    putchar('@');
    gotoxy(snakeX[1], snakeY[1]);
    putchar('o');

    return 1;
}

int main(void) {
    int again = 1;
    srand((unsigned)time(NULL));
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    setupConsole();

    while (again) {
        initGame();
        checkWindow();

        while (1) {
            handleInput();
            if (!step()) break;              /* 撞墙或撞到自己 */
            if (snakeLen >= MAX_LEN) break;  /* 蛇充满屏幕, 胜利 */
            Sleep(speed);
        }

        /* 结算画面: 居中显示, 分数清晰可见 */
        gotoxy(WIDTH / 2 - 5, HEIGHT / 2 - 1);
        printf("Game Over!");
        gotoxy(WIDTH / 2 - 8, HEIGHT / 2);
        printf("最终得分: %d", score);
        gotoxy(WIDTH / 2 - 11, HEIGHT / 2 + 1);
        printf("按 R 开始新一局, 按 Q 退出游戏");

        /* 只有按 R/Q 才响应, 避免误触其他按键直接跳过结算画面 */
        while (1) {
            if (kbhit()) {
                int c = getch();
                if (c == 'r' || c == 'R') break;                       /* 再来一局 */
                if (c == 'q' || c == 'Q') { again = 0; break; }        /* 退出游戏 */
            }
            Sleep(50);
        }
    }
    return 0;
}
