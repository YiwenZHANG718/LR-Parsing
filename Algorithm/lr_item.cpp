/**
 * @file lr_item.h
 * @brief LR项目和项目集的定义与操作
 *
 * 本文件定义了LR语法分析中的核心数据结构：
 * - LRItem类：表示单个LR项目
 * - ItemSet类：表示项目集合
 * - 相关的操作函数：闭包计算、GOTO函数等
 *
 * LR项目是LR分析的基础概念，形式为A -> α·β，其中：
 * - A是产生式左部（非终结符）
 * - α·β是产生式右部，点(·)标记当前分析进度
 * - 点左边是已识别部分，点右边是期待识别部分
 *
 * 项目集是LR项目的集合，对应于LR自动机的一个状态。
 * 通过计算项目集的闭包和GOTO函数，可以构造LR分析表。
 *
 * @author B22040310朱家骏
 */

#include "lr_item.h"
#include <iostream>

/**
 * @brief 比较两个LR项目是否相等
 *
 * 两个LR项目相等当且仅当：
 * 1. 产生式左部相同
 * 2. 产生式右部相同
 * 3. 点的位置相同
 * 4. 前瞻符号集合相同（对于LR(1)项目）
 */
bool LRItem::operator==(const LRItem& other) const {
    return left == other.left && right == other.right && dotPos == other.dotPos && lookaheads == other.lookaheads;
}

/**
 * @brief 定义LR项目的字典序比较
 *
 * 比较顺序：
 * 1. 首先比较产生式左部
 * 2. 然后比较产生式右部
 * 3. 再比较点的位置
 * 4. 最后比较前瞻符号集合
 */
bool LRItem::operator<(const LRItem& other) const {
    if (left != other.left) return left < other.left;
    if (right != other.right) return right < other.right;
    if (dotPos != other.dotPos) return dotPos < other.dotPos;
    return lookaheads < other.lookaheads;
}

/**
 * @brief 获取点后面的符号
 *
 * 在LR项目中，点表示当前的分析位置。这个函数返回点后面的第一个符号。
 * 如果点已经在产生式右部的最后，说明该符号序列已经完全识别，返回空字符串。
 *
 * @return 点后面的符号，如果点在最后则返回空字符串
 */
std::string LRItem::getNextSymbol() const {
    if (dotPos < right.size()) {
        return right[dotPos];
    }
    return "";
}

/**
 * @brief 判断是否为归约项目
 *
 * 归约项目是指点位于产生式右部最后位置的项目，表示已经完全识别了
 * 产生式的右部，可以进行归约操作。
 *
 * 特殊情况：对于空产生式（right = {"ε"}），也被认为是归约项目。
 *
 * @return 如果是归约项目返回true，否则返回false
 */
bool LRItem::isReduceItem() const {
    return dotPos >= right.size() || (right.size() == 1 && right[0] == "ε");
}

/**
 * @brief 判断是否为接受项目
 *
 * 接受项目是增广文法中起始产生式对应的归约项目，形如 S' -> S·
 * 当分析器遇到接受项目时，表示整个输入串已经被成功分析。
 *
 * 增广文法的起始符号通常以单引号(')结尾来标识。
 *
 * @return 如果是接受项目返回true，否则返回false
 */
bool LRItem::isAcceptItem() const {
    return left.back() == '\'' && isReduceItem();
}

/**
 * @brief 将LR项目转换为可读的字符串表示
 *
 * 输出格式：A -> α·β
 * 其中·表示当前分析位置
 *
 * 特殊处理：
 * 1. 对于空产生式，显示为 A -> ·ε
 * 2. 如果有前瞻符号，会在后面添加，格式为 A -> α·β, {a, b, c}
 *
 * @return 项目的字符串表示
 */
std::string LRItem::toString() const {
    std::string result = left + " -> ";

    // 处理空产生式的特殊情况
    if (right.size() == 1 && right[0] == "ε") {
        result += ". ε";
    }
    else {
        // 在适当位置插入点符号
        for (size_t i = 0; i < right.size(); ++i) {
            if (i == dotPos) result += ". ";
            result += right[i];
            if (i < right.size() - 1) result += " ";
        }
        // 如果点在最后，在末尾添加点
        if (dotPos >= right.size()) result += " .";
    }

    // 如果有前瞻符号，添加到字符串末尾
    if (!lookaheads.empty()) {
        result += ", {";
        bool first = true;
        for (const auto& la : lookaheads) {
            if (!first) result += ", ";
            result += la;
            first = false;
        }
        result += "}";
    }

    return result;
}

/**
 * @brief 打印项目集的内容
 *
 * 输出格式：
 * I{id}:
 *   项目1
 *   项目2
 *   ...
 *
 * 每个项目都会调用toString()方法来格式化输出
 */
void ItemSet::print() const {
    std::cout << "I" << id << ":" << std::endl;
    for (const auto& item : items) {
        std::cout << "  " << item.toString() << std::endl;
    }
    std::cout << std::endl;
}
