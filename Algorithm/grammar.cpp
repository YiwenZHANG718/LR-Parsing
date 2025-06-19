/**
 * @file grammar.cpp
 * @brief 上下文无关文法的定义和处理
 *
 * 本文件定义了上下文无关文法的表示和基本操作，包括：
 * - 产生式的表示和操作
 * - 文法的存储和管理
 * - 文法的读取（从文件或标准输入）
 * - 文法的增广（添加新的开始符号）
 * - 终结符和非终结符的自动识别
 * - 文法验证和错误检测
 *
 * @author B22040310朱家骏
 */

#include "grammar.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <queue>

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
 * @brief 从指定文件读取并解析文法
 * 
 * 文件解析过程：
 * 1. 逐行读取文件内容
 * 2. 跳过空行和以#开头的注释行
 * 3. 对每一行进行格式验证
 * 4. 解析产生式并添加到文法中
 * 5. 执行完整的文法验证
 * 
 * @param filename 文法文件的路径
 * @return 文件成功读取且文法验证通过返回true，否则返回false
 *
 * 支持的文件格式：
 * - 每行一个产生式，格式为 A -> alpha | beta
 * - 以#开头的行为注释，会被忽略
 * - 空行会被忽略
 * - 支持多选择分支（用|分隔）
 */
bool Grammar::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        reportError("Error: Cannot open file: " + filename);
        return false;
    }

    std::string line;
    int lineNumber = 0;
      while (std::getline(file, line)) {
        lineNumber++;
        
        // 去除行首行尾空白字符
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        // 忽略空行和注释行
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // 检查合法性
        if (!isValidProductionFormat(line)) {
            reportError("Error: Invalid production format at line " + std::to_string(lineNumber) + ": " + line);
            return false;
        }
        
        parseProduction(line);
    }

    return validateGrammar();
}

/**
 * @brief 从标准输入读取并验证文法
 *
 * 交互式输入过程：
 * 1. 显示格式说明和示例
 * 2. 逐行读取用户输入
 * 3. 对每行进行格式验证
 * 4. 解析有效的产生式
 * 5. 执行完整的文法验证
 * 
 * 用户输入空行时结束输入。
 * 如果发现格式错误或文法无效，会输出错误信息。
 */
void Grammar::loadFromInput() {
    std::cout << "请输入文法（每行一个产生式，空行结束）：" << std::endl;
    std::cout << "格式：A -> alpha | beta" << std::endl;
    std::cout << "示例：E -> E + T | T" << std::endl;

    std::string line;
    int lineNumber = 0;
    bool hasError = false;
    
    while (std::getline(std::cin, line)) {
        if (line.empty()) break;
        
        lineNumber++;
        
        // 验证产生式格式
        if (!isValidProductionFormat(line)) {
            reportError("Error: Invalid production format at line " + std::to_string(lineNumber) + ": " + line);
            hasError = true;
            continue; // 继续读取但标记为错误
        }
        
        parseProduction(line);
    }
    
    // 仅在没有格式错误时验证文法
    if (!hasError) {
        validateGrammar();
    }
}

/**
 * @brief 解析单行产生式字符串并添加到文法中
 * 
 * 解析步骤：
 * 1. 查找 "->" 分隔符，分离左部和右部
 * 2. 去除左部的空白字符，获得非终结符
 * 3. 处理右部的多个选择分支（用|分隔）
 * 4. 对每个分支，按空格分离各个符号
 * 5. 创建产生式对象并添加到产生式列表
 * 
 * @param line 包含产生式的字符串
 *
 * 特殊处理：
 * - 自动去除多余的空白字符
 * - 空右部自动转换为ε产生式
 * - 支持 A -> alpha | beta 的多选择格式
 */
void Grammar::parseProduction(const std::string& line) {
    size_t arrowPos = line.find("->");
    if (arrowPos == std::string::npos) return;

    std::string left = line.substr(0, arrowPos);
    std::string right = line.substr(arrowPos + 2);

    // 去除左部空白字符（找出所有空格、tab、换行等字符，把它们“移动”到序列末尾，然后一次性删掉这些空白）
    left.erase(std::remove_if(left.begin(), left.end(), ::isspace), left.end());

    // 处理右部的多个选择分支（用|分隔）
    std::stringstream ss(right);
    std::string choice;

    while (std::getline(ss, choice, '|')) {
        std::vector<std::string> symbols;
        std::stringstream choiceSS(choice);
        std::string symbol;

        // 提取右部符号
        while (choiceSS >> symbol) {
            symbols.push_back(symbol);
        }

        // 处理空右部
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

    // 第二遍：提取所有终结符
    for (const auto& prod : productions) {
        for (const auto& symbol : prod.right) {
            // 只将非终结符以外的符号标记为终结符
            if (symbol != "ε" && nonterminals.find(symbol) == nonterminals.end()) {
                terminals.insert(symbol);
            }
        }
    }

    // 确定开始符号
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
    // 检查是否已经增广过（开始符号以单引号结尾）
    if (startSymbol.back() == '\'') {
        return; // 已经增广过，直接返回
    }
    
    std::string newStart = startSymbol + "'";
    productions.insert(productions.begin(), Production(newStart, { startSymbol }));
    nonterminals.insert(newStart);
    startSymbol = newStart;

    // 添加结束符号
    terminals.insert("$");
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
    std::cout << "Grammar Productions:" << std::endl;
    for (size_t i = 0; i < productions.size(); ++i) {
        std::cout << i << ": " << productions[i].toString() << std::endl;
    }

    std::cout << "\nNonterminals:";
    for (const auto& nt : nonterminals) {
        std::cout << " " << nt;
    }
    std::cout << std::endl;

    std::cout << "Terminals:";
    for (const auto& t : terminals) {
        std::cout << " " << t;
    }
    std::cout << std::endl;

    std::cout << "Start Symbol: " << startSymbol << std::endl;
}

/**
 * @brief 验证文法的完整性和正确性
 * 
 * 执行以下验证步骤：
 * 1. 检查文法是否为空（无产生式）
 * 2. 提取并验证终结符和非终结符
 * 3. 验证起始符号的存在性
 * 4. 检查所有非终结符是否都能推导出终结符串
 * 5. 检查是否存在从起始符号无法到达的产生式
 * 
 * @return 文法完全有效返回true，发现任何错误返回false
 */
bool Grammar::validateGrammar() {
    // 1. 检查空文法
    if (productions.empty()) {
        reportError("Error: Grammar file is empty, no productions found");
        return false;
    }
    
    // 2. 提取符号
    extractSymbols();
    
    // 3. 检查起始符号
    if (startSymbol.empty()) {
        reportError("Error: No start symbol found");
        return false;
    }
    // 4. 检查符号可达性
    if (!hasReachableSymbols()) {
        return false;
    }
    
    // 5. 检查无用产生式
    if (!checkReachableProductions()) {
        return false;
    }
    
    return true;
}

/**
 * @brief 验证产生式的语法格式
 * 
 * 检查产生式是否符合标准格式：
 * - 必须包含 "->" 分隔符
 * - 左部不能为空
 * - 左部必须是有效的非终结符标识符
 * 
 * @param line 待验证的产生式字符串
 * @return 格式完全正确返回true，存在格式错误返回false
 */
bool Grammar::isValidProductionFormat(const std::string& line) {
    // 检查是否包含 "->" 分隔符
    size_t arrowPos = line.find("->");
    if (arrowPos == std::string::npos) {
        return false;
    }
    
    // 检查左部是否为空
    std::string left = line.substr(0, arrowPos);
    left.erase(std::remove_if(left.begin(), left.end(), ::isspace), left.end());
    if (left.empty()) {
        return false;
    }
    
    // 检查括号匹配
    if (!checkParenthesesBalance(line)) {
        return false;
    }
    
    return true;
}

/**
 * @brief 检查产生式中的括号是否匹配
 * 
 * 支持的括号类型：
 * - 圆括号 ()
 * - 方括号 []
 * - 花括号 {}
 * 
 * 检查规则：
 * 1. 左括号和右括号数量必须相等
 * 2. 任何位置左括号数量都不能少于右括号数量
 * 3. 不同类型的括号必须正确嵌套
 * 
 * @param line 待检查的产生式字符串
 * @return 括号匹配返回true，存在不匹配返回false
 */
bool Grammar::checkParenthesesBalance(const std::string& line) {
    std::vector<char> stack;
    
    for (char c : line) {
        // 遇到左括号，入栈
        if (c == '(' || c == '[' || c == '{') {
            stack.push_back(c);
        }
        // 遇到右括号，检查匹配
        else if (c == ')' || c == ']' || c == '}') {
            if (stack.empty()) {
                // 右括号多于左括号
                return false;
            }
            
            char top = stack.back();
            stack.pop_back();
            
            // 检查括号类型是否匹配
            if ((c == ')' && top != '(') ||
                (c == ']' && top != '[') ||
                (c == '}' && top != '{')) {
                return false;
            }
        }
    }
    
    // 栈应该为空，表示所有括号都匹配
    return stack.empty();
}

/**
 * @brief 检查所有非终结符是否都能推导出终结符串
 * 
 * 使用固定点算法迭代计算：
 * 1. 初始化所有终结符为可推导
 * 2. 迭代检查每个产生式，如果右部所有符号都可推导，则左部也可推导
 * 3. 重复直到没有新的可推导符号被发现
 * 4. 检查是否所有非终结符都在可推导集合中
 * 
 * @return 所有非终结符都能推导出终结符串返回true，存在无法推导的非终结符返回false
 */
bool Grammar::hasReachableSymbols() {
    // 1. 初始化：把 所有终结符 都标记为 已知可以推导出终结符串
    std::set<std::string> canDeriveTerminals;
    //std::set<std::string> visited;
        for (const auto& terminal : terminals) {
        canDeriveTerminals.insert(terminal);
    }
    
    // 2. 固定点迭代：只要新增了可推导的非终结符就继续
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& prod : productions) {
            // 如果左部已经在可推导集合中，跳过
            if (canDeriveTerminals.find(prod.left) != canDeriveTerminals.end()) {
                continue; 
            }
            
            // 检查右部的每个符号是否都“已知可推导”
            bool canDerive = true;
            for (const auto& symbol : prod.right) {
                if (symbol == "ε") {
                    continue; // ε跳过
                }
                // 如果符号是非终结符，检查它是否在可推导集合中
                if (canDeriveTerminals.find(symbol) == canDeriveTerminals.end()) {
                    canDerive = false;
                    break;
                }
            }
            
            if (canDerive) {
                canDeriveTerminals.insert(prod.left);
                changed = true;
            }
        }
    }

    // 3. 最终检查：跳过增广符号（末尾带'），其余非终结符都必须在 canDeriveTerminals 中
    for (const auto& nt : nonterminals) {
        if (nt.back() == '\'') continue; // 跳过推广符号
        if (canDeriveTerminals.find(nt) == canDeriveTerminals.end()) {
            reportError("Error: Nonterminal '" + nt + "' cannot derive any terminal string");
            return false;
        }
    }
    
    return true;
}

/**
 * @brief 报告错误信息
 * @param message 错误消息
 */
void Grammar::reportError(const std::string& message) {
    if (!g_silent_mode) {
        std::cerr << message << std::endl;
    }
}

/**
 * @brief 检测从起始符号无法到达的产生式和符号
 * 
 * 使用广度优先搜索算法：
 * 1. 从起始符号开始，将其加入可达符号集合
 * 2. 遍历所有以可达符号为左部的产生式
 * 3. 将产生式右部的所有符号标记为可达
 * 4. 重复此过程直到没有新的可达符号
 * 5. 检查是否存在不可达的非终结符
 * 
 * @return 所有非终结符都从起始符号可达返回true，存在不可达符号返回false
 */
bool Grammar::checkReachableProductions() {
    std::set<std::string> reachableSymbols;
    std::queue<std::string> toProcess;
    
    // 从起始符号开始
    reachableSymbols.insert(startSymbol);
    toProcess.push(startSymbol);
    
    // 使用 BFS 查找所有可达符号
    while (!toProcess.empty()) {
        std::string current = toProcess.front();
        toProcess.pop();
        
        // 查找所有左部为当前符号的产生式
        for (const auto& prod : productions) {
            if (prod.left == current) {
                // 将右部所有符号加入可达集合
                for (const auto& symbol : prod.right) {
                    if (symbol != "ε" && reachableSymbols.find(symbol) == reachableSymbols.end()) {
                        reachableSymbols.insert(symbol);
                        // 如果是非终结符，则加入待处理队列
                        if (nonterminals.find(symbol) != nonterminals.end()) {
                            toProcess.push(symbol);
                        }
                    }
                }
            }
        }
    }

    // 检查不可达的非终结符
    for (const auto& nt : nonterminals) {
        if (nt.back() == '\'') continue; 
        if (reachableSymbols.find(nt) == reachableSymbols.end()) {
            reportError("Error: Nonterminal '" + nt + "' is unreachable from start symbol '" + startSymbol + "'");
            return false;
        }
    }
    
    return true;
}
