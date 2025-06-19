# TestGrammar 文法测试集合

## 📋 概述

TestGrammar目录包含了一套完整的LR语法分析器文法测试集合，用于验证和测试LR分析器的功能正确性和性能表现。该测试集合涵盖了从基础算术表达式到复杂编程语言结构的各种文法类型，以及各种错误情况的测试用例。

## 📁 目录结构

```
TestGrammar/
├── README.md                                   # 本说明文档
├── SCRIPT_DOCUMENTATION.md                     # 自动化测试脚本详细文档
├── test_all_grammars.sh                        # Linux/macOS自动测试脚本
├── test_results.log                            # 测试日志文件（运行后生成）
├── *_grammar.txt                               # 基础文法文件
├── *_complex_grammar.txt                       # 复杂语言文法文件
├── *_export.txt                                # 文法分析输出文件
└── 📁 FaultGrammar/                            # 错误文法集合
```


## 🚀 自动化测试系统

### 测试脚本使用
```bash
# 显示帮助信息
./test_all_grammars.sh --help

# 快速测试（简单文法 + 所有错误文法）
./test_all_grammars.sh simple

# 完整测试（所有文法）
./test_all_grammars.sh all

# 对比测试（三种分析器对比）
./test_all_grammars.sh comparison example_grammar.txt

# 压力测试（复杂文法性能测试）
./test_all_grammars.sh stress
```

> 📖 **详细文档**: 如需深入了解CLI接口的完整功能和使用方法，请参考 [SCRIPT_DOCUMENTATION.md](./SCRIPT_DOCUMENTATION.md)

## 🛠️ 开发者指南

### 添加新的正确文法
1. **文件命名**: 使用`*_grammar.txt`格式
2. **文件位置**: 放在TestGrammar根目录
3. **内容格式**: 遵循标准产生式格式
4. **测试集成**: 修改测试脚本中的测试用例

### 添加新的错误文法
1. **文件位置**: 放在`FaultGrammar/`目录
2. **命名建议**: 使用描述性名称（如`new_error_type.txt`）
3. **错误描述**: 在文件开头注释说明错误类型
4. **自动测试**: 会被自动包含在错误测试中

### 文法格式规范
```
# 注释行以#开头，会被忽略
# 文法概述
# 特点:
# 复杂度:
# 分析器兼容性:
# 典型输入示例：
#   - This is an example
#   - Add test INPUT here

文法 -> 具 体 内 容
左部 -> 右部
# 避免文法错误，相关案例详见 /FaultGrammar
```

### 测试脚本修改
如需为新文法添加特定的测试输入串，需要修改`test_all_grammars.sh`：

```bash
# 在脚本的case语句中添加
"your_new_grammar.txt")
    test_grammar "$grammar_file" "您的文法描述" "$analyzer" "test input string"
    ;;
```
> 📖 **详细文档**: 如需深入了解新的文法文件添加方法，请参考 [SCRIPT_DOCUMENTATION.md](./SCRIPT_DOCUMENTATION.md)

## 📖 相关文档

- **[SCRIPT_DOCUMENTATION.md](./SCRIPT_DOCUMENTATION.md)** - 自动化测试脚本的详细使用说明
- **[../Algorithm/LR_CLI_DOCUMENTATION.md](../Algorithm/LR_CLI_DOCUMENTATION.md)** - LR命令行工具完整文档
- **[../GUI/GUI_OVERVIEW.md](../GUI/GUI_OVERVIEW.md)** - GUI界面完整概览
