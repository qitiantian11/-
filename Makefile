# 编译器定义
CC = gcc
# 编译参数：启用C99标准、开启所有警告、生成调试信息（用于gdb调试）
CFLAGS = -std=c99 -Wall -g
# 头文件搜索路径
INCLUDES = -I include

# 目录定义
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# 自动收集所有 .c 源文件
SRCS = $(wildcard $(SRC_DIR)/*.c)
# 把 .c 文件路径替换为 .o 目标文件路径
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
# 最终可执行文件名
TARGET = $(BIN_DIR)/library_ms

# 默认目标：编译整个项目
all: $(TARGET)

# 链接生成可执行文件
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@
	@echo "编译完成！可执行文件：$(TARGET)"

# 编译每个 .c 文件为 .o 目标文件
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 清理编译产物
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "清理完成"

# 编译并直接运行程序
run: all
	./$(TARGET)

.PHONY: all clean run
