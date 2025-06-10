# LR分析算法详细说明

## 算法理论基础

### LR分析概述
LR分析是一种自底向上的语法分析方法，其中：
- **L** - 从左到右扫描输入
- **R** - 构造最右推导的逆过程

### 核心概念

#### 1. LR项目 (LR Item)
LR项目是在产生式右部某个位置加上一个点(·)的产生式。

```
例如，对于产生式 E -> E + T
可能的LR项目有：
E -> · E + T
E -> E · + T  
E -> E + · T
E -> E + T ·
```

#### 2. 项目集 (Item Set)
项目集是LR项目的集合，表示分析器在某个状态下可能的分析情况。

#### 3. 闭包 (Closure)
给定项目集I，CLOSURE(I)是包含I中所有项目以及由这些项目推导出的所有项目的集合。

**闭包算法**：
```
CLOSURE(I):
1. 将I中所有项目加入J
2. 对J中每个项目 A -> α·Bβ (B是非终结符)
   - 对每个产生式 B -> γ
   - 将项目 B -> ·γ 加入J (如果不存在)
3. 重复步骤2直到J不再变化
4. 返回J
```

#### 4. 转移函数 (GOTO Function)
GOTO(I,X)表示从项目集I经过符号X转移到的新项目集。

**GOTO算法**：
```
GOTO(I, X):
1. J = ∅
2. 对I中每个项目 A -> α·Xβ
   - 将 A -> αX·β 加入J
3. 返回 CLOSURE(J)
```

## 三种LR分析算法

### 1. LR(0)分析

#### 算法特点
- 最简单的LR分析算法
- 不使用向前看符号
- 适用范围有限，容易产生冲突

#### 项目集构造
```cpp
void constructLR0ItemSets() {
    // 1. 构造初始项目集
    ItemSet I0;
    I0.add(LRItem(0, 0));  // S' -> ·S
    I0 = closure(I0);
    
    // 2. 构造所有项目集
    queue<ItemSet> workList;
    workList.push(I0);
    
    while (!workList.empty()) {
        ItemSet current = workList.front();
        workList.pop();
        
        // 对每个可能的符号计算GOTO
        for (string symbol : grammar.getAllSymbols()) {
            ItemSet next = goto(current, symbol);
            if (!next.empty() && !contains(next)) {
                addItemSet(next);
                workList.push(next);
            }
        }
    }
}
```

#### 分析表构造
```cpp
void constructLR0Table() {
    for (int i = 0; i < itemSets.size(); i++) {
        for (LRItem item : itemSets[i]) {
            if (item.dotAtEnd()) {
                // 归约项目
                if (item.isStartProduction()) {
                    action[i]["$"] = "acc";
                } else {
                    // 对所有终结符设置归约动作
                    for (string terminal : grammar.terminals) {
                        action[i][terminal] = "r" + to_string(item.productionId);
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

| 算法   | 时间复杂度      | 空间复杂度    | 状态数 |
|--------|----------------|--------------|-------|
| LR(0)  | O(n³)          | O(n²)        | 最少   |
| SLR(1) | O(n³)          | O(n²)        | 中等   |
| LR(1)  | O(n³)          | O(n²k)       | 最多   |

其中n是文法大小，k是向前看符号数。
