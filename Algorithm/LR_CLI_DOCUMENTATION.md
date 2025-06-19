# LR命令行工具 (lr_cli) 完整使用文档

## 📋 概述

`lr_cli` 是LR语法分析器的现代化命令行接口工具，提供了专业的批处理友好的LR语法分析功能。它支持LR(0)、SLR(1)、LR(1)三种分析算法，可以构造分析表、分析输入串、显示项目集，并支持多种输出格式。

## ✨ 核心特性

### 🔧 分析功能
- **多种LR算法**: 支持LR(0)、SLR(1)、LR(1)分析方法
- **分析表构造**: 智能生成ACTION表和GOTO表
- **输入串分析**: 完整的语法分析过程追踪和可视化
- **项目集显示**: 可视化LR项目集族构造过程
- **冲突检测**: 智能识别和报告移进-归约、归约-归约冲突

### 💻 技术特点
- **批处理友好**: 专为脚本调用和自动化测试设计
- **跨平台支持**: Windows/Linux/macOS全平台兼容
- **多格式输出**: 人类可读的文本格式和机器可读的JSON格式
- **静默模式**: JSON输出时自动屏蔽调试信息
- **错误恢复**: 智能错误检测并提供解决建议
- **UTF-8支持**: 完整的中文和Unicode字符支持

## 🚀 快速开始

### 编译安装

#### Windows环境
```powershell
# 使用提供的构建脚本（推荐）
cd Algorithm
.\build.bat

# 或者使用特定模式
.\build.bat release    # 发布版本
.\build.bat debug      # 调试版本
.\build.bat quick      # 快速编译

# 手动编译
g++ -o lr_cli.exe lr_cli.cpp lr_analyzer.cpp grammar.cpp lr_item.cpp -std=c++11 -O2
```

#### Linux环境
```bash
# 使用Makefile（推荐）
cd Algorithm
make                   # 编译所有目标
make lr_cli            # 只编译CLI工具
make debug             # 调试版本

# 手动编译
g++ -o lr_cli lr_cli.cpp lr_analyzer.cpp grammar.cpp lr_item.cpp -std=c++11 -O2 -finput-charset=UTF-8 -fexec-charset=UTF-8
```

### 验证安装
```bash
# 显示版本和帮助信息
./lr_cli --help

# 快速测试
./lr_cli ../TestGrammar/example_grammar.txt -t slr1 --table
```

> 💡 **提示**: 更多编译选项和详细说明请参考 [BACKEND_CODE_OVERVIEW.md](./BACKEND_CODE_OVERVIEW.md) 中的编译部分。

## 📖 命令语法和参数

### 基本语法
```bash
lr_cli <grammar_file> [options]
```

### 📝 必需参数
- `<grammar_file>`: 文法文件路径（支持相对路径和绝对路径）

### ⚙️ 选项参数详解

#### 分析器类型选择
```bash
-t, --type <type>     指定分析器类型（默认: slr1）
```

| 分析器 | 说明 | 适用场景 |
|-------|------|----------|
| `lr0` | LR(0)分析器 | 简单文法，教学演示 |
| `slr1` | SLR(1)分析器 | 大多数实用文法，性能平衡 |
| `lr1` | LR(1)分析器 | 复杂文法，最强解析能力 |

#### 功能选项
```bash
-s, --string <str>    分析指定的输入串
--table              显示ACTION和GOTO分析表
--items              显示LR项目集族
--json               启用JSON格式输出（适合程序调用）
-h, --help           显示详细帮助信息
```

#### 高级选项
```bash
--verbose            显示详细调试信息
--quiet              静默模式（仅输出结果）
--timeout <seconds>  设置超时时间（防止死循环）
```

## 🌟 使用示例

### 基础用法

#### 1. 构造分析表
```bash
# 使用默认SLR(1)算法构造分析表
./lr_cli ../TestGrammar/example_grammar.txt --table

# 使用LR(1)算法构造复杂文法的分析表
./lr_cli ../TestGrammar/c_language_complex_grammar.txt -t lr1 --table
```

#### 2. 分析输入串
```bash
# 基本表达式分析
./lr_cli ../TestGrammar/example_grammar.txt -s "a + a * a"

# 复杂编程语言结构分析
./lr_cli ../TestGrammar/programming_language_complex_grammar.txt -t lr1 -s "if ( id > number ) { id = number ; }"
```

#### 3. 显示项目集
```bash
# 查看项目集构造过程
./lr_cli ../TestGrammar/example_grammar.txt --items

# 组合使用：同时显示分析表和项目集
./lr_cli ../TestGrammar/example_grammar.txt --table --items
```

### 高级用法

#### JSON格式输出（程序集成）
```bash
# 获取JSON格式的分析表（适合程序解析）
./lr_cli ../TestGrammar/example_grammar.txt --table --json

# 分析输入串并获取JSON格式结果
./lr_cli ../TestGrammar/example_grammar.txt -s "a + a * a" --json
```

> 🔧 **静默模式**: JSON 输出时自动启用静默模式，不会输出分析过程信息，确保纯净的 JSON 格式输出。

#### 批量处理和输出重定向
```bash
# 保存分析结果到文件
./lr_cli ../TestGrammar/example_grammar.txt --table > analysis_result.txt

# 同时显示和保存结果
./lr_cli ../TestGrammar/example_grammar.txt --table | tee analysis_result.txt

# 保存JSON格式结果
./lr_cli ../TestGrammar/example_grammar.txt --table --json > analysis.json

# 批量测试多个文法
for grammar in ../TestGrammar/*_grammar.txt; do
    echo "=== 测试: $grammar ==="
    ./lr_cli "$grammar" -t slr1 --table
done
```

#### 错误处理和调试
```bash
# 测试错误文法并查看详细错误信息
./lr_cli ../TestGrammar/FaultGrammar/left_recursive.txt --verbose
```

## 🔗 集成自动化测试

### 官方测试脚本

项目提供了完整的自动化测试脚本，可以系统性地测试lr_cli的所有功能：

```bash
# Linux/macOS环境
cd ../TestGrammar
./test_all_grammars.sh
```

#### 测试模式详解

| 模式 | 命令 | 说明 |
|------|------|------|
| 简单测试 | `./test_all_grammars.sh simple` | 快速验证核心功能 |
| 完整测试 | `./test_all_grammars.sh all` | 运行所有测试（默认） |
| 对比测试 | `./test_all_grammars.sh comparison <文法>` | 三种分析器对比 |
| 压力测试 | `./test_all_grammars.sh stress` | 复杂文法性能测试 |

#### 使用示例
```bash
# 快速验证lr_cli基本功能
./test_all_grammars.sh simple

# 对特定文法进行深入对比测试
./test_all_grammars.sh comparison example_grammar.txt

# 测试复杂文法的处理能力
./test_all_grammars.sh stress
```

> 📚 **详细文档**: 自动化测试的完整功能和使用方法请参考 [TestGrammar/SCRIPT_DOCUMENTATION.md](../TestGrammar/SCRIPT_DOCUMENTATION.md)

### 文法文件说明

测试文法文件位于 `../TestGrammar/` 目录，包含：

#### 正确文法文件
- **基础文法**: `example_grammar.txt`, `expression_grammar.txt`
- **语言结构**: `assignment_grammar.txt`, `conditional_grammar.txt`
- **复杂文法**: `c_language_complex_grammar.txt`, `programming_language_complex_grammar.txt`

#### 错误文法文件（FaultGrammar/目录）
- **格式错误**: `syntax_error.txt`, `empty_grammar.txt`
- **结构问题**: `left_recursive.txt`, `ambiguous.txt`
- **冲突文法**: `shift_reduce_conflict.txt`, `reduce_reduce_conflict.txt`

> 📖 **文法集合说明**: 完整的文法文件列表和用途请参考 [TestGrammar/README.md](../TestGrammar/README.md)
## 📊 输出格式详解

### 文本格式输出

#### 分析表显示
```
构造SLR(1)分析表...
ACTION表:
ACTION表：
状态    $       (       )       *       +       a       $
0               s1                              s5
1               s1                              s5
2       acc                             s7              acc
3       r4              r4      r4      r4              r4
4       r2              r2      s8      r2              r2
5       r6              r6      r6      r6              r6
6                       s9              s7
7               s1                              s5
8               s1                              s5
9       r5              r5      r5      r5              r5
10      r1              r1      s8      r1              r1
11      r3              r3      r3      r3              r3

GOTO表:
GOTO表：
状态    E       E'      F       T
0       2               3       4
1       6               3       4
7                       3       10
8                       11
```

#### 分析过程可视化
```
构造SLR(1)分析表...
语法分析过程：
步骤    状态栈  符号栈          输入            动作
0       0                       a + a * a $             s5
1       0 5     a               + a * a $               r6
                                                归约用产生式: F -> a
2       0 3     F               + a * a $               r4
                                                归约用产生式: T -> F
3       0 4     T               + a * a $               r2
                                                归约用产生式: E -> T
4       0 2     E               + a * a $               s7
5       0 2 7   E +             a * a $                 s5
6       0 2 7 5         E + a           * a $           r6
                                                归约用产生式: F -> a
7       0 2 7 3         E + F           * a $           r4
                                                归约用产生式: T -> F
8       0 2 7 10        E + T           * a $           s8
9       0 2 7 10 8      E + T *                 a $             s5
10      0 2 7 10 8 5    E + T * a               $               r6
                                                归约用产生式: F -> a
11      0 2 7 10 8 11   E + T * F               $               r3
                                                归约用产生式: T -> T * F
12      0 2 7 10        E + T           $               r1
                                                归约用产生式: E -> E + T
13      0 2     E               $               acc
✓ 分析成功！输入串被接受。
✓ 分析成功
```

#### 项目集族显示
```
构造SLR(1)分析表...
项目集族:
项目集族：
I0:
  E -> . E + T
  E -> . T
  E' -> . E
  F -> . ( E )
  F -> . a
  T -> . F
  T -> . T * F

I1:
  E -> . E + T
  E -> . T
  F -> . ( E )
  F -> ( . E )
  F -> . a
  T -> . F
  T -> . T * F

I2:
  E -> E . + T
  E' -> E .

I3:
  T -> F .

I4:
  E -> T .
  T -> T . * F
...
```

### JSON格式输出

#### 分析结果JSON结构
```json
{
"parse_result": {
"input": "a + a * a",
"success": true
}
}
```

#### 实际JSON输出示例

**分析成功示例**：
```bash
$ ./lr_cli.exe ../TestGrammar/example_grammar.txt -s "a + a * a" --json
{
"parse_result": {
"input": "a + a * a",
"success": true
}
}
```

**分析失败示例**：
```bash
$ ./lr_cli.exe ../TestGrammar/example_grammar.txt -s "invalid" --json
{
"parse_result": {
"input": "invalid",
"success": false
}
}
```

**仅显示分析表**：
```bash
$ ./lr_cli.exe ../TestGrammar/example_grammar.txt --table --json
{
"success": true,
"message": "分析表构造成功"
}
```

> **✅ 纯净JSON输出**：JSON 模式下会自动启用静默模式，确保输出只包含纯净的 JSON 格式数据，不会混入任何分析过程的文本信息，便于程序解析和集成。

## 错误处理

### 常见错误类型和输出

#### 文法错误
```bash
$ ./lr_cli.exe ../TestGrammar/FaultGrammar/syntax_error.txt --table
Error: Invalid production format at line 4: E E + T
Command exited with code 1
```

#### 空文法错误
```bash
$ ./lr_cli.exe ../TestGrammar/FaultGrammar/empty_grammar.txt --table
Error: Grammar file is empty, no productions found
Command exited with code 1
```

#### 冲突检测
```bash
$ ./lr_cli.exe ../TestGrammar/FaultGrammar/shift_reduce_conflict.txt --table
构造SLR(1)分析表...
错误: 分析表构造失败
✗ 检测到冲突：
  移进/归约冲突在状态 10 符号 else
Command exited with code 1
```

#### 输入串错误
```bash
$ ./lr_cli.exe ../TestGrammar/example_grammar.txt -s "invalid_symbol"
构造SLR(1)分析表...
语法分析过程：
步骤    状态栈  符号栈          输入            动作
0       0                       invalid_symbol $                错误：无对应动作
✗ 分析失败
```

#### 分析器类型错误
```bash
$ ./lr_cli.exe ../TestGrammar/example_grammar.txt -t invalid_type --table
错误: 未知的分析器类型 invalid_type
Command exited with code 1
```

## ⚠️ 错误处理和诊断

### 实际错误输出示例

lr_cli 提供了清晰简洁的错误信息，帮助用户快速定位和解决问题。

#### 1. 参数错误
```bash
$ ./lr_cli.exe
用法: lr_cli.exe <文法文件> [选项]
选项:
  -t <类型>    分析器类型: lr0, slr1, lr1 (默认: slr1)
  -i <文件>    要分析的输入串文件
  -s <字符串>  要分析的输入串
  --table      只显示分析表
  --items      只显示项目集族
  --json       以JSON格式输出
  --help       显示此帮助信息

示例:
  lr_cli.exe grammar.txt -t slr1 --table
  lr_cli.exe grammar.txt -s "a + a * a"
Command exited with code 1
```

#### 2. 文件错误
```bash
$ ./lr_cli.exe nonexistent.txt
Error: Cannot open file: nonexistent.txt
Command exited with code 1
```

#### 3. 文法格式错误
```bash
$ ./lr_cli.exe ../TestGrammar/FaultGrammar/syntax_error.txt --table
Error: Invalid production format at line 4: E E + T
Command exited with code 1
```

#### 4. 空文法错误
```bash
$ ./lr_cli.exe ../TestGrammar/FaultGrammar/empty_grammar.txt --table
Error: Grammar file is empty, no productions found
Command exited with code 1
```

#### 5. 分析器类型错误
```bash
$ ./lr_cli.exe ../TestGrammar/example_grammar.txt -t invalid_type --table
错误: 未知的分析器类型 invalid_type
Command exited with code 1
```

#### 6. 冲突检测
```bash
$ ./lr_cli.exe ../TestGrammar/FaultGrammar/shift_reduce_conflict.txt --table
构造SLR(1)分析表...
错误: 分析表构造失败
✗ 检测到冲突：
  移进/归约冲突在状态 10 符号 else
Command exited with code 1
```

#### 7. 输入串分析错误
```bash
$ ./lr_cli.exe ../TestGrammar/example_grammar.txt -s "invalid_symbol"
构造SLR(1)分析表...
语法分析过程：
步骤    状态栈  符号栈          输入            动作
0       0                       invalid_symbol $                错误：无对应动作
✗ 分析失败
```

### 常见问题和解决方案

#### 1. 可执行文件问题
```bash
# Windows PowerShell
$ .\lr_cli.exe
用法: lr_cli.exe <文法文件> [选项]
...
Command exited with code 1

🔧 解决方案：
1. 检查编译是否成功：cd Algorithm && .\build.bat
2. 确认文件存在：dir lr_cli.exe
3. 使用完整路径或确保在正确目录中
```

#### 2. 文件路径问题
```bash
$ ./lr_cli.exe wrong_path.txt
Error: Cannot open file: wrong_path.txt
Command exited with code 1

🔧 解决方案：
1. 检查文件是否存在：dir wrong_path.txt
2. 使用相对路径：.\lr_cli.exe ..\TestGrammar\example_grammar.txt
3. 使用绝对路径：.\lr_cli.exe "C:\full\path\to\grammar.txt"
```

#### 3. 文法格式问题
```bash
$ ./lr_cli.exe ../TestGrammar/FaultGrammar/syntax_error.txt --table
Error: Invalid production format at line 4: E E + T
Command exited with code 1

🔧 解决方案：
1. 检查产生式格式，每行应为：左部 -> 右部
2. 确保使用 "->" 而不是 "=" 或其他符号
3. 正确格式：E -> E + T
```

#### 4. 分析器冲突问题
```bash
$ ./lr_cli.exe ../TestGrammar/FaultGrammar/shift_reduce_conflict.txt --table
构造SLR(1)分析表...
错误: 分析表构造失败
✗ 检测到冲突：
  移进/归约冲突在状态 10 符号 else
Command exited with code 1

🔧 解决方案：
1. 尝试更强的分析器：-t lr1
2. 检查文法是否存在二义性
3. 重新设计文法以消除冲突
```

#### 5. 输入串分析失败
```bash
$ ./lr_cli.exe ../TestGrammar/example_grammar.txt -s "invalid_symbol"
构造SLR(1)分析表...
语法分析过程：
步骤    状态栈  符号栈          输入            动作
0       0                       invalid_symbol $                错误：无对应动作
✗ 分析失败

🔧 解决方案：
1. 检查输入串中的符号是否都在文法中定义
2. 确保输入串符合文法的语法规则
3. 使用空格正确分隔符号
```

#### 6. JSON输出格式问题
```bash
# JSON输出可能包含分析过程信息，需要适当解析
$ ./lr_cli.exe ../TestGrammar/example_grammar.txt -s "a + a" --json

🔧 建议：
1. JSON输出中可能包含混合的文本和JSON格式
2. 提取JSON部分进行解析
3. 使用--quiet模式（如果可用）减少额外输出
```

## 🔗 自动化集成

### 专业测试脚本系统

项目提供了完整的自动化测试系统，可以全面验证lr_cli的功能：

#### 测试脚本功能概览
- **全覆盖测试**: 测试所有文法文件和分析器组合
- **智能分类**: 自动区分简单文法、复杂文法、错误文法
- **性能测试**: 包含压力测试和性能基准测试
- **对比分析**: 三种分析器的详细对比
- **详细报告**: 生成完整的测试报告和日志

#### 快速测试验证
```bash
# 验证lr_cli基本功能（推荐新手）
cd ../TestGrammar
./test_all_grammars.sh simple

# 完整功能测试（推荐正式验证）
./test_all_grammars.sh all

# 特定文法的深入测试
./test_all_grammars.sh comparison example_grammar.txt
```

#### 高级测试功能
```bash
# 压力测试（测试复杂文法处理能力）
./test_all_grammars.sh stress

# Windows用户
.\test_all_grammars.bat
```

> 📚 **完整测试文档**: 
> - 自动化测试系统的详细功能和使用方法：[TestGrammar/SCRIPT_DOCUMENTATION.md](../TestGrammar/SCRIPT_DOCUMENTATION.md)
> - 测试文法集合说明：[TestGrammar/README.md](../TestGrammar/README.md)

### 程序集成示例

#### 🏆 JSON 输出最佳实践

**1. 静默模式特性**
- JSON模式下自动启用静默模式，输出纯净的JSON数据
- 无需额外配置，适合脚本和程序直接调用
- 支持标准输出重定向和管道操作

**2. 错误处理**
```bash
# 使用错误码判断执行结果
if ./lr_cli.exe grammar.txt --json > result.json 2>error.log; then
    echo "分析成功，结果保存在 result.json"
else
    echo "分析失败，错误信息保存在 error.log"
fi
```

**3. 输出解析**
JSON输出格式稳定可靠，支持以下解析模式：
- **分析结果**: `parse_result.success` (boolean)
- **输入字符串**: `parse_result.input` (string)
- **分析表数据**: `action_table` 和 `goto_table` (object)

#### Python脚本集成
```python
#!/usr/bin/env python3
import subprocess
import json
import sys

class LRAnalyzer:
    def __init__(self, cli_path="./lr_cli"):
        self.cli_path = cli_path
    
    def analyze_grammar(self, grammar_file, algorithm="slr1", input_string=None):
        """调用lr_cli分析文法"""
        cmd = [self.cli_path, grammar_file, "-t", algorithm, "--json"]
        
        if input_string:
            cmd.extend(["-s", input_string])
        else:
            cmd.append("--table")
        
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, 
                                  encoding='utf-8', timeout=30)
            
            if result.returncode == 0:
                return json.loads(result.stdout)
            else:
                return {"error": result.stderr, "success": False}
                
        except subprocess.TimeoutExpired:
            return {"error": "分析超时", "success": False}
        except json.JSONDecodeError as e:
            return {"error": f"JSON解析失败: {e}", "success": False}
    
    def batch_test(self, grammar_files, algorithms=["lr0", "slr1", "lr1"]):
        """批量测试多个文法和算法"""
        results = {}
        
        for grammar in grammar_files:
            results[grammar] = {}
            for algo in algorithms:
                print(f"测试 {grammar} 使用 {algo}...")
                results[grammar][algo] = self.analyze_grammar(grammar, algo)
        
        return results

# 使用示例
if __name__ == "__main__":
    analyzer = LRAnalyzer("../Algorithm/lr_cli")
    
    # 单个文法测试
    result = analyzer.analyze_grammar("../TestGrammar/example_grammar.txt", "slr1")
    if result.get("success"):
        print(f"✅ 分析成功，状态数：{len(result.get('action_table', {}))}")
    else:
        print(f"❌ 分析失败：{result.get('error')}")
    
    # 批量测试
    grammars = ["../TestGrammar/example_grammar.txt", 
                "../TestGrammar/assignment_grammar.txt"]
    batch_results = analyzer.batch_test(grammars)
    
    # 输出结果统计
    for grammar, results in batch_results.items():
        success_count = sum(1 for r in results.values() if r.get("success"))
        print(f"{grammar}: {success_count}/3 分析器成功")
```

## 📖 文档资源

### 相关文档链接

- **[后端代码架构](./BACKEND_CODE_OVERVIEW.md)**: 深入了解lr_cli的实现原理
- **[自动化测试文档](../TestGrammar/SCRIPT_DOCUMENTATION.md)**: 完整的测试脚本使用指南
- **[测试文法说明](../TestGrammar/README.md)**: 测试文法集合的详细说明
- **[GUI用户指南](../GUI/USER_GUIDE_BEST_PRACTICES.md)**: 图形界面使用指南