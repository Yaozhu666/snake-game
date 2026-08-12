# 贪吃蛇 (Snake) —— 项目 Makefile
# 用法:
#   make         编译并生成 bin/snake.exe
#   make run     编译并运行
#   make clean   清理产物
#
# 依赖: MinGW-w64 GCC (gcc 需在 PATH 中)

CC      = gcc
CFLAGS  = -Wall -Wextra -O2
SRC     = src/snake.c
BIN     = bin/snake.exe

all: $(BIN)

$(BIN): $(SRC)
	@mkdir -p bin build
	$(CC) $(CFLAGS) -o $(BIN) $(SRC)

run: $(BIN)
	$(BIN)

clean:
	rm -f $(BIN) build/*.o

.PHONY: all run clean
