# TestGrammar 文法测试集合

## 概述

TestGrammar目录包含了一套完整的LR语法分析器文法测试集合，用于验证和测试LR分析器的功能正确性和性能表现。

## 目录结构

```
TestGrammar/
├── README.md                    # 本说明文档
├── test_all_grammars.sh         # Linux自动测试脚本
├── test_all_grammars.bat        # Windows自动测试脚本
├── test_results.log             # 测试日志文件（运行后生成）
├── *_grammar.txt                # 正确文法文件集合
├── *_export.txt                 # 导出测试文件
└── FaultGrammar/                # 错误文法集合
    └── *.txt                    # 各种错误文法示例
```

## 文法集合说明

### 正确文法文件
- `*_grammar.txt` - 包含各种类型的正确文法，从基础算术表达式到复杂编程语言结构
- `*_export.txt` - 用于导出和特殊测试的文法文件

### 错误文法文件
- `FaultGrammar/` - 保存各种类型的错误文法，用于测试分析器的错误检测能力
  - 包括二义性文法、左递归、冲突文法、格式错误等

## 自动化测试

### 运行测试
```bash
# Linux系统
./test_all_grammars.sh

### 测试内容
- 正确文法的分析表构造验证
- 错误文法的错误检测验证
- 不同分析器的性能对比
- 复杂文法的压力测试

## 开发指南

### 添加新文法
- 正确文法：放在根目录，命名为 `*_grammar.txt`
- 错误文法：放在 `FaultGrammar/` 目录
- 导出测试：命名为 `*_export.txt`

### 文法格式
```
# 注释行以#开头
Start -> Expression
Expression -> Expression + Term | Term
Term -> Term * Factor | Factor
Factor -> ( Expression ) | id | number
```

---

*详细的文法特点和用途说明请参考各个.txt文件内的注释。*
