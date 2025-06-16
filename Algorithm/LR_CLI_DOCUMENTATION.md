# LR命令行工具 (lr_cli) 使用文档

## 概述

`lr_cli` 是LR语法分析器的命令行接口工具，提供了批处理友好的LR语法分析功能。它支持LR(0)、SLR(1)、LR(1)三种分析算法，可以构造分析表、分析输入串、显示项目集，并支持多种输出格式。

## 特性

### 核心功能
- **多种LR算法**：支持LR(0)、SLR(1)、LR(1)分析方法
- **分析表构造**：生成ACTION表和GOTO表
- **输入串分析**：完整的语法分析过程追踪
- **项目集显示**：可视化LR项目集族构造
- **多格式输出**：人类可读的文本格式和机器可读的JSON格式

### 技术特点
- **批处理友好**：适合脚本调用和自动化测试
- **跨平台支持**：Windows、Linux、macOS全平台兼容
- **静默模式**：JSON输出时自动屏蔽调试信息
- **错误检测**：智能识别语法冲突并提供解决建议

## 编译安装

### Windows (MSYS2/MinGW)
```powershell
cd Algorithm
./build.bat
#或手动编译
g++ -o lr_cli.exe lr_cli.cpp lr_analyzer.cpp grammar.cpp lr_item.cpp -std=c++11
```

### Linux/macOS
```bash
cd Algorithm
make lr_cli
# 或手动编译
g++ -o lr_cli lr_cli.cpp lr_analyzer.cpp grammar.cpp lr_item.cpp -std=c++11
```

### 验证安装
```bash
./lr_cli --help
```

## 命令语法

```
lr_cli <grammar_file> [options]
```

### 基本参数
- `<grammar_file>`：文法文件路径（必需）

### 选项参数

#### 分析器类型
```
-t, --type <type>     指定分析器类型
```
支持的类型：
- `lr0`：LR(0)分析器
- `slr1`：SLR(1)分析器（默认）
- `lr1`：LR(1)分析器

#### 功能选项
```
-s, --string <str>    分析指定的输入串
--table              显示ACTION和GOTO表
--items              显示项目集
--json               启用JSON格式输出
-h, --help           显示帮助信息
```

## 使用示例

### 基础用法

#### 构造SLR(1)分析表
```bash
./lr_cli ../TestGrammar/example_grammar.txt -t slr1 --table
```

#### 分析输入串
```bash
./lr_cli ../TestGrammar/example_grammar.txt -t slr1 -s "a + a * a"
```

#### 显示项目集
```bash
./lr_cli ../TestGrammar/example_grammar.txt -t slr1 --items
```

### 高级用法

#### JSON格式输出（适合程序调用）
```bash
./lr_cli ../TestGrammar/example_grammar.txt -t slr1 --table --json
```

#### 算法对比测试
```bash
# 测试LR(0)
./lr_cli ../TestGrammar/example_grammar.txt -t lr0 --table

# 测试SLR(1)
./lr_cli ../TestGrammar/example_grammar.txt -t slr1 --table

# 测试LR(1)
./lr_cli ../TestGrammar/example_grammar.txt -t lr1 --table
```

#### 组合功能使用
```bash
# 同时显示分析表和项目集
./lr_cli ../TestGrammar/example_grammar.txt -t slr1 --table --items

# 分析输入串并以JSON格式输出
./lr_cli ../TestGrammar/example_grammar.txt -t slr1 -s "a + a * a" --json
```

## 输出格式详解

### 文本格式输出

#### 分析表显示
```
ACTION表:
状态\符号    a    +    *    (    )    $
======================================
0          s5        s4             
1               s6             acc
2               r2   s7        r2
3               r4   r4        r4
4          s5        s4             
5               r6   r6        r6
6          s5        s4             
7          s5        s4             
8               r1   s7        r1
9               r3   r3        r3

GOTO表:
状态\符号    E    T    F
===================
0          1    2    3
4          8    2    3
6               9    3
7                    10
```

#### 分析过程显示
```
语法分析过程：
步骤	状态栈	符号栈		输入		动作
0	0 		        	a + a * a $ 	s5
1	0 5 	a 		+ a * a $ 	r6
2	0 3 	F 		+ a * a $ 	r4
3	0 2 	T 		+ a * a $ 	r2
4	0 1 	E 		+ a * a $ 	s6
...
13	0 1 	E 		$ 		acc

✓ 分析成功！输入串被接受。
```

#### 项目集显示
```
I0:
    E' -> .E
    E -> .E + T
    E -> .T
    T -> .T * F
    T -> .F
    F -> .( E )
    F -> .a

I1:
    E' -> E.
    E -> E.+ T
...
```

### JSON格式输出

#### ACTION表JSON
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
    }
  }
}
```

#### 项目集JSON
```json
{
  "type": "item_sets",
  "data": [
    {
      "id": 0,
      "items": [
        "E' -> .E",
        "E -> .E + T",
        "E -> .T"
      ]
    }
  ]
}
```

## 错误处理

### 文法错误
```bash
$ ./lr_cli invalid_grammar.txt -t slr1 --table
错误：文法格式错误
第3行：缺少 '->' 符号
E = E + T

建议：检查文法格式，每行应为 '左部 -> 右部' 格式
```

### 冲突检测
```bash
$ ./lr_cli conflict_grammar.txt -t lr0 --table
构造LR(0)分析表...
⚠ 发现冲突：
状态 2 在符号 '+' 上存在移进/归约冲突：
- 移进到状态 6
- 使用产生式 E -> T 进行规约

建议：尝试使用SLR(1)或LR(1)算法解决此冲突。
```

### 输入串错误
```bash
$ ./lr_cli grammar.txt -t slr1 -s "invalid_symbol"
错误：输入串包含未定义的符号 'invalid_symbol'
已定义的终结符：a, +, *, (, )

建议：检查输入串中的符号是否都在文法中定义
```

## 脚本集成

### 批处理脚本示例

#### Windows PowerShell
```powershell
# 测试多个文法文件
$grammars = @("example_grammar.txt", "expression_grammar.txt", "conditional_grammar.txt")
$algorithms = @("lr0", "slr1", "lr1")

foreach ($grammar in $grammars) {
    Write-Host "=== 测试文法: $grammar ==="
    foreach ($algo in $algorithms) {
        Write-Host "--- $algo 算法 ---"
        .\lr_cli.exe "..\TestGrammar\$grammar" -t $algo --table
        Write-Host ""
    }
}
```

#### Linux/macOS Bash
```bash
#!/bin/bash
# 自动化测试脚本
GRAMMARS=("example_grammar.txt" "expression_grammar.txt" "conditional_grammar.txt")
ALGORITHMS=("lr0" "slr1" "lr1")

for grammar in "${GRAMMARS[@]}"; do
    echo "=== 测试文法: $grammar ==="
    for algo in "${ALGORITHMS[@]}"; do
        echo "--- $algo 算法 ---"
        ./lr_cli "../TestGrammar/$grammar" -t "$algo" --table
        echo
    done
done
```

### JSON输出解析

#### Python解析示例
```python
import subprocess
import json

def analyze_grammar(grammar_file, algorithm="slr1"):
    """调用lr_cli并解析JSON结果"""
    cmd = ["./lr_cli", grammar_file, "-t", algorithm, "--table", "--json"]
    result = subprocess.run(cmd, capture_output=True, text=True, encoding='utf-8')
    
    if result.returncode == 0:
        try:
            data = json.loads(result.stdout)
            return data
        except json.JSONDecodeError:
            print("JSON解析失败")
            return None
    else:
        print(f"分析失败: {result.stderr}")
        return None

# 使用示例
data = analyze_grammar("../TestGrammar/example_grammar.txt", "slr1")
if data and data.get("type") == "action_table":
    print("ACTION表构造成功")
    print(f"状态数: {len(data['data'])}")
```

#### JavaScript解析示例
```javascript
const { exec } = require('child_process');

function analyzeGrammar(grammarFile, algorithm = 'slr1') {
    return new Promise((resolve, reject) => {
        const cmd = `./lr_cli ${grammarFile} -t ${algorithm} --table --json`;
        exec(cmd, (error, stdout, stderr) => {
            if (error) {
                reject(stderr);
                return;
            }
            
            try {
                const data = JSON.parse(stdout);
                resolve(data);
            } catch (e) {
                reject('JSON解析失败');
            }
        });
    });
}

// 使用示例
analyzeGrammar('../TestGrammar/example_grammar.txt', 'slr1')
    .then(data => {
        if (data.type === 'action_table') {
            console.log('ACTION表构造成功');
            console.log(`状态数: ${Object.keys(data.data).length}`);
        }
    })
    .catch(err => console.error('分析失败:', err));
```

## 性能优化

### 文法复杂度建议
```
LR(0):   适用于简单文法（< 10个产生式）
SLR(1):  适用于中等文法（< 50个产生式）
LR(1):   适用于复杂文法（< 200个产生式）
```

### 内存使用
```
LR(0):   最小内存占用
SLR(1):  中等内存占用
LR(1):   较大内存占用（约为SLR(1)的2-3倍）
```

### 处理时间
```
典型文法分析时间：
- 简单文法：< 1ms
- 中等文法：< 10ms  
- 复杂文法：< 100ms
```

## 故障排除

### 常见问题

#### 1. 可执行文件未找到
```bash
# Windows
$ .\lr_cli.exe
'lr_cli.exe' 不是内部或外部命令

解决：检查编译是否成功，文件是否存在
```

#### 2. 文法文件路径错误
```bash
$ ./lr_cli nonexistent.txt -t slr1 --table
错误：无法打开文件 'nonexistent.txt'

解决：检查文件路径，使用相对路径或绝对路径
```

#### 3. 编码问题
```bash
$ ./lr_cli chinese_grammar.txt -t slr1 --table
错误：文法解析失败

解决：确保文法文件使用UTF-8编码保存
```

#### 4. 算法不支持
```bash
$ ./lr_cli grammar.txt -t invalid --table
错误：不支持的分析器类型 'invalid'

解决：使用 lr0、slr1 或 lr1
```

### 调试技巧

#### 逐步分析
```bash
# 1. 先检查文法格式
cat ../TestGrammar/example_grammar.txt

# 2. 尝试简单算法
./lr_cli ../TestGrammar/example_grammar.txt -t lr0 --table

# 3. 如果失败，升级算法
./lr_cli ../TestGrammar/example_grammar.txt -t slr1 --table

# 4. 查看详细项目集
./lr_cli ../TestGrammar/example_grammar.txt -t slr1 --items
```

#### 输出重定向
```bash
# 保存输出到文件
./lr_cli grammar.txt -t slr1 --table > analysis_result.txt

# 同时显示和保存
./lr_cli grammar.txt -t slr1 --table | tee analysis_result.txt

# JSON输出保存
./lr_cli grammar.txt -t slr1 --table --json > analysis.json
```

## 版本信息

- **当前版本**：v2.0
- **最后更新**：2025年6月12日
- **兼容性**：C++11及以上
- **平台支持**：Windows、Linux、macOS

## 技术支持

### 获取帮助
```bash
./lr_cli --help
```

### 报告问题
如遇到问题，请提供：
1. 操作系统和版本
2. 编译器版本
3. 使用的文法文件内容
4. 完整的命令行和错误输出
5. 期望的结果

---

*本文档涵盖了lr_cli命令行工具的完整使用方法，适用于自动化测试、批处理分析和编程集成等场景。*
