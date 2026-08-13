#pragma once
#ifndef __EDGE_H__
#define __EDGE_H__

#include "Point.h"

template <typename T>
class CEdgeEx
{
public:
	CPointEx<T> pMin;
	CPointEx<T> pMax;

	CEdgeEx() = delete;
	CEdgeEx(const CPointEx<T>& p1, const CPointEx<T>& p2)
	{
		pMin = p1 < p2 ? p1 : p2;
		pMax = p1 < p2 ? p2 : p1;
	}

	bool operator==(const CEdgeEx& other) const
	{
		return (pMin == other.pMin) && (pMax == other.pMax);
	}
	bool operator<(const CEdgeEx& other) const
	{
		return (pMin < other.pMin) || (pMin == other.pMin && pMax < other.pMax);
	}
};

typedef  CEdgeEx<double>	CEdgeD;

#endif