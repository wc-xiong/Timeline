#pragma once
#include <vector>
#include "CommonGlobal.h"
#include <set>

class COMMON_EXPORT CGraph
{
public:
    CGraph(int v, bool directedv = false);

    void addEdge(int src, int dest);

	std::vector<std::vector<int>> findAllSimpleCycles();

    std::vector<std::vector<int>> findAllSimpleCyclesFromVertex(int start);

private:
    void DFSForCycles(int current, int parent, int start, std::vector<bool>& visited,
        std::vector<int>& path, std::set<std::set<int>>& uniqueCycles,
        std::vector<std::vector<int>>& allCycles);

private:
    int vertices;
    std::vector<std::vector<int>> adjList;

    bool directed;
};