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

#define WIDTH  40                 /* 地图宽度(格) */
#define HEIGHT 20                 /* 地图高度(格) */
#define MAX_LEN (WIDTH * HEIGHT)  /* 蛇身最大长度 */

/* 蛇身坐标数组, snake[0] 为蛇头 */
static int snakeX[MAX_LEN], snakeY[MAX_LEN];
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

/* 设置控制台: ①UTF-8 输出修复中文乱码 ②窗口/缓冲区尺寸贴合地图, 消除大片空白 */
static void setupConsole(void) {
    COORD bufSize;
    SMALL_RECT win;

    /* 源码是 UTF-8, 而 Windows 控制台默认 GBK(代码页 936), 需切换为 UTF-8 才不会乱码 */
    SetConsoleOutputCP(CP_UTF8);

    /* 缓冲区 = 窗口 = 地图(40x20) + 边框(2) + 状态栏与边距, 关闭滚动条 */
    bufSize.X = WIDTH + 4;
    bufSize.Y = HEIGHT + 6;
    win.Left = 0;
    win.Top = 0;
    win.Right = (SHORT)(WIDTH + 3);
    win.Bottom = (SHORT)(HEIGHT + 5);
    SetConsoleScreenBufferSize(hOut, bufSize);
    SetConsoleWindowInfo(hOut, TRUE, &win);
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
    printf("Score: %d    WASD/方向键移动  Space暂停  Q退出", score);
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
                case ' ': paused = !paused; break;
                case 'q': case 'Q': case 27: exit(0);
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

    /* 蛇身前移 */
    for (i = snakeLen - 1; i > 0; i--) {
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
        printf("Score: %d    WASD/方向键移动  Space暂停  Q退出", score);
    }

    /* 画新头(保持蛇身连贯) */
    gotoxy(nx, ny);
    putchar('@');
    gotoxy(snakeX[1], snakeY[1]);
    putchar('o');

    return 1;
}

int main(void) {
    srand((unsigned)time(NULL));
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    setupConsole();
    initGame();

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
    gotoxy(WIDTH / 2 - 9, HEIGHT / 2 + 1);
    printf("按 Q 键退出游戏...");

    /* 只有按 Q 键才退出, 避免误触其他按键直接跳过结算画面 */
    while (1) {
        if (kbhit() && (getch() == 'q' || getch() == 'Q')) break;
        Sleep(50);
    }
    return 0;
}
