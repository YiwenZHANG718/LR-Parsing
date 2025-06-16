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

## JSON接口设计

### 1. 数据交换格式
```cpp
// JSON输出接口 - 供GUI调用
class JSONExporter {
public:
    static string exportActionTable(const LRAnalyzer& analyzer) {
        json result;
        result["type"] = "action_table";
        result["method"] = analyzer.getMethodName();
        result["success"] = !analyzer.hasConflicts();
        
        json table;
        for (const auto& entry : analyzer.getActionTable()) {
            string key = to_string(entry.first.first) + "," + entry.first.second;
            table[key] = entry.second;
        }
        result["table"] = table;
        
        if (analyzer.hasConflicts()) {
            result["conflicts"] = exportConflicts(analyzer);
        }
        
        return result.dump(4);
    }
    
    static string exportParseSteps(const vector<ParseStep>& steps) {
        json result;
        result["type"] = "parse_steps";
        result["steps"] = json::array();
        
        for (const auto& step : steps) {
            json stepJson;
            stepJson["step"] = step.stepNumber;
            stepJson["stack"] = step.stateStack;
            stepJson["symbols"] = step.symbolStack;
            stepJson["input"] = step.remainingInput;
            stepJson["action"] = step.action;
            stepJson["description"] = step.description;
            result["steps"].push_back(stepJson);
        }
        
        return result.dump(4);
    }
};
```

### 2. GUI通信协议
```cpp
// 命令行接口统一输出格式
class OutputFormatter {
public:
    enum OutputMode { TEXT, JSON, XML };
    
    static void setMode(OutputMode mode) { currentMode = mode; }
    
    static void outputResult(const LRAnalyzer& analyzer, const string& input = "") {
        switch (currentMode) {
            case JSON:
                outputJSON(analyzer, input);
                break;
            case XML:
                outputXML(analyzer, input);
                break;
            default:
                outputText(analyzer, input);
        }
    }
    
private:
    static OutputMode currentMode;
    
    static void outputJSON(const LRAnalyzer& analyzer, const string& input) {
        json result;
        result["grammar"] = analyzer.getGrammarJSON();
        result["itemSets"] = analyzer.getItemSetsJSON();
        result["actionTable"] = analyzer.getActionTableJSON();
        result["gotoTable"] = analyzer.getGotoTableJSON();
        
        if (!input.empty()) {
            result["parseResult"] = analyzer.parseStringJSON(input);
        }
        
        cout << result.dump(4) << endl;
    }
};
```

## 扩展功能实现

### 1. 语法制导翻译
```cpp
class SyntaxDirectedTranslator {
    struct SemanticAction {
        int productionId;
        function<void(vector<any>&)> action;
    };
    
    map<int, SemanticAction> semanticActions;
    
public:
    void addAction(int prodId, function<void(vector<any>&)> action) {
        semanticActions[prodId] = {prodId, action};
    }
    
    any translateParse(const vector<string>& input, LRAnalyzer& analyzer) {
        stack<any> semanticStack;
        // ... 修改后的分析过程，在归约时执行语义动作
        return semanticStack.top();
    }
};

// 示例：算术表达式求值
void setupArithmeticActions(SyntaxDirectedTranslator& translator) {
    // E -> E + T 
    translator.addAction(1, [](vector<any>& attrs) {
        double right = any_cast<double>(attrs[2]);
        double left = any_cast<double>(attrs[0]);
        attrs.clear();
        attrs.push_back(left + right);
    });
    
    // T -> T * F
    translator.addAction(3, [](vector<any>& attrs) {
        double right = any_cast<double>(attrs[2]);
        double left = any_cast<double>(attrs[0]);
        attrs.clear();
        attrs.push_back(left * right);
    });
}
```

### 2. 错误恢复机制
```cpp
class ErrorRecovery {
public:
    enum Strategy {
        PANIC_MODE,      // 恐慌模式恢复
        PHRASE_LEVEL,    // 短语级恢复
        ERROR_PRODUCTION // 错误产生式
    };
    
    struct RecoveryPoint {
        int state;
        string syncSymbol;
        string errorMessage;
    };
    
    bool recover(ParsingContext& context, Strategy strategy = PANIC_MODE) {
        switch (strategy) {
            case PANIC_MODE:
                return panicModeRecovery(context);
            case PHRASE_LEVEL:
                return phraseLevelRecovery(context);
            case ERROR_PRODUCTION:
                return errorProductionRecovery(context);
        }
        return false;
    }
    
private:
    set<string> syncSymbols = {";", "}", ")", "end", "else"};
    
    bool panicModeRecovery(ParsingContext& context) {
        // 跳过输入直到同步符号
        while (!context.atEnd()) {
            if (syncSymbols.count(context.currentSymbol())) {
                return true;
            }
            context.advance();
        }
        return false;
    }
};
```

### 3. 分析表优化
```cpp
class TableOptimizer {
public:
    // 合并相似状态减少表大小
    void mergeSimilarStates(LRAnalyzer& analyzer) {
        auto& states = analyzer.getItemSets();
        map<string, vector<int>> stateGroups;
        
        // 按核心项目分组
        for (int i = 0; i < states.size(); i++) {
            string coreSignature = states[i].getCoreSignature();
            stateGroups[coreSignature].push_back(i);
        }
        
        // 合并兼容状态
        for (auto& group : stateGroups) {
            if (group.second.size() > 1) {
                mergeStates(analyzer, group.second);
            }
        }
    }
    
    // 压缩稀疏表
    void compressSparseTables(LRAnalyzer& analyzer) {
        // 实现稀疏矩阵压缩算法
        compressActionTable(analyzer);
        compressGotoTable(analyzer);
    }
};
```

## 测试框架

### 1. 单元测试
```cpp
class LRAnalyzerTest {
public:
    void runAllTests() {
        testGrammarLoading();
        testFirstFollowComputation();
        testItemSetConstruction();
        testTableConstruction();
        testParsing();
    }
    
private:
    void testItemSetConstruction() {
        Grammar g;
        g.loadFromString("E -> E + T | T\nT -> T * F | F\nF -> ( E ) | id");
        
        LRAnalyzer analyzer(LR1);
        analyzer.loadGrammar(g);
        analyzer.constructParseTable();
        
        // 验证项目集数量和内容
        assert(analyzer.getItemSetCount() == 12);
        assert(!analyzer.hasConflicts());
    }
    
    void testParsing() {
        // 测试各种输入串
        vector<string> inputs = {
            {"id", "+", "id", "*", "id"},
            {"(", "id", "+", "id", ")", "*", "id"},
            {"id", "*", "(", "id", "+", "id", ")"}
        };
        
        for (auto& input : inputs) {
            assert(analyzer.parseString(input));
        }
    }
};
```

### 2. 性能测试
```cpp
class PerformanceBenchmark {
public:
    void benchmarkConstruction() {
        vector<string> grammarFiles = {
            "simple_grammar.txt",
            "medium_grammar.txt", 
            "complex_grammar.txt"
        };
        
        for (const string& file : grammarFiles) {
            auto start = chrono::high_resolution_clock::now();
            
            LRAnalyzer analyzer(LR1);
            analyzer.loadGrammar(file);
            analyzer.constructParseTable();
            
            auto end = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
            
            cout << file << ": " << duration.count() << "ms" << endl;
            cout << "状态数: " << analyzer.getItemSetCount() << endl;
            cout << "内存使用: " << getMemoryUsage() << "KB" << endl;
        }
    }
};
```

## 部署和分发

### 1. 跨平台编译
```bash
# Windows (MSYS2/MinGW)
g++ -std=c++17 -O2 *.cpp -o lr_analyzer.exe

# Linux/macOS
g++ -std=c++17 -O2 *.cpp -o lr_analyzer

# 静态链接版本（便于分发）
g++ -std=c++17 -O2 -static *.cpp -o lr_analyzer_static
```

### 2. 安装脚本
```cpp
// install.cpp - 自动化安装程序
class Installer {
public:
    bool install(const string& targetDir) {
        // 1. 检查系统环境
        if (!checkSystemRequirements()) {
            return false;
        }
        
        // 2. 复制文件
        if (!copyFiles(targetDir)) {
            return false;
        }
        
        // 3. 设置环境变量
        if (!setupEnvironment(targetDir)) {
            return false;
        }
        
        // 4. 验证安装
        return verifyInstallation(targetDir);
    }
    
private:
    bool checkSystemRequirements() {
        // 检查编译器、Python版本等
        return checkCompiler() && checkPython() && checkDiskSpace();
    }
};
```

## 项目维护

### 1. 版本控制
- 主版本号：重大架构变更
- 次版本号：新功能添加
- 修订版本号：错误修复和小改进

### 2. 文档维护
- API文档自动生成（Doxygen）
- 用户手册定期更新
- 示例代码验证

### 3. 质量保证
- 代码审查流程
- 自动化测试集成
- 性能回归测试

这种模块化的设计使得系统具有良好的可维护性和可扩展性，同时保证了算法实现的正确性和效率。项目架构支持持续集成和持续部署，便于长期维护和功能扩展。
