# LR分析算法详细说明

## 项目概述

本文档详细介绍了LR语法分析器项目中实现的核心算法，包括LR(0)、SLR(1)、LR(1)三种分析方法的理论基础、实现细节和技术特点。

## 算法理论基础

### LR分析概述
LR分析是一种自底向上的语法分析方法，其中：
- **L** - 从左到右扫描输入（Left-to-right）
- **R** - 构造最右推导的逆过程（Rightmost derivation in reverse）

### 项目支持的LR方法
1. **LR(0)**：基础LR分析，无前瞻符号
2. **SLR(1)**：简单LR分析，使用FOLLOW集作为前瞻
3. **LR(1)**：规范LR分析，使用精确的前瞻符号

### 核心概念

#### 1. LR项目 (LR Item)
LR项目是在产生式右部某个位置加上一个点(.)的产生式，表示分析进度。

```
例如，对于产生式 E -> E + T
可能的LR项目有：
E -> . E + T    (期待看到E)
E -> E . + T    (已识别E，期待+)  
E -> E + . T    (已识别E+，期待T)
E -> E + T .    (完全识别，可以归约)
```

**LR(1)项目扩展**：
```
LR(1)项目格式：[A -> α.β, a]
其中：
- A -> α.β 是核心项目
- a 是前瞻符号（终结符）

例如：[E -> E . + T, $]
表示在前瞻符号为$时，期待归约E -> E + T
```

#### 2. 项目集 (Item Set)
项目集是LR项目的集合，表示分析器在某个状态下可能的分析情况。每个项目集对应DFA中的一个状态。

**项目集分类**：
- **内核项目**：初始项目或点不在最左端的项目
- **闭包项目**：通过闭包操作添加的项目

#### 3. 闭包运算 (Closure)
给定项目集I，CLOSURE(I)是包含I中所有项目以及由这些项目推导出的所有项目的集合。

**LR(0)闭包算法**：
```
CLOSURE(I):
1. 初始化：J = I
2. 重复直到J不再变化：
   对J中每个项目 A -> α.Bβ (B是非终结符)：
     对每个产生式 B -> γ：
       将项目 B -> .γ 加入J (如果不存在)
3. 返回J
```

**LR(1)闭包算法**：
```
CLOSURE(I):
1. 初始化：J = I
2. 重复直到J不再变化：
   对J中每个项目 [A -> α.Bβ, a]：
     对每个产生式 B -> γ：
       对FIRST(βa)中每个终结符b：
         将项目 [B -> .γ, b] 加入J (如果不存在)
3. 返回J
```

#### 4. 转移函数 (GOTO Function)
GOTO(I,X)表示从项目集I经过符号X转移到的新项目集。

**GOTO算法**：
```
GOTO(I, X):
1. J = ∅
2. 对I中每个项目（核心或闭包）：
   - LR(0): A -> α.Xβ → 将 A -> αX.β 加入J
   - LR(1): [A -> α.Xβ, a] → 将 [A -> αX.β, a] 加入J
3. 返回 CLOSURE(J)
```

## 三种LR分析算法对比

### 算法能力对比表

| 特性 | LR(0) | SLR(1) | LR(1) |
|------|-------|--------|-------|
| 前瞻符号 | 无 | FOLLOW集 | 精确前瞻 |
| 冲突解决能力 | 最弱 | 中等 | 最强 |
| 状态数量 | 最少 | 与LR(0)相同 | 最多 |
| 适用文法范围 | LR(0)文法 | SLR(1)文法 | LR(1)文法 |
| 构造复杂度 | O(n²) | O(n²) | O(n³) |
| 实际应用 | 教学演示 | 简单语言 | 复杂语言 |

### 1. LR(0)分析算法

#### 算法特点
- **最简单的LR分析**：不使用任何前瞻信息
- **容易产生冲突**：移进/归约冲突、归约/归约冲突
- **适用范围有限**：只能处理非常简单的文法
- **教学价值高**：便于理解LR分析的基本原理

#### 核心实现
```cpp
// 项目集构造算法
void LRAnalyzer::constructLR0ItemSets() {
    // 1. 增广文法（确保开始符号唯一出现在左部）
    const_cast<Grammar&>(grammar).augment();
    
    // 2. 构造初始项目集 I0
    ItemSet I0;
    LRItem startItem(grammar.getProductions()[0], 0);  // S' -> .S
    I0.add(startItem);
    I0 = closureLR0(I0);
    itemSets.push_back(I0);
    
    // 3. 使用工作队列构造所有项目集
    queue<int> workQueue;
    workQueue.push(0);
    
    while (!workQueue.empty()) {
        int currentId = workQueue.front();
        workQueue.pop();
        
        // 计算当前项目集的所有转移
        map<string, ItemSet> transitions = computeTransitions(itemSets[currentId]);
        
        for (auto& [symbol, nextSet] : transitions) {
            if (!nextSet.empty()) {
                int nextId = findOrAddItemSet(nextSet);
                if (nextId == itemSets.size() - 1) {
                    workQueue.push(nextId);  // 新状态加入队列
                }
                // 记录转移关系
                gotoFunction[{currentId, symbol}] = nextId;
            }
        }
    }
}

// LR(0)闭包算法
ItemSet LRAnalyzer::closureLR0(const ItemSet& items) {
    ItemSet result = items;
    bool changed = true;
    
    while (changed) {
        changed = false;
        ItemSet newItems;
        
        for (const auto& item : result.getItems()) {
            if (!item.isComplete()) {
                string nextSymbol = item.getNextSymbol();
                if (grammar.isNonterminal(nextSymbol)) {
                    // 添加以nextSymbol为左部的所有产生式
                    for (const auto& production : grammar.getProductions()) {
                        if (production.left == nextSymbol) {
                            LRItem newItem(production, 0);
                            if (!result.contains(newItem) && !newItems.contains(newItem)) {
                                newItems.add(newItem);
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
        
        // 将新项目添加到结果中
        for (const auto& item : newItems.getItems()) {
            result.add(item);
        }
    }
    
    return result;
}
```

#### 分析表构造
```cpp
void LRAnalyzer::constructLR0Table() {
    actionTable.clear();
    gotoTable.clear();
    
    for (int i = 0; i < itemSets.size(); i++) {
        for (const auto& item : itemSets[i].getItems()) {
            if (item.isComplete()) {
                // 归约项目处理
                if (item.getProduction().left == grammar.getAugmentedStart()) {
                    // 接受项目
                    actionTable[{i, "$"}] = "acc";
                } else {
                    // 归约项目：对所有终结符设置归约动作
                    int prodId = grammar.getProductionId(item.getProduction());
                    for (const auto& terminal : grammar.getTerminals()) {
                        string action = "r" + to_string(prodId);
                        
                        // 冲突检测
                        if (actionTable.count({i, terminal}) && 
                            actionTable[{i, terminal}] != action) {
                            conflicts.push_back({i, terminal, 
                                actionTable[{i, terminal}], action});
                        }
                        actionTable[{i, terminal}] = action;
                    }
                }
            } else {
                // 移进项目处理
                string nextSymbol = item.getNextSymbol();
                auto gotoKey = make_pair(i, nextSymbol);
                
                if (gotoFunction.count(gotoKey)) {
                    int nextState = gotoFunction[gotoKey];
                    
                    if (grammar.isTerminal(nextSymbol)) {
                        string action = "s" + to_string(nextState);
                        auto actionKey = make_pair(i, nextSymbol);
                        
                        // 冲突检测
                        if (actionTable.count(actionKey) && 
                            actionTable[actionKey] != action) {
                            conflicts.push_back({i, nextSymbol, 
                                actionTable[actionKey], action});
                        }
                        actionTable[actionKey] = action;
                    } else {
                        gotoTable[gotoKey] = nextState;
                    }
                }
            }
        }
    }
}
```
    }
}
```

### 2. SLR(1)分析

#### 算法特点
- 在LR(0)基础上增加FOLLOW集
- 归约动作只在FOLLOW集中的符号上进行
- 解决了LR(0)的部分冲突问题

#### FOLLOW集计算
```cpp
void computeFollowSets() {
    // 1. 将$加入开始符号的FOLLOW集
    follow[startSymbol].insert("$");
    
    bool changed = true;
    while (changed) {
        changed = false;
        
        for (Production prod : productions) {
            for (int i = 0; i < prod.right.size(); i++) {
                string symbol = prod.right[i];
                
                if (grammar.isNonterminal(symbol)) {
                    // 计算FIRST(β)，β是symbol后面的符号串
                    set<string> firstBeta = computeFirst(prod.right, i + 1);
                    
                    // 将FIRST(β)\{ε}加入FOLLOW(symbol)
                    for (string s : firstBeta) {
                        if (s != "ε" && follow[symbol].find(s) == follow[symbol].end()) {
                            follow[symbol].insert(s);
                            changed = true;
                        }
                    }
                    
                    // 如果ε∈FIRST(β)，则将FOLLOW(A)加入FOLLOW(symbol)
                    if (firstBeta.count("ε")) {
                        for (string s : follow[prod.left]) {
                            if (follow[symbol].find(s) == follow[symbol].end()) {
                                follow[symbol].insert(s);
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
    }
}
```

#### SLR(1)分析表构造
```cpp
void constructSLR1Table() {
    for (int i = 0; i < itemSets.size(); i++) {
        for (LRItem item : itemSets[i]) {
            if (item.dotAtEnd()) {
                if (item.isStartProduction()) {
                    action[i]["$"] = "acc";
                } else {
                    // 只在FOLLOW集中的符号上设置归约动作
                    string leftSymbol = grammar.getProduction(item.productionId).left;
                    for (string terminal : follow[leftSymbol]) {
                        action[i][terminal] = "r" + to_string(item.productionId);
                    }
                }
            } else {
                // 移进项目处理同LR(0)
                string nextSymbol = item.getNextSymbol();
                int nextState = getGoto(i, nextSymbol);
                if (grammar.isTerminal(nextSymbol)) {
                    action[i][nextSymbol] = "s" + to_string(nextState);
                } else {
                    gotoTable[i][nextSymbol] = nextState;
                }
            }
        }
    }
}
```

### 3. LR(1)分析

#### 算法特点
- 每个LR项目携带向前看符号集合
- 最精确的LR分析算法
- 状态数可能很大，但分析能力最强

#### LR(1)项目定义
```cpp
class LR1Item {
public:
    int productionId;        // 产生式编号
    int dotPosition;         // 点的位置
    set<string> lookaheads;  // 向前看符号集合
    
    LR1Item(int prodId, int dotPos, set<string> la) 
        : productionId(prodId), dotPosition(dotPos), lookaheads(la) {}
};
```

#### LR(1)闭包计算
```cpp
ItemSet closureLR1(ItemSet I) {
    ItemSet J = I;
    bool changed = true;
    
    while (changed) {
        changed = false;
        
        for (LR1Item item : J) {
            if (!item.dotAtEnd()) {
                string B = item.getNextSymbol();
                
                if (grammar.isNonterminal(B)) {
                    // 计算向前看符号
                    vector<string> beta = item.getSymbolsAfterNext();
                    
                    for (string lookahead : item.lookaheads) {
                        beta.push_back(lookahead);
                        set<string> firstBetaA = computeFirst(beta);
                        beta.pop_back();
                        
                        // 对B的每个产生式
                        for (int prodId : grammar.getProductionsFor(B)) {
                            LR1Item newItem(prodId, 0, firstBetaA);
                            if (!J.contains(newItem)) {
                                J.add(newItem);
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
    }
    
    return J;
}
```

#### LR(1)分析表构造
```cpp
void constructLR1Table() {
    for (int i = 0; i < itemSets.size(); i++) {
        for (LR1Item item : itemSets[i]) {
            if (item.dotAtEnd()) {
                if (item.isStartProduction()) {
                    action[i]["$"] = "acc";
                } else {
                    // 只在项目的向前看符号上设置归约动作
                    for (string lookahead : item.lookaheads) {
                        action[i][lookahead] = "r" + to_string(item.productionId);
                    }
                }
            } else {
                string nextSymbol = item.getNextSymbol();
                int nextState = getGoto(i, nextSymbol);
                if (grammar.isTerminal(nextSymbol)) {
                    action[i][nextSymbol] = "s" + to_string(nextState);
                } else {
                    gotoTable[i][nextSymbol] = nextState;
                }
            }
        }
    }
}
```

## 语法分析过程

### 分析算法
```cpp
bool parse(vector<string> input) {
    stack<int> stateStack;
    stack<string> symbolStack;
    
    stateStack.push(0);
    input.push_back("$");
    
    int step = 0;
    int inputPos = 0;
    
    while (true) {
        int currentState = stateStack.top();
        string currentSymbol = input[inputPos];
        
        string action = getAction(currentState, currentSymbol);
        
        printStep(step++, stateStack, symbolStack, input, inputPos, action);
        
        if (action == "acc") {
            return true;  // 接受
        } else if (action[0] == 's') {
            // 移进
            int nextState = stoi(action.substr(1));
            stateStack.push(nextState);
            symbolStack.push(currentSymbol);
            inputPos++;
        } else if (action[0] == 'r') {
            // 归约
            int prodId = stoi(action.substr(1));
            Production prod = grammar.getProduction(prodId);
            
            // 弹出产生式右部长度个状态和符号
            for (int i = 0; i < prod.right.size(); i++) {
                stateStack.pop();
                symbolStack.pop();
            }
            
            // 压入产生式左部
            symbolStack.push(prod.left);
            
            // 查GOTO表
            int gotoState = getGoto(stateStack.top(), prod.left);
            stateStack.push(gotoState);
            
        } else {
            return false;  // 错误
        }
    }
}
```

## 冲突检测

### 移进-归约冲突
当某个状态既可以移进又可以归约时发生。

### 归约-归约冲突  
当某个状态可以用多个不同产生式归约时发生。

### 冲突处理策略
1. **算法升级**: LR(0) → SLR(1) → LR(1)
2. **文法重写**: 消除歧义
3. **优先级定义**: 使用算符优先级解决冲突

## 复杂度分析

### 算法复杂度对比

| 算法   | 构造时间复杂度  | 分析时间复杂度 | 空间复杂度    | 状态数规模 |
|--------|----------------|---------------|--------------|-----------|
| LR(0)  | O(n²)          | O(n)         | O(n²)        | 最少       |
| SLR(1) | O(n²)          | O(n)         | O(n²)        | 与LR(0)相同 |
| LR(1)  | O(n³)          | O(n)         | O(n²k)       | 最多       |

**说明**：
- n：文法规模（产生式数量）
- k：向前看符号数量
- 分析时间复杂度均为O(n)，其中n为输入串长度

### 实际性能特征

1. **LR(0)**：
   - 构造最快，状态数最少
   - 适用文法范围极其有限
   - 内存占用最小

2. **SLR(1)**：
   - 构造速度与LR(0)相同
   - 解决了LR(0)的大部分冲突
   - 实用性较强

3. **LR(1)**：
   - 构造最慢，状态数可能呈指数增长
   - 理论上能力最强
   - 实际应用中通常使用LALR(1)优化

## 实现技术特点

### 1. 高效数据结构
```cpp
// 使用哈希表优化查找
unordered_map<pair<int,string>, int, PairHash> gotoFunction;
unordered_map<pair<int,string>, string, PairHash> actionTable;

// 项目集去重优化
class ItemSetManager {
    vector<ItemSet> itemSets;
    unordered_map<string, int> itemSetIndex;  // 项目集签名到索引的映射
    
public:
    int findOrAdd(const ItemSet& itemSet) {
        string signature = itemSet.getSignature();
        if (itemSetIndex.count(signature)) {
            return itemSetIndex[signature];
        }
        itemSets.push_back(itemSet);
        itemSetIndex[signature] = itemSets.size() - 1;
        return itemSets.size() - 1;
    }
};
```

### 2. 内存优化
```cpp
// 项目共享优化
class ItemPool {
    static vector<LRItem> pool;
    static unordered_map<string, int> itemIndex;
    
public:
    static int getItem(const Production& prod, int dotPos) {
        string key = prod.toString() + ":" + to_string(dotPos);
        if (!itemIndex.count(key)) {
            pool.emplace_back(prod, dotPos);
            itemIndex[key] = pool.size() - 1;
        }
        return itemIndex[key];
    }
};
```

### 3. 冲突检测与报告
```cpp
struct ConflictInfo {
    int state;
    string symbol;
    string action1, action2;
    ConflictType type;  // SHIFT_REDUCE, REDUCE_REDUCE
    
    string describe() const {
        switch (type) {
            case SHIFT_REDUCE:
                return "状态 " + to_string(state) + " 在符号 '" + symbol + 
                       "' 上存在移进-归约冲突: " + action1 + " vs " + action2;
            case REDUCE_REDUCE:
                return "状态 " + to_string(state) + " 在符号 '" + symbol + 
                       "' 上存在归约-归约冲突: " + action1 + " vs " + action2;
        }
    }
};
```

## 错误处理机制

### 1. 语法错误恢复
```cpp
class SyntaxErrorRecovery {
public:
    enum RecoveryStrategy {
        PANIC_MODE,     // 恐慌模式
        PHRASE_LEVEL,   // 短语级恢复
        ERROR_PRODUCTION, // 错误产生式
        GLOBAL_CORRECTION // 全局修正
    };
    
    bool recoverFromError(ParsingContext& context, 
                         RecoveryStrategy strategy = PANIC_MODE) {
        switch (strategy) {
            case PANIC_MODE:
                return panicModeRecovery(context);
            // 其他策略实现...
        }
    }
    
private:
    bool panicModeRecovery(ParsingContext& context) {
        // 跳过输入直到找到同步符号
        set<string> syncSymbols = {";", "}", "begin", "end"};
        while (!context.atEnd() && 
               !syncSymbols.count(context.currentSymbol())) {
            context.advance();
        }
        return !context.atEnd();
    }
};
```

### 2. 文法验证
```cpp
class GrammarValidator {
public:
    struct ValidationResult {
        bool isValid;
        vector<string> errors;
        vector<string> warnings;
    };
    
    ValidationResult validate(const Grammar& grammar) {
        ValidationResult result;
        result.isValid = true;
        
        // 检查开始符号
        if (!validateStartSymbol(grammar, result)) {
            result.isValid = false;
        }
        
        // 检查产生式
        if (!validateProductions(grammar, result)) {
            result.isValid = false;
        }
        
        // 检查符号一致性
        if (!validateSymbolConsistency(grammar, result)) {
            result.isValid = false;
        }
        
        // 检查左递归
        checkLeftRecursion(grammar, result);
        
        return result;
    }
};
```

## 扩展特性

### 1. 调试支持
```cpp
class LRDebugger {
public:
    void enableTrace() { traceEnabled = true; }
    void setBreakpoint(int state) { breakpoints.insert(state); }
    
    void traceStep(const ParsingStep& step) {
        if (traceEnabled) {
            cout << "步骤 " << step.stepNumber << ":\n";
            cout << "  状态栈: " << step.stateStack << "\n";
            cout << "  符号栈: " << step.symbolStack << "\n";
            cout << "  输入: " << step.remainingInput << "\n";
            cout << "  动作: " << step.action << "\n\n";
        }
    }
    
private:
    bool traceEnabled = false;
    set<int> breakpoints;
};
```

### 2. 性能分析
```cpp
class PerformanceProfiler {
    struct Statistics {
        int totalStates;
        int conflicts;
        double constructionTime;
        double parsingTime;
        size_t memoryUsage;
    };
    
public:
    void startProfiling() {
        startTime = chrono::high_resolution_clock::now();
    }
    
    Statistics getStatistics() const {
        Statistics stats;
        stats.constructionTime = getConstructionTime();
        stats.totalStates = analyzer.getStateCount();
        stats.conflicts = analyzer.getConflictCount();
        stats.memoryUsage = getMemoryUsage();
        return stats;
    }
};
```

## 实际应用示例

### 编程语言子集文法
```
// 简化C语言表达式文法
S -> program
program -> stmt_list
stmt_list -> stmt_list stmt | stmt
stmt -> expr_stmt | if_stmt | while_stmt
expr_stmt -> expr ;
if_stmt -> if ( expr ) stmt | if ( expr ) stmt else stmt
while_stmt -> while ( expr ) stmt
expr -> expr + term | expr - term | term
term -> term * factor | term / factor | factor
factor -> ( expr ) | id | number
```

### 算术表达式处理
```cpp
// 算术表达式求值器集成
class ExpressionEvaluator {
    LRAnalyzer analyzer;
    
public:
    double evaluate(const string& expression) {
        auto tokens = tokenize(expression);
        auto parseTree = analyzer.parse(tokens);
        return evaluateParseTree(parseTree);
    }
    
private:
    double evaluateParseTree(const ParseTreeNode& node) {
        if (node.isLeaf()) {
            return stod(node.getValue());
        }
        
        double left = evaluateParseTree(node.getLeft());
        double right = evaluateParseTree(node.getRight());
        
        switch (node.getOperator()) {
            case '+': return left + right;
            case '-': return left - right;
            case '*': return left * right;
            case '/': return left / right;
        }
    }
};
```

## 总结

本项目实现了完整的LR分析器框架，具有以下特点：

1. **完整性**：支持LR(0)、SLR(1)、LR(1)三种算法
2. **可扩展性**：模块化设计，易于添加新功能
3. **实用性**：提供命令行和图形界面
4. **教学价值**：详细的调试和可视化功能
5. **性能优化**：高效的数据结构和算法实现

该实现适合用于：
- 编译原理教学演示
- 简单编程语言的语法分析
- 表达式求值器
- 语法分析算法研究
