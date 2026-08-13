#include "Graph.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end


CGraph::CGraph(int v, bool directedv) : vertices(v), adjList(v), directed(directedv)
{}

void CGraph::addEdge(int src, int dest)
{
    adjList[src].push_back(dest);
    if (!directed)
        adjList[dest].push_back(src);
}

std::vector<std::vector<int>> CGraph::findAllSimpleCycles()
{
	std::vector<bool> visited(vertices, false);
	std::vector<int> path;
	std::set<std::set<int>> uniqueCycles;
	std::vector<std::vector<int>> allCycles;

	for (int i = 0; i < vertices; i++)
	{
		if(!visited[i])
			DFSForCycles(i, -1, i, visited, path, uniqueCycles, allCycles);
	}

	return allCycles;
}

std::vector<std::vector<int>> CGraph::findAllSimpleCyclesFromVertex(int start)
{
    std::vector<bool> visited(vertices, false);
    std::vector<int> path;
    std::set<std::set<int>> uniqueCycles;
    std::vector<std::vector<int>> allCycles;

    DFSForCycles(start, -1, start, visited, path, uniqueCycles, allCycles);
    return allCycles;
}

void CGraph::DFSForCycles(int current, int parent, int start, std::vector<bool>& visited,
    std::vector<int>& path, std::set<std::set<int>>& uniqueCycles,
    std::vector<std::vector<int>>& allCycles) 
{
	if (current < 0 || current >= visited.size())
		return;

    visited[current] = true;
    path.push_back(current);

    for (int neighbor : adjList[current]) 
    {
        if (neighbor == parent) continue;

        if (!visited[neighbor]) 
        {
            DFSForCycles(neighbor, current, start, visited, path, uniqueCycles, allCycles);
        }
        else 
        {
            if (neighbor == start && path.size() > 2) 
            {
                std::set<int> cycleSet(path.begin(), path.end());
                if (uniqueCycles.find(cycleSet) == uniqueCycles.end()) 
                {
                    uniqueCycles.insert(cycleSet);
                    std::vector<int> fullPath = path;
                    fullPath.push_back(start);
                    allCycles.push_back(fullPath);
                }
            }
        }
    }

    visited[current] = false;
    path.pop_back();
}