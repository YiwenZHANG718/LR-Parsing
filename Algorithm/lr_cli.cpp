/**
 * @file lr_cli.cpp
 * @brief LR语法分析器命令行接口实现
 *
 * 本文件实现了LR语法分析器的命令行界面，提供批处理友好的接口。
 * 支持多种输出格式（文本/JSON），适合脚本调用和自动化测试。
 *
 * 主要功能：
 * - 支持LR(0)、SLR(1)、LR(1)三种分析算法
 * - 提供分析表构造和显示
 * - 支持输入串的语法分析
 * - 提供JSON格式输出用于程序间通信
 * - 支持项目集的显示和查看
 *
 * 使用示例：
 * - 构造分析表: ./lr_cli grammar.txt -t slr1 --table
 * - 分析输入串: ./lr_cli grammar.txt -s "a + a * a"
 * - JSON输出: ./lr_cli grammar.txt --table --json
 *
 * @author B22040310朱家骏
 */

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include "grammar.h"
#include "lr_analyzer.h"

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

/**
 * @brief 打印程序使用说明
 * @param program 程序名称
 *
 * 显示命令行参数的详细说明和使用示例，帮助用户正确使用程序。
 */
void printUsage(const char* program) {
    std::cout << "用法: " << program << " <文法文件> [选项]" << std::endl;
    std::cout << "选项:" << std::endl;
    std::cout << "  -t <类型>    分析器类型: lr0, slr1, lr1 (默认: slr1)" << std::endl;
    std::cout << "  -i <文件>    要分析的输入串文件" << std::endl;
    std::cout << "  -s <字符串>  要分析的输入串" << std::endl;
    std::cout << "  --table      只显示分析表" << std::endl;
    std::cout << "  --items      只显示项目集族" << std::endl;
    std::cout << "  --json       以JSON格式输出" << std::endl;
    std::cout << "  --help       显示此帮助信息" << std::endl;
    std::cout << std::endl;
    std::cout << "示例:" << std::endl;
    std::cout << "  " << program << " grammar.txt -t slr1 --table" << std::endl;
    std::cout << "  " << program << " grammar.txt -s \"a + a * a\"" << std::endl;
}

/**
 * @brief 将输入字符串分词为token序列
 * @param input 待分词的输入字符串
 * @return std::vector<std::string> token序列
 *
 * 使用空格作为分隔符，将输入字符串分解为独立的符号序列。
 * 这些token将作为语法分析器的输入。
 */
std::vector<std::string> tokenizeInput(const std::string& input) {
    std::vector<std::string> tokens;
    std::stringstream ss(input);
    std::string token;
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

/**
 * @brief 从文件读取输入字符串
 * @param filename 输入文件路径
 * @return std::string 文件第一行的内容，失败时返回空字符串
 *
 * 读取指定文件的第一行作为待分析的输入串。
 * 常用于批处理模式下从文件获取测试用例。
 */
std::string readInputFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "";
    }
    std::string content;
    std::getline(file, content);
    return content;
}

/**
 * @brief 输出ACTION表
 * @param analyzer 指向LRAnalyzer对象的指针
 * @param json 是否使用JSON格式输出，默认为false
 *
 * 根据指定的格式输出ACTION分析表。
 * - 文本格式：人类可读的表格形式
 * - JSON格式：结构化数据，便于程序解析
 */
void outputActionTable(LRAnalyzer* analyzer, bool json = false) {
    if (json) {
        std::cout << "\"action_table\": {" << std::endl;
        // JSON格式输出ACTION表
        analyzer->printActionTableJSON();
        std::cout << "}," << std::endl;
    } else {
        std::cout << "ACTION表:" << std::endl;
        analyzer->printActionTable();
    }
}

/**
 * @brief 输出GOTO表
 * @param analyzer 指向LRAnalyzer对象的指针
 * @param json 是否使用JSON格式输出，默认为false
 *
 * 根据指定的格式输出GOTO分析表。
 * GOTO表定义了在非终结符上的状态转移规则。
 */
void outputGotoTable(LRAnalyzer* analyzer, bool json = false) {
    if (json) {
        std::cout << "\"goto_table\": {" << std::endl;
        // JSON格式输出GOTO表
        analyzer->printGotoTableJSON();
        std::cout << "}" << std::endl;
    } else {
        std::cout << "GOTO表:" << std::endl;
        analyzer->printGotoTable();
    }
}

/**
 * @brief 输出项目集族
 * @param analyzer 指向LRAnalyzer对象的指针
 * @param json 是否使用JSON格式输出，默认为false
 *
 * 输出LR分析器构造过程中生成的所有项目集。
 * 项目集是构造分析表的基础，显示了分析器的内部状态。
 */
void outputItemSets(LRAnalyzer* analyzer, bool json = false) {
    if (json) {
        std::cout << "\"item_sets\": [" << std::endl;
        analyzer->printItemSetsJSON();
        std::cout << "]" << std::endl;
    } else {
        std::cout << "项目集族:" << std::endl;
        analyzer->printItemSets();
    }
}

/**
 * @brief 程序主入口函数
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return int 程序退出状态码
 *
 * 处理命令行参数，执行相应的LR分析操作：
 * 1. 解析命令行参数
 * 2. 加载文法文件
 * 3. 选择并构造分析器（LR0/SLR1/LR1）
 * 4. 根据参数执行对应操作：
 *    - 构造并显示分析表
 *    - 显示项目集族
 *    - 分析输入字符串
 * 5. 按指定格式输出结果
 *
 * 退出状态码：
 * - 0: 成功
 * - 1: 参数错误或操作失败
 */
int main(int argc, char* argv[]) {
#ifdef _WIN32
    // 设置Windows控制台编码页为UTF-8
    system("chcp 65001 >nul");
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    // 检查最少参数要求
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    // 提前检查帮助参数，优先响应帮助请求
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--help") {
            printUsage(argv[0]);
            return 0;
        }
    }

    // 初始化参数变量
    std::string grammarFile = argv[1];        // 文法文件路径（必需参数）
    std::string analyzerType = "slr1";        // 分析器类型，默认SLR(1)
    std::string inputFile = "";               // 输入文件路径（可选）
    std::string inputString = "";             // 输入字符串（可选）
    bool showTable = false;                   // 是否显示分析表
    bool showItems = false;                   // 是否显示项目集
    bool jsonOutput = false;                  // 是否使用JSON格式输出

    // 解析命令行参数（从第二个参数开始，第一个是文法文件）
    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--table") {
            showTable = true;                      // 显示ACTION和GOTO表
        } else if (arg == "--items") {
            showItems = true;                      // 显示项目集族
        } else if (arg == "--json") {
            jsonOutput = true;                     // 启用JSON格式输出
        } else if (arg == "-t" && i + 1 < argc) {
            analyzerType = argv[++i];              // 设置分析器类型
        } else if (arg == "-i" && i + 1 < argc) {
            inputFile = argv[++i];                 // 从文件读取输入串
        } else if (arg == "-s" && i + 1 < argc) {
            inputString = argv[++i];               // 直接指定输入串
        }
    }

    // 加载并解析文法文件
    Grammar grammar;
    if (!grammar.loadFromFile(grammarFile)) {
        // grammar.loadFromFile 已经报告了具体错误信息
        return 1;
    }
    
    // 文法增广：添加新的开始产生式 S' -> S
    grammar.augment();
    LRAnalyzer analyzer(grammar);

    // JSON模式下启用静默模式，避免调试信息干扰JSON输出
    if (jsonOutput) {
        g_silent_mode = true;
    }

    // 根据指定的算法类型构造分析表
    bool success = false;
    if (analyzerType == "lr0") {
        success = analyzer.constructLR0Table();       // LR(0)算法
    } else if (analyzerType == "slr1") {
        success = analyzer.constructSLR1Table();      // SLR(1)算法
    } else if (analyzerType == "lr1") {
        success = analyzer.constructLR1Table();       // LR(1)算法
    } else {
        std::cerr << "错误: 未知的分析器类型 " << analyzerType << std::endl;
        return 1;
    }

    // 检查分析表构造是否成功
    if (!success) {
        std::cerr << "错误: 分析表构造失败" << std::endl;
        analyzer.printConflicts(std::cerr);           // 将冲突信息也输出到stderr
        return 1;
    }

    // JSON输出的开始标记
    if (jsonOutput) {
        std::cout << "{" << std::endl;
    }

    // 根据用户指定的选项执行相应操作
    if (showItems) {
        // 仅显示项目集族
        outputItemSets(&analyzer, jsonOutput);
    } else if (showTable) {
        // 仅显示分析表（ACTION + GOTO）
        outputActionTable(&analyzer, jsonOutput);
        if (!jsonOutput) std::cout << std::endl;      // 文本模式下添加空行分隔
        outputGotoTable(&analyzer, jsonOutput);
    } else {
        // 默认行为：处理输入串分析或显示完整信息
        std::string input;
        
        // 确定输入来源：文件或命令行参数
        if (!inputFile.empty()) {
            input = readInputFile(inputFile);
            if (input.empty()) {
                std::cerr << "错误: 无法读取输入文件 " << inputFile << std::endl;
                return 1;
            }
        } else if (!inputString.empty()) {
            input = inputString;
        } else {
            // 既没有指定输入串，也没有指定特定显示选项
            // 默认显示完整的分析信息（表格 + 项目集）
            if (jsonOutput) {
                outputActionTable(&analyzer, true);
                outputGotoTable(&analyzer, true);
                std::cout << "," << std::endl;
                outputItemSets(&analyzer, true);
            } else {
                // 文本模式：分别输出各个组件
                outputActionTable(&analyzer, false);
                std::cout << std::endl;
                outputGotoTable(&analyzer, false);
                std::cout << std::endl;
                outputItemSets(&analyzer, false);
            }
        }

        // 如果指定了输入串，执行语法分析
        if (!input.empty()) {
            std::vector<std::string> tokens = tokenizeInput(input);
            
            if (jsonOutput) {
                // JSON格式：结构化输出分析结果
                std::cout << "\"parse_result\": {" << std::endl;
                std::cout << "\"input\": \"" << input << "\"," << std::endl;
                std::cout << "\"success\": ";
            }
            
            // 执行语法分析
            bool parseSuccess = analyzer.parse(tokens);
            
            if (jsonOutput) {
                // JSON格式：输出成功/失败状态
                std::cout << (parseSuccess ? "true" : "false") << std::endl;
                std::cout << "}" << std::endl;
            } else {
                // 文本格式：输出用户友好的结果信息
                if (parseSuccess) {
                    std::cout << "✓ 分析成功" << std::endl;
                } else {
                    std::cout << "✗ 分析失败" << std::endl;
                }
            }
        }
    }

    // JSON格式的结束标记
    if (jsonOutput) {
        std::cout << "}" << std::endl;
    }

    return 0;  // 程序正常退出
}
