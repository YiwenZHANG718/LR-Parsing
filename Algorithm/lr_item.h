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
 * @author ZJJ
 * @date 2025.6.10
 * @version 2.0
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

    /**
     * @brief 构造LR项目（不带前瞻符号）
     * @param l 产生式左部
     * @param r 产生式右部符号序列
     * @param pos 点的位置，默认为0（点在最左边）
     */
    LRItem(const std::string& l, const std::vector<std::string>& r, size_t pos = 0)
        : left(l), right(r), dotPos(pos) {}

    /**
     * @brief 构造LR(1)项目（带前瞻符号）
     * @param l 产生式左部
     * @param r 产生式右部符号序列
     * @param pos 点的位置
     * @param la 前瞻符号集合
     */
    LRItem(const std::string& l, const std::vector<std::string>& r, size_t pos,
        const std::set<std::string>& la)
        : left(l), right(r), dotPos(pos), lookaheads(la) {}

    /**
     * @brief 判断两个LR项目是否相等
     * @param other 另一个LR项目
     * @return 如果产生式、点位置和前瞻符号都相同则返回true
     */
    bool operator==(const LRItem& other) const;

    /**
     * @brief 定义LR项目的排序规则（用于在set中存储）
     * @param other 另一个LR项目
     * @return 按字典序比较的结果
     */
    bool operator<(const LRItem& other) const;

    /**
     * @brief 获取点后面的符号
     * @return 点后面的第一个符号，如果点在最后则返回空字符串
     *
     * 例如：对于项目 E -> E + ·T，返回 "T"
     *      对于项目 E -> E + T·，返回 ""
     */
    std::string getNextSymbol() const;

    /**
     * @brief 判断是否为归约项目
     * @return 如果点在产生式右部的最后位置则返回true
     *
     * 归约项目表示已经完全识别了产生式的右部，可以进行归约操作
     * 例如：E -> E + T· 是归约项目
     */
    bool isReduceItem() const;

    /**
     * @brief 判断是否为接受项目
     * @return 如果是增广文法的起始项目且为归约项目则返回true
     *
     * 接受项目形如：S' -> S·，表示整个输入已经被成功分析
     */
    bool isAcceptItem() const;

    /**
     * @brief 将LR项目转换为字符串表示
     * @return 项目的字符串形式，例如 "E -> E + ·T"
     *
     * 如果有前瞻符号，会在后面添加，例如 "E -> E + ·T, {$, +}"
     */
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

    /**
     * @brief 默认构造函数
     * 创建一个空的项目集，id初始化为-1表示未分配编号
     */
    ItemSet() : id(-1) {}

    /**
     * @brief 从项目集合构造ItemSet
     * @param itemSet 初始的项目集合
     */
    ItemSet(const std::set<LRItem>& itemSet) : items(itemSet), id(-1) {}

    /**
     * @brief 判断两个项目集是否相等
     * @param other 另一个项目集
     * @return 如果包含的项目完全相同则返回true
     *
     * 注意：比较时不考虑id，只比较项目内容
     */
    bool operator==(const ItemSet& other) const {
        return items == other.items;
    }

    /**
     * @brief 定义项目集的排序规则
     * @param other 另一个项目集
     * @return 按项目集合的字典序比较结果
     */
    bool operator<(const ItemSet& other) const {
        return items < other.items;
    }

    /**
     * @brief 打印项目集的内容
     *
     * 输出格式：
     * I0:
     *   E' -> ·E
     *   E -> ·E + T
     *   E -> ·T
     *   ...
     */
    void print() const;
};

#endif // LR_ITEM_H
