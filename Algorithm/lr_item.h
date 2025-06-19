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

#ifndef LR_ITEM_H
#define LR_ITEM_H

#include <string>
#include <vector>
#include <set>
#include <map>

/**
 * @brief LR项目类 - 表示LR分析中的一个项目
 *
 * LR项目的形式为：A -> α·β，其中：
 * - A是产生式的左部（非终结符）
 * - α·β是产生式的右部，点(·)表示当前分析位置
 * - 点左边的α是已经识别的部分
 * - 点右边的β是期待识别的部分
 */
class LRItem {
public:
    std::string left;                    // 产生式左部（非终结符）
    std::vector<std::string> right;      // 产生式右部（符号序列）
    size_t dotPos;                       // 点的位置（0到right.size()）
    std::set<std::string> lookaheads;    // 前瞻符号集合（用于LR(1)分析）

    // LR(0)项目构造函数
    LRItem(const std::string& l, const std::vector<std::string>& r, size_t pos = 0)
        : left(l), right(r), dotPos(pos) {}
    //LR(1)项目构造函数，带有前瞻符号集合
    LRItem(const std::string& l, const std::vector<std::string>& r, size_t pos,
        const std::set<std::string>& la)
        : left(l), right(r), dotPos(pos), lookaheads(la) {}

    // 重载比较运算符    
    bool operator==(const LRItem& other) const;
    bool operator<(const LRItem& other) const;

    std::string getNextSymbol() const;
    bool isReduceItem() const;
    bool isAcceptItem() const;
    std::string toString() const;
};

/**
 * @brief 项目集类 - 表示LR分析中的一个状态（项目集合）
 *
 * 在LR分析中，每个状态对应一个项目集。项目集是若干LR项目的集合，
 * 表示在某个分析状态下所有可能的分析情况。
 *
 * 例如：I0 = {E' -> ·E, E -> ·E + T, E -> ·T, T -> ·F, F -> ·id}
 */
class ItemSet {
public:
    std::set<LRItem> items;  // 项目集合，使用set保证唯一性和有序性
    int id;                  // 项目集的编号（状态编号）

    ItemSet() : id(-1) {}
    ItemSet(const std::set<LRItem>& itemSet) : items(itemSet), id(-1) {}

    // 重载比较运算符（不比较 id ）
    bool operator==(const ItemSet& other) const {
        return items == other.items;
    }
    bool operator<(const ItemSet& other) const {
        return items < other.items;
    }

    void print() const;
};

#endif // LR_ITEM_H
