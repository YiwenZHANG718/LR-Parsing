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
 * @author 语法分析课程设计
 * @date 2025
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

    /**
     * @brief 构造函数
     * @param l 产生式左部
     * @param r 产生式右部
     */
    Production(const std::string& l, const std::vector<std::string>& r)
        : left(l), right(r) {}

    /**
     * @brief 比较两个产生式是否相等
     * @param other 另一个产生式
     * @return 如果两个产生式相等返回true
     */
    bool operator==(const Production& other) const {
        return left == other.left && right == other.right;
    }

    /**
     * @brief 将产生式转换为字符串表示
     * @return 产生式的字符串形式，格式为 "A -> α"
     */
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

    /**
     * @brief 解析单行产生式
     * @param line 包含产生式的字符串
     *
     * 解析格式如 "E -> E + T | T" 的产生式，支持：
     * - 多选择分支（用|分隔）
     * - 自动处理空白字符
     * - 空产生式处理
     */
    void parseProduction(const std::string& line);

    /**
     * @brief 从产生式中提取终结符和非终结符
     *
     * 算法：
     * 1. 所有出现在产生式左部的符号都是非终结符
     * 2. 出现在产生式右部但不是非终结符的符号都是终结符
     * 3. 第一个产生式的左部作为开始符号
     */
    void extractSymbols();

public:
    /**
     * @brief 默认构造函数
     */
    Grammar();

    // 文法输入方法
    /**
     * @brief 从文件读取文法
     * @param filename 文法文件名
     * @return 成功返回true，失败返回false
     *
     * 文件格式要求：
     * - 每行一个产生式，格式为 A -> alpha | beta
     * - 支持注释行（以#开头）
     * - 空行会被忽略
     */
    bool loadFromFile(const std::string& filename);

    /**
     * @brief 从标准输入读取文法
     *
     * 交互式输入文法，用户输入产生式直到输入空行结束。
     * 会显示输入格式提示和示例。
     */
    void loadFromInput();

    // 访问器方法
    /**
     * @brief 获取所有产生式的常量引用
     * @return 产生式向量的常量引用
     */
    const std::vector<Production>& getProductions() const { return productions; }

    /**
     * @brief 获取所有非终结符的常量引用
     * @return 非终结符集合的常量引用
     */
    const std::set<std::string>& getNonterminals() const { return nonterminals; }

    /**
     * @brief 获取所有终结符的常量引用
     * @return 终结符集合的常量引用
     */
    const std::set<std::string>& getTerminals() const { return terminals; }

    /**
     * @brief 获取开始符号的常量引用
     * @return 开始符号的常量引用
     */
    const std::string& getStartSymbol() const { return startSymbol; }

    // 文法处理方法
    /**
     * @brief 增广文法
     *
     * 为LR分析添加新的开始符号S'，并添加产生式S' -> S。
     * 这样可以：
     * 1. 确保开始符号只出现在一个产生式的左部
     * 2. 为LR分析提供明确的接受条件
     *
     * 例如：如果原文法开始符号是E，增广后会添加E' -> E
     */
    void augment();

    /**
     * @brief 打印文法信息
     *
     * 输出内容包括：
     * - 所有产生式（带编号）
     * - 非终结符集合
     * - 终结符集合
     * - 开始符号
     */
    void print() const;
};

#endif // GRAMMAR_H
