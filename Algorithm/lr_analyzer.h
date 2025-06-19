// LR语法分析器头文件
// 支持LR(0)、SLR(1)、LR(1)语法分析
// 项目集族构造、分析表构造、冲突检测、语法分析过程可视化
// @author B22040317张舜灵

#ifndef LR_ANALYZER_H
#define LR_ANALYZER_H

#include "grammar.h"
#include "lr_item.h"
#include <map>
#include <vector>
#include <string>
#include <set>

// 注意：g_silent_mode 在 grammar.h 中声明

// LR分析器的动作类型枚举
// SHIFT: 移进动作，将当前输入符号压入栈中
// REDUCE: 归约动作，使用某个产生式进行归约
// ACCEPT: 接受动作，表示输入串被成功分析
// ERROR: 错误动作，表示分析失败
enum class ActionType {
    SHIFT,      // 移进动作
    REDUCE,     // 归约动作
    ACCEPT,     // 接受动作
    ERROR       // 错误动作
};

// 表示LR分析器的一个动作
// 每个动作包含：动作类型、动作值、产生式信息
struct Action {
    ActionType type;        // 动作类型
    int value;              // 状态号（移进）或产生式号（归约）
    Production production;  // 用于归约的产生式

    // 默认构造函数，创建错误动作
    Action() : type(ActionType::ERROR), value(-1), production("", {}) {}

    // 构造函数，创建移进或接受动作
    Action(ActionType t, int v) : type(t), value(v), production("", {}) {}

    // 构造函数，创建归约动作
    Action(ActionType t, int v, const Production& p) : type(t), value(v), production(p) {}

    // 将动作转换为字符串表示
    std::string toString() const;
};

// LR语法分析器主类
// 实现完整的LR语法分析功能，包括：
// 1. 项目集族的构造
// 2. FIRST集和FOLLOW集的计算
// 3. LR(0)、SLR(1)、LR(1)分析表的构造
// 4. 语法分析过程的执行和可视化
// 5. 冲突检测和报告
class LRAnalyzer {
private:
    // 核心数据结构
    Grammar grammar;                                            // 输入文法
    std::vector<ItemSet> itemSets;                              // 项目集族
    std::map<std::pair<int, std::string>, Action> actionTable;  // ACTION表
    std::map<std::pair<int, std::string>, int> gotoTable;       // GOTO表
    std::vector<std::string> conflicts;                         // 冲突信息列表

    // FIRST集相关
    std::map<std::string, std::set<std::string>> firstSets;    // 所有符号的FIRST集

    // 计算所有符号的FIRST集
    // 使用经典的FIRST集算法计算文法中所有终结符和非终结符的FIRST集
    void computeFirstSets();

    // 计算符号串的FIRST集
    // 计算给定符号串X₁X₂...Xₙ的FIRST集，这在构造LR项目的闭包时需要使用
    std::set<std::string> getFirst(const std::vector<std::string>& symbols);

    // FOLLOW集相关
    std::map<std::string, std::set<std::string>> followSets;  // 所有非终结符的FOLLOW集

    // 计算所有非终结符的FOLLOW集
    // 使用经典的FOLLOW集算法计算文法中所有非终结符的FOLLOW集
    void computeFollowSets();

    // LR(0)项目集构造相关
    // 计算项目集的闭包
    // 实现CLOSURE操作，对给定的项目集添加所有由闭包规则得到的项目
    ItemSet closure(const ItemSet& items);

    // 计算GOTO转移函数
    // 实现GOTO(I,X)操作，计算从项目集I经由符号X转移得到的项目集
    ItemSet gotoSet(const ItemSet& items, const std::string& symbol);

    // 构造LR(0)项目集族
    // 使用经典算法构造完整的LR(0)项目集族（规范LR(0)项目集族）
    void constructLR0ItemSets();

    // LR(1)项目集构造相关
    // 计算LR(1)项目集的闭包
    ItemSet closureLR1(const ItemSet& items);

    // 计算LR(1)的GOTO转移函数
    ItemSet gotoSetLR1(const ItemSet& items, const std::string& symbol);

    // 构造LR(1)项目集族
    void constructLR1ItemSets();

    // 分析表构造
    // 构造ACTION表和GOTO表
    // 根据项目集族构造LR分析表
    void constructActionGotoTable(bool isLR1 = false);

    // 检测并记录分析表中的冲突
    // 检测移进/归约冲突和归约/归约冲突
    void detectConflicts();

public:
    // 构造函数
    // 创建LR分析器实例，需要传入已经处理好的文法对象
    LRAnalyzer(const Grammar& g);

    // 分析表构造方法
    // 构造LR(0)分析表
    bool constructLR0Table();

    // 构造SLR(1)分析表
    bool constructSLR1Table();

    // 构造LR(1)分析表
    bool constructLR1Table();

    // 信息展示方法
    // 打印项目集族
    void printItemSets() const;

    // 打印ACTION表
    void printActionTable() const;

    // 打印GOTO表
    void printGotoTable() const;

    // 打印ACTION表（JSON格式）
    void printActionTableJSON() const;

    // 打印GOTO表（JSON格式）
    void printGotoTableJSON() const;

    // 打印项目集族（JSON格式）
    void printItemSetsJSON() const;

    // 打印冲突信息
    void printConflicts() const;
    void printConflicts(std::ostream& os) const;

    // 语法分析
    // 对输入串进行语法分析
    // 执行LR语法分析过程，动态展示状态栈、符号栈、剩余输入、执行动作
    bool parse(const std::vector<std::string>& input);

    // 访问器方法
    // 获取项目集族的常量引用
    const std::vector<ItemSet>& getItemSets() const { return itemSets; }

    // 获取冲突信息的常量引用
    const std::vector<std::string>& getConflicts() const { return conflicts; }

    // 调试方法
    // 调试用的公有方法，用于测试closureLR1
    ItemSet debugClosureLR1(const ItemSet& items) {
        return closureLR1(items);
    }

    // 调试用的公有方法，用于计算FIRST集
    void debugComputeFirstSets() {
        computeFirstSets();
    }
};

#endif // LR_ANALYZER_H
