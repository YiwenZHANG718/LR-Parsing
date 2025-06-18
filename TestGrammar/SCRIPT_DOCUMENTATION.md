# LR语法分析器自动化测试脚本说明文档

## 概述

本文档详细说明了 `test_all_grammars.sh` 自动化测试脚本的使用方法、功能特性和技术细节。该脚本是为LR语法分析器设计的全面测试工具，支持多种测试模式和智能分析器选择。

## 脚本基本信息

- **脚本名称**: `test_all_grammars.sh`
- **版本**: 3.2
- **支持的分析器**: LR(0), SLR(1), LR(1)
- **测试覆盖**: 正确文法、错误文法、冲突检测、性能测试

## 执行方法

### 1. 基本用法

```bash
# 显示帮助信息
./test_all_grammars.sh --help
./test_all_grammars.sh -h

# 运行默认测试（完整测试模式）
./test_all_grammars.sh
./test_all_grammars.sh all
```

### 2. 测试模式详解

#### 2.1 简单测试模式
```bash
./test_all_grammars.sh simple
```

**功能**: 快速验证核心功能，测试基础文法和所有错误文法  
**适用场景**: 日常开发验证、快速回归测试  

#### 2.2 对比测试模式
```bash
./test_all_grammars.sh comparison <文法文件>
```

**示例**:
```bash
./test_all_grammars.sh comparison example_grammar.txt
./test_all_grammars.sh comparison ultra_long_grammar.txt
./test_all_grammars.sh comparison complex_grammar.txt
```

**功能**: 使用三种分析器(LR0, SLR1, LR1)对比测试指定文法  
**适用场景**: 分析器性能对比、文法复杂度分析   

#### 2.3 压力测试模式
```bash
./test_all_grammars.sh stress
```

**功能**: 测试复杂文法和极限情况  
**适用场景**: 性能验证、稳定性测试  

#### 2.4 完整测试模式
```bash
./test_all_grammars.sh all
```

**功能**: 运行所有测试项目  
**适用场景**: 完整验证、发布前测试  

## 简单测试模式详解

### 简单文法判断依据

简单测试模式通过以下规则判断文法是否属于"简单文法"：

#### 1. 排除规则（复杂文法）

基于文件名和文法特征，以下文法被认为是复杂文法，在简单模式中被跳过：

- **`complex_grammar.txt`**: 复杂C语言文法
  - 包含大量语言结构（函数、控制流、数据类型）
  - 产生式数量多，构造时间长
  - 通常包含50+个产生式规则
  
- **`ultra_long_grammar.txt`**: 超长无冲突文法
  - 产生式数量超过150个
  - 文法规模巨大，测试耗时长
  - 主要用于压力测试和性能基准测试
  
- **`if_else_language_grammar.txt`**: 带if-else的编程语言文法
  - 包含复杂的编程语言结构
  - 支持多种语句类型和表达式
  - 构造时间相对较长

#### 2. 包含规则（简单文法）

所有不在排除列表中的文法都被视为简单文法，包括但不限于：

- **基础算术文法**: 
  - `example_grammar.txt`: 基本的加法和乘法表达式
  - `expression_grammar.txt`: 表达式文法的变体版本
  
- **语言结构文法**: 
  - `assignment_grammar.txt`: 简单的赋值语句
  - `conditional_grammar.txt`: 基础的条件语句结构
  
- **分析器测试文法**: 
  - `classic_lr1_grammar.txt`: 经典的LR(1)测试用例
  - `lr1_test_grammar.txt`: LR(1)特性验证文法
  
- **冲突测试文法**: 
  - `shift_reduce_conflict_grammar.txt`: 移进-归约冲突示例
  - `reduce_reduce_conflict_grammar.txt`: 归约-归约冲突示例
  - `slr1_limitation_grammar.txt`: SLR(1)局限性演示

#### 3. 错误文法处理

简单测试模式包含 `FaultGrammar/` 目录下的所有错误文法：

- **格式错误**: 
  - `syntax_error.txt`: 产生式格式错误
  - `empty_grammar.txt`: 空文法文件
  - `no_start_symbol.txt`: 缺少起始符号
  
- **语义错误**: 
  - `left_recursive.txt`: 左递归文法
  - `unreachable_terminals.txt`: 不可达符号
  - `circular_dependency.txt`: 循环依赖
  
- **结构错误**: 
  - `ambiguous.txt`: 二义性文法
  - `shift_reduce_conflict.txt`: 移进-归约冲突
  - `reduce_reduce_conflict.txt`: 归约-归约冲突

### 智能分析器选择算法

脚本使用 `smart_analyzer_selection()` 函数智能选择合适的分析器：

```bash
# 分析器选择逻辑
function smart_analyzer_selection() {
    local grammar_file="$1"
    local rule_count=$(grep -c " -> " "$grammar_file")
    
    if [ $rule_count -gt 50 ]; then
        echo "lr1"    # 复杂文法使用LR(1)
    elif [[ "$grammar_file" =~ conflict ]] || [[ "$grammar_file" =~ lr1 ]] || [[ "$grammar_file" =~ ambiguous ]]; then
        echo "lr1"    # 冲突相关文法使用LR(1)
    else
        echo "slr1"   # 其他使用SLR(1)
    fi
}
```

**选择依据**:
1. **产生式数量**: 超过50个规则的文法使用LR(1)
2. **文件名特征**: 包含"conflict"或"ambiguous"的文法使用LR(1)
3. **默认选择**: 其他情况使用SLR(1)作为平衡选择

## 添加新文法到自动化测试

### 1. 添加正确文法

#### 步骤1：放置文法文件
```bash
# 将您的文法文件复制到TestGrammar目录
cp your_new_grammar.txt /path/to/LR/TestGrammar/your_new_grammar.txt
```

#### 步骤2：命名规范
确保文法文件使用标准命名格式：
- **文件名**: 必须以`_grammar.txt`结尾
- **示例**: `my_language_grammar.txt`, `assignment_grammar.txt`

#### 步骤3：文法格式
确保文法内容符合LR分析器要求的格式：
```
# 示例文法格式
S -> E
E -> E + T | T
T -> T * F | F  
F -> ( E ) | id
```

#### 步骤4：自动发现和测试
脚本会自动发现新文法：
```bash
# 运行简单测试（如果是基础文法）
./test_all_grammars.sh simple

# 或者针对特定文法进行对比测试
./test_all_grammars.sh comparison your_new_grammar.txt
```

### 2. 添加错误文法

#### 步骤1：放置到错误文法目录
```bash
# 将错误文法文件放入FaultGrammar目录
cp your_error_grammar.txt /path/to/LR/TestGrammar/FaultGrammar/
```

#### 步骤2：命名建议
使用描述性名称以便识别错误类型：
- `left_recursive_example.txt` - 左递归问题
- `format_error_test.txt` - 格式错误
- `circular_dependency_case.txt` - 循环依赖
- `ambiguous_if_else.txt` - 二义性问题

#### 步骤3：自动测试
错误文法会自动被简单测试和完整测试包含：
```bash
# 简单测试会包含所有FaultGrammar/下的文件
./test_all_grammars.sh simple
```

### 3. 添加自定义测试输入串

为了更好地测试新文法，您需要将测试输入串添加到脚本中：

#### 在简单测试模式中添加
找到脚本中的 `run_simple_tests()` 函数，在相应的case语句中添加您的文法：

```bash
# 在test_all_grammars.sh中的run_simple_tests()函数中添加
case "$grammar_file" in
    "example_grammar.txt")
        test_grammar "$grammar_file" "基本算术表达式文法" "$analyzer" "a + a * a"
        ;;
    "assignment_grammar.txt")
        test_grammar "$grammar_file" "赋值语句文法" "$analyzer" "id = id + num"
        ;;
    # 添加您的新文法
    "your_new_grammar.txt")
        test_grammar "$grammar_file" "您的文法描述" "$analyzer" "your test input string"
        ;;
    *)
        # 默认情况：没有指定输入串的文法
        rule_count=$(grep -c " -> " "$grammar_file" 2>/dev/null || echo "0")
        test_grammar "$grammar_file" "文法测试 ($(basename "$grammar_file" .txt), $rule_count 规则)" "$analyzer"
        ;;
esac
```

#### 在完整测试模式中添加
找到脚本中的 `run_full_tests()` 函数，在相应的case语句中添加类似的条目：

```bash
# 在test_all_grammars.sh中的run_full_tests()函数中添加
case "$grammar_file" in
    # ...existing cases...
    "your_new_grammar.txt")
        test_grammar "$grammar_file" "您的文法描述" "$analyzer" "your test input string"
        ;;
    # ...existing cases...
esac
```

#### 在对比测试模式中添加
找到脚本中的 `run_comparison_tests()` 函数，在选择测试输入串的部分添加：

```bash
# 在test_all_grammars.sh中的run_comparison_tests()函数中添加
case "$base_name" in
    *"example"*|*"expression"*)
        test_input="a + a * a"
        ;;
    *"assignment"*)
        test_input="id = id + num"
        ;;
    # 添加您的新文法
    *"your_new"*)
        test_input="your test input string"
        ;;
    *)
        test_input="id"  # 通用简单输入
        ;;
esac
```

### 4. 自定义分析器选择

如果新文法需要特定的分析器，可以修改智能选择逻辑：

```bash
# 在smart_analyzer_selection()函数中添加特定规则
function smart_analyzer_selection() {
    local grammar_file="$1"
    local rule_count=$(grep -c " -> " "$grammar_file")
    
    # 特定文法的分析器选择
    case "$grammar_file" in
        "your_special_grammar.txt")
            echo "lr1"  # 强制使用LR(1)
            return
            ;;
        *"complex"*|*"ultra_long"*)
            echo "lr1"  # 复杂文法使用LR(1)
            return
            ;;
    esac
    
    # 通用规则
    if [ $rule_count -gt 50 ]; then
        echo "lr1"
    elif [[ "$grammar_file" =~ conflict ]] || [[ "$grammar_file" =~ ambiguous ]]; then
        echo "lr1"
    else
        echo "slr1"
    fi
}
```

### 5. 复杂文法处理

如果您的文法非常复杂，需要特殊处理：

#### 步骤1：标记为复杂文法
在`run_simple_tests()`函数中将其添加到排除列表：

```bash
# 修改exclude_grammars数组
exclude_grammars=("complex_grammar.txt" "ultra_long_grammar.txt" "if_else_language_grammar.txt" "your_complex_grammar.txt")
```

#### 步骤2：添加超时保护
在测试函数调用中使用timeout：

```bash
# 示例：给复杂文法更长的超时时间
if timeout 60s $CLI "your_complex_grammar.txt" -t slr1 --table --json > /dev/null 2>&1; then
    echo "✓ 分析表构造成功"
else
    echo "✗ 分析表构造失败或超时"
fi
```

### 6. 验证新文法

添加新文法后，建议按以下步骤验证：

#### 手动验证
```bash
# 首先手动测试新文法
../Algorithm/lr_cli your_new_grammar.txt -t slr1 --table

# 如果成功，测试特定输入串
../Algorithm/lr_cli your_new_grammar.txt -t slr1 -s "your test input"
```

#### 自动化验证
```bash
# 运行简单测试，验证新文法被正确包含
./test_all_grammars.sh simple | grep "your_new_grammar"

# 或者单独对比测试
./test_all_grammars.sh comparison your_new_grammar.txt
```

### 7. 完整示例

#### 示例：添加声明语句文法

**步骤1**: 创建文法文件
```bash
cat > declaration_grammar.txt << EOF
# 声明语句文法
S -> DECL_LIST
DECL_LIST -> DECL_LIST DECL | DECL
DECL -> TYPE ID ;
TYPE -> int | bool | string
ID -> a | b | c | x | y | z
EOF
```

**步骤2**: 在脚本中添加测试输入（编辑 test_all_grammars.sh）
```bash
# 在run_simple_tests()和run_full_tests()函数的case语句中添加：
"declaration_grammar.txt")
    test_grammar "$grammar_file" "声明语句文法" "$analyzer" "int a ;"
    ;;
```

**步骤3**: 在对比测试中添加输入（编辑 test_all_grammars.sh）
```bash
# 在run_comparison_tests()函数的case语句中添加：
*"declaration"*)
    test_input="int a ;"
    ;;
```

**步骤4**: 测试新文法
```bash
# 运行对比测试
./test_all_grammars.sh comparison declaration_grammar.txt

# 验证在简单测试中被包含
./test_all_grammars.sh simple | grep "declaration_grammar"
```

## 测试结果分析

### 输出格式

脚本使用美观的Unicode边框格式显示测试结果：

```
───────────────────────────────────────
   测试 #1  : 基本算术表达式文法
   文法: example_grammar.txt
   分析器: slr1
───────────────────────────────────────
>> 构造分析表...
✓ 分析表构造成功
>> 测试输入串: a + a * a
✓ 输入串解析成功
```

### 测试统计

每次测试完成后，脚本会显示详细统计信息：

```
=======================================
           测试总结报告
=======================================
测试结束时间: Wed Jun 18 20:13:50 CST 2025
测试模式: all
=======================================
测试统计:
  总测试数: 93
  成功: 67
  失败: 26
  成功率: 72.04%    请注意，在没有增删文法的情况下，这个准确率是正常的

详细日志文件: test_results.log
```

## 文件结构说明

### 输入文件

- **正确文法文件**: `*_grammar.txt` (主目录)
- **错误文法文件**: `FaultGrammar/*.txt`
- **CLI可执行文件**: `../Algorithm/lr_cli`

### 输出文件

- **日志文件**: `test_results.log` - 详细的测试日志
- **终端输出**: 实时测试进度和结果摘要

## 测试覆盖范围

### 简单模式测试覆盖

| 文法类型 | 数量 | 说明 |
|---------|------|------|
| 基础正确文法 | ~9个 | 排除3个复杂文法的所有正确文法 |
| 错误文法 | ~9个 | FaultGrammar/目录下的所有错误文法 |
| **总计** | **~18个** | **快速验证核心功能** |

### 完整模式测试覆盖

| 测试类型 | 范围 | 说明 |
|---------|------|------|
| 正确文法测试 | 所有*_grammar.txt | 包括复杂文法 |
| 错误文法测试 | 所有FaultGrammar/*.txt | 错误检测验证 |
| 分析器对比 | 3种分析器 | LR0, SLR1, LR1对比 |
| 压力测试 | 复杂文法 | 连续构造、性能测试 |
| 极限测试 | 所有文法 | 稳定性验证 |

## 错误处理

### 环境检查

脚本启动时会检查：
1. **CLI可执行文件**: 确认`../Algorithm/lr_cli`存在
2. **文法文件**: 验证指定文法文件可访问
3. **系统工具**: 检查`bc`计算器等必要工具

### 错误恢复机制

- **单测试失败隔离**: 单个测试失败不影响其他测试
- **超时保护**: 防止无限等待和资源耗尽
- **详细日志记录**: 所有错误信息记录到日志文件
- **友好错误提示**: 提供具体的解决建议

## 性能优化

### 超时机制

```bash
# 不同复杂度的超时设置
timeout 10s  $CLI "simple_grammar.txt"     # 简单文法
timeout 30s  $CLI "complex_grammar.txt"    # 复杂文法
timeout 60s  $CLI "ultra_long_grammar.txt" # 超长文法
```

### 并发控制

- **串行执行**: 避免资源竞争和结果混淆
- **独立测试**: 每个测试使用独立的临时文件
- **资源清理**: 自动清理临时文件和进程

## 扩展和定制

### 添加新测试模式

可以在脚本中添加新的测试模式：

```bash
# 在parse_arguments()函数中添加新的case
newmode)
    TEST_MODE="newmode"
    shift
    ;;

# 在main()函数中添加新的case
newmode)
    run_new_mode_tests
    ;;
```

### 修改测试参数

可以通过修改脚本中的配置来调整：

```bash
# 修改排除的复杂文法列表
exclude_grammars=("complex_grammar.txt" "ultra_long_grammar.txt" "if_else_language_grammar.txt")

# 调整超时时间
timeout 30s $CLI "$grammar_file"

# 修改智能选择的阈值
if [ $rule_count -gt 50 ]; then
    echo "lr1"
```

## 故障排除

### 常见问题及解决方案

1. **CLI可执行文件不存在**
   ```
   错误: 找不到lr_cli可执行文件
   请先在Algorithm目录中运行 make 或 make lr_cli
   ```
   **解决**: 
   ```bash
   cd ../Algorithm
   make clean && make
   ```

2. **文法文件不存在**
   ```
   错误: 找不到指定的文法文件: xxx.txt
   ```
   **解决**: 检查文件名和路径，查看脚本提示的可用文件列表

3. **权限问题**
   ```
   Permission denied: ./test_all_grammars.sh
   ```
   **解决**: 
   ```bash
   chmod +x test_all_grammars.sh
   ```

4. **bc计算器缺失**
   ```
   command not found: bc
   ```
   **解决**:
   ```bash
   # Ubuntu/Debian
   sudo apt-get install bc
   # CentOS/RHEL
   sudo yum install bc
   ```

### 调试技巧

1. **查看详细日志**: 
   ```bash
   cat test_results.log | grep "错误\|失败"
   ```

2. **单独测试特定文法**: 
   ```bash
   ./test_all_grammars.sh comparison example_grammar.txt
   ```

3. **简化测试定位问题**: 
   ```bash
   ./test_all_grammars.sh simple
   ```

4. **手动运行CLI验证**:
   ```bash
   ../Algorithm/lr_cli example_grammar.txt -t slr1 --table
   ```

## 版本历史

| 版本 | 日期 | 主要更新 |
|------|------|----------|
| v3.2 | 2025-06 | 优化输出格式，完善简单测试模式 |
| v3.1 | 2025-06 | 添加对比测试和压力测试模式 |
| v3.0 | 2025-06 | 重构脚本架构，添加模块化设计 |
| v2.x | 2025-06 | 基础自动化测试功能 |
| v1.x | 2025-06 | 初始版本，基本测试能力 |

## 最佳实践

### 开发阶段
- 使用`simple`模式进行快速验证
- 修改代码后运行简单测试确保基本功能正常

### 测试阶段  
- 使用`comparison`模式对比分析器性能
- 针对特定文法进行深入测试

### 发布阶段
- 运行`all`模式进行完整回归测试
- 检查测试日志确保所有功能正常

---

**注意**: 本脚本需要在包含测试文法文件的目录中运行，并确保相对路径 `../Algorithm/lr_cli` 指向正确的CLI可执行文件。
