/**
 * @file lr_analyzer.cpp
 * @brief LR语法分析器的实现文件
 *
 * 本文件实现了完整的LR语法分析器，包括：
 * - FIRST集和FOLLOW集的计算
 * - LR(0)、SLR(1)、LR(1)项目集族的构造
 * - ACTION表和GOTO表的构造
 * - 语法分析过程的动态展示
 * - 冲突检测和报告
 */

#include "lr_analyzer.h"
#include <iostream>
#include <queue>
#include <algorithm>
#include <stack>

 /**
  * @brief 将Action转换为字符串表示
  * @return 动作的字符串形式
  *
  * 动作表示：
  * - 移进：s<状态号>，如s3表示移进到状态3
  * - 归约：r<产生式号>，如r2表示用第2个产生式归约
  * - 接受：acc，表示分析成功
  * - 错误：error，表示分析失败
  */
std::string Action::toString() const {
    switch (type) {
    case ActionType::SHIFT:
        return "s" + std::to_string(value);
    case ActionType::REDUCE:
        return "r" + std::to_string(value);
    case ActionType::ACCEPT:
        return "acc";
    case ActionType::ERROR:
    default:
        return "error";
    }
}

/**
 * @brief LR分析器构造函数
 * @param g 输入的文法对象
 */
LRAnalyzer::LRAnalyzer(const Grammar& g) : grammar(g) {}

/**
 * @brief 计算所有符号的FIRST集
 *
 * FIRST集算法：
 * 1. 对每个终结符a，FIRST(a) = {a}
 * 2. 对每个非终结符A，初始化FIRST(A) = ∅
 * 3. 对每个产生式A → X₁X₂...Xₙ：
 *    - 将FIRST(X₁) - {ε} 加入FIRST(A)
 *    - 如果ε ∈ FIRST(X₁)，则将FIRST(X₂) - {ε} 加入FIRST(A)
 *    - 依此类推，直到某个Xᵢ不包含ε或到达末尾
 *    - 如果所有Xi都包含ε，则将ε加入FIRST(A)
 * 4. 重复步骤3直到不再有变化
 */
void LRAnalyzer::computeFirstSets() {
    firstSets.clear();

    // 初始化：终结符的FIRST集就是它自己
    for (const auto& terminal : grammar.getTerminals()) {
        firstSets[terminal].insert(terminal);
    }

    // 初始化：非终结符的FIRST集为空
    for (const auto& nonterminal : grammar.getNonterminals()) {
        firstSets[nonterminal] = std::set<std::string>();
    }

    // 迭代计算直到不再变化
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& prod : grammar.getProductions()) {
            std::set<std::string> firstOfRight = getFirst(prod.right);
            size_t oldSize = firstSets[prod.left].size();
            firstSets[prod.left].insert(firstOfRight.begin(), firstOfRight.end());
            if (firstSets[prod.left].size() > oldSize) {
                changed = true;
            }
        }
    }
}

/**
 * @brief 计算符号串的FIRST集
 * @param symbols 符号串
 * @return 符号串的FIRST集
 *
 * 算法：
 * 1. 如果符号串为空或只有ε，返回{ε}
 * 2. 对于符号串X₁X₂...Xₙ：
 *    - 将FIRST(X₁) - {ε} 加入结果
 *    - 如果ε ∈ FIRST(X₁)，继续处理X₂
 *    - 重复直到某个Xi不包含ε或到达末尾
 *    - 如果所有Xi都包含ε，则将ε加入结果
 */
std::set<std::string> LRAnalyzer::getFirst(const std::vector<std::string>& symbols) {
    std::set<std::string> result;

    // 空串或ε的FIRST集是{ε}
    if (symbols.empty() || (symbols.size() == 1 && symbols[0] == "ε")) {
        result.insert("ε");
        return result;
    }

    // 遍历符号串中的每个符号
    for (const auto& symbol : symbols) {
        if (grammar.getTerminals().count(symbol)) {
            // 终结符：直接加入结果并结束
            result.insert(symbol);
            break;
        }
        else {
            // 非终结符：加入其FIRST集（除了ε）
            auto first = firstSets[symbol];
            result.insert(first.begin(), first.end());
            result.erase("ε");

            // 如果不包含ε，停止处理后续符号
            if (first.find("ε") == first.end()) {
                break;
            }
        }
    }

    // 检查是否所有符号都能推导出ε
    bool allHaveEpsilon = true;
    for (const auto& symbol : symbols) {
        if (grammar.getTerminals().count(symbol) ||
            firstSets[symbol].find("ε") == firstSets[symbol].end()) {
            allHaveEpsilon = false;
            break;
        }
    }

    // 如果所有符号都能推导出ε，则将ε加入结果
    if (allHaveEpsilon) {
        result.insert("ε");
    }

    return result;
}

/**
 * @brief 计算所有非终结符的FOLLOW集
 *
 * FOLLOW集算法：
 * 1. 将$ 加入 FOLLOW(S)，其中S是开始符号
 * 2. 对每个产生式A → αBβ：
 *    - 将FIRST(β) - {ε} 加入FOLLOW(B)
 *    - 如果β = ε 或 ε ∈ FIRST(β)，则将FOLLOW(A)加入FOLLOW(B)
 * 3. 重复步骤2直到不再有变化
 */
void LRAnalyzer::computeFollowSets() {
    followSets.clear();

    // 初始化：所有非终结符的FOLLOW集为空
    for (const auto& nonterminal : grammar.getNonterminals()) {
        followSets[nonterminal] = std::set<std::string>();
    }

    // 将$加入开始符号的FOLLOW集
    followSets[grammar.getStartSymbol()].insert("$");

    // 迭代计算直到不再变化
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& prod : grammar.getProductions()) {
            // 遍历产生式右部的每个符号
            for (size_t i = 0; i < prod.right.size(); ++i) {
                const std::string& symbol = prod.right[i];
                if (grammar.getNonterminals().count(symbol)) {
                    // 获取β (symbol之后的符号串)
                    std::vector<std::string> beta(prod.right.begin() + i + 1, prod.right.end());
                    auto firstBeta = getFirst(beta);

                    size_t oldSize = followSets[symbol].size();
                    // 将FIRST(β) - {ε} 加入FOLLOW(symbol)
                    followSets[symbol].insert(firstBeta.begin(), firstBeta.end());
                    followSets[symbol].erase("ε");

                    // 如果β为空或ε ∈ FIRST(β)，将FOLLOW(A)加入FOLLOW(symbol)
                    if (firstBeta.count("ε") || beta.empty()) {
                        followSets[symbol].insert(followSets[prod.left].begin(),
                            followSets[prod.left].end());
                    }

                    if (followSets[symbol].size() > oldSize) {
                        changed = true;
                    }
                }
            }
        }
    }
}

/**
 * @brief 计算项目集的闭包
 * @param items 输入的项目集
 * @return 闭包后的项目集
 *
 * 闭包算法：
 * 1. 将输入项目集中的所有项目加入闭包
 * 2. 对闭包中的每个项目[A → α·Bβ]：
 *    - 如果B是非终结符，则对B的每个产生式B → γ
 *    - 将项目[B → ·γ]加入闭包（如果尚未存在）
 * 3. 重复步骤2直到闭包不再变化
 */
ItemSet LRAnalyzer::closure(const ItemSet& items) {
    std::set<LRItem> closure = items.items;
    std::queue<LRItem> workList;

    // 将所有输入项目加入工作列表
    for (const auto& item : items.items) {
        workList.push(item);
    }

    while (!workList.empty()) {
        LRItem item = workList.front();
        workList.pop();

        // 如果项目不是归约项目（点号不在最后）
        if (!item.isReduceItem()) {
            std::string nextSymbol = item.getNextSymbol();
            // 如果下一个符号是非终结符
            if (grammar.getNonterminals().count(nextSymbol)) {
                // 为该非终结符的每个产生式创建新项目
                for (const auto& prod : grammar.getProductions()) {
                    if (prod.left == nextSymbol) {
                        LRItem newItem(prod.left, prod.right, 0);
                        // 如果新项目不在闭包中，加入闭包和工作列表
                        if (closure.find(newItem) == closure.end()) {
                            closure.insert(newItem);
                            workList.push(newItem);
                        }
                    }
                }
            }
        }
    }

    return ItemSet(closure);
}

/**
 * @brief 计算GOTO(I, X)，即从项目集I通过符号X转移到的项目集
 * @param items 输入的项目集I
 * @param symbol 转移符号X
 * @return 转移后的项目集
 *
 * GOTO算法：
 * 1. 从项目集I中选择所有形如[A → α·Xβ]的项目
 * 2. 将它们转换为[A → αX·β]
 * 3. 对结果项目集求闭包
 */
ItemSet LRAnalyzer::gotoSet(const ItemSet& items, const std::string& symbol) {
    std::set<LRItem> gotoItems;

    // 查找所有可以通过symbol转移的项目
    for (const auto& item : items.items) {
        if (!item.isReduceItem() && item.getNextSymbol() == symbol) {
            // 创建点号向前移动一位的新项目
            LRItem newItem(item.left, item.right, item.dotPos + 1);
            gotoItems.insert(newItem);
        }
    }

    // 如果没有可转移的项目，返回空集
    if (gotoItems.empty()) {
        return ItemSet();
    }

    // 对转移后的项目集求闭包
    return closure(ItemSet(gotoItems));
}

/**
 * @brief 构造LR(0)项目集族
 *
 * 项目集族构造算法：
 * 1. 构造初始项目集I₀ = CLOSURE({[S' → ·S]})
 * 2. 对每个项目集Iᵢ和每个符号X：
 *    - 计算J = GOTO(Iᵢ, X)
 *    - 如果J非空且不在项目集族中，则加入项目集族
 * 3. 重复步骤2直到不再产生新的项目集
 */
void LRAnalyzer::constructLR0ItemSets() {
    itemSets.clear();

    // 构造初始项目集I₀
    LRItem startItem(grammar.getStartSymbol(),
        grammar.getProductions()[0].right, 0);
    ItemSet I0 = closure(ItemSet({ startItem }));
    I0.id = 0;
    itemSets.push_back(I0);

    // 使用工作列表算法构造所有项目集
    std::queue<int> workList;
    workList.push(0);

    while (!workList.empty()) {
        int currentIndex = workList.front();
        workList.pop();

        // 获取所有符号（终结符和非终结符）
        std::set<std::string> symbols = grammar.getTerminals();
        symbols.insert(grammar.getNonterminals().begin(),
            grammar.getNonterminals().end());

        // 对每个符号计算GOTO
        for (const auto& symbol : symbols) {
            ItemSet gotoResult = gotoSet(itemSets[currentIndex], symbol);

            if (!gotoResult.items.empty()) {
                // 检查是否已存在相同的项目集
                bool found = false;
                for (size_t i = 0; i < itemSets.size(); ++i) {
                    if (itemSets[i] == gotoResult) {
                        found = true;
                        break;
                    }
                }

                // 如果是新的项目集，添加到项目集族中
                if (!found) {
                    gotoResult.id = itemSets.size();
                    itemSets.push_back(gotoResult);
                    workList.push(gotoResult.id);
                }
            }
        }
    }
}

/**
 * @brief 构造ACTION表和GOTO表
 * @param isLR1 是否为LR(1)分析表
 *
 * 分析表构造算法：
 * 1. 对每个项目集Iᵢ和每个项目：
 *    a) [A → α·aβ] 且 a是终结符：ACTION[i,a] = shift j（其中j = GOTO(Iᵢ,a)的编号）
 *    b) [A → α·Bβ] 且 B是非终结符：GOTO[i,B] = j（其中j = GOTO(Iᵢ,B)的编号）
 *    c) [A → α·] 且 A ≠ S'：
 *       - LR(0): 对所有终结符a，ACTION[i,a] = reduce A→α
 *       - SLR(1): 对FOLLOW(A)中的每个a，ACTION[i,a] = reduce A→α
 *       - LR(1): 对前瞻符号集中的每个a，ACTION[i,a] = reduce A→α
 *    d) [S' → S·]：ACTION[i,$] = accept
 * 2. 检测并报告移进/归约冲突和归约/归约冲突
 */
void LRAnalyzer::constructActionGotoTable(bool isLR1) {
    actionTable.clear();
    gotoTable.clear();
    conflicts.clear();

    for (size_t i = 0; i < itemSets.size(); ++i) {
        for (const auto& item : itemSets[i].items) {
            if (!item.isReduceItem()) {
                // 处理移进和GOTO动作
                std::string nextSymbol = item.getNextSymbol();
                if (grammar.getTerminals().count(nextSymbol)) {
                    // 移进动作：ACTION[i, a] = shift j
                    ItemSet gotoResult = gotoSet(itemSets[i], nextSymbol);
                    for (size_t j = 0; j < itemSets.size(); ++j) {
                        if (itemSets[j] == gotoResult) {
                            auto key = std::make_pair(i, nextSymbol);
                            // 检测移进/归约冲突
                            if (actionTable.count(key) &&
                                actionTable[key].type != ActionType::SHIFT) {
                                conflicts.push_back("移进/归约冲突在状态 " +
                                    std::to_string(i) +
                                    " 符号 " + nextSymbol);
                            }
                            actionTable[key] = Action(ActionType::SHIFT, j);
                            break;
                        }
                    }
                }
                // GOTO动作：GOTO[i, A] = j
                else if (grammar.getNonterminals().count(nextSymbol)) {
                    ItemSet gotoResult = gotoSet(itemSets[i], nextSymbol);
                    for (size_t j = 0; j < itemSets.size(); ++j) {
                        if (itemSets[j] == gotoResult) {
                            gotoTable[std::make_pair(i, nextSymbol)] = j;
                            break;
                        }
                    }
                }
            }
            else {
                // 处理归约和接受动作
                if (item.isAcceptItem()) {
                    // 接受动作：ACTION[i, $] = accept
                    actionTable[std::make_pair(i, "$")] = Action(ActionType::ACCEPT, 0);
                }
                else {
                    // 归约动作：ACTION[i, a] = reduce A→α
                    // 找到对应的产生式编号
                    for (size_t p = 0; p < grammar.getProductions().size(); ++p) {
                        const auto& prod = grammar.getProductions()[p];
                        if (prod.left == item.left && prod.right == item.right) {
                            if (isLR1) {
                                // LR(1): 只对前瞻符号添加归约动作
                                for (const auto& lookahead : item.lookaheads) {
                                    auto key = std::make_pair(i, lookahead);
                                    // 检测归约/归约冲突
                                    if (actionTable.count(key)) {
                                        conflicts.push_back("归约/归约冲突在状态 " +
                                            std::to_string(i) +
                                            " 符号 " + lookahead);
                                    }
                                    actionTable[key] = Action(ActionType::REDUCE, p, prod);
                                }
                            }
                            else {
                                // LR(0)或SLR(1): 确定归约符号集合
                                std::set<std::string> reduceSymbols;
                                if (followSets.empty()) {
                                    // LR(0): 对所有终结符和$添加归约动作
                                    reduceSymbols = grammar.getTerminals();
                                    reduceSymbols.insert("$");
                                }
                                else {
                                    // SLR(1): 对FOLLOW集中的符号添加归约动作
                                    reduceSymbols = followSets[item.left];
                                }

                                // 为每个归约符号添加归约动作
                                for (const auto& symbol : reduceSymbols) {
                                    auto key = std::make_pair(i, symbol);
                                    // 检测冲突
                                    if (actionTable.count(key)) {
                                        if (actionTable[key].type == ActionType::SHIFT) {
                                            conflicts.push_back("移进/归约冲突在状态 " +
                                                std::to_string(i) +
                                                " 符号 " + symbol);
                                        }
                                        else {
                                            conflicts.push_back("归约/归约冲突在状态 " +
                                                std::to_string(i) +
                                                " 符号 " + symbol);
                                        }
                                    }
                                    actionTable[key] = Action(ActionType::REDUCE, p, prod);
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
}

/**
 * @brief 构造LR(0)分析表
 * @return 如果成功构造且无冲突返回true，否则返回false
 *
 * LR(0)分析表构造步骤：
 * 1. 构造LR(0)项目集族
 * 2. 构造ACTION表和GOTO表（不使用FOLLOW集）
 * 3. 检查是否存在冲突
 */
bool LRAnalyzer::constructLR0Table() {
    if (!g_silent_mode) {
        std::cout << "构造LR(0)分析表..." << std::endl;
    }
    constructLR0ItemSets();
    constructActionGotoTable(false);
    return conflicts.empty();
}

/**
 * @brief 构造SLR(1)分析表
 * @return 如果成功构造且无冲突返回true，否则返回false
 *
 * SLR(1)分析表构造步骤：
 * 1. 计算FIRST集和FOLLOW集
 * 2. 构造LR(0)项目集族
 * 3. 构造ACTION表（使用FOLLOW集解决冲突）和GOTO表
 * 4. 检查是否存在冲突
 */
bool LRAnalyzer::constructSLR1Table() {
    if (!g_silent_mode) {
        std::cout << "构造SLR(1)分析表..." << std::endl;
    }
    computeFirstSets();
    computeFollowSets();
    constructLR0ItemSets();
    constructActionGotoTable(false);
    return conflicts.empty();
}

/**
 * @brief 构造LR(1)分析表
 * @return 如果成功构造且无冲突返回true，否则返回false
 *
 * LR(1)分析表构造步骤：
 * 1. 计算FIRST集和FOLLOW集
 * 2. 构造LR(1)项目集族（带前瞻符号）
 * 3. 构造ACTION表（使用前瞻符号）和GOTO表
 * 4. 检查是否存在冲突
 */
bool LRAnalyzer::constructLR1Table() {
    if (!g_silent_mode) {
        std::cout << "构造LR(1)分析表..." << std::endl;
    }
    computeFirstSets();
    computeFollowSets();
    constructLR1ItemSets();
    constructActionGotoTable(true);
    return conflicts.empty();
}

// LR(1)项目集构造（简化版本）
ItemSet LRAnalyzer::closureLR1(const ItemSet& items) {
    // 这里是简化的LR(1) closure实现
    // 完整实现需要处理前瞻符号的传播
    return closure(items);
}

ItemSet LRAnalyzer::gotoSetLR1(const ItemSet& items, const std::string& symbol) {
    // 这里是简化的LR(1) goto实现
    return gotoSet(items, symbol);
}

void LRAnalyzer::constructLR1ItemSets() {
    // 简化版本，实际上使用LR(0)的构造方法
    // 完整的LR(1)需要在项目中携带前瞻符号
    constructLR0ItemSets();
}

void LRAnalyzer::printItemSets() const {
    std::cout << "\n项目集族：" << std::endl;
    for (const auto& itemSet : itemSets) {
        itemSet.print();
    }
}

void LRAnalyzer::printActionTable() const {
    std::cout << "\nACTION表：" << std::endl;
    std::cout << "状态\t";
    for (const auto& terminal : grammar.getTerminals()) {
        std::cout << terminal << "\t";
    }
    std::cout << "$" << std::endl;

    for (size_t i = 0; i < itemSets.size(); ++i) {
        std::cout << i << "\t";
        for (const auto& terminal : grammar.getTerminals()) {
            auto key = std::make_pair(i, terminal);
            if (actionTable.count(key)) {
                std::cout << actionTable.at(key).toString() << "\t";
            }
            else {
                std::cout << "\t";
            }
        }
        auto key = std::make_pair(i, "$");
        if (actionTable.count(key)) {
            std::cout << actionTable.at(key).toString();
        }
        std::cout << std::endl;
    }
}

void LRAnalyzer::printGotoTable() const {
    std::cout << "\nGOTO表：" << std::endl;
    std::cout << "状态\t";
    for (const auto& nonterminal : grammar.getNonterminals()) {
        std::cout << nonterminal << "\t";
    }
    std::cout << std::endl;

    for (size_t i = 0; i < itemSets.size(); ++i) {
        std::cout << i << "\t";
        for (const auto& nonterminal : grammar.getNonterminals()) {
            auto key = std::make_pair(i, nonterminal);
            if (gotoTable.count(key)) {
                std::cout << gotoTable.at(key) << "\t";
            }
            else {
                std::cout << "\t";
            }
        }
        std::cout << std::endl;
    }
}

void LRAnalyzer::printConflicts() const {
    if (conflicts.empty()) {
        std::cout << "\n✓ 无冲突，文法是LR的" << std::endl;
    }
    else {
        std::cout << "\n✗ 检测到冲突：" << std::endl;
        for (const auto& conflict : conflicts) {
            std::cout << "  " << conflict << std::endl;
        }
    }
}

void LRAnalyzer::printActionTableJSON() const {
    bool first_state = true;
    for (size_t i = 0; i < itemSets.size(); ++i) {
        if (!first_state) std::cout << ",";
        std::cout << "\"" << i << "\": {";
        
        bool first_symbol = true;
        auto terminals = grammar.getTerminals();
        terminals.insert("$");
        
        for (const auto& terminal : terminals) {
            auto key = std::make_pair(i, terminal);
            if (actionTable.count(key)) {
                if (!first_symbol) std::cout << ",";
                std::cout << "\"" << terminal << "\": \"" << actionTable.at(key).toString() << "\"";
                first_symbol = false;
            }
        }
        std::cout << "}";
        first_state = false;
    }
}

void LRAnalyzer::printGotoTableJSON() const {
    bool first_state = true;
    for (size_t i = 0; i < itemSets.size(); ++i) {
        if (!first_state) std::cout << ",";
        std::cout << "\"" << i << "\": {";
        
        bool first_symbol = true;
        for (const auto& nonterminal : grammar.getNonterminals()) {
            auto key = std::make_pair(i, nonterminal);
            if (gotoTable.count(key)) {
                if (!first_symbol) std::cout << ",";
                std::cout << "\"" << nonterminal << "\": " << gotoTable.at(key);
                first_symbol = false;
            }
        }
        std::cout << "}";
        first_state = false;
    }
}

void LRAnalyzer::printItemSetsJSON() const {
    bool first_set = true;
    for (const auto& itemSet : itemSets) {
        if (!first_set) std::cout << ",";
        std::cout << "{\"id\": " << itemSet.id << ", \"items\": [";
        
        bool first_item = true;
        for (const auto& item : itemSet.items) {
            if (!first_item) std::cout << ",";
            std::cout << "\"" << item.toString() << "\"";
            first_item = false;
        }
        std::cout << "]}";
        first_set = false;
    }
}

bool LRAnalyzer::parse(const std::vector<std::string>& input) {
    if (itemSets.empty()) {
        std::cout << "错误：分析表未构造" << std::endl;
        return false;
    }

    std::stack<int> stateStack;
    std::stack<std::string> symbolStack;
    std::vector<std::string> inputBuffer = input;
    inputBuffer.push_back("$");

    stateStack.push(0);

    std::cout << "\n语法分析过程：" << std::endl;
    std::cout << "步骤\t状态栈\t符号栈\t\t输入\t\t动作" << std::endl;

    int step = 0;
    while (true) {
        // 打印当前状态
        std::cout << step++ << "\t";

        std::stack<int> tempStateStack = stateStack;
        std::vector<int> states;
        while (!tempStateStack.empty()) {
            states.push_back(tempStateStack.top());
            tempStateStack.pop();
        }
        std::reverse(states.begin(), states.end());
        for (int state : states) {
            std::cout << state << " ";
        }
        std::cout << "\t";

        std::stack<std::string> tempSymbolStack = symbolStack;
        std::vector<std::string> symbols;
        while (!tempSymbolStack.empty()) {
            symbols.push_back(tempSymbolStack.top());
            tempSymbolStack.pop();
        }
        std::reverse(symbols.begin(), symbols.end());
        for (const auto& symbol : symbols) {
            std::cout << symbol << " ";
        }
        std::cout << "\t\t";

        for (const auto& symbol : inputBuffer) {
            std::cout << symbol << " ";
        }
        std::cout << "\t\t";

        int currentState = stateStack.top();
        std::string currentInput = inputBuffer[0];

        auto actionKey = std::make_pair(currentState, currentInput);
        if (actionTable.find(actionKey) == actionTable.end()) {
            std::cout << "错误：无对应动作" << std::endl;
            return false;
        }

        Action action = actionTable[actionKey];
        std::cout << action.toString() << std::endl;

        switch (action.type) {
        case ActionType::SHIFT:
            stateStack.push(action.value);
            symbolStack.push(currentInput);
            inputBuffer.erase(inputBuffer.begin());
            break;

        case ActionType::REDUCE:
        {
            const Production& prod = action.production;
            std::cout << "\t\t\t\t\t\t归约用产生式: " << prod.toString() << std::endl;

            // 弹出产生式右部长度个状态和符号
            if (!(prod.right.size() == 1 && prod.right[0] == "ε")) {
                for (size_t i = 0; i < prod.right.size(); ++i) {
                    stateStack.pop();
                    symbolStack.pop();
                }
            }

            symbolStack.push(prod.left);
            int gotoState = stateStack.top();
            auto gotoKey = std::make_pair(gotoState, prod.left);

            if (gotoTable.find(gotoKey) == gotoTable.end()) {
                std::cout << "错误：GOTO表中无对应项" << std::endl;
                return false;
            }

            stateStack.push(gotoTable[gotoKey]);
        }
        break;

        case ActionType::ACCEPT:
            std::cout << "\n✓ 分析成功！输入串被接受。" << std::endl;
            return true;

        case ActionType::ERROR:
        default:
            std::cout << "错误：分析失败" << std::endl;
            return false;
        }
    }
}

