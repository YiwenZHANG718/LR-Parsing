/**
 * @file lr_analyzer.h
 * @brief LR语法分析器头文件
 *
 * 本文件定义了完整的LR语法分析器类，支持：
 * - LR(0)语法分析
 * - SLR(1)语法分析
 * - LR(1)语法分析（简化版本）
 * - 项目集族的构造和管理
 * - ACTION表和GOTO表的构造
 * - 冲突检测和报告
 * - 语法分析过程的可视化
 *
 * @author 语法分析课程设计
 * @date 2025
 */

#ifndef LR_ANALYZER_H
#define LR_ANALYZER_H

#include "grammar.h"
#include "lr_item.h"
#include <map>
#include <vector>
#include <string>
#include <set>

 /**
  * @enum ActionType
  * @brief LR分析器的动作类型枚举
  *
  * LR分析器在分析过程中可能执行的四种基本动作：
  * - SHIFT: 移进动作，将当前输入符号压入栈中
  * - REDUCE: 归约动作，使用某个产生式进行归约
  * - ACCEPT: 接受动作，表示输入串被成功分析
  * - ERROR: 错误动作，表示分析失败
  */
enum class ActionType {
    SHIFT,      ///< 移进动作
    REDUCE,     ///< 归约动作
    ACCEPT,     ///< 接受动作
    ERROR       ///< 错误动作
};

/**
 * @struct Action
 * @brief 表示LR分析器的一个动作
 *
 * 每个动作包含：
 * - 动作类型（移进、归约、接受、错误）
 * - 动作值（状态号或产生式号）
 * - 产生式信息（仅用于归约动作）
 */
struct Action {
    ActionType type;        ///< 动作类型
    int value;             ///< 状态号（移进）或产生式号（归约）
    Production production; ///< 用于归约的产生式

    /**
     * @brief 默认构造函数，创建错误动作
     */
    Action() : type(ActionType::ERROR), value(-1), production("", {}) {}

    /**
     * @brief 构造函数，创建移进或接受动作
     * @param t 动作类型
     * @param v 动作值（状态号）
     */
    Action(ActionType t, int v) : type(t), value(v), production("", {}) {}

    /**
     * @brief 构造函数，创建归约动作
     * @param t 动作类型
     * @param v 产生式号
     * @param p 产生式对象
     */
    Action(ActionType t, int v, const Production& p) : type(t), value(v), production(p) {}

    /**
     * @brief 将动作转换为字符串表示
     * @return 动作的字符串形式
     */
    std::string toString() const;
};

/**
 * @class LRAnalyzer
 * @brief LR语法分析器主类
 *
 * 这个类实现了完整的LR语法分析功能，包括：
 * 1. 项目集族的构造
 * 2. FIRST集和FOLLOW集的计算
 * 3. LR(0)、SLR(1)、LR(1)分析表的构造
 * 4. 语法分析过程的执行和可视化
 * 5. 冲突检测和报告
 *
 * 使用示例：
 * @code
 * Grammar grammar;
 * grammar.loadFromFile("grammar.txt");
 * grammar.augment();
 *
 * LRAnalyzer analyzer(grammar);
 * if (analyzer.constructSLR1Table()) {
 *     std::vector<std::string> input = {"id", "+", "id", "*", "id"};
 *     analyzer.parse(input);
 * }
 * @endcode
 */
class LRAnalyzer {
private:
    // 核心数据结构
    Grammar grammar;                                            ///< 输入文法
    std::vector<ItemSet> itemSets;                            ///< 项目集族
    std::map<std::pair<int, std::string>, Action> actionTable; ///< ACTION表
    std::map<std::pair<int, std::string>, int> gotoTable;     ///< GOTO表
    std::vector<std::string> conflicts;                       ///< 冲突信息列表

    // FIRST集相关
    std::map<std::string, std::set<std::string>> firstSets;   ///< 所有符号的FIRST集

    /**
     * @brief 计算所有符号的FIRST集
     *
     * 使用经典的FIRST集算法计算文法中所有终结符和非终结符的FIRST集。
     * FIRST(X)表示可以从X推导出的串的第一个终结符的集合。
     */
    void computeFirstSets();

    /**
     * @brief 计算符号串的FIRST集
     * @param symbols 符号串
     * @return 符号串的FIRST集
     *
     * 计算给定符号串X₁X₂...Xₙ的FIRST集，这在构造LR项目的闭包时需要使用。
     */
    std::set<std::string> getFirst(const std::vector<std::string>& symbols);

    // FOLLOW集相关
    std::map<std::string, std::set<std::string>> followSets;  ///< 所有非终结符的FOLLOW集

    /**
     * @brief 计算所有非终结符的FOLLOW集
     *
     * 使用经典的FOLLOW集算法计算文法中所有非终结符的FOLLOW集。
     * FOLLOW(A)表示在某些句型中可能紧跟在A后面的终结符的集合。
     */
    void computeFollowSets();

    // LR(0)项目集构造相关
    /**
     * @brief 计算项目集的闭包
     * @param items 输入项目集
     * @return 闭包后的项目集
     *
     * 实现CLOSURE操作，对给定的项目集添加所有由闭包规则得到的项目。
     */
    ItemSet closure(const ItemSet& items);

    /**
     * @brief 计算GOTO转移函数
     * @param items 输入项目集
     * @param symbol 转移符号
     * @return 转移后的项目集
     *
     * 实现GOTO(I,X)操作，计算从项目集I经由符号X转移得到的项目集。
     */
    ItemSet gotoSet(const ItemSet& items, const std::string& symbol);

    /**
     * @brief 构造LR(0)项目集族
     *
     * 使用经典算法构造完整的LR(0)项目集族（规范LR(0)项目集族）。
     */
    void constructLR0ItemSets();


    // LR(1)项目集构造相关（简化版本）
    /**
     * @brief 计算LR(1)项目集的闭包（简化版本）
     * @param items 输入项目集
     * @return 闭包后的项目集
     *
     * 注意：当前实现是简化版本，实际上调用LR(0)的closure函数。
     * 完整的LR(1)实现需要处理前瞻符号的传播。
     */
    ItemSet closureLR1(const ItemSet& items);

    /**
     * @brief 计算LR(1)的GOTO转移函数（简化版本）
     * @param items 输入项目集
     * @param symbol 转移符号
     * @return 转移后的项目集
     *
     * 注意：当前实现是简化版本，实际上调用LR(0)的gotoSet函数。
     */
    ItemSet gotoSetLR1(const ItemSet& items, const std::string& symbol);

    /**
     * @brief 构造LR(1)项目集族（简化版本）
     *
     * 注意：当前实现是简化版本，实际上使用LR(0)的项目集构造方法。
     * 完整的LR(1)需要在项目中维护前瞻符号信息。
     */
    void constructLR1ItemSets();

    // 分析表构造
    /**
     * @brief 构造ACTION表和GOTO表
     * @param isLR1 是否为LR(1)分析表
     *
     * 根据项目集族构造LR分析表：
     * - ACTION表：定义对终结符的动作（移进、归约、接受、错误）
     * - GOTO表：定义对非终结符的状态转移
     *
     * 对于不同的LR方法：
     * - LR(0)：对所有终结符添加归约动作
     * - SLR(1)：仅对FOLLOW集中的终结符添加归约动作
     * - LR(1)：仅对前瞻符号添加归约动作
     */
    void constructActionGotoTable(bool isLR1 = false);

    /**
     * @brief 检测并记录分析表中的冲突
     *
     * 检测两种主要冲突类型：
     * 1. 移进/归约冲突：同一状态下某个符号既可移进又可归约
     * 2. 归约/归约冲突：同一状态下某个符号可用多个产生式归约
     */
    void detectConflicts();

public:
    /**
     * @brief 构造函数
     * @param g 输入的文法对象
     *
     * 创建LR分析器实例，需要传入已经处理好的文法对象。
     * 通常文法需要先进行增广处理（添加新的开始符号）。
     */
    LRAnalyzer(const Grammar& g);

    // 分析表构造方法
    /**
     * @brief 构造LR(0)分析表
     * @return 如果成功构造且无冲突返回true，否则返回false
     *
     * LR(0)方法的特点：
     * - 最简单的LR方法
     * - 容易产生冲突
     * - 适用于简单文法
     */
    bool constructLR0Table();

    /**
     * @brief 构造SLR(1)分析表
     * @return 如果成功构造且无冲突返回true，否则返回false
     *
     * SLR(1)方法的特点：
     * - 在LR(0)基础上使用FOLLOW集减少冲突
     * - 比LR(0)更强大，比LR(1)更简单
     * - 适用于大多数实际文法
     */
    bool constructSLR1Table();

    /**
     * @brief 构造LR(1)分析表
     * @return 如果成功构造且无冲突返回true，否则返回false
     *
     * LR(1)方法的特点：
     * - 最强大的LR方法
     * - 使用前瞻符号，冲突最少
     * - 项目集族可能很大
     *
     * 注意：当前实现是简化版本
     */
    bool constructLR1Table();

    // 信息展示方法
    /**
     * @brief 打印项目集族
     *
     * 以易读的格式输出所有构造的项目集，包括：
     * - 项目集编号
     * - 每个项目集中的所有项目
     * - 项目的详细信息（产生式、点的位置等）
     */
    void printItemSets() const;

    /**
     * @brief 打印ACTION表
     *
     * 以表格形式输出ACTION表，显示每个状态对每个终结符的动作。
     * 动作格式：s<状态号>（移进）、r<产生式号>（归约）、acc（接受）
     */
    void printActionTable() const;

    /**
     * @brief 打印GOTO表
     *
     * 以表格形式输出GOTO表，显示每个状态对每个非终结符的转移状态。
     */
    void printGotoTable() const;

    /**
     * @brief 打印ACTION表（JSON格式）
     */
    void printActionTableJSON() const;

    /**
     * @brief 打印GOTO表（JSON格式）
     */
    void printGotoTableJSON() const;

    /**
     * @brief 打印项目集族（JSON格式）
     */
    void printItemSetsJSON() const;

    /**
     * @brief 打印冲突信息
     *
     * 输出分析表构造过程中检测到的所有冲突，包括：
     * - 冲突类型（移进/归约、归约/归约）
     * - 冲突位置（状态号、符号）
     * - 冲突详情
     */
    void printConflicts() const;

    // 语法分析
    /**
     * @brief 对输入串进行语法分析
     * @param input 输入的符号串
     * @return 如果分析成功返回true，否则返回false
     *
     * 执行LR语法分析过程，动态展示：
     * - 状态栈的变化
     * - 符号栈的变化
     * - 剩余输入串
     * - 执行的动作（移进、归约、接受）
     *
     * 这是LR分析器的核心功能，提供完整的分析过程可视化。
     */
    bool parse(const std::vector<std::string>& input);

    // 访问器方法
    /**
     * @brief 获取项目集族的常量引用
     * @return 项目集族的常量引用
     */
    const std::vector<ItemSet>& getItemSets() const { return itemSets; }

    /**
     * @brief 获取冲突信息的常量引用
     * @return 冲突信息列表的常量引用
     */
    const std::vector<std::string>& getConflicts() const { return conflicts; }
};

#endif // LR_ANALYZER_H
