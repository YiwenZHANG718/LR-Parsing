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
LR/                                    # 项目根目录
├── Algorithm/                         # 算法核心模块
│   ├── lr_analyzer.cpp                # LR分析器核心实现（主要算法）
│   ├── lr_analyzer.h                  # LR分析器头文件（类声明、JSON输出接口）
│   ├── lr_item.cpp                    # LR项目和项目集管理实现
│   ├── lr_item.h                      # LR项目类声明
│   ├── grammar.cpp                    # 文法表示、FIRST/FOLLOW集计算实现
│   ├── grammar.h                      # 文法相关类声明
│   ├── main.cpp                       # 交互式主程序入口
│   ├── lr_cli.cpp                     # 命令行接口程序（支持JSON输出）
│   ├── build.bat                      # Windows自动化构建脚本（UTF-8支持）
│   ├── Makefile                       # 跨平台构建配置
│   ├── lr_analyzer(.exe)              # 交互式可执行文件
│   ├── lr_cli(.exe)                   # 命令行可执行文件
│   ├── ALGORITHM_DETAILS.md           # LR算法详细技术说明
│   ├── CODE_ARCHITECTURE.md           # 代码架构和模块间关系文档
│   ├── LR_CLI_DOCUMENTATION.md        # 命令行工具完整使用文档
│   └── LR1_DEVELOPMENT_ISSUES.md      # LR(1)开发问题与解决方案记录
│
├── GUI/                              # 图形用户界面模块
│   ├── GUI.py                        # 主GUI程序（完整中文界面、表格显示）
│   ├── GUI_DESIGN_ARCHITECTURE.md    # GUI设计思想与架构文档
│   ├── GUI_IMPLEMENTATION_DETAILS.md # GUI功能实现技术细节
│   └── GUI_USER_GUIDE.md             # GUI使用指南和最佳实践
│
├── TestGrammar/                      # 测试文法集合与自动化测试
│   ├── *_grammar.txt                 # 各种测试文法文件（表达式、条件语句、if-else等）
│   ├── FaultGrammar/                 # 错误文法测试集合
│   │   └── *.txt                     # 错误类型测试
│   ├── test_all_grammars.sh          # Linux/macOS自动化测试脚本 (Windows暂不支持)
│   ├── test_results.log              # 测试日志文件（运行后生成）
│   └── README.md                     # 测试用例说明文档
│
├── BACKEND_UNRESOLVED_ISSUES.md      # 后端开发未解决问题记录
├── SOFTWARE_DIVISION_AND_VERSION_HISTORY.md  # 团队分工与版本历史
└── README.md                         # 项目主要说明文档（本文件）
```

### 文件说明

#### Algorithm/ - 核心算法模块
- **lr_analyzer.cpp/h**：LR分析器核心实现（主要算法）
- **lr_item.cpp/h**：LR项目和项目集管理实现
- **grammar.cpp/h**：文法表示、FIRST/FOLLOW集计算实现
- **main.cpp**：交互式主程序入口
- **lr_cli.cpp**：命令行接口程序（支持JSON输出）
- **build.bat**：Windows自动化构建脚本（UTF-8支持）
- **Makefile**：跨平台构建配置
- **lr_analyzer.exe/lr_cli.exe**：编译生成的可执行文件
- **ALGORITHM_DETAILS.md**：LR算法详细技术说明
- **CODE_ARCHITECTURE.md**：代码架构和模块间关系文档
- **LR_CLI_DOCUMENTATION.md**：命令行工具完整使用文档
- **LR1_DEVELOPMENT_ISSUES.md**：LR(1)开发问题与解决方案记录

#### GUI/ - 图形界面模块  
- **GUI.py**：主要的图形用户界面程序
- **GUI_DESIGN_ARCHITECTURE.md**：GUI设计思想与架构文档
- **GUI_IMPLEMENTATION_DETAILS.md**：GUI功能实现技术细节
- **GUI_USER_GUIDE.md**：GUI使用指南和最佳实践

#### TestGrammar/ - 测试用例与自动化测试
- **测试文法文件**：包含各种复杂度和特性的测试文法
  - expression_grammar.txt：表达式语法测试
  - conditional_grammar.txt：条件语句语法测试  
  - if_else_language_grammar.txt：完整if-else语言测试
  - assignment_grammar.txt：赋值语句测试
  - complex_grammar.txt：复杂语法结构测试
- **FaultGrammar/**：错误文法测试集合
  - ambiguous.txt：二义性文法测试
  - left_recursive.txt：左递归文法测试
  - circular_dependency.txt：循环依赖测试
  - reduce_reduce_conflict.txt：归约-归约冲突测试
- **自动化测试脚本**：
  - test_all_grammars.sh：Linux/macOS自动化测试脚本 (Windows暂不支持)
- **README.md**：测试用例详细说明

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

#### 自动化测试

**Linux/macOS (支持自动化脚本)**:
```bash
cd TestGrammar
chmod +x test_all_grammars.sh
./test_all_grammars.sh
```

**Windows (仅支持手动测试)**:
```powershell
cd TestGrammar  
..\Algorithm\lr_cli.exe example_grammar.txt -t slr1 --table
```

#### Linux/macOS
```bash
cd Algorithm
make all
# 或者手动编译
g++ -o lr_cli lr_cli.cpp lr_analyzer.cpp grammar.cpp lr_item.cpp -std=c++17 -Wall -Wextra
g++ -o lr_analyzer main.cpp lr_analyzer.cpp grammar.cpp lr_item.cpp -std=c++17 -Wall -Wextra
```

### 快速测试 - if-else语句支持验证

本项目完全支持if-else语句的语法分析。可以通过以下方式快速验证：

#### 使用自动化测试脚本 (仅Linux/macOS)
```bash
cd TestGrammar
./test_all_grammars.sh

# 将看到如下输出：
# ===== 测试: conditional_grammar.txt =====
# SUCCESS: 条件语句语法构造成功
# ===== 测试: if_else_language_grammar.txt =====  
# SUCCESS: if-else语言语法构造成功
```

#### 手动测试if-else语句 (跨平台)
```bash
# 测试简单条件语句
cd Algorithm
./lr_cli ../TestGrammar/conditional_grammar.txt -t slr1 -s "if id then id = num else id = num"

# 测试复杂if-else语言 (Windows用户使用lr_cli.exe)
./lr_cli ../TestGrammar/if_else_language_grammar.txt -t slr1 -s "if ( id > number ) { id = number ; }"
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

### 启动GUI
```bash
cd GUI
python3 GUI.py
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
- ✅ **命令行批处理支持** - 适合脚本调用和Linux/macOS自动化测试
- ✅ **实时错误报告** - 详细的错误位置和建议修复方案
- ✅ **结果导出功能** - 支持完整分析报告的文本和JSON导出

### 技术改进功能
- ✅ **字符编码兼容** - 解决了Unicode字符乱码问题
- ✅ **异步处理机制** - GUI界面非阻塞，提升用户体验  
- ✅ **静默模式支持** - JSON输出时屏蔽调试信息
- ✅ **多线程架构** - 后端计算与前端界面分离
- ✅ **智能错误处理** - 区分不同类型错误并提供针对性建议

## 文法格式

```
E -> E + T
E -> T
T -> T * F
T -> F
F -> ( E )
F -> a
```

- 每行一个产生式
- 使用 `->` 分隔左部和右部
- 符号间用空格分隔
- 开始符号为第一个产生式的左部

## 命令行参数

```
Usage: ./lr_cli <grammar_file> [options]

Options:
  -t, --type <type>     分析器类型: lr0, slr1, lr1 (默认: slr1)
  -s, --string <str>    待分析的输入串
  --table              显示ACTION和GOTO表
  --items              显示项目集
  --json               JSON格式输出
  -h, --help           显示帮助信息
```

## 示例

### 基础使用示例
```bash
# 使用SLR(1)分析算术表达式
cd Algorithm
./lr_cli ../TestGrammar/example_grammar.txt -t slr1 -s "a + a * a"

# 显示ACTION和GOTO表
./lr_cli ../TestGrammar/example_grammar.txt -t slr1 --table

# 显示所有项目集
./lr_cli ../TestGrammar/example_grammar.txt -t slr1 --items

# JSON格式输出（适合程序调用）
./lr_cli ../TestGrammar/example_grammar.txt -t slr1 --table --json
```

### 不同算法比较
```bash
# LR(0)算法 - 可能存在冲突
./lr_cli ../TestGrammar/example_grammar.txt -t lr0 -s "a + a"

# SLR(1)算法 - 使用FOLLOW集解决部分冲突  
./lr_cli ../TestGrammar/example_grammar.txt -t slr1 -s "a + a"

# LR(1)算法 - 最强大，能处理更复杂文法
./lr_cli ../TestGrammar/conditional_grammar.txt -t lr1 -s "if a then a else a"
```

### 输出示例

#### 成功分析的输出
```
构造SLR(1)分析表...

语法分析过程：
步骤	状态栈	符号栈		输入		动作
0	0 		        	a + a * a $ 	s5
1	0 5 	a 		+ a * a $ 	r6
					归约用产生式: F -> a
2	0 3 	F 		+ a * a $ 	r4
					归约用产生式: T -> F
3	0 2 	T 		+ a * a $ 	r2
					归约用产生式: E -> T
4	0 1 	E 		+ a * a $ 	s6
5	0 1 6 	E + 		a * a $ 	s5
6	0 1 6 5 	E + a 		* a $ 		r6
					归约用产生式: F -> a
7	0 1 6 3 	E + F 		* a $ 		r4
					归约用产生式: T -> F
8	0 1 6 7 	E + T 		* a $ 		s8
9	0 1 6 7 8 	E + T * 	a $ 		s5
10	0 1 6 7 8 5 	E + T * a 	$ 		r6
					归约用产生式: F -> a
11	0 1 6 7 8 3 	E + T * F 	$ 		r3
					归约用产生式: T -> T * F
12	0 1 6 7 	E + T 		$ 		r1
					归约用产生式: E -> E + T
13	0 1 	E 		$ 		acc
					接受

✓ 分析成功！输入串被接受。
```

#### 冲突检测输出
```
构造LR(0)分析表...
⚠ 发现冲突：
状态 2 在符号 '+' 上存在移进/规约冲突：
- 移进到状态 6
- 使用产生式 E -> T 进行规约

建议：尝试使用SLR(1)或LR(1)算法解决此冲突。
```

#### JSON格式输出示例
```json
{
  "type": "action_table",
  "data": {
    "0": {
      "a": "s5",
      "(": "s4"
    },
    "1": {
      "+": "s6",
      "$": "acc"
    },
    "2": {
      "+": "r2",
      "*": "s8",
      "$": "r2"
    }
  }
}
```

### GUI使用示例
```bash
cd GUI
python3 GUI.py
```

GUI主要功能：
1. **文法输入** - 支持文件选择和直接编辑
2. **算法选择** - LR(0)/SLR(1)/LR(1)单选
3. **字符串分析** - 输入待分析串，实时显示结果
4. **表格查看** - ACTION表、GOTO表、项目集的表格化显示
5. **结果导出** - 完整分析报告导出为文本文件

### 高级使用技巧

#### 批量测试脚本
```bash
#!/bin/bash
# 测试所有算法对同一文法的处理能力
cd Algorithm
for algo in lr0 slr1 lr1; do
    echo "=== 测试 $algo 算法 ==="
    ./lr_cli ../TestGrammar/example_grammar.txt -t $algo --table
    echo
done
```

#### 文法冲突诊断
```bash
cd Algorithm
# 先用LR(0)测试，观察冲突
./lr_cli ../my_grammar.txt -t lr0 --table

# 如果有冲突，尝试SLR(1)
./lr_cli ../my_grammar.txt -t slr1 --table

# 最后尝试LR(1)
./lr_cli ../my_grammar.txt -t lr1 --table
```

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

## 许可证

本项目为教学和研究目的开发，遵循学术使用原则。

---

## 开发过程中的技术问题与解决方案

### 1. 字符编码问题（乱码）

**问题描述**: 
原始C++代码中使用了Unicode字符（如 `·` 用于标记LR项目中的点），在不同系统和终端环境下显示为乱码。

**根本原因**:
- C++源码包含非ASCII字符
- 不同系统的字符编码设置不一致
- JSON输出时Unicode字符解析困难

**解决方案**:
```cpp
// 原始代码（问题版本）
std::string LRItem::toString() const {
    return production.left + " -> " + "·" + production.right;
}

// 修复后代码
std::string LRItem::toString() const {
    return production.left + " -> " + "." + production.right;  // 使用ASCII点
}
```

**技术要点**:
- 统一使用ASCII字符集
- 确保所有字符串输出兼容性
- JSON输出格式标准化

### 2. GUI与C++后端通信问题

**问题描述**:
- Python GUI无法正确解析C++程序的输出
- 调试信息干扰JSON数据解析
- 子进程调用阻塞用户界面

**根本原因**:
- C++程序输出混合了调试信息和结构化数据
- 缺少静默模式控制
- GUI缺少异步处理机制

**解决方案**:

1. **添加静默模式控制**:
```cpp
// lr_cli.cpp 中添加全局控制变量
bool g_silent_mode = false;

// 在调试输出前添加检查
if (!g_silent_mode) {
    std::cout << "调试信息..." << std::endl;
}
```

2. **JSON专用输出接口**:
```cpp
// lr_analyzer.h 中新增方法
void printActionTableJSON() const;
void printGotoTableJSON() const;
void printItemSetsJSON() const;
```

3. **GUI异步处理**:
```python
import threading

def analyze_grammar_async(self):
    def worker():
        # C++后端调用
        result = subprocess.run([cmd], capture_output=True, text=True, encoding='utf-8')
        # 更新GUI（线程安全）
        self.master.after(0, self.update_results, result)
    
    threading.Thread(target=worker, daemon=True).start()
```

### 3. ACTION/GOTO表显示问题

**问题描述**:
初始GUI版本只显示硬编码的示例数据，无法显示真实的分析表。

**根本原因**:
- 缺少C++后端的结构化数据输出
- GUI解析逻辑不完整
- 表格格式化复杂

**解决方案**:

1. **C++端JSON格式输出**:
```cpp
void LRAnalyzer::printActionTableJSON() const {
    nlohmann::json jsonOutput;
    jsonOutput["type"] = "action_table";
    
    for (const auto& entry : actionTable) {
        std::string state = std::to_string(entry.first.first);
        std::string symbol = entry.first.second;
        std::string action = entry.second;
        jsonOutput["data"][state][symbol] = action;
    }
    
    std::cout << jsonOutput.dump(2) << std::endl;
}
```

2. **Python端智能解析**:
```python
def parse_action_table(self, json_str):
    try:
        data = json.loads(json_str)
        if data.get("type") == "action_table":
            return self.format_action_table(data["data"])
    except json.JSONDecodeError:
        return "解析错误"
```

### 4. 中文界面本地化问题

**问题描述**:
- 字体渲染问题
- 界面布局适配中文文本
- 编码兼容性

**解决方案**:
```python
# 自动字体检测和配置
def get_chinese_font(self):
    chinese_fonts = ['Microsoft YaHei', 'SimHei', 'SimSun', 'DejaVu Sans']
    for font_name in chinese_fonts:
        if font_name in tkinter.font.families():
            return (font_name, 10)
    return ('TkDefaultFont', 10)

# UTF-8编码确保
def safe_read_file(file_path):
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            return f.read()
    except UnicodeDecodeError:
        with open(file_path, 'r', encoding='gbk') as f:
            return f.read()
```

### 5. 运行 `python main.py` 时没有任何输出，GUI窗口无法显示，程序看似挂起。

**问题描述**: 通过详细的调试过程发现问题出现在原始GUI代码中：

1. **线程问题**: 原始GUI代码在初始化时启动了多个后台线程
2. **主线程冲突**: 这些线程试图在tkinter主循环开始之前访问GUI组件
3. **错误信息**: 调试过程中发现"main thread is not in main loop"错误
4. **复杂度过高**: 原始GUI代码过于复杂，包含了大量可能导致问题的功能

**修复方案：**
采用简化和重构的方法：

#### 1. 简化GUI架构
- 移除了复杂的多线程构造表操作
- 使用同步方式处理后端调用
- 简化了GUI界面布局和组件

#### 2. 修复的核心问题
- **线程安全**: 移除了在GUI初始化时启动的后台线程
- **同步操作**: 将异步的分析表构造改为同步操作
- **错误处理**: 添加了更好的异常处理和用户反馈

#### 3. 保留核心功能
- 文法编辑和输入
- 分析方法选择(LR(0), SLR(1), LR(1))
- 分析表构造和显示
- 结果展示

**验证结果：**
✅ **已完全解决**: 程序现在可以正常启动和运行

这些问题的解决过程体现了软件工程中常见的**编码兼容性**、**系统集成**、**用户界面设计**和**项目管理**等关键技术领域的实践经验。

## 版本历史

本项目采用迭代式开发模型，版本更新遵循语义化版本控制。详细的版本历史和开发进程请参见 [SOFTWARE_DIVISION_AND_VERSION_HISTORY.md](SOFTWARE_DIVISION_AND_VERSION_HISTORY.md)。

### 主要版本里程碑

| 版本 | 发布时间 | 主要特性 | 开发者 |
|------|----------|----------|--------|
| **v1.0.0** | 2025.6.9-6.11 | 核心LR算法实现，基础架构建立 | 算法组 + 后端组 |
| **v1.1.0** | 2025.6.11 | 文件输入支持，测试增强 | 后端组 |
| **v2.0.0** | 2025.6.12 | 图形界面引入，前后端分离 | GUI组 + 后端组 |
| **v2.1.0** | 2025.6.14 | 跨平台兼容性实现 | 全体开发组 |
| **v2.2.0** | 2025.6.16-6.17 | **测试框架与自动化脚本** | **后端组** |
| **v2.3.0** | 2025.6.18 | LR(1)算法完整实现 | 算法组 |
| **v3.0.0** | 2025.6.19 | 错误恢复与诊断增强 | 全体开发组 |
| **v3.1.0** | 2025.6.21 | 系统完善与文档化 | 全体开发组 |

### 当前版本特性

- ✅ **完整的LR分析器**: 支持LR(0)、SLR(1)、LR(1)三种算法
- ✅ **if-else语句支持**: 完全支持条件语句和复杂if-else语言结构  
- ✅ **跨平台构建**: Windows、Linux、macOS全平台支持
- ✅ **UTF-8编码**: 完美的中文字符支持
- ✅ **自动化测试**: 完整的测试框架和Shell脚本 (Linux/macOS)
- ✅ **图形化界面**: Python Tkinter用户友好界面
- ✅ **命令行工具**: 强大的CLI接口，支持脚本调用

## 团队信息

### 开发团队分工

本项目采用模块化开发模式，团队分为三个专业开发组：

1. **🎨 图形用户界面 (GUI) 开发组 (1人)**
   - 负责Python Tkinter界面设计与实现
   - 用户交互逻辑和可视化展示
   - 跨平台界面兼容性

2. **🧮 核心分析算法开发组 (1人)**  
   - LR(0)、SLR(1)、LR(1)算法核心实现
   - 分析表构造和冲突检测
   - 算法性能优化

3. **⚙️ 后端基础及命令行接口 (CLI) 开发组 (1人)**
   - 文法数据结构和解析逻辑
   - 命令行接口和JSON数据交换
   - 跨平台构建脚本和自动化测试框架
   - **测试用例设计和Shell脚本开发** (Linux/macOS)

### 技术栈

- **后端**: C++17, STL, JSON
- **前端**: Python 3.6+, Tkinter
- **构建**: Make, 批处理脚本 (Windows), Shell脚本 (Linux/macOS)
- **测试**: 自动化测试框架
- **文档**: Markdown, 中英文双语

### 项目管理

- **版本控制**: Git语义化版本管理
- **质量保证**: 单元测试 + 集成测试 + 用户验收
- **文档标准**: 完整的技术文档和用户指南
- **持续集成**: 多平台自动化构建和测试

---

**📧 联系方式**: 项目技术支持和问题反馈  
**📄 许可证**: MIT License  
**📅 最后更新**: 2025年6月17日  
**🏷️ 当前版本**: v3.1.1

**🔗 相关文档**:
- [团队分工与版本历史详细说明](SOFTWARE_DIVISION_AND_VERSION_HISTORY.md)
- [LR算法技术详细说明](Algorithm/ALGORITHM_DETAILS.md)
- [GUI设计架构文档](GUI/GUI_DESIGN_ARCHITECTURE.md)
- [命令行工具使用文档](Algorithm/LR_CLI_DOCUMENTATION.md)
- [测试用例说明](TestGrammar/README.md)
