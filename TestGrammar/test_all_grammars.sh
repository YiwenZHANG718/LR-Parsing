#!/bin/bash
# LR语法分析器自动化测试脚本 - 高级版
# 支持多种测试模式：简单测试、压力测试、对比测试等
# @author: B22040310朱家骏
# 脚本版本和信息
SCRIPT_VERSION="3.2"
SCRIPT_NAME="test_all_grammars.sh"

# 显示帮助信息
show_help() {
    echo "======================================================================"
    echo "                LR语法分析器自动化测试脚本 v$SCRIPT_VERSION"
    echo "======================================================================"
    echo
    echo "用法: ./$SCRIPT_NAME [选项] [参数]"
    echo
    echo "测试模式:"
    echo "  simple             简单测试模式 - 测试所有文法的基本功能"
    echo "  all                完整测试模式 - 运行所有测试 (默认)"
    echo "  comparison <文法>  对比测试模式 - 对指定文法运行不同分析器对比"
    echo "  stress             压力测试模式 - 运行复杂文法和极限测试"
    echo "  --help, -h         显示此帮助信息"
    echo
    echo "对比测试示例:"
    echo "  ./$SCRIPT_NAME comparison example_grammar.txt"
    echo "  ./$SCRIPT_NAME comparison c_language_complex_grammar.txt"
    echo "  ./$SCRIPT_NAME comparison programming_language_complex_grammar.txt"
    echo
    echo "其他示例:"
    echo "  ./$SCRIPT_NAME simple          # 运行简单测试"
    echo "  ./$SCRIPT_NAME stress          # 运行压力测试"
    echo "  ./$SCRIPT_NAME all             # 运行完整测试"
    echo "  ./$SCRIPT_NAME                 # 运行完整测试(默认)"
    echo
    echo "注意: 更多高级测试方法请查看SCRIPT_DOCUMENTATION.md"
    echo
    echo "支持的文法文件:"
    echo "  - example_grammar.txt                 基本算术表达式文法"
    echo "  - assignment_grammar.txt              赋值语句文法"
    echo "  - conditional_grammar.txt             条件语句文法"
    echo "  - c_language_complex_grammar.txt      复杂C语言文法"
    echo "  - programming_language_complex_grammar.txt 带if-else的编程语言文法"
    echo "  - modular_language_complex_grammar.txt 模块化编程语言文法"
    echo "  - 以及 FaultGrammar/ 目录下的错误文法文件"
    echo "注意: 如需添加新文法请查看SCRIPT_DOCUMENTATION.md或README.md获取文法命名规则"
    echo
    echo "注意: 对比测试将使用 LR(0), SLR(1), LR(1) 三种分析器分别测试指定文法"
    echo "======================================================================"
    exit 0
}

# 解析命令行参数
parse_arguments() {
    TEST_MODE="all"  # 默认模式
    COMPARISON_GRAMMAR=""
    
    while [[ $# -gt 0 ]]; do
        case $1 in
            --help|-h)
                show_help
                ;;
            simple)
                TEST_MODE="simple"
                shift
                ;;
            all)
                TEST_MODE="all"
                shift
                ;;
            comparison)
                TEST_MODE="comparison"
                shift
                if [[ $# -gt 0 && ! $1 =~ ^- ]]; then
                    COMPARISON_GRAMMAR="$1"
                    shift
                else
                    echo "错误: comparison 模式需要指定文法文件"
                    echo "用法: ./$SCRIPT_NAME comparison <文法文件>"
                    echo "例如: ./$SCRIPT_NAME comparison c_language_complex_grammar.txt"
                    exit 1
                fi
                ;;
            stress)
                TEST_MODE="stress"
                shift
                ;;
            *)
                echo "错误: 未知选项 '$1'"
                echo "使用 './$SCRIPT_NAME --help' 查看帮助信息"
                exit 1
                ;;
        esac
    done
}

# 检查环境和依赖
check_environment() {
    echo "==================================="
    echo "  LR语法分析器测试脚本 v$SCRIPT_VERSION"
    echo "  测试模式: $TEST_MODE"
    echo "==================================="
    echo

    # 检查lr_cli可执行文件
    if [ ! -f "../Algorithm/lr_cli" ]; then
        echo "错误: 找不到lr_cli可执行文件"
        echo "请先在Algorithm目录中运行 make 或 make lr_cli"
        exit 1
    fi
    
    # 检查对比测试的文法文件
    if [ "$TEST_MODE" = "comparison" ] && [ ! -f "$COMPARISON_GRAMMAR" ]; then
        echo "错误: 找不到指定的文法文件: $COMPARISON_GRAMMAR"
        echo
        echo "可用的文法文件:"
        ls -1 *_grammar.txt 2>/dev/null | sed 's/^/  - /'
        echo
        exit 1
    fi
}

CLI="../Algorithm/lr_cli"
LOG_FILE="test_results.log"

# 初始化测试环境
initialize_testing() {
    # 清空日志文件
    > $LOG_FILE
    
    echo "测试开始时间: $(date)" | tee -a $LOG_FILE
    echo "测试模式: $TEST_MODE" | tee -a $LOG_FILE
    if [ "$TEST_MODE" = "comparison" ]; then
        echo "对比文法: $COMPARISON_GRAMMAR" | tee -a $LOG_FILE
    fi
    echo | tee -a $LOG_FILE
    
    # 压力测试计数器
    total_tests=0
    successful_tests=0
    failed_tests=0
}

# 测试函数
test_grammar() {
    local grammar_file="$1"
    local test_name="$2"
    local analyzer_type="$3"
    local input_string="$4"
    
    ((total_tests++))
    
    echo "───────────────────────────────────────" | tee -a $LOG_FILE
    printf "   测试 #%-3d: %s\n" "$total_tests" "$test_name" | tee -a $LOG_FILE
    printf "   文法: %s\n" "$grammar_file" | tee -a $LOG_FILE
    printf "   分析器: %s\n" "$analyzer_type" | tee -a $LOG_FILE
    echo "───────────────────────────────────────" | tee -a $LOG_FILE
    
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
    elif [[ "$grammar_file" =~ conflict ]] || [[ "$grammar_file" =~ lr1 ]] || [[ "$grammar_file" =~ ambiguous ]]; then
        echo "lr1"  # 冲突相关文法使用 LR(1)
    else
        echo "slr1" # 其他使用 SLR(1)
    fi
}

# 简单测试模式
run_simple_tests() {
    echo "=======================================" | tee -a $LOG_FILE
    echo "      简单测试模式 - 基础文法测试"   | tee -a $LOG_FILE
    echo "=======================================" | tee -a $LOG_FILE
    
    echo ">>> 测试所有基础正确文法（自动排除复杂文法）" | tee -a $LOG_FILE
    grammar_count=0
    for grammar_file in *_grammar.txt; do
        if [ -f "$grammar_file" ]; then
            # 检查是否为复杂文法（*_complex_grammar.txt 模式）
            skip_file=false
            if [[ "$grammar_file" =~ .*_complex_grammar\.txt$ ]]; then
                skip_file=true
                echo "跳过复杂文法: $grammar_file" | tee -a $LOG_FILE
            fi
            
            if [ "$skip_file" = true ]; then
                echo "跳过复杂文法: $grammar_file" | tee -a $LOG_FILE
                continue
            fi
            
            analyzer=$(smart_analyzer_selection "$grammar_file")
            
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
                "classic_lr1_grammar.txt")
                    test_grammar "$grammar_file" "经典LR(1)测试文法" "lr1" "id = * id"
                    ;;
                "lr1_test_grammar.txt")
                    test_grammar "$grammar_file" "LR(1)测试文法" "lr1" "a b"
                    ;;
                "reduce_reduce_conflict_grammar.txt")
                    test_grammar "$grammar_file" "归约-归约冲突文法" "lr1" "a"
                    ;;
                "shift_reduce_conflict_grammar.txt")
                    test_grammar "$grammar_file" "移进-归约冲突文法" "lr1" "if a then if a then a else a"
                    ;;
                "slr1_limitation_grammar.txt")
                    test_grammar "$grammar_file" "SLR(1)限制文法" "lr1" "a a a"
                    ;;
                *)
                    rule_count=$(grep -c " -> " "$grammar_file" 2>/dev/null || echo "0")
                    test_grammar "$grammar_file" "文法测试 ($(basename "$grammar_file" .txt), $rule_count 规则)" "$analyzer"
                    ;;
            esac
            ((grammar_count++))
        fi
    done
    echo "已测试 $grammar_count 个基础文法文件" | tee -a $LOG_FILE
    
    # 测试所有错误文法
    echo | tee -a $LOG_FILE
    echo ">>> 测试所有错误文法" | tee -a $LOG_FILE
    fault_count=0
    for fault_file in FaultGrammar/*.txt; do
        if [ -f "$fault_file" ]; then
            fault_name=$(basename "$fault_file" .txt)
            # 根据文件名确定错误类型描述
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
                "ambiguous")
                    test_grammar "$fault_file" "二义性文法错误 ($fault_name)" "slr1"
                    ;;
                "circular_dependency")
                    test_grammar "$fault_file" "循环依赖错误 ($fault_name)" "slr1"
                    ;;
                "unreachable_terminals")
                    test_grammar "$fault_file" "不可达符号错误 ($fault_name)" "slr1"
                    ;;
                *)
                    test_grammar "$fault_file" "错误文法 ($fault_name)" "slr1"
                    ;;
            esac
            ((fault_count++))
        fi
    done
    echo "已测试 $fault_count 个错误文法文件" | tee -a $LOG_FILE
}

# 对比测试模式
run_comparison_tests() {
    echo "=======================================" | tee -a $LOG_FILE
    echo "        对比测试模式 - $COMPARISON_GRAMMAR" | tee -a $LOG_FILE
    echo "=======================================" | tee -a $LOG_FILE
    
    if [ ! -f "$COMPARISON_GRAMMAR" ]; then
        echo "错误: 找不到指定的文法文件: $COMPARISON_GRAMMAR" | tee -a $LOG_FILE
        return 1
    fi
    
    # 获取文法基本信息
    rule_count=$(grep -c " -> " "$COMPARISON_GRAMMAR" 2>/dev/null || echo "0")
    line_count=$(wc -l < "$COMPARISON_GRAMMAR")
    base_name=$(basename "$COMPARISON_GRAMMAR" .txt)
    
    echo "文法文件: $COMPARISON_GRAMMAR" | tee -a $LOG_FILE
    echo "文法规模: $line_count 行, $rule_count 个产生式" | tee -a $LOG_FILE
    echo | tee -a $LOG_FILE
    
    # 选择合适的测试输入串
    local test_input=""
    case "$base_name" in
        *"example"*|*"expression"*)
            test_input="a + a * a"
            ;;
        *"assignment"*)
            test_input="id = id + num"
            ;;
        *"conditional"*)
            test_input="if id then id = num else id = id"
            ;;
        *"c_language_complex"*)
            test_input="int id ( ) { return num ; }"
            ;;
        *"programming_language_complex"*)
            test_input="if ( id > number ) { id = number ; }"
            ;;
        *"modular_language_complex"*)
            test_input="var id : bool ;"
            ;;
        *)
            test_input="id"  # 通用简单输入
            ;;
    esac
    
    # 使用三种分析器进行对比测试
    for analyzer in lr0 slr1 lr1; do
        echo | tee -a $LOG_FILE
        echo "╔════════════════════════════════════════╗" | tee -a $LOG_FILE
        printf "║          分析器对比测试: %-8s      ║\n" "$analyzer" | tee -a $LOG_FILE
        echo "╚════════════════════════════════════════╝" | tee -a $LOG_FILE
        
        # 分析表构造测试
        test_grammar "$COMPARISON_GRAMMAR" "$base_name ($analyzer) - 分析表构造" "$analyzer"
        
        # 输入串解析测试
        if [ -n "$test_input" ]; then
            test_grammar "$COMPARISON_GRAMMAR" "$base_name ($analyzer) - 输入串解析" "$analyzer" "$test_input"
        fi
        
        echo | tee -a $LOG_FILE
    done
    
    # 性能对比测试
    echo | tee -a $LOG_FILE
    echo "╔════════════════════════════════════════╗" | tee -a $LOG_FILE
    echo "║              性能对比测试              ║" | tee -a $LOG_FILE
    echo "╚════════════════════════════════════════╝" | tee -a $LOG_FILE
    
    for analyzer in slr1 lr1; do
        echo ">> 测试 $analyzer 连续构造性能..." | tee -a $LOG_FILE
        local start_time=$(date +%s)
        local success_count=0
        
        for ((i=1; i<=5; i++)); do
            if timeout 30s $CLI "$COMPARISON_GRAMMAR" -t "$analyzer" --table --json > /dev/null 2>&1; then
                ((success_count++))
            fi
        done
        
        local end_time=$(date +%s)
        local elapsed=$((end_time - start_time))
        
        echo "分析器: $analyzer" | tee -a $LOG_FILE
        echo "成功次数: $success_count/5" | tee -a $LOG_FILE
        echo "总耗时: ${elapsed}秒" | tee -a $LOG_FILE
        echo | tee -a $LOG_FILE
    done
}

# 压力测试模式
run_stress_tests() {
    echo "=======================================" | tee -a $LOG_FILE
    echo "         压力测试模式 - 复杂文法测试" | tee -a $LOG_FILE
    echo "=======================================" | tee -a $LOG_FILE
    
    # 自动发现复杂文法进行压力测试
    echo ">> 自动发现复杂文法..." | tee -a $LOG_FILE
    complex_count=0
    for grammar in *_complex_grammar.txt; do
        if [ -f "$grammar" ]; then
            ((complex_count++))
            echo ">> 压力测试: $grammar" | tee -a $LOG_FILE
            rule_count=$(grep -c " -> " "$grammar")
            echo "文法规模: $rule_count 个产生式" | tee -a $LOG_FILE
            
            # 基础压力测试
            test_grammar "$grammar" "压力测试 - $(basename "$grammar" .txt)" "lr1"
            
            # 连续构造测试
            echo ">> 连续构造压力测试..." | tee -a $LOG_FILE
            for ((i=1; i<=10; i++)); do
                ((total_tests++))
                echo "测试 #$total_tests: 连续构造 $grammar ($i/10)" | tee -a $LOG_FILE
                if timeout 30s $CLI "$grammar" -t lr1 --table --json > /dev/null 2>&1; then
                    echo "✓ 构造成功" | tee -a $LOG_FILE
                    ((successful_tests++))
                else
                    echo "✗ 构造失败或超时" | tee -a $LOG_FILE
                    ((failed_tests++))
                fi
            done
            echo | tee -a $LOG_FILE
        fi
    done
    
    # 极限压力测试
    echo ">> 极限压力测试..." | tee -a $LOG_FILE
    available_grammars=($(ls *_grammar.txt 2>/dev/null))
    
    for grammar in "${available_grammars[@]}"; do
        if [ -f "$grammar" ]; then
            for analyzer in lr0 slr1 lr1; do
                ((total_tests++))
                echo "测试 #$total_tests: 极限压力 $grammar ($analyzer)" | tee -a $LOG_FILE
                if timeout 45s $CLI "$grammar" -t "$analyzer" --table --json > /dev/null 2>&1; then
                    echo "✓ 成功" | tee -a $LOG_FILE
                    ((successful_tests++))
                else
                    echo "✗ 失败或超时" | tee -a $LOG_FILE
                    ((failed_tests++))
                fi
            done
        fi
    done
}

# 完整测试模式（原有的所有测试）
run_full_tests() {
    echo "=======================================" | tee -a $LOG_FILE
    echo "         完整测试模式 - 所有测试" | tee -a $LOG_FILE
    echo "=======================================" | tee -a $LOG_FILE
    
    # 正确文法测试
    echo ">>> 正确文法测试" | tee -a $LOG_FILE
    grammar_count=0
    for grammar_file in *_grammar.txt; do
        if [ -f "$grammar_file" ]; then
            analyzer=$(smart_analyzer_selection "$grammar_file")
            
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
                "c_language_complex_grammar.txt")
                    test_grammar "$grammar_file" "复杂C语言文法" "$analyzer" "int id ( ) { return num ; }"
                    ;;
                "programming_language_complex_grammar.txt")
                    test_grammar "$grammar_file" "带if-else的编程语言文法" "$analyzer" "if ( id > number ) { id = number ; }"
                    ;;
                "modular_language_complex_grammar.txt")
                    test_grammar "$grammar_file" "模块化编程语言文法" "$analyzer" "var id : bool ;"
                    ;;
                *)
                    rule_count=$(grep -c " -> " "$grammar_file" 2>/dev/null || echo "0")
                    test_grammar "$grammar_file" "文法测试 ($(basename "$grammar_file" .txt), $rule_count 规则)" "$analyzer"
                    ;;
            esac
            ((grammar_count++))
        fi
    done
    echo "已测试 $grammar_count 个正确文法文件" | tee -a $LOG_FILE
    
    # 分析器对比测试
    echo ">>> 分析器对比测试" | tee -a $LOG_FILE
    if [ -f "example_grammar.txt" ]; then
        for analyzer in lr0 slr1 lr1; do
            test_grammar "example_grammar.txt" "基本算术表达式文法 ($analyzer)" "$analyzer" "a + a * a"
        done
    fi
    
    # 错误文法测试
    echo ">>> 错误文法测试" | tee -a $LOG_FILE
    fault_count=0
    for fault_file in FaultGrammar/*.txt; do
        if [ -f "$fault_file" ]; then
            fault_name=$(basename "$fault_file" .txt)
            test_grammar "$fault_file" "错误文法 ($fault_name)" "slr1"
            ((fault_count++))
        fi
    done
    echo "已测试 $fault_count 个错误文法文件" | tee -a $LOG_FILE
    
    # 复杂文法压力测试
    echo ">>> 复杂文法压力测试" | tee -a $LOG_FILE
    run_stress_tests
}

# 显示测试总结
show_test_summary() {
    echo | tee -a $LOG_FILE
    echo "=======================================" | tee -a $LOG_FILE
    echo "           测试总结报告" | tee -a $LOG_FILE
    echo "=======================================" | tee -a $LOG_FILE
    
    echo "测试结束时间: $(date)" | tee -a $LOG_FILE
    echo "测试模式: $TEST_MODE" | tee -a $LOG_FILE
    if [ "$TEST_MODE" = "comparison" ]; then
        echo "对比文法: $COMPARISON_GRAMMAR" | tee -a $LOG_FILE
    fi
    echo "=======================================" | tee -a $LOG_FILE
    echo "测试统计:" | tee -a $LOG_FILE
    echo "  总测试数: $total_tests" | tee -a $LOG_FILE
    echo "  成功: $successful_tests" | tee -a $LOG_FILE
    echo "  失败: $failed_tests" | tee -a $LOG_FILE
    if [ $total_tests -gt 0 ]; then
        success_rate=$(echo "scale=2; $successful_tests * 100 / $total_tests" | bc 2>/dev/null || echo "N/A")
        echo "  成功率: $success_rate%" | tee -a $LOG_FILE
    fi
    echo | tee -a $LOG_FILE
    
    echo "详细日志文件: $LOG_FILE" | tee -a $LOG_FILE
    echo | tee -a $LOG_FILE
    echo "=======================================" | tee -a $LOG_FILE
    echo "   测试完成！检查日志文件获取详细信息" | tee -a $LOG_FILE
    echo "=======================================" | tee -a $LOG_FILE
}

# 主执行函数
main() {
    # 解析命令行参数
    parse_arguments "$@"
    
    # 检查环境
    check_environment
    
    # 初始化测试环境
    initialize_testing
    
    # 根据测试模式执行相应的测试
    case "$TEST_MODE" in
        simple)
            run_simple_tests
            ;;
        comparison)
            run_comparison_tests
            ;;
        stress)
            run_stress_tests
            ;;
        all)
            run_full_tests
            ;;
        *)
            echo "错误: 未知的测试模式: $TEST_MODE"
            exit 1
            ;;
    esac
    
    # 显示测试总结
    show_test_summary
}

# 脚本入口点
main "$@"
