/**
 * @file main.cpp
 * @brief LR语法分析器交互式主程序
 *
 * 本文件实现了LR语法分析器的交互式用户界面，提供菜单驱动的操作方式。
 * 用户可以通过选择不同的菜单选项来执行各种分析操作。
 *
 * 主要功能：
 * - 文法输入（文件或键盘）
 * - 多种LR分析算法选择（LR(0), SLR(1), LR(1)）
 * - 分析表构造和显示
 * - 项目集族查看
 * - 输入串的语法分析
 * - 交互式操作界面
 *
 * 适用场景：
 * - 教学演示
 * - 交互式实验
 * - 调试和测试
 *
 * @author ZJJ
 * @date 2025.6.11
 * @version 1.2
 */

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include "grammar.h"
#include "lr_analyzer.h"

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <locale.h>
#endif

/**
 * @brief 显示程序主菜单
 *
 * 打印所有可用的操作选项，为用户提供清晰的操作指导。
 * 菜单采用数字编号方式，便于用户快速选择。
 */
void printMenu() {
    std::cout << "\n=== LR语法分析器 ===" << std::endl;
    std::cout << "1. 从文件读取文法" << std::endl;
    std::cout << "2. 从键盘输入文法" << std::endl;
    std::cout << "3. 构造LR(0)分析表" << std::endl;
    std::cout << "4. 构造SLR(1)分析表" << std::endl;
    std::cout << "5. 构造LR(1)分析表" << std::endl;
    std::cout << "6. 显示项目集族" << std::endl;
    std::cout << "7. 显示分析表" << std::endl;
    std::cout << "8. 分析输入串" << std::endl;
    std::cout << "9. 显示文法" << std::endl;
    std::cout << "0. 退出" << std::endl;    std::cout << "请选择操作: ";
}

/**
 * @brief 将输入字符串分词为token序列
 * @param input 待分词的输入字符串
 * @return std::vector<std::string> 分词后的token序列
 *
 * 使用空格作为分隔符，将输入字符串分解为独立的符号序列。
 * 这些token将用作语法分析器的输入。
 */
std::vector<std::string> tokenizeInput(const std::string& input) {
    std::vector<std::string> tokens;
    std::stringstream ss(input);
    std::string token;

    while (ss >> token) {
        tokens.push_back(token);
    }    return tokens;
}

/**
 * @brief 程序主入口函数
 * @return int 程序退出状态码
 *
 * 实现交互式的LR语法分析器主程序：
 * 1. 初始化程序状态和变量
 * 2. 显示欢迎信息和版本信息
 * 3. 进入主循环，处理用户的菜单选择
 * 4. 根据用户选择执行相应的操作
 * 5. 维护程序状态（文法是否加载、分析表是否构造等）
 * 6. 提供错误处理和用户引导
 *
 * 程序状态变量：
 * - grammarLoaded: 标记文法是否已加载
 * - tableConstructed: 标记分析表是否已构造
 * - analyzer: 当前使用的分析器实例
 */
int main() {
#ifdef _WIN32
    // 设置Windows控制台为UTF-8模式
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    
    // 设置C locale
    setlocale(LC_ALL, ".UTF8");
    
    // 强制切换到UTF-8编码页并设置环境
    system("chcp 65001 >nul 2>&1");
#endif

    // 初始化程序状态
    Grammar grammar;                    // 文法对象
    LRAnalyzer* analyzer = nullptr;     // 分析器指针
    bool grammarLoaded = false;         // 文法加载状态
    bool tableConstructed = false;      // 分析表构造状态

    // 显示程序信息
    std::cout << "LR语法分析器 - 支持LR(0), SLR(1), LR(1)分析" << std::endl;    std::cout << "作者：语法分析课程设计" << std::endl;

    int choice;
    // 主程序循环：处理用户交互
    while (true) {
        printMenu();
        std::cin >> choice;
        std::cin.ignore(); // 清除输入缓冲区中的换行符

        switch (choice) {
        case 1: { // 从文件读取文法
            std::cout << "请输入文法文件名: ";
            std::string filename;
            std::getline(std::cin, filename);

            if (grammar.loadFromFile(filename)) {
                grammar.augment();              // 文法增广
                grammarLoaded = true;           // 更新状态
                tableConstructed = false;       // 重置分析表状态
                delete analyzer;                // 清理旧的分析器
                analyzer = new LRAnalyzer(grammar);  // 创建新的分析器
                std::cout << "✓ 文法加载成功" << std::endl;
            }
            else {
                std::cout << "✗ 文法加载失败" << std::endl;            }
            break;
        }

        case 2: { // 从键盘输入文法
            grammar = Grammar();            // 重置文法对象
            grammar.loadFromInput();        // 从标准输入读取文法
            grammar.augment();              // 文法增广
            grammarLoaded = true;           // 更新状态
            tableConstructed = false;       // 重置分析表状态
            delete analyzer;                // 清理旧的分析器
            analyzer = new LRAnalyzer(grammar);  // 创建新的分析器
            std::cout << "✓ 文法输入完成" << std::endl;
            break;
        }

        case 3: { // 构造LR(0)分析表
            if (!grammarLoaded) {
                std::cout << "✗ 请先加载文法" << std::endl;
                break;
            }            bool success = analyzer->constructLR0Table();  // 尝试构造LR(0)分析表
            tableConstructed = success;                     // 更新构造状态
            analyzer->printConflicts();                     // 显示冲突信息（如果有）

            if (success) {
                std::cout << "✓ LR(0)分析表构造成功" << std::endl;
            }
            else {
                std::cout << "✗ LR(0)分析表构造失败，存在冲突" << std::endl;
            }
            break;
        }

        case 4: { // 构造SLR(1)分析表
            if (!grammarLoaded) {
                std::cout << "✗ 请先加载文法" << std::endl;
                break;
            }

            bool success = analyzer->constructSLR1Table(); // 尝试构造SLR(1)分析表
            tableConstructed = success;                     // 更新构造状态
            analyzer->printConflicts();                     // 显示冲突信息（如果有）

            if (success) {
                std::cout << "✓ SLR(1)分析表构造成功" << std::endl;
            }
            else {
                std::cout << "✗ SLR(1)分析表构造失败，存在冲突" << std::endl;
            }
            break;
        }        case 5: { // 构造LR(1)分析表
            if (!grammarLoaded) {
                std::cout << "✗ 请先加载文法" << std::endl;
                break;
            }

            bool success = analyzer->constructLR1Table(); // 尝试构造LR(1)分析表
            tableConstructed = success;                    // 更新构造状态
            analyzer->printConflicts();                    // 显示冲突信息（如果有）

            if (success) {
                std::cout << "✓ LR(1)分析表构造成功" << std::endl;
            }
            else {
                std::cout << "✗ LR(1)分析表构造失败，存在冲突" << std::endl;
            }
            break;
        }        case 6: { // 显示项目集
            if (!grammarLoaded) {
                std::cout << "✗ 请先加载文法" << std::endl;
                break;
            }

            // 检查是否已经构造了分析表（项目集）
            if (analyzer->getItemSets().empty()) {
                std::cout << "✗ 请先构造分析表" << std::endl;
                break;
            }

            analyzer->printItemSets();  // 打印所有项目集
            break;
        }        case 7: { // 显示分析表
            if (!tableConstructed) {
                std::cout << "✗ 请先成功构造分析表" << std::endl;
                break;
            }

            // 打印ACTION表和GOTO表
            analyzer->printActionTable();  // 显示动作表
            analyzer->printGotoTable();    // 显示转移表
            break;
        }        case 8: { // 语法分析
            if (!tableConstructed) {
                std::cout << "✗ 请先成功构造分析表" << std::endl;
                break;
            }

            std::cout << "请输入待分析的符号串（空格分隔）: ";
            std::string input;
            std::getline(std::cin, input);

            // 将输入字符串分词为符号序列
            std::vector<std::string> tokens = tokenizeInput(input);
            if (tokens.empty()) {
                std::cout << "✗ 输入为空" << std::endl;
                break;
            }

            // 使用构造好的分析表进行语法分析
            bool success = analyzer->parse(tokens);
            if (!success) {
                std::cout << "✗ 分析失败，输入串不符合文法" << std::endl;
            }
            break;
        }        case 9: { // 显示文法
            if (!grammarLoaded) {
                std::cout << "✗ 请先加载文法" << std::endl;
                break;
            }

            grammar.print();  // 打印当前加载的文法规则
            break;
        }

        case 0: { // 退出程序
            std::cout << "感谢使用LR语法分析器！" << std::endl;
            delete analyzer;  // 清理内存
            return 0;
        }

        default: { // 无效输入处理
            std::cout << "✗ 无效选择，请重新输入" << std::endl;
            break;
        }
        }

        // 等待用户按回车键继续，提供更好的用户体验
        std::cout << "\n按回车键继续...";
        std::cin.get();
    }

    // 程序结束前清理内存
    delete analyzer;
    return 0;
}
