# LR语法分析器自动化测试脚本完整说明文档

## 📋 概述

本文档详细说明了 `test_all_grammars.sh` 自动化测试脚本的使用方法、功能特性和技术细节。该脚本是为LR语法分析器设计的全面测试工具，支持多种测试模式、智能分析器选择，并提供完整的测试覆盖和性能分析功能。

## ℹ️ 脚本基本信息

- **脚本名称**: `test_all_grammars.sh`
- **支持的分析器**: LR(0), SLR(1), LR(1)
- **测试覆盖**: 正确文法、错误文法、冲突检测、性能测试、JSON输出验证
- **依赖工具**: bc (计算器), grep, sed, awk, timeout, tee

## 🚀 执行方法

### 1. 基本用法

```bash
# 显示详细帮助信息
./test_all_grammars.sh --help
./test_all_grammars.sh -h

# 运行默认测试（完整测试模式）
./test_all_grammars.sh
./test_all_grammars.sh all

# 快速权限设置（如果需要）
chmod +x test_all_grammars.sh
```

### 2. 高级用法技巧

#### 2.1 输出重定向和过滤 📝
```bash
# 只显示失败的测试结果
./test_all_grammars.sh simple 2>&1 | grep -A 10 -B 2 "错误文法\|FaultGrammar"

# 将输出同时保存到文件和显示在终端
./test_all_grammars.sh all | tee my_test_results.log

# 追加输出到现有日志文件
./test_all_grammars.sh simple >> existing_log.txt

# 只保存到文件，不在终端显示
./test_all_grammars.sh all > silent_test.log 2>&1
```

#### 2.2 超时控制 ⏱️
```bash
# 为整个测试脚本设置总超时时间（30分钟）
timeout 1800s ./test_all_grammars.sh all

# 设置较短超时用于快速验证（5分钟）
timeout 300s ./test_all_grammars.sh simple

```

#### 2.3 结果分析和统计 📊
```bash
# 查看测试日志的前10行
head -10 test_results.log

# 查看测试日志的后20行
tail -20 test_results.log

# 实时监控测试进度（类似tail -f）
tail -f test_results.log

# 统计成功测试数量
grep -c "✓" test_results.log

# 统计失败测试数量
grep -c "✗" test_results.log

# 查找特定文法的测试结果
grep -A 5 -B 2 "example_grammar.txt" test_results.log

# 提取所有错误信息
grep -A 3 "错误详情" test_results.log
```

#### 2.4 条件执行和自动化 🔄
```bash
# 仅在简单测试通过后才运行完整测试
./test_all_grammars.sh simple && ./test_all_grammars.sh all

# 运行多个测试模式，忽略失败
./test_all_grammars.sh simple; ./test_all_grammars.sh stress

# 后台运行测试（适合长时间压力测试）
nohup ./test_all_grammars.sh stress > stress_test.log 2>&1 &

# 检查后台任务状态
jobs
ps aux | grep test_all_grammars.sh
```

#### 2.5 日志管理和分析 📋
```bash
# 带时间戳的日志记录
./test_all_grammars.sh all | while read line; do echo "$(date '+%Y-%m-%d %H:%M:%S') $line"; done

# 压缩保存历史测试日志
tar -czf test_logs_$(date +%Y%m%d).tar.gz test_results.log

# 比较两次测试结果
diff old_test_results.log test_results.log

# 生成测试报告摘要
echo "=== 测试摘要 ===" > summary.txt
echo "测试时间: $(date)" >> summary.txt
echo "成功次数: $(grep -c '✓' test_results.log)" >> summary.txt
echo "失败次数: $(grep -c '✗' test_results.log)" >> summary.txt
```

### 3. 测试模式详解

#### 3.1 简单测试模式 ⚡
```bash
./test_all_grammars.sh simple
```

**功能**: 快速验证核心功能，测试所有文法（包括复杂文法）和错误文法  
**适用场景**: 日常开发验证、快速回归测试、持续集成  

**测试内容**:
- **所有正确文法**: 包括基础文法和复杂文法
- **所有错误文法**: 测试FaultGrammar/目录下的所有错误文法
- **智能分析器选择**: 根据文法特征自动选择合适的分析器

**性能特点**: 简单测试模式对复杂文法使用优化的测试策略，平衡测试覆盖和执行速度

#### 3.2 对比测试模式 📊
```bash
./test_all_grammars.sh comparison <文法文件>
```

**示例**:
```bash
./test_all_grammars.sh comparison example_grammar.txt
./test_all_grammars.sh comparison c_language_complex_grammar.txt
```

**功能**: 使用三种分析器(LR0, SLR1, LR1)对比测试指定文法  
**适用场景**: 算法性能对比、教学演示、文法复杂度分析

**测试流程**:
1. **文法信息分析**: 显示文法规模（行数、产生式数量）
2. **三种分析器对比**: LR(0), SLR(1), LR(1)分别测试
3. **智能输入串选择**: 根据文法类型选择合适的测试输入
4. **性能对比测试**: 测试连续构造性能（每种分析器5次构造）

**智能输入串映射**:
- `example_grammar.txt` → `"a + a * a"`
- `assignment_grammar.txt` → `"id = id + num"`
- `conditional_grammar.txt` → `"if id then id = num else id = id"`
- `c_language_complex_grammar.txt` → `"int id ( ) { return num ; }"`
- `programming_language_complex_grammar.txt` → `"if ( id > number ) { id = number ; }"`
- `modular_language_complex_grammar.txt` → `"var id : bool ;"`

#### 3.3 压力测试模式 🔥
```bash
./test_all_grammars.sh stress
```

**功能**: 测试复杂文法和极限情况  
**适用场景**: 性能验证、稳定性测试、内存泄漏检测

**测试内容**:
1. **复杂文法自动发现**: 自动发现所有*_complex_grammar.txt文件
2. **连续构造测试**: 每个复杂文法连续构造10次
3. **极限压力测试**: 所有文法 × 三种分析器的组合测试
4. **超时保护**: 单次测试最多30-45秒超时

#### 3.4 完整测试模式 🎯
```bash
./test_all_grammars.sh all
```

**功能**: 运行所有测试项目，包括所有文法和分析器组合  
**适用场景**: 完整验证、发布前测试、全面回归测试

**测试覆盖**:
1. **所有正确文法测试**: 包括简单和复杂文法
2. **分析器对比测试**: 使用example_grammar.txt进行三种分析器对比
3. **所有错误文法测试**: FaultGrammar/目录下的所有错误文法
4. **复杂文法压力测试**: 调用压力测试模式的功能

## 智能分析器选择算法

脚本使用 `smart_analyzer_selection()` 函数智能选择合适的分析器：

```bash
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
2. **文件名特征**: 包含"conflict"、"lr1"或"ambiguous"的文法使用LR(1)
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
- **基础文法**: 必须以`_grammar.txt`结尾
- **复杂文法**: 必须以`_complex_grammar.txt`结尾（注意：复杂文法会被所有测试模式包含，包括简单测试模式）
- **示例**: 
  - `my_language_grammar.txt` - 基础文法，会被所有测试模式包含
  - `advanced_language_complex_grammar.txt` - 复杂文法，会被所有测试模式包含，简单测试使用优化策略

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

### 3. 添加复杂文法 🔧

复杂文法是指包含大量产生式、复杂语言结构或需要较长构造时间的文法。所有测试模式（包括简单测试）都会包含复杂文法，但简单测试使用优化策略以平衡测试覆盖和执行速度。

#### 步骤1：创建复杂文法文件
复杂文法必须使用_complex_grammar.txt结尾的命名

#### 步骤2：设计复杂文法的测试输入串
复杂文法需要更精心设计的测试输入串：

```bash
# 层次化的测试输入串设计
# 1. 最小测试输入（验证基本功能）
minimal_input="class A extends B { int main ( ) { return number ; } }"

# 2. 中等复杂度输入（验证核心特性）
medium_input="class Test extends Base { int calculate ( int x ) { if ( x > number ) { return x ; } else { return number ; } } }"

# 3. 完整复杂度输入（验证所有特性）
complex_input="class Calculator extends Object { int add ( int a , int b ) { return a + b ; } boolean check ( ) { while ( condition ) { result = obj . method ( arg1 , arg2 ) ; } return true ; } }"
```

#### 步骤3：在脚本中添加复杂文法支持
编辑 `test_all_grammars.sh`，在相应函数中添加复杂文法的测试逻辑：

```bash
# 在run_simple_tests()和run_full_tests()函数中添加：
"my_language_complex_grammar.txt")
    test_grammar "$grammar_file" "我的复杂编程语言文法" "$analyzer" "class Test extends Base { int main ( ) { return number ; } }"
    ;;

# 在run_comparison_tests()函数中添加：
*"my_language_complex"*)
    test_input="class Test extends Base { int main ( ) { return number ; } }"
    ;;
```

### 4. 添加自定义测试输入串

为了更好地测试新文法，您可以在脚本中添加特定的测试输入串。

#### 在简单测试和完整测试中添加
找到脚本中的 `run_simple_tests()` 和 `run_full_tests()` 函数，在case语句中添加：

```bash
# 在test_all_grammars.sh中添加
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

#### 在对比测试中添加输入串映射
在 `run_comparison_tests()` 函数中添加：

```bash
# 选择合适的测试输入串
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

### 5. 自定义分析器选择

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
        *"complex"*)
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

## 测试结果分析

### 输出格式

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

### 测试统计报告

每次测试完成后，脚本会显示详细统计信息：

```
=======================================
           测试总结报告
=======================================
测试结束时间: Wed Jun 19 20:13:50 CST 2025
测试模式: all
=======================================
测试统计:
  总测试数: 99
  成功: 70
  失败: 29
  成功率: 70.70% （由于存在错误文法，如果没有增删测试文法文件，此成功率是正确的）

详细日志文件: test_results.log
=======================================
   测试完成！检查日志文件获取详细信息
=======================================
```

### 压力测试输出示例

压力测试模式提供额外的性能分析：

```
>> 压力测试: c_language_complex_grammar.txt
文法规模: 45 个产生式
>> 连续构造压力测试...
测试 #15: 连续构造 c_language_complex_grammar.txt (1/10)
✓ 构造成功
测试 #16: 连续构造 c_language_complex_grammar.txt (2/10)
✓ 构造成功
...
>> 极限压力测试...
测试 #50: 极限压力 example_grammar.txt (lr0)
✓ 成功
```

### 对比测试详细输出

对比测试模式显示性能对比信息：

```
文法文件: c_language_complex_grammar.txt
文法规模: 52 行, 45 个产生式

╔════════════════════════════════════════╗
║          分析器对比测试: lr0            ║
╚════════════════════════════════════════╝
[测试结果...]

╔════════════════════════════════════════╗
║              性能对比测试               ║
╚════════════════════════════════════════╝
>> 测试 slr1 连续构造性能...
分析器: slr1
成功次数: 5/5
总耗时: 3秒

>> 测试 lr1 连续构造性能...
分析器: lr1
成功次数: 5/5
总耗时: 8秒
```

## 文件结构和日志管理

### 输入文件

- **正确文法文件**: `*_grammar.txt` (主目录)
- **错误文法文件**: `FaultGrammar/*.txt`
- **CLI可执行文件**: `../Algorithm/lr_cli`

### 输出文件

- **日志文件**: `test_results.log` - 详细的测试日志
- **终端输出**: 实时测试进度和结果摘要

### 日志文件内容

`test_results.log` 包含：
- 测试开始和结束时间
- 每个测试的详细执行过程
- 错误信息和失败原因
- 最终统计摘要

## 环境检查和错误处理

### 环境检查

脚本启动时会检查：
1. **CLI可执行文件**: 确认`../Algorithm/lr_cli`存在并可执行
2. **文法文件**: 验证指定文法文件可访问
3. **系统工具**: 检查`bc`计算器等必要工具
4. **权限检查**: 确保脚本有执行权限

### 超时保护机制

```bash
# 不同测试的超时设置
timeout 30s  $CLI "$grammar_file" -t lr1 --table  # 标准测试
timeout 45s  $CLI "$grammar_file" -t lr1 --table  # 极限测试
```

**超时策略**:
- **标准测试**: 30秒超时
- **极限压力测试**: 45秒超时
- **性能测试**: 30秒超时
- **整体脚本**: 可使用外部timeout控制

### 错误恢复机制

- **单测试失败隔离**: 单个测试失败不影响其他测试
- **详细日志记录**: 所有错误信息记录到日志文件
- **友好错误提示**: 提供具体的解决建议
- **自动跳过**: 自动跳过不存在的文法文件

## 故障排除

### 常见问题及解决方案

#### 1. CLI可执行文件不存在
```
错误: 找不到lr_cli可执行文件
请先在Algorithm目录中运行 make 或 make lr_cli
```
**解决方案**: 
```bash
cd ../Algorithm
make clean && make
# 或者
make lr_cli
```

#### 2. 文法文件不存在
```
错误: 找不到指定的文法文件: xxx.txt
```
**解决方案**: 
- 检查文件名和路径
- 查看脚本输出的可用文件列表
- 确保文件在正确的目录中

#### 3. 权限问题
```
Permission denied: ./test_all_grammars.sh
```
**解决方案**: 
```bash
chmod +x test_all_grammars.sh
```

#### 4. bc计算器缺失
```
command not found: bc
```
**解决方案**:
```bash
# Ubuntu/Debian
sudo apt-get install bc
# CentOS/RHEL
sudo yum install bc
# macOS
brew install bc
```

#### 5. 超时问题
如果测试经常超时，可能的原因：
- 文法过于复杂
- 系统性能不足
- 内存不足

**解决方案**:
```bash
# 只运行简单测试
./test_all_grammars.sh simple

# 使用更长的超时时间
timeout 1800s ./test_all_grammars.sh all

# 逐个测试复杂文法
./test_all_grammars.sh comparison c_language_complex_grammar.txt
```
