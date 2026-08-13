#pragma once

#include "../CommonGlobal.h"
#include "Point.h"
#include <vector>

class COMMON_EXPORT CPolygon
{
public:
		CPolygon(const std::vector<CPointD> &pPoints);

		bool GetPointCloser(CPointD* pPoint);							//计算多边形中离外部点最近的点

		bool GetPointCloser2Concave(CPointD* pPoint);					//计算多边形中离外部点最近的点(支持凹多边形)			

		bool GetPointCloser2Convex(CPointD* pPoint);					//计算多边形中离外部点最近的点

		bool IsConvex();												//判断多边形凹凸性

const	std::vector<CPointD>& GetData()				{return m_arrPoints;}

		void GrahamScan();												//寻找凸包
protected:
		CPointD MakePointCloser(CPointD& pOri, const CPointD& pDst);		//GetPointCloser辅助函数

		std::vector<CPointD> m_arrPoints;
};


