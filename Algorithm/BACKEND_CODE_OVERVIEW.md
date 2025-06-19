# LR语法分析器后端代码架构总览

## 📁 文件结构概述

本文档提供Algorithm文件夹中核心C++代码的架构总览，包括各模块的功能和相互关系。

## 🏗️ 核心模块架构

### 1. 文法处理模块 (`grammar.h` / `grammar.cpp`)

**核心功能**: 上下文无关文法的定义和处理

#### Production类
- 表示单个产生式 (A → α)
- 包含产生式左部（非终结符）和右部（符号串）
- 支持产生式比较和字符串转换
- 右部使用vector存储符号序列

#### Grammar类
- 管理整个文法的产生式集合
- 自动识别和分类终结符与非终结符
- 提供文法增广功能（为LR分析添加新的开始符号）
- 支持从文件或标准输入读取文法
- 自动对文法进行错误检测

**核心功能实现**:
- **文法解析**: 支持多种输入格式（文件/标准输入）
- **符号识别**: 自动提取终结符和非终结符集合
- **文法增广**: 为LR分析添加新的开始符号
- **格式验证**: 产生式格式检查和错误报告
- **文法完整性检查**: 验证符号可达性和完整性

**输入格式支持**:
- 标准产生式格式：`A -> alpha | beta`
- 空产生式：用 `ε` 表示
- 注释支持：以 `#` 开头的行
- 多选择分支：用 `|` 分隔不同右部

### 2. LR项目处理模块 (`lr_item.h` / `lr_item.cpp`)

**核心功能**: LR项目和项目集的定义与操作

#### LRItem类
- 表示单个LR项目，包含产生式左部、右部、点位置和前瞻符号集合
- 支持LR(0)项目（不含前瞻符号）和LR(1)项目（含前瞻符号）
- 提供获取点后符号、判断完整项目、字符串转换等功能

**项目类型**:
- **LR(0)项目**: 形式为 A → α·β，不包含前瞻信息
- **LR(1)项目**: 形式为 [A → α·β, a]，包含前瞻符号集合
- **核心项目**: 不是由闭包运算得到的项目
- **非核心项目**: 通过闭包运算产生的项目

#### ItemSet类
- 表示LR自动机的一个状态，管理LR项目的集合
- 提供基本的项目集合操作（添加、查找、比较）
- 支持项目集的打印和调试功能
- 使用`std::set<LRItem>`保证项目的唯一性和有序性

**设计特点**:
- **简单容器**: ItemSet类本身是一个轻量级的容器类
- **状态表示**: 每个ItemSet对象代表LR自动机中的一个状态
- **比较操作**: 重载了`==`和`<`运算符，用于状态去重和排序
- **项目管理**: 通过set容器自动处理项目的唯一性

**注意**: ItemSet类不直接实现闭包计算和GOTO函数，这些核心算法在`LRAnalyzer`类中实现：
- **闭包算法**: `LRAnalyzer::closure()` 和 `LRAnalyzer::closureLR1()`
- **GOTO函数**: `LRAnalyzer::gotoSet()` 和 `LRAnalyzer::gotoSetLR1()`
- **项目集族构造**: `LRAnalyzer::constructLR0ItemSets()` 和 `LRAnalyzer::constructLR1ItemSets()`

### 3. LR分析器核心模块 (`lr_analyzer.h` / `lr_analyzer.cpp`)

**核心功能**: 完整的LR语法分析器实现

#### 分析动作定义
```cpp
enum class ActionType {
    SHIFT,      // 移进动作 - 将当前输入符号压入栈中
    REDUCE,     // 归约动作 - 使用某个产生式进行归约  
    ACCEPT,     // 接受动作 - 表示输入串被成功分析
    ERROR       // 错误动作 - 表示分析失败
};

struct Action {
    ActionType type;        // 动作类型
    int value;              // 状态号（移进）或产生式号（归约）
    Production production;  // 用于归约的产生式
    
    Action();  // 默认构造ERROR动作
    Action(ActionType t, int v);  // 移进/接受动作
    Action(ActionType t, int v, const Production& p);  // 归约动作
    std::string toString() const;
};
```

#### LRAnalyzer类
```cpp
class LRAnalyzer {
private:
    Grammar grammar;                              // 待分析的文法
    std::vector<ItemSet> itemSets;               // 项目集族
    std::map<std::pair<int, std::string>, Action> actionTable;  // ACTION表
    std::map<std::pair<int, std::string>, int> gotoTable;       // GOTO表
    
    // FIRST集和FOLLOW集计算
    std::map<std::string, std::set<std::string>> firstSets;
    std::map<std::string, std::set<std::string>> followSets;
    
public:
    bool constructLR0Parser();   // 构造LR(0)分析器
    bool constructSLR1Parser();  // 构造SLR(1)分析器  
    bool constructLR1Parser();   // 构造LR(1)分析器
    
    bool parseInput(const std::string& input);  // 分析输入串
    void printTables() const;    // 打印分析表
    void printItemSets() const;  // 打印项目集族
};
```

**核心算法实现**:

1. **项目集闭包计算**:
   ```cpp
   // LR(0)闭包算法
   ItemSet closure(const ItemSet& items);
   
   // LR(1)闭包算法 
   ItemSet closureLR1(const ItemSet& items);
   ```
   闭包算法实现：
   ```
   CLOSURE(I) = I ∪ {[B → ·γ, FIRST(βa)] | [A → α·Bβ, a] ∈ I, B → γ ∈ P}
   ```

2. **GOTO转移函数**:
   ```cpp
   // LR(0) GOTO函数
   ItemSet gotoSet(const ItemSet& items, const std::string& symbol);
   
   // LR(1) GOTO函数
   ItemSet gotoSetLR1(const ItemSet& items, const std::string& symbol);
   ```
   GOTO函数实现：
   ```
   GOTO(I, X) = CLOSURE({[A → αX·β, a] | [A → α·Xβ, a] ∈ I})
   ```

3. **项目集族构造**:
   ```cpp
   void constructLR0ItemSets();  // LR(0)项目集族构造
   void constructLR1ItemSets();  // LR(1)项目集族构造
   ```

4. **分析表构造**:
   ```cpp
   void constructActionGotoTable(bool isLR1 = false);  // ACTION/GOTO表构造
   void detectConflicts();                             // 冲突检测
   ```

5. **语法分析执行**:
   ```cpp
   bool parse(const std::vector<std::string>& input);  // 执行LR分析
   ```

**LR分析器特性**:
- **多算法支持**: LR(0)、SLR(1)、LR(1)三种分析算法
- **智能冲突检测**: 自动检测和报告移进-归约、归约-归约冲突
- **可视化分析**: 提供分析过程的详细跟踪和调试信息
- **多输出格式**: 支持文本和JSON格式的分析表输出

### 4. 用户界面模块

#### 4.1 交互式界面 (`main.cpp`)
**功能**: 菜单驱动的交互式用户界面
- 文法输入选择（文件/键盘）
- 分析算法选择
- 分析表查看
- 输入串测试
- 项目集族显示

#### 4.2 命令行界面 (`lr_cli.cpp`)
**功能**: 批处理友好的命令行接口
- 支持脚本调用和自动化测试
- 多种输出格式（文本/JSON）
- 命令行参数解析
- 适合集成测试环境

> 📖 **详细文档**: 如需深入了解CLI接口的完整功能和使用方法，请参考 [Algorithm/LR_CLI_DOCUMENTATION.md](./LR_CLI_DOCUMENTATION.md)

## 🔄 模块间关系图

```
┌─────────────────┐    ┌─────────────────┐
│   main.cpp      │    │   lr_cli.cpp    │
│ (交互式界面)     │    │ (命令行界面)     │
└─────────┬───────┘    └─────────┬───────┘
          │                      │
          └──────────┬───────────┘
                     │
                     ▼
          ┌─────────────────┐
          │  lr_analyzer.h  │
          │    (核心分析器)  │
          └─────────┬───────┘
                    │
          ┌─────────┼───────────┐
          │         │           │
          ▼         ▼           ▼
   ┌──────────┐ ┌─────────┐ ┌─────────┐
   │grammar.h │ │lr_item.h│ │ 算法库  │
   │(文法处理) │ │(项目集) │ │(STL等)  │
   └──────────┘ └─────────┘ └─────────┘
```

## 🎯 核心算法流程

### 1. 文法预处理阶段
```
输入文法 → 文法验证 → 符号识别 → 文法增广 → FIRST/FOLLOW集计算
```
**详细步骤**:
- **格式验证**: 检查产生式语法格式
- **符号分类**: 自动识别终结符和非终结符
- **文法增广**: 添加新开始符号 S' → S
- **FIRST集计算**: `computeFirstSets()` - 计算所有符号的FIRST集
- **FOLLOW集计算**: `computeFollowSets()` - 计算所有非终结符的FOLLOW集

### 2. 项目集族构造阶段
```
增广文法 → 初始项目集I₀ → 闭包计算 → GOTO转换 → 项目集族C
```
**LR(0)构造算法**(`constructLR0ItemSets()`):
```
算法：构造LR(0)项目集族
输入：增广文法G'
输出：项目集族C = {I₀, I₁, ..., Iₙ}

1. 创建初始项目集: I₀ = CLOSURE({[S' → ·S]})
2. C := {I₀}
3. repeat
4.   for each I in C do
5.     for each grammar symbol X do
6.       J := GOTO(I, X)  // 调用gotoSet(I, X)
7.       if J ≠ ∅ and J ∉ C then
8.         C := C ∪ {J}
9.       endif
10.    endfor
11.  endfor
12. until no new item sets are added to C
```

**LR(1)构造算法**(`constructLR1ItemSets()`):
- 与LR(0)类似，但使用`closureLR1()`和`gotoSetLR1()`
- 项目包含前瞻符号信息，提供更精确的分析能力

### 3. 分析表构造阶段
```
项目集族C → ACTION表构造 → GOTO表构造 → 冲突检测 → 最终分析表
```
**分析表构造算法**(`constructActionGotoTable()`):
- **ACTION表构造规则**:
  - **移进动作**: 若 [A → α·aβ] ∈ Iᵢ 且 GOTO(Iᵢ, a) = Iⱼ，则 ACTION[i,a] = shift j
  - **归约动作**: 
    - LR(0): 若 [A → α·] ∈ Iᵢ，则对所有终结符a，ACTION[i,a] = reduce A → α
    - SLR(1): 若 [A → α·] ∈ Iᵢ，则对所有 a ∈ FOLLOW(A)，ACTION[i,a] = reduce A → α
    - LR(1): 若 [A → α·, a] ∈ Iᵢ，则 ACTION[i,a] = reduce A → α
  - **接受动作**: 若 [S' → S·] ∈ Iᵢ，则 ACTION[i,$] = accept

- **GOTO表构造规则**:
  - 若 GOTO(Iᵢ, A) = Iⱼ，则 GOTO[i,A] = j（A为非终结符）

**冲突检测**(`detectConflicts()`):
- **移进-归约冲突**: 同一单元格既有移进动作又有归约动作
- **归约-归约冲突**: 同一单元格有多个归约动作

### 4. 语法分析执行阶段
```
输入串 + 分析表 → 栈式分析 → 移进/归约操作 → 接受/拒绝结果
```
**分析算法**(`parse()`):
```
算法：LR语法分析
输入：输入串w，LR分析表
输出：如果w∈L(G)则接受，否则拒绝

1. 初始化状态栈为[0]，符号栈为空
2. 设输入指针ip指向w的第一个符号
3. repeat
4.   设s为状态栈顶状态，a为ip指向的符号
5.   if ACTION[s,a] = shift t then
6.     状态栈压入t，符号栈压入a，ip前进
7.   else if ACTION[s,a] = reduce A → β then
8.     弹出|β|个状态和符号，设新栈顶状态为t
9.     状态栈压入GOTO[t,A]，符号栈压入A
10.  else if ACTION[s,a] = accept then
11.    return "接受"
12.  else
13.    return "拒绝"
14.  endif
15. until forever
```

## 🔧 编译和构建

### 构建脚本详解

#### Windows构建脚本 (`build.bat`)
**功能完备的Windows批处理构建系统**

```batch
# 基本用法
build.bat           # 编译所有目标（默认）
build.bat clean     # 清理编译文件
build.bat rebuild   # 清理后重新编译
build.bat debug     # 调试版本编译
build.bat release   # 发布版本编译
build.bat test      # 编译并运行测试
```

#### Linux/macOS构建脚本 (`Makefile`)
**专业的GNU Make构建系统**

```makefile
# 基本用法
make              # 编译所有目标并自动清理
make keep-objects # 编译但保留中间文件
make clean        # 清理所有编译文件
make run          # 编译并运行交互式程序
make debug        # 编译调试版本
make test         # 运行基础测试
make help         # 显示详细帮助
```

**Makefile特性**:
- **依赖管理**: 自动处理头文件和源文件依赖
- **增量编译**: 只重新编译修改过的文件
- **UTF-8支持**: 特殊的中文字符编码处理
- **模块化构建**: 分离的目标文件管理
- **交叉编译**: 支持不同架构和系统

**编译器配置**:
```makefile
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -finput-charset=UTF-8 -fexec-charset=UTF-8
TARGET = lr_analyzer
CLI_TARGET = lr_cli
```

### 生成目标详解

#### 主要可执行文件
- **lr_analyzer** / **lr_analyzer.exe**: 交互式分析程序
  - 菜单驱动的用户界面
  - 适合教学和演示
  - 支持实时文法输入和测试

- **lr_cli** / **lr_cli.exe**: 命令行分析工具
  - 批处理友好的接口
  - 支持脚本自动化
  - JSON格式输出支持
  - 集成测试环境兼容

#### 编译依赖关系
```
lr_analyzer <- main.o + grammar.o + lr_analyzer.o + lr_item.o
lr_cli <- lr_cli.o + grammar.o + lr_analyzer.o + lr_item.o
```

### 跨平台兼容性
- **Windows**: 使用MinGW-w64或MSYS2环境
- **Linux**: 标准GCC工具链
- **macOS**: Clang或GCC编译器
- **编码支持**: 全平台UTF-8字符处理

## 📊 代码统计

| 模块 | 头文件 | 源文件 | 主要功能 |
|------|--------|--------|----------|
| Grammar | grammar.h | grammar.cpp | 文法处理和计算 |
| LRItem | lr_item.h | lr_item.cpp | 项目集操作 |
| LRAnalyzer | lr_analyzer.h | lr_analyzer.cpp | 核心分析算法 |
| Main | - | main.cpp | 交互式界面 |
| CLI | - | lr_cli.cpp | 命令行界面 |

## 🔍 设计特点

### 1. 模块化设计
- 每个模块职责清晰，低耦合高内聚
- 便于单独测试和维护

### 2. 算法抽象
- 支持多种LR分析算法
- 统一的接口设计

### 3. 用户友好
- 双重界面支持（交互式+命令行）
- 详细的错误信息和调试输出

### 4. 扩展性
- 易于添加新的分析算法
- 支持多种输出格式
