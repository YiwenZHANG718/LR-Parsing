/**
 * @file grammar.h
 * @brief 上下文无关文法的定义和处理
 *
 * 本文件定义了上下文无关文法的表示和基本操作，包括：
 * - 产生式的表示和操作
 * - 文法的存储和管理
 * - 文法的读取（从文件或标准输入）
 * - 文法的增广（添加新的开始符号）
 * - 终结符和非终结符的自动识别
 *
 * @author ZJJ
 * @date 2025.6.19
 * @version 3.0
 */

#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <vector>
#include <string>
#include <set>
#include <map>
#include <iostream>

// 全局静默模式控制变量声明
extern bool g_silent_mode;

/**
 * @class Production
 * @brief 表示一个文法产生式
 *
 * 产生式是文法的基本组成单位，形式为 A → α，其中：
 * - A是非终结符（产生式的左部）
 * - α是符号串（产生式的右部，可能包含终结符和非终结符）
 *
 * 使用示例：
 * @code
  * Production prod("E", {"E", "+", "T"});  // E → E + T
  * Production epsilon("A", {"ε"});         // A → ε (空产生式)
  * @endcode
  */
class Production {
public:
    std::string left;                    ///< 产生式左部（非终结符）
    std::vector<std::string> right;      ///< 产生式右部（符号串）

    Production(const std::string& l, const std::vector<std::string>& r)
    : left(l), right(r) {}

    bool operator==(const Production& other) const {
        return left == other.left && right == other.right;
    }

    std::string toString() const;
};

/**
 * @class Grammar
 * @brief 上下文无关文法类
 *
 * 这个类管理整个上下文无关文法，提供以下功能：
 * 1. 文法的存储和管理
 * 2. 从文件或标准输入读取文法
 * 3. 自动识别终结符和非终结符
 * 4. 文法增广（为LR分析做准备）
 * 5. 文法信息的展示
 *
 * 文法输入格式：
 * - 每行一个产生式
 * - 格式：A -> alpha | beta
 * - 支持多选择分支（用|分隔）
 * - 支持空产生式（用ε表示）
 * - 支持注释行（以#开头）
 *
 * 使用示例：
 * @code
 * Grammar grammar;
 * grammar.loadFromFile("expr.txt");
 * grammar.augment();  // 为LR分析增广文法
 * grammar.print();    // 显示文法信息
 * @endcode
 */
class Grammar {
private:
    std::vector<Production> productions;    ///< 文法的所有产生式
    std::set<std::string> nonterminals;    ///< 非终结符集合
    std::set<std::string> terminals;       ///< 终结符集合
    std::string startSymbol;               ///< 开始符号

    void parseProduction(const std::string& line);
    void extractSymbols();
    
    // 验证文法的完整性和格式
    bool validateGrammar();                                   // 检查文法是否完整
    bool isValidProductionFormat(const std::string& line);    // 检查产生式格式是否正确
    bool hasReachableSymbols();                               // 检查是否有可达符号（警告）
    bool checkReachableProductions();                         // 检查可达产生式
    void reportError(const std::string& message);             // 报告错误信息

public:

    Grammar();

    bool loadFromFile(const std::string& filename);
    void loadFromInput();

    const std::vector<Production>& getProductions() const { return productions; }
    const std::set<std::string>& getNonterminals() const { return nonterminals; }
    const std::set<std::string>& getTerminals() const { return terminals; }
    const std::string& getStartSymbol() const { return startSymbol; }

    void augment();
    void print() const;
};

#endif // GRAMMAR_H
