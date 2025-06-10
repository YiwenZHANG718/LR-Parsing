/**
 * @file grammar.cpp
 * @brief 上下文无关文法类的实现文件
 *
 * 本文件实现了文法的读取、解析、增广和显示功能。
 * 支持从文件和标准输入读取文法，自动识别终结符和非终结符。
 */

#include "grammar.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

// 全局静默模式控制变量
bool g_silent_mode = false;

 /**
  * @brief 将产生式转换为字符串表示
  * @return 产生式的字符串形式
  *
  * 格式：A -> α
  * 特殊处理：
  * - 空产生式显示为 A -> ε
  * - 多符号产生式用空格分隔
  */
std::string Production::toString() const {
    std::string result = left + " -> ";
    if (right.empty() || (right.size() == 1 && right[0] == "ε")) {
        result += "ε";
    }
    else {
        for (size_t i = 0; i < right.size(); ++i) {
            if (i > 0) result += " ";
            result += right[i];
        }
    }
    return result;
}

/**
 * @brief 文法类的默认构造函数
 */
Grammar::Grammar() {}

/**
 * @brief 从文件读取文法
 * @param filename 文法文件路径
 * @return 成功读取返回true，否则返回false
 *
 * 文件格式：
 * - 每行一个产生式，格式为 A -> alpha | beta
 * - 以#开头的行为注释，会被忽略
 * - 空行会被忽略
 * - 支持多选择分支（用|分隔）
 */
bool Grammar::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        // 忽略空行和注释行
        if (!line.empty() && line[0] != '#') {
            parseProduction(line);
        }
    }

    // 从产生式中提取终结符和非终结符
    extractSymbols();
    return true;
}

/**
 * @brief 从标准输入读取文法
 *
 * 提供交互式输入界面，显示格式说明和示例。
 * 用户输入空行时结束输入。
 */
void Grammar::loadFromInput() {
    std::cout << "请输入文法（每行一个产生式，空行结束）：" << std::endl;
    std::cout << "格式：A -> alpha | beta" << std::endl;
    std::cout << "示例：E -> E + T | T" << std::endl;

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) break;
        parseProduction(line);
    }

    extractSymbols();
}

/**
 * @brief 解析单行产生式字符串
 * @param line 包含产生式的字符串
 *
 * 解析格式为 "A -> alpha | beta" 的产生式：
 * 1. 查找 "->" 分隔符
 * 2. 提取左部非终结符
 * 3. 处理右部的多个选择分支（用|分隔）
 * 4. 将每个符号分离并创建产生式对象
 *
 * 特殊处理：
 * - 自动去除空白字符
 * - 空右部转换为ε产生式
 */
void Grammar::parseProduction(const std::string& line) {
    size_t arrowPos = line.find("->");
    if (arrowPos == std::string::npos) return;

    std::string left = line.substr(0, arrowPos);
    std::string right = line.substr(arrowPos + 2);

    // 去除左部的空格
    left.erase(std::remove_if(left.begin(), left.end(), ::isspace), left.end());

    // 处理右部的多个选择（用|分隔）
    std::stringstream ss(right);
    std::string choice;

    while (std::getline(ss, choice, '|')) {
        std::vector<std::string> symbols;
        std::stringstream choiceSS(choice);
        std::string symbol;

        // 分离每个符号
        while (choiceSS >> symbol) {
            symbols.push_back(symbol);
        }

        // 如果右部为空，添加ε产生式
        if (symbols.empty()) {
            symbols.push_back("ε");
        }

        productions.emplace_back(left, symbols);
    }
}

/**
 * @brief 从产生式中提取终结符和非终结符
 *
 * 算法步骤：
 * 1. 第一遍扫描：将所有出现在产生式左部的符号标记为非终结符
 * 2. 第二遍扫描：将出现在产生式右部但不是非终结符的符号标记为终结符
 * 3. 确定开始符号：第一个产生式的左部
 *
 * 注意：ε不被视为终结符
 */
void Grammar::extractSymbols() {
    // 第一遍：提取所有非终结符
    for (const auto& prod : productions) {
        nonterminals.insert(prod.left);
    }

    // 第二遍：提取终结符
    for (const auto& prod : productions) {
        for (const auto& symbol : prod.right) {
            // 如果符号不是ε且不是非终结符，则为终结符
            if (symbol != "ε" && nonterminals.find(symbol) == nonterminals.end()) {
                terminals.insert(symbol);
            }
        }
    }

    // 设置开始符号为第一个产生式的左部
    if (!productions.empty()) {
        startSymbol = productions[0].left;
    }
}

/**
 * @brief 增广文法
 *
 * 为LR分析添加新的开始符号，规则：
 * 1. 创建新开始符号：原开始符号 + "'"
 * 2. 在产生式列表开头插入新产生式：S' -> S
 * 3. 将新符号添加到非终结符集合
 * 4. 更新开始符号
 *
 * 例如：E -> E + T | T  增广后变为：
 *      E' -> E
 *      E -> E + T | T
 */
void Grammar::augment() {
    std::string newStart = startSymbol + "'";
    productions.insert(productions.begin(), Production(newStart, { startSymbol }));
    nonterminals.insert(newStart);
    startSymbol = newStart;
}

/**
 * @brief 打印文法的完整信息
 *
 * 输出内容：
 * 1. 所有产生式（带编号，便于在分析中引用）
 * 2. 非终结符集合
 * 3. 终结符集合
 * 4. 开始符号
 *
 * 这些信息对于理解文法结构和调试LR分析过程非常有用。
 */
void Grammar::print() const {
    std::cout << "文法产生式：" << std::endl;
    for (size_t i = 0; i < productions.size(); ++i) {
        std::cout << i << ": " << productions[i].toString() << std::endl;
    }

    std::cout << "\n非终结符：";
    for (const auto& nt : nonterminals) {
        std::cout << " " << nt;
    }
    std::cout << std::endl;

    std::cout << "终结符：";
    for (const auto& t : terminals) {
        std::cout << " " << t;
    }
    std::cout << std::endl;

    std::cout << "开始符号：" << startSymbol << std::endl;
}
