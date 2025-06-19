# LR语法分析器项目 - 完整实现

本项目是一个功能完整的LR语法分析器实现，支持LR(0)、SLR(1)和LR(1)三种分析方法。项目包含C++核心算法实现和Python图形化界面，为学习编译原理和语法分析提供了实用的工具。

## 技术特点

### 核心功能
- **多种LR分析方法**：完整实现LR(0)、SLR(1)、LR(1)算法
- **分析表构造**：自动生成ACTION表和GOTO表
- **语法分析**：支持输入串的语法分析和步骤追踪
- **项目集生成**：可视化LR项目集族的构造过程
- **跨平台支持**：Windows、Linux、macOS全平台兼容

### 技术架构
- **后端**：C++实现核心算法，高性能计算
- **前端**：Python Tkinter图形界面，用户友好
- **数据交换**：JSON格式数据传输，结构化清晰
- **多线程**：异步处理，避免界面冻结

## 项目结构

```
LR/                                   # 项目根目录
├── Algorithm/                        # 算法核心模块
│   ├── grammar.cpp                   # 文法表示、FIRST/FOLLOW集计算实现
│   ├── grammar.h                     # 文法相关类声明
│   ├── lr_item.cpp                   # LR项目和项目集管理实现
│   ├── lr_item.h                     # LR项目类声明
│   ├── lr_analyzer.cpp               # LR分析器核心实现（主要算法）
│   ├── lr_analyzer.h                 # LR分析器头文件（类声明、JSON输出接口）
│   ├── main.cpp                      # 交互式主程序入口
│   ├── lr_cli.cpp                    # 命令行接口程序（支持JSON输出）
│   ├── build.bat                     # Windows自动化构建脚本
│   ├── Makefile                      # 跨平台构建配置
│   ├── lr_analyzer(.exe)             # 交互式可执行文件
│   ├── lr_cli(.exe)                  # 命令行可执行文件
│   ├── BACKEND_CODE_OVERVIEW.md      # LR语法分析器后端代码架构总览
│   ├── LR_CLI_DOCUMENTATION.md       # 命令行工具完整使用文档
│
├── GUI/                              # 图形用户界面模块
│   ├── GUI.py                        # 主GUI程序
│   └── GUI_OVERVIEW.md               # GUI完整概览
│
├── TestGrammar/                      # 测试文法集合与自动化测试
│   ├── *_grammar.txt                 # 各种测试文法文件（表达式、条件语句、复杂语言等）
│   ├── *_complex_grammar.txt         # 复杂文法文件
│   ├── *_export.txt                  # 文法分析输出文件
│   ├── FaultGrammar/                 # 错误文法测试集合
│   │   └── *.txt                     # 错误类型测试
│   ├── test_all_grammars.sh          # Linux/macOS自动化测试脚本（支持多种测试模式）
│   ├── SCRIPT_DOCUMENTATION.md       # 测试脚本完整使用文档和最佳实践
│   ├── test_results.log              # 测试日志文件（运行后生成）
│   └── README.md                     # 测试用例说明文档
│
├── SOFTWARE_DIVISION_AND_VERSION_HISTORY.md  # 团队分工与版本历史
├── FUTURE_DEVELOPMENT.md             # 后续开发路线
└── README.md                         # 项目主要说明文档（本文件）
```

## 📚 完整文档导航

**核心算法与后端文档**:
- [LR语法分析器后端代码架构总览](Algorithm/BACKEND_CODE_OVERVIEW.md) - 核心算法实现详解
- [命令行工具完整使用文档](Algorithm/LR_CLI_DOCUMENTATION.md) - CLI工具详细使用指南

**图形界面文档**:
- [GUI完整概览](GUI/GUI_OVERVIEW.md) - 图形界面功能和使用说明

**测试框架与用例文档**:
- [测试用例说明](TestGrammar/README.md) - 测试文法集合和基本用法
- [自动化测试脚本完整使用文档](TestGrammar/SCRIPT_DOCUMENTATION.md) - **高级测试技巧和复杂文法最佳实践**

**项目管理文档**:
- [团队分工与版本历史详细说明](SOFTWARE_DIVISION_AND_VERSION_HISTORY.md) - 开发团队分工和版本迭代记录
- [后续开发路线](FUTURE_DEVELOPMENT.md) - 后续开发路线

💡 **快速开始建议**: 新用户建议先阅读本文档的"快速开始"部分，然后根据需要查阅上述专项文档。

## 快速开始

### 环境要求
- **C++编译器**：支持C++11标准（GCC 4.8+、MSVC 2015+、Clang 3.4+）
- **Python环境**：Python 3.6+，包含tkinter库
- **操作系统**：Windows 7+、Linux、macOS

### 编译安装

#### Windows (推荐使用MSYS2或MinGW)
```powershell
cd Algorithm
# 使用自动化构建脚本（推荐）
.\build.bat

# 或者手动编译
g++ -o lr_cli.exe lr_cli.cpp lr_analyzer.cpp grammar.cpp lr_item.cpp -std=c++17 -Wall -Wextra
g++ -o lr_analyzer.exe main.cpp lr_analyzer.cpp grammar.cpp lr_item.cpp -std=c++17 -Wall -Wextra
```

#### Linux/macOS
```bash
cd Algorithm
make

# 或者手动编译
g++ -o lr_cli lr_cli.cpp lr_analyzer.cpp grammar.cpp lr_item.cpp -std=c++17 -Wall -Wextra
g++ -o lr_analyzer main.cpp lr_analyzer.cpp grammar.cpp lr_item.cpp -std=c++17 -Wall -Wextra
```

## 快速测试

### 启动GUI
```bash
cd GUI
python3 GUI.py
```
#### 使用自动化测试脚本
```bash
# Linux/macOS - 完整测试脚本（推荐）
cd TestGrammar
chmod +x test_all_grammars.sh
./test_all_grammars.sh

# 不同测试模式：
./test_all_grammars.sh simple      # 快速测试（包含所有文法，包括复杂文法）
./test_all_grammars.sh all         # 完整测试（所有功能）
./test_all_grammars.sh stress      # 压力测试（复杂文法性能测试）
./test_all_grammars.sh comparison example_grammar.txt  # 算法对比测试

# 显示帮助信息
./test_all_grammars.sh --help
```

### 使用交互式版本
```bash
cd Algorithm
./lr_analyzer
# 然后按提示输入文法和待分析字符串
```

### 使用命令行版本
```bash
cd Algorithm
# 构造分析表
./lr_cli ../TestGrammar/example_grammar.txt -t slr1 --table

# 分析输入串
./lr_cli ../TestGrammar/example_grammar.txt -t slr1 -s "a + a * a"

# 显示项目集
./lr_cli ../TestGrammar/example_grammar.txt -t slr1 --items

# JSON格式输出
./lr_cli ../TestGrammar/example_grammar.txt -t slr1 --table --json
```

## 支持的算法

- **LR(0)** - 基础LR分析，适用于简单文法
- **SLR(1)** - 简单LR分析，使用FOLLOW集解决部分冲突
- **LR(1)** - 规范LR分析，最强大的LR分析算法

## 功能特性

### 核心算法功能
- ✅ **完整的LR分析算法实现** - LR(0)、SLR(1)、LR(1)三种算法
- ✅ **智能冲突检测** - 自动识别移进/规约和规约/规约冲突
- ✅ **详细的语法分析过程展示** - 步骤化显示状态栈、符号栈变化
- ✅ **ACTION表和GOTO表生成** - 完整的分析表构造和验证
- ✅ **LR项目集自动构造** - 闭包计算和GOTO函数实现
- ✅ **FIRST/FOLLOW集计算** - 支持ε产生式的完整算法

### 输出和接口功能  
- ✅ **多种输出格式** - 人类友好的文本格式 + 机器可读的JSON格式
- ✅ **完整中文图形界面** - 基于Tkinter的GUI，支持表格显示和交互
- ✅ **命令行批处理支持** - 适合脚本调用和自动化测试
- ✅ **实时错误报告** - 详细的错误位置和建议修复方案
- ✅ **结果导出功能** - 支持完整分析报告的文本和JSON导出

### 技术改进功能
- ✅ **字符编码兼容** - 解决了Unicode字符乱码问题
- ✅ **异步处理机制** - GUI界面非阻塞，提升用户体验  
- ✅ **静默模式支持** - JSON输出时屏蔽调试信息
- ✅ **多线程架构** - 后端计算与前端界面分离
- ✅ **智能错误处理** - 区分不同类型错误并提供针对性建议

### 自动化测试框架 
- ✅ **多种测试模式** - simple、all、comparison、stress四种测试策略
- ✅ **复杂文法支持** - 所有测试模式均支持复杂文法，包括简单测试模式
- ✅ **智能分析器选择** - 自动根据文法特征选择最适合的分析器
- ✅ **全面文法覆盖** - 包含基础文法、复杂文法、错误文法的完整测试集
- ✅ **性能压力测试** - 专门的复杂文法连续构造和极限测试
- ✅ **算法对比测试** - 三种分析器的性能和功能对比
- ✅ **详细测试报告** - 完整的测试日志和统计信息
- ✅ **高级测试技巧** - 支持输出过滤、超时控制、结果分析等高级功能

## 开发说明

本项目使用C++17标准开发，主要数据结构：

- `Grammar` - 文法表示
- `LRItem` - LR项目表示  
- `ItemSet` - 项目集表示
- `LRAnalyzer` - 分析器主类

核心算法实现：
- 项目集闭包计算
- GOTO函数实现
- FIRST/FOLLOW集计算
- 分析表构造
- 语法分析过程
