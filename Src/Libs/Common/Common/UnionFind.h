#pragma once
#include <vector>
#include "CommonGlobal.h"

class COMMON_EXPORT CUnionFind
{
public:
    CUnionFind(int n);

    // 查找元素 x 的根节点（包含路径压缩）
    int Find(int x);

    // 合并元素 x 和 y 所在的集合
    void Union(int x, int y);

    // 判断元素 x 和 y 是否属于同一集合
    bool IsConnected(int x, int y);

    // 返回当前连通分量数量
    int GetCount();

private:
    std::vector<int> parent; // 存储每个节点的父节点
    std::vector<int> rank;   // 存储节点的秩（树高度）
    int count;          // 连通分量的数量

};