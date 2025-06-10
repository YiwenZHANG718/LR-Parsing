# 代码架构与实现文档

## 项目架构概览

```
LR语法分析器
├── 核心模块
│   ├── Grammar - 文法表示和处理
│   ├── LRItem - LR项目管理  
│   └── LRAnalyzer - 分析器主控制
├── 接口模块
│   ├── 交互式接口 (main.cpp)
│   └── 命令行接口 (lr_cli.cpp)
└── 图形界面
    └── Python GUI (GUI/GUI.py)
```

## 核心类设计

### 1. Grammar类 - 文法管理

```cpp
class Grammar {
private:
    vector<Production> productions;      // 产生式集合
    set<string> terminals;              // 终结符集合  
    set<string> nonterminals;           // 非终结符集合
    string start_symbol;                // 开始符号
    
    map<string, set<string>> first_sets;   // FIRST集
    map<string, set<string>> follow_sets;  // FOLLOW集

public:
    // 文法加载和验证
    bool loadFromFile(const string& filename);
    bool validateGrammar();
    
    // 基本查询
    bool isTerminal(const string& symbol);
    bool isNonterminal(const string& symbol);
    Production getProduction(int index);
    
    // FIRST/FOLLOW集计算
    void computeFirstSets();
    void computeFollowSets();
    set<string> getFirstSet(const string& symbol);
    set<string> getFollowSet(const string& symbol);
    
    // 工具函数
    void print();
    vector<string> getAllSymbols();
};
```

**关键实现要点**：
- 产生式用结构体表示：`struct Production { string left; vector<string> right; }`
- 符号集合自动推导：从产生式中提取终结符和非终结符
- FIRST/FOLLOW集使用迭代算法计算直到不动点

### 2. LRItem类 - LR项目表示

```cpp
class LRItem {
private:
    int production_id;                  // 产生式编号
    int dot_position;                   // 点的位置
    set<string> lookaheads;            // 向前看符号(LR1用)

public:
    LRItem(int prod_id, int dot_pos);
    LRItem(int prod_id, int dot_pos, const set<string>& la);
    
    // 状态查询
    bool dotAtEnd() const;
    bool canShift() const;
    string getNextSymbol() const;
    vector<string> getSymbolsAfterDot() const;
    
    // LR(1)支持
    void addLookahead(const string& symbol);
    set<string> getLookaheads() const;
    
    // 比较和哈希(用于集合操作)
    bool operator==(const LRItem& other) const;
    bool operator<(const LRItem& other) const;
    
    string toString() const;
};
```

**设计亮点**：
- 统一接口支持LR(0)/SLR(1)/LR(1)
- 点位置用整数表示，简化计算
- 重载比较运算符支持STL容器

### 3. ItemSet类 - 项目集管理

```cpp
class ItemSet {
private:
    set<LRItem> items;                 // 项目集合
    int set_id;                        // 项目集编号

public:
    ItemSet(int id = -1);
    
    // 基本操作
    void addItem(const LRItem& item);
    bool contains(const LRItem& item) const;
    bool empty() const;
    size_t size() const;
    
    // 迭代器支持
    auto begin() const { return items.begin(); }
    auto end() const { return items.end(); }
    
    // 项目集操作
    ItemSet closure(const Grammar& grammar) const;
    ItemSet gotoSet(const string& symbol, const Grammar& grammar) const;
    
    // 比较操作
    bool operator==(const ItemSet& other) const;
    
    void print() const;
};
```

### 4. LRAnalyzer类 - 分析器核心

```cpp
class LRAnalyzer {
private:
    Grammar grammar;
    vector<ItemSet> item_sets;                    // 项目集族
    map<pair<int,string>, string> action_table;   // ACTION表
    map<pair<int,string>, int> goto_table;        // GOTO表
    
    enum AnalyzerType { LR0, SLR1, LR1 } type;

public:
    LRAnalyzer(AnalyzerType t = SLR1);
    
    // 主要接口
    bool loadGrammar(const string& filename);
    bool constructParseTable();
    bool parseString(const vector<string>& input);
    
    // 结果输出
    void printActionTable() const;
    void printGotoTable() const;
    void printItemSets() const;
    
    // JSON输出(GUI接口用)
    string getActionTableJSON() const;
    string getGotoTableJSON() const;
    string getItemSetsJSON() const;

private:
    // 内部算法实现
    void constructLR0ItemSets();
    void constructSLR1Table();
    void constructLR1ItemSets();
    void constructLR1Table();
    
    bool hasConflicts() const;
    void reportConflicts() const;
};
```

## 关键算法实现

### 1. 项目集闭包计算

```cpp
ItemSet ItemSet::closure(const Grammar& grammar) const {
    ItemSet result = *this;
    bool changed = true;
    
    while (changed) {
        changed = false;
        ItemSet newItems;
        
        for (const LRItem& item : result) {
            if (!item.dotAtEnd()) {
                string nextSymbol = item.getNextSymbol();
                
                if (grammar.isNonterminal(nextSymbol)) {
                    // 对该非终结符的所有产生式
                    vector<int> productions = grammar.getProductionsFor(nextSymbol);
                    
                    for (int prodId : productions) {
                        LRItem newItem(prodId, 0);
                        
                        // LR(1)需要计算向前看符号
                        if (type == LR1) {
                            set<string> newLookaheads = computeLookaheads(item, grammar);
                            newItem = LRItem(prodId, 0, newLookaheads);
                        }
                        
                        if (!result.contains(newItem)) {
                            newItems.addItem(newItem);
                            changed = true;
                        }
                    }
                }
            }
        }
        
        // 将新项目加入结果集
        for (const LRItem& item : newItems) {
            result.addItem(item);
        }
    }
    
    return result;
}
```

### 2. GOTO函数实现

```cpp
ItemSet ItemSet::gotoSet(const string& symbol, const Grammar& grammar) const {
    ItemSet result;
    
    // 收集可以通过symbol转移的项目
    for (const LRItem& item : items) {
        if (!item.dotAtEnd() && item.getNextSymbol() == symbol) {
            // 创建点向前移动一位的新项目
            LRItem newItem(item.getProductionId(), 
                          item.getDotPosition() + 1,
                          item.getLookaheads());
            result.addItem(newItem);
        }
    }
    
    // 计算闭包
    return result.closure(grammar);
}
```

### 3. 语法分析器

```cpp
bool LRAnalyzer::parseString(const vector<string>& input) {
    stack<int> stateStack;
    stack<string> symbolStack;
    vector<string> inputBuffer = input;
    
    stateStack.push(0);  // 初始状态
    inputBuffer.push_back("$");  // 结束符
    
    size_t inputPos = 0;
    int step = 0;
    
    // 分析主循环
    while (true) {
        int currentState = stateStack.top();
        string currentSymbol = inputBuffer[inputPos];
        
        // 查ACTION表
        auto actionKey = make_pair(currentState, currentSymbol);
        if (action_table.find(actionKey) == action_table.end()) {
            reportError(step, stateStack, symbolStack, inputBuffer, inputPos);
            return false;
        }
        
        string action = action_table[actionKey];
        
        // 输出当前步骤
        printStep(step++, stateStack, symbolStack, inputBuffer, inputPos, action);
        
        if (action == "acc") {
            cout << "\n✓ 分析成功！输入串被接受。" << endl;
            return true;
            
        } else if (action[0] == 's') {
            // 移进动作
            int nextState = stoi(action.substr(1));
            stateStack.push(nextState);
            symbolStack.push(currentSymbol);
            inputPos++;
            
        } else if (action[0] == 'r') {
            // 归约动作
            int prodId = stoi(action.substr(1));
            Production prod = grammar.getProduction(prodId);
            
            cout << "\t\t\t\t归约用产生式: " << prod.toString() << endl;
            
            // 弹出产生式右部对应的状态和符号
            for (size_t i = 0; i < prod.right.size(); i++) {
                stateStack.pop();
                symbolStack.pop();
            }
            
            // 压入产生式左部
            symbolStack.push(prod.left);
            
            // 查GOTO表确定新状态
            auto gotoKey = make_pair(stateStack.top(), prod.left);
            int gotoState = goto_table[gotoKey];
            stateStack.push(gotoState);
            
        } else {
            reportError(step, stateStack, symbolStack, inputBuffer, inputPos);
            return false;
        }
    }
}
```

## 接口设计

### 1. 命令行接口

```cpp
// lr_cli.cpp - 支持批处理和脚本调用
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }
    
    string grammarFile = argv[1];
    AnalyzerType type = SLR1;  // 默认
    string inputString = "";
    bool showTable = false;
    bool showItems = false;
    bool jsonOutput = false;
    
    // 解析命令行参数
    for (int i = 2; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-t" || arg == "--type") {
            type = parseAnalyzerType(argv[++i]);
        } else if (arg == "-s" || arg == "--string") {
            inputString = argv[++i];
        } else if (arg == "--table") {
            showTable = true;
        } else if (arg == "--items") {
            showItems = true;
        } else if (arg == "--json") {
            jsonOutput = true;
        }
    }
    
    // 执行分析
    LRAnalyzer analyzer(type);
    if (!analyzer.loadGrammar(grammarFile)) {
        return 1;
    }
    
    if (!analyzer.constructParseTable()) {
        return 1;
    }
    
    // 输出结果
    if (jsonOutput) {
        outputJSON(analyzer, showTable, showItems);
    } else {
        outputText(analyzer, showTable, showItems, inputString);
    }
    
    return 0;
}
```

### 2. 交互式接口

```cpp
// main.cpp - 用户友好的交互界面
int main() {
    LRAnalyzer analyzer;
    
    cout << "欢迎使用LR语法分析器！" << endl;
    
    while (true) {
        displayMenu();
        int choice = getUserChoice();
        
        switch (choice) {
            case 1:
                loadGrammarInteractive(analyzer);
                break;
            case 2:
                selectAnalyzerType(analyzer);
                break;
            case 3:
                constructTableInteractive(analyzer);
                break;
            case 4:
                analyzeStringInteractive(analyzer);
                break;
            case 5:
                showResultsInteractive(analyzer);
                break;
            case 0:
                cout << "再见！" << endl;
                return 0;
        }
    }
}
```

## 错误处理机制

### 1. 文法验证
```cpp
bool Grammar::validateGrammar() {
    // 检查开始符号
    if (start_symbol.empty() || productions.empty()) {
        cerr << "错误：文法为空或缺少开始符号" << endl;
        return false;
    }
    
    // 检查产生式格式
    for (const Production& prod : productions) {
        if (prod.left.empty() || prod.right.empty()) {
            cerr << "错误：产生式格式不正确" << endl;
            return false;
        }
        
        // 检查左部是否为非终结符
        if (terminals.count(prod.left)) {
            cerr << "错误：产生式左部不能是终结符：" << prod.left << endl;
            return false;
        }
    }
    
    // 检查符号引用
    for (const Production& prod : productions) {
        for (const string& symbol : prod.right) {
            if (!terminals.count(symbol) && !nonterminals.count(symbol)) {
                cerr << "错误：未定义的符号：" << symbol << endl;
                return false;
            }
        }
    }
    
    return true;
}
```

### 2. 冲突检测
```cpp
bool LRAnalyzer::hasConflicts() const {
    for (const auto& entry : action_table) {
        const string& action = entry.second;
        
        // 检查是否有多个动作定义
        if (action.find('/') != string::npos) {
            return true;  // 发现冲突
        }
    }
    return false;
}

void LRAnalyzer::reportConflicts() const {
    cout << "错误: 分析表构造失败\n" << endl;
    cout << "✗ 检测到冲突：" << endl;
    
    for (const auto& entry : action_table) {
        const auto& key = entry.first;
        const string& action = entry.second;
        
        if (action.find('/') != string::npos) {
            cout << "  ";
            if (action.find('s') != string::npos && action.find('r') != string::npos) {
                cout << "移进/归约冲突";
            } else if (action.find('r') != string::npos) {
                cout << "归约/归约冲突";
            }
            cout << "在状态 " << key.first << " 符号 " << key.second << endl;
        }
    }
}
```

## 性能优化

### 1. 内存优化
- 使用`set`和`map`等STL容器提高查找效率
- 项目集去重避免重复计算
- 智能指针管理动态内存

### 2. 计算优化  
- FIRST/FOLLOW集缓存计算结果
- 项目集构造使用工作列表算法
- 分析表压缩存储

### 3. 输出优化
- JSON格式支持结构化数据交换
- 增量更新减少重复输出
- 异步处理提高响应性

这种模块化的设计使得系统具有良好的可维护性和可扩展性，同时保证了算法实现的正确性和效率。
