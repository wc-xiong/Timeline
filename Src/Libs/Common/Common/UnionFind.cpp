#include "UnionFind.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end


CUnionFind::CUnionFind(int n)
{
    parent.resize(n);
    rank.resize(n, 0);
    count = n;
    for (int i = 0; i < n; i++)
    {
        parent[i] = i; // 每个元素初始化为独立集合
    }
}

int CUnionFind::Find(int x)
{
    if (parent[x] != x)
    {
        parent[x] = Find(parent[x]); // 递归压缩路径
    }
    return parent[x];
}

void CUnionFind::Union(int x, int y)
{
    int rootX = Find(x);
    int rootY = Find(y);

    if (rootX == rootY)
        return; // 已在同一集合中

    // 按秩合并：将秩小的树合并到秩大的树上
    if (rank[rootX] < rank[rootY])
    {
        parent[rootX] = rootY;
    }
    else if (rank[rootX] > rank[rootY])
    {
        parent[rootY] = rootX;
    }
    else
    {
        parent[rootY] = rootX;
        rank[rootX]++; // 秩相同时合并后深度+1
    }
    count--; // 合并后连通分量减少
}

bool CUnionFind::IsConnected(int x, int y)
{
    return Find(x) == Find(y);
}

int CUnionFind::GetCount()
{
    return count;
}