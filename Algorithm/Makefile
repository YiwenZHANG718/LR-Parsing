# LR语法分析器 - Makefile
# 支持 LR(0)/SLR(1)/LR(1) 分析算法
# @author ZJJ
# @date 2025.6.11
# @version 2.0
# 使用方法：
#   make         - 编译所有目标（自动删除.o文件）
#   make keep-objects - 编译但保留.o文件（用于调试）
#   make clean   - 清理编译文件
#   make run     - 编译并运行交互式程序
#   make debug   - 编译调试版本
#   make test    - 运行基础测试
#   make help    - 显示帮助信息

# 编译配置
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -finput-charset=UTF-8 -fexec-charset=UTF-8

# 目标程序
TARGET = lr_analyzer
CLI_TARGET = lr_cli

# 源文件
SOURCES = main.cpp grammar.cpp lr_analyzer.cpp lr_item.cpp
CLI_SOURCES = lr_cli.cpp grammar.cpp lr_analyzer.cpp lr_item.cpp

# 目标文件
OBJECTS = $(SOURCES:.cpp=.o)
CLI_OBJECTS = $(CLI_SOURCES:.cpp=.o)

# 头文件
HEADERS = grammar.h lr_analyzer.h lr_item.h

# 编译规则
all: $(TARGET) $(CLI_TARGET) auto-clean

auto-clean:
	@echo "Cleaning up object files..."
	@rm -f $(OBJECTS) $(CLI_OBJECTS)
	@echo "Object files removed."

keep-objects: build-with-objects

build-with-objects: $(OBJECTS) $(CLI_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(filter main.o grammar.o lr_analyzer.o lr_item.o, $(OBJECTS))
	$(CXX) $(CXXFLAGS) -o $(CLI_TARGET) $(filter lr_cli.o grammar.o lr_analyzer.o lr_item.o, $(CLI_OBJECTS))
	@echo "Build successful! Executables: $(TARGET), $(CLI_TARGET) (objects kept)"

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)
	@echo "Build successful! Executable: $(TARGET)"

$(CLI_TARGET): $(CLI_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(CLI_TARGET) $(CLI_OBJECTS)
	@echo "Build successful! CLI Executable: $(CLI_TARGET)"

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理规则
clean:
	rm -f $(OBJECTS) $(CLI_OBJECTS) $(TARGET) $(CLI_TARGET)
	@echo "Clean completed."

# 运行和测试
run: $(TARGET)
	./$(TARGET)

debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

test: $(TARGET)
	@echo "Running basic test..."
	@echo "Testing with sample grammar..."
	@if [ -f GUI/example_grammar.txt ]; then \
		echo "GUI/example_grammar.txt" | ./$(TARGET); \
	else \
		echo "No test grammar file found."; \
	fi

install-deps:
	@echo "Checking build dependencies..."
	@which g++ > /dev/null || (echo "g++ not found. Please install build-essential."; exit 1)
	@echo "All dependencies are satisfied."

help:
	@echo "Available targets:"
	@echo "  all         - Build the LR analyzer (auto-clean .o files)"
	@echo "  keep-objects- Build but keep .o files (for debugging)"
	@echo "  clean       - Remove build files"
	@echo "  run         - Build and run the program"
	@echo "  debug       - Build with debug information"
	@echo "  test        - Run basic test"
	@echo "  install-deps- Install build dependencies"
	@echo "  help        - Show this help message"

.PHONY: all auto-clean keep-objects build-with-objects clean run debug test install-deps help
