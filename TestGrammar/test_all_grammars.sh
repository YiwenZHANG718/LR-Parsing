#!/bin/bash
# LR语法分析器测试脚本 - 简化版
# 测试正确文法和错误文法的基本处理能力

# 测试所有核心文法，确保分析器的基本功能正常
echo "==================================="
echo "  LR语法分析器测试脚本"
echo "==================================="
echo

# 检查lr_cli可执行文件
if [ ! -f "../Algorithm/lr_cli" ]; then
    echo "错误: 找不到lr_cli可执行文件"
    echo "请先在Algorithm目录中运行 make lr_cli"
    exit 1
fi

CLI="../Algorithm/lr_cli"
LOG_FILE="test_results.log"

# 清空日志文件
> $LOG_FILE

echo "测试开始时间: $(date)" | tee -a $LOG_FILE
echo | tee -a $LOG_FILE

# 压力测试计数器
total_tests=0
successful_tests=0
failed_tests=0

# 测试函数
test_grammar() {
    local grammar_file="$1"
    local test_name="$2"
    local analyzer_type="$3"
    local input_string="$4"
    
    ((total_tests++))
    
    echo "----------------------------------------" | tee -a $LOG_FILE
    echo "测试 #$total_tests: $test_name" | tee -a $LOG_FILE
    echo "文法: $grammar_file" | tee -a $LOG_FILE
    echo "分析器: $analyzer_type" | tee -a $LOG_FILE
    
    # 构造分析表测试
    echo ">> 构造分析表..." | tee -a $LOG_FILE
    
    if $CLI "$grammar_file" -t "$analyzer_type" --table --json > /dev/null 2>&1; then
        echo "✓ 分析表构造成功" | tee -a $LOG_FILE
        ((successful_tests++))
        
        # 如果有输入串，测试解析
        if [ -n "$input_string" ]; then
            echo ">> 测试输入串: $input_string" | tee -a $LOG_FILE
            if $CLI "$grammar_file" -t "$analyzer_type" -s "$input_string" > /dev/null 2>&1; then
                echo "✓ 输入串解析成功" | tee -a $LOG_FILE
            else
                echo "✗ 输入串解析失败" | tee -a $LOG_FILE
            fi
        fi
        
    else
        echo "✗ 分析表构造失败" | tee -a $LOG_FILE
        ((failed_tests++))
        
        # 显示错误信息
        echo "错误详情:" | tee -a $LOG_FILE
        $CLI "$grammar_file" -t "$analyzer_type" --table 2>&1 | head -3 | sed 's/^/  /' | tee -a $LOG_FILE
    fi
    echo | tee -a $LOG_FILE
}

# 智能选择分析器的函数
smart_analyzer_selection() {
    local grammar_file="$1"
    
    # 根据文法复杂度和特征智能选择分析器
    local rule_count=$(grep -c " -> " "$grammar_file")
    
    if [ $rule_count -gt 50 ]; then
        echo "lr1"  # 复杂文法使用 LR(1)
    elif [[ "$grammar_file" =~ conflict ]] || [[ "$grammar_file" =~ ambiguous ]]; then
        echo "lr1"  # 冲突相关文法使用 LR(1)
    else
        echo "slr1" # 其他使用 SLR(1)
    fi
}

# 测试正确文法
echo "=======================================" | tee -a $LOG_FILE
echo "           正确文法测试   " | tee -a $LOG_FILE
echo "=======================================" | tee -a $LOG_FILE

# 自动发现并测试所有 *_grammar.txt 文件
grammar_count=0
for grammar_file in *_grammar.txt; do
    if [ -f "$grammar_file" ]; then
        # 智能选择分析器
        analyzer=$(smart_analyzer_selection "$grammar_file")
        
        # 根据文法名称选择合适的测试输入和描述
        case "$grammar_file" in
            "example_grammar.txt")
                test_grammar "$grammar_file" "基本算术表达式文法" "$analyzer" "a + a * a"
                ;;
            "assignment_grammar.txt")
                test_grammar "$grammar_file" "赋值语句文法" "$analyzer" "id = id + num"
                ;;
            "conditional_grammar.txt")
                test_grammar "$grammar_file" "条件语句文法" "$analyzer" "if id then id = num else id = id"
                ;;
            "expression_grammar.txt")
                test_grammar "$grammar_file" "表达式文法变体" "$analyzer" "a + a * a"
                ;;
            "complex_grammar.txt")
                test_grammar "$grammar_file" "复杂C语言文法" "$analyzer" "int id ( ) { if ( id ) return num ; }"
                ;;
            "ultra_long_grammar.txt")
                test_grammar "$grammar_file" "超长无冲突文法" "slr1" "var id : bool ;"
                ;;
            "classic_lr1_grammar.txt")
                test_grammar "$grammar_file" "经典LR(1)测试文法" "lr1" "id = * id"
                ;;
            "if_else_language_grammar.txt")
                test_grammar "$grammar_file" "带if-else的编程语言文法" "slr1" "if ( id > number ) { id = number ; } else { id = false ; }"
                ;;
            *"ambiguous"*"grammar.txt")
                # 二义性文法测试
                test_grammar "$grammar_file" "二义性文法测试 ($(basename "$grammar_file" .txt))" "lr1"
                ;;
            *)
                # 其他文法使用智能选择的分析器
                rule_count=$(grep -c " -> " "$grammar_file" 2>/dev/null || echo "0")
                if [ $rule_count -gt 30 ]; then
                    description="复杂文法 ($(basename "$grammar_file" .txt), $rule_count 规则)"
                else
                    description="文法测试 ($(basename "$grammar_file" .txt), $rule_count 规则)"
                fi
                test_grammar "$grammar_file" "$description" "$analyzer"
                ;;
        esac
        ((grammar_count++))
    fi
done

echo "已测试 $grammar_count 个正确文法文件" | tee -a $LOG_FILE
echo | tee -a $LOG_FILE

# 测试不同分析器对同一文法的处理
echo "=======================================" | tee -a $LOG_FILE
echo "        分析器对比测试" | tee -a $LOG_FILE
echo "=======================================" | tee -a $LOG_FILE

# 选择一个基础文法进行对比测试
if [ -f "example_grammar.txt" ]; then
    base_grammar="example_grammar.txt"
    test_name="基本算术表达式文法"
elif [ -f "assignment_grammar.txt" ]; then
    base_grammar="assignment_grammar.txt"
    test_name="赋值语句文法"
else
    # 选择第一个可用的文法文件
    base_grammar=$(ls *_grammar.txt 2>/dev/null | head -1)
    test_name="$(basename "$base_grammar" .txt)"
fi

if [ -f "$base_grammar" ]; then
    echo "使用 $base_grammar 进行分析器对比测试" | tee -a $LOG_FILE
    for analyzer in lr0 slr1 lr1; do
        test_grammar "$base_grammar" "$test_name ($analyzer)" "$analyzer" "a + a * a"
    done
else
    echo "未找到合适的文法文件进行对比测试" | tee -a $LOG_FILE
fi

# 测试错误文法
echo "=======================================" | tee -a $LOG_FILE
echo "           错误文法测试" | tee -a $LOG_FILE
echo "=======================================" | tee -a $LOG_FILE

# 自动发现并测试所有 FaultGrammar/*.txt 文件
fault_count=0
for fault_file in FaultGrammar/*.txt; do
    if [ -f "$fault_file" ]; then
        fault_name=$(basename "$fault_file" .txt)
        # 根据文件名确定错误类型
        case "$fault_name" in
            *"recursive"*)
                test_grammar "$fault_file" "递归错误文法 ($fault_name)" "slr1"
                ;;
            *"conflict"*)
                test_grammar "$fault_file" "冲突错误文法 ($fault_name)" "slr1"
                ;;
            *"error"*)
                test_grammar "$fault_file" "格式错误文法 ($fault_name)" "slr1"
                ;;
            "empty"*)
                test_grammar "$fault_file" "空文法错误 ($fault_name)" "slr1"
                ;;
            *)
                test_grammar "$fault_file" "错误文法 ($fault_name)" "slr1"
                ;;
        esac
        ((fault_count++))
    fi
done

echo "已测试 $fault_count 个错误文法文件" | tee -a $LOG_FILE
echo | tee -a $LOG_FILE

# 复杂文法压力测试
echo "=======================================" | tee -a $LOG_FILE
echo "         复杂文法压力测试" | tee -a $LOG_FILE
echo "=======================================" | tee -a $LOG_FILE

# 测试complex_grammar.txt
if [ -f "complex_grammar.txt" ]; then
    echo ">> 测试复杂C语言文法..." | tee -a $LOG_FILE
    
    # 基础测试
    test_grammar "complex_grammar.txt" "复杂C语言文法 (SLR1)" "slr1" "int id ( ) { return num ; }"
    test_grammar "complex_grammar.txt" "复杂C语言文法 (LR1)" "lr1" "int id ( ) { return num ; }"
    
    # 压力测试：多种复杂输入串
    echo ">> 复杂输入串压力测试..." | tee -a $LOG_FILE
    complex_inputs=(
        "int id ( ) { return num ; }"
        "int id ( int id ) { if ( id ) return id ; else return num ; }"
        "int id ( ) { while ( id < num ) { id = id + num ; } return id ; }"
        "int id ( int id , int id ) { if ( id > id ) { return id ; } return num ; }"
        "int id ( ) { for ( id = num ; id < num ; id = id + num ) { return id ; } }"
    )
    
    for input in "${complex_inputs[@]}"; do
        ((total_tests++))
        echo "----------------------------------------" | tee -a $LOG_FILE
        echo "测试 #$total_tests: 复杂C语言文法复杂输入测试" | tee -a $LOG_FILE
        echo "文法: complex_grammar.txt" | tee -a $LOG_FILE
        echo "分析器: slr1" | tee -a $LOG_FILE
        echo ">> 测试输入串: $input" | tee -a $LOG_FILE
        if $CLI "complex_grammar.txt" -t slr1 -s "$input" > /dev/null 2>&1; then
            echo "✓ 输入串解析成功" | tee -a $LOG_FILE
            ((successful_tests++))
        else
            echo "✗ 输入串解析失败" | tee -a $LOG_FILE
            ((failed_tests++))
        fi
        echo | tee -a $LOG_FILE
    done
    
    # 连续构造测试
    echo ">> 连续构造压力测试..." | tee -a $LOG_FILE
    for ((i=1; i<=10; i++)); do
        ((total_tests++))
        echo "----------------------------------------" | tee -a $LOG_FILE
        echo "测试 #$total_tests: 复杂C语言文法连续构造测试 ($i/10)" | tee -a $LOG_FILE
        echo "文法: complex_grammar.txt" | tee -a $LOG_FILE
        echo "分析器: slr1" | tee -a $LOG_FILE
        echo ">> 构造分析表..." | tee -a $LOG_FILE
        if $CLI "complex_grammar.txt" -t slr1 --table --json > /dev/null 2>&1; then
            echo "✓ 分析表构造成功" | tee -a $LOG_FILE
            ((successful_tests++))
        else
            echo "✗ 分析表构造失败" | tee -a $LOG_FILE
            ((failed_tests++))
        fi
        echo | tee -a $LOG_FILE
    done
else
    echo ">> 未找到complex_grammar.txt文件" | tee -a $LOG_FILE
fi

# 测试ultra_long_grammar.txt
if [ -f "ultra_long_grammar.txt" ]; then
    echo ">> 测试超长无冲突文法..." | tee -a $LOG_FILE
    line_count=$(wc -l < "ultra_long_grammar.txt")
    rule_count=$(grep -c " -> " "ultra_long_grammar.txt")
    echo "文法规模: $line_count 行, $rule_count 个产生式" | tee -a $LOG_FILE
    
    # 基础测试
    test_grammar "ultra_long_grammar.txt" "超长无冲突文法 (SLR1)" "slr1" "var id : bool ;"
    test_grammar "ultra_long_grammar.txt" "超长无冲突文法 (LR1)" "lr1" "var id : bool ;"
    
    # 压力测试：多种复杂输入串
    echo ">> 超长文法复杂输入串压力测试..." | tee -a $LOG_FILE
    ultra_inputs=(
        "var id : bool ;"
        "const id = decimal ;"
        "import id ; var id : int8 = true ;"
        "func id ( ) { return decimal ; }"
        "var id : array [ decimal ] of bool = [ true , false ] ;"
        "import id ; const id = true ; func id ( ) { return id ; }"
        "var id : map [ string ] int16 = nil ;"
        "func id ( id : string , id : int32 ) : bool { return false ; }"
    )
    
    for input in "${ultra_inputs[@]}"; do
        ((total_tests++))
        echo "----------------------------------------" | tee -a $LOG_FILE
        echo "测试 #$total_tests: 超长无冲突文法复杂输入测试" | tee -a $LOG_FILE
        echo "文法: ultra_long_grammar.txt" | tee -a $LOG_FILE
        echo "分析器: slr1" | tee -a $LOG_FILE
        echo ">> 测试输入串: $input" | tee -a $LOG_FILE
        if timeout 10s $CLI "ultra_long_grammar.txt" -t slr1 -s "$input" > /dev/null 2>&1; then
            echo "✓ 输入串解析成功" | tee -a $LOG_FILE
            ((successful_tests++))
        else
            echo "✗ 输入串解析失败或超时" | tee -a $LOG_FILE
            ((failed_tests++))
        fi
        echo | tee -a $LOG_FILE
    done
    
    # 快速连续测试
    echo ">> 超长文法快速连续测试..." | tee -a $LOG_FILE
    for ((i=1; i<=5; i++)); do
        ((total_tests++))
        echo "----------------------------------------" | tee -a $LOG_FILE
        echo "测试 #$total_tests: 超长无冲突文法快速连续测试 ($i/5)" | tee -a $LOG_FILE
        echo "文法: ultra_long_grammar.txt" | tee -a $LOG_FILE
        echo "分析器: slr1" | tee -a $LOG_FILE
        echo ">> 构造分析表..." | tee -a $LOG_FILE
        if timeout 15s $CLI "ultra_long_grammar.txt" -t slr1 --table --json > /dev/null 2>&1; then
            echo "✓ 分析表构造成功" | tee -a $LOG_FILE
            ((successful_tests++))
        else
            echo "✗ 分析表构造失败或超时" | tee -a $LOG_FILE
            ((failed_tests++))
        fi
        echo | tee -a $LOG_FILE
    done
    
    # 不同分析器稳定性测试
    echo ">> 多分析器稳定性测试..." | tee -a $LOG_FILE
    for analyzer in lr0 slr1 lr1; do
        for ((i=1; i<=3; i++)); do
            ((total_tests++))
            echo "----------------------------------------" | tee -a $LOG_FILE
            echo "测试 #$total_tests: 超长无冲突文法多分析器稳定性测试 ($analyzer $i/3)" | tee -a $LOG_FILE
            echo "文法: ultra_long_grammar.txt" | tee -a $LOG_FILE
            echo "分析器: $analyzer" | tee -a $LOG_FILE
            echo ">> 构造分析表..." | tee -a $LOG_FILE
            if timeout 20s $CLI "ultra_long_grammar.txt" -t "$analyzer" --table --json > /dev/null 2>&1; then
                echo "✓ 分析表构造成功" | tee -a $LOG_FILE
                ((successful_tests++))
            else
                echo "✗ 分析表构造失败或超时" | tee -a $LOG_FILE
                ((failed_tests++))
            fi
            echo | tee -a $LOG_FILE
        done
    done
else
    echo ">> 未找到ultra_long_grammar.txt文件" | tee -a $LOG_FILE
fi

# 测试if_else_language_grammar.txt
if [ -f "if_else_language_grammar.txt" ]; then
    echo ">> 测试带if-else的编程语言文法..." | tee -a $LOG_FILE
    rule_count=$(grep -c " -> " "if_else_language_grammar.txt")
    echo "文法规模: $rule_count 个产生式，支持多种编程语言结构" | tee -a $LOG_FILE
    
    # 基础测试
    test_grammar "if_else_language_grammar.txt" "带if-else的编程语言文法 (SLR1)" "slr1" "if ( id > number ) { id = number ; }"
    test_grammar "if_else_language_grammar.txt" "带if-else的编程语言文法 (LR1)" "lr1" "if ( id > number ) { id = number ; }"
    
    # 压力测试：多种复杂输入串
    echo ">> if-else文法复杂输入串压力测试..." | tee -a $LOG_FILE
    ifelse_inputs=(
        "int id ;"
        "id [ number ] = number ;"
        "id ( number , id ) ;"
        "if ( id > number ) { id = number ; } else { id = false ; }"
        "for ( id = number number > id id = id + number ) { id = number ; }"
        "if ( id == true ) { id ( number ) ; } else { id [ number ] = false ; }"
    )
    
    for input in "${ifelse_inputs[@]}"; do
        ((total_tests++))
        echo "----------------------------------------" | tee -a $LOG_FILE
        echo "测试 #$total_tests: if-else编程语言文法复杂输入测试" | tee -a $LOG_FILE
        echo "文法: if_else_language_grammar.txt" | tee -a $LOG_FILE
        echo "分析器: slr1" | tee -a $LOG_FILE
        echo ">> 测试输入串: $input" | tee -a $LOG_FILE
        if timeout 10s $CLI "if_else_language_grammar.txt" -t slr1 -s "$input" > /dev/null 2>&1; then
            echo "✓ 输入串解析成功" | tee -a $LOG_FILE
            ((successful_tests++))
        else
            echo "✗ 输入串解析失败或超时" | tee -a $LOG_FILE
            ((failed_tests++))
        fi
        echo | tee -a $LOG_FILE
    done
    
    # 连续构造测试
    echo ">> if-else文法连续构造测试..." | tee -a $LOG_FILE
    for ((i=1; i<=5; i++)); do
        ((total_tests++))
        echo "----------------------------------------" | tee -a $LOG_FILE
        echo "测试 #$total_tests: if-else编程语言文法连续构造测试 ($i/5)" | tee -a $LOG_FILE
        echo "文法: if_else_language_grammar.txt" | tee -a $LOG_FILE
        echo "分析器: slr1" | tee -a $LOG_FILE
        echo ">> 构造分析表..." | tee -a $LOG_FILE
        if timeout 10s $CLI "if_else_language_grammar.txt" -t slr1 --table --json > /dev/null 2>&1; then
            echo "✓ 分析表构造成功" | tee -a $LOG_FILE
            ((successful_tests++))
        else
            echo "✗ 分析表构造失败或超时" | tee -a $LOG_FILE
            ((failed_tests++))
        fi
        echo | tee -a $LOG_FILE
    done
else
    echo ">> 未找到if_else_language_grammar.txt文件" | tee -a $LOG_FILE
fi

# 极限压力测试
echo "=======================================" | tee -a $LOG_FILE
echo "           极限压力测试" | tee -a $LOG_FILE
echo "=======================================" | tee -a $LOG_FILE
echo "测试所有可用文法的连续处理能力..." | tee -a $LOG_FILE

available_grammars=("example_grammar.txt" "assignment_grammar.txt" "conditional_grammar.txt" "complex_grammar.txt" "ultra_long_grammar.txt" "if_else_language_grammar.txt")

for grammar in "${available_grammars[@]}"; do
    if [ -f "$grammar" ]; then
        for ((i=1; i<=3; i++)); do
            ((total_tests++))
            echo "----------------------------------------" | tee -a $LOG_FILE
            echo "测试 #$total_tests: 极限压力测试 ($grammar $i/3)" | tee -a $LOG_FILE
            echo "文法: $grammar" | tee -a $LOG_FILE
            echo "分析器: slr1" | tee -a $LOG_FILE
            echo ">> 构造分析表..." | tee -a $LOG_FILE
            if timeout 30s $CLI "$grammar" -t slr1 --table --json > /dev/null 2>&1; then
                echo "✓ 分析表构造成功" | tee -a $LOG_FILE
                ((successful_tests++))
            else
                echo "✗ 分析表构造失败或超时" | tee -a $LOG_FILE
                ((failed_tests++))
            fi
            echo | tee -a $LOG_FILE
        done
    fi
done

echo | tee -a $LOG_FILE

# 测试总结
echo "=======================================" | tee -a $LOG_FILE
echo "           测试总结报告" | tee -a $LOG_FILE
echo "=======================================" | tee -a $LOG_FILE

echo "测试结束时间: $(date)" | tee -a $LOG_FILE
echo "=======================================" | tee -a $LOG_FILE
echo "测试统计:" | tee -a $LOG_FILE
echo "  总测试数: $total_tests" | tee -a $LOG_FILE
echo "  成功: $successful_tests" | tee -a $LOG_FILE
echo "  失败: $failed_tests" | tee -a $LOG_FILE
if [ $total_tests -gt 0 ]; then
    echo "  成功率: $(echo "scale=2; $successful_tests * 100 / $total_tests" | bc)%" | tee -a $LOG_FILE
fi
echo | tee -a $LOG_FILE

echo "详细日志文件: $LOG_FILE" | tee -a $LOG_FILE

echo | tee -a $LOG_FILE
echo "=======================================" | tee -a $LOG_FILE
echo "测试完成！检查日志文件获取详细信息。" | tee -a $LOG_FILE
echo "=======================================" | tee -a $LOG_FILE
