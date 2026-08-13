#include "Polygon.h"
#include "Line.h"
#include "CommonFunctionMath.h"

CPolygon::CPolygon(const std::vector<CPointD> &pArrPoints)
{
	if(pArrPoints.size() < 3)
	{
		return;
	}
	m_arrPoints = pArrPoints;
}

bool CPolygon::GetPointCloser(CPointD* pPoint)
{
	if(IsConvex())
		return GetPointCloser2Convex(pPoint);
	return GetPointCloser2Concave(pPoint);

}

bool CPolygon::GetPointCloser2Concave(CPointD* pPoint)
{
	//思路：
	//1、判断点在多边形内，则直接返回true
	//2、目标点可能在边上或在多边形的角上，遍历所有可能，找到离输入点最近的点作为目标点
	//3、将输入点向目标点移动

	if(G_Math_IsPtInPts(m_arrPoints, *pPoint))
		return true;

	size_t nSize = m_arrPoints.size();

	CPointD ptNearest = m_arrPoints[0]; 
	float distanceNearest = CalcPlaneDistance(m_arrPoints[0].x - pPoint->x, m_arrPoints[0].y - pPoint->y);

	for(int i = 1; i < nSize; i++)
	{
		float distanceTmp = CalcPlaneDistance(m_arrPoints[i].x - pPoint->x, m_arrPoints[i].y - pPoint->y);
		if(distanceTmp < distanceNearest)
		{
			distanceNearest = distanceTmp;
			ptNearest		= m_arrPoints[i];
		}
	}

	for(int i = 0; i < nSize; i++)
	{
		int j = i + 1;
		if(j == nSize)
			j = 0;

		CLine segment(m_arrPoints[i], m_arrPoints[j]);
		CPointD footPoint = segment.GetFootPoint(*pPoint);

		float distanceTmp = CalcPlaneDistance(footPoint.x - pPoint->x, footPoint.y - pPoint->y);
		if(distanceTmp >= distanceNearest)
			continue;

		if(footPoint.x < m_arrPoints[i].x && footPoint.x < m_arrPoints[j].x)
			continue;

		if(footPoint.x > m_arrPoints[i].x && footPoint.x > m_arrPoints[j].x)
			continue;

		distanceNearest = distanceTmp;
		ptNearest		= footPoint;
	}

	CPointD ptCenter;

	for(int i = 0; i < nSize; i++)
	{
		ptCenter.x += m_arrPoints[i].x;
		ptCenter.y += m_arrPoints[i].y;
	}
	ptCenter.x /= nSize;
	ptCenter.y /= nSize;

	*pPoint = MakePointCloser(ptNearest, ptCenter);

	return true;
}

bool CPolygon::GetPointCloser2Convex(CPointD* pPoint)
{
	//思路：
	//1、判断点在多边形内，则直接返回true
	//2、根据重心连线判断所属边线
	//3、计算外部点到边线所连成直线的垂足
	//4、如果垂足在多边形上，外部点变为垂足，返回true。
	//5、否则，外部点变为最近的多边形顶点，返回true。

	if(G_Math_IsPtInPts(m_arrPoints, *pPoint))
		return true;

	size_t nSize = m_arrPoints.size();

	CPointD ptCenter;

	for(int i = 0; i < nSize; i++)
	{
		ptCenter.x += m_arrPoints[i].x;
		ptCenter.y += m_arrPoints[i].y;
	}
	ptCenter.x /= nSize;
	ptCenter.y /= nSize;

	for(int i = 0; i < nSize; i++)
	{
		int j = i + 1;
		if(j == nSize)
			j = 0;
		if(CLine::IsCross(ptCenter, *pPoint, m_arrPoints[i], m_arrPoints[j]))
		{
			CLine segment(m_arrPoints[i], m_arrPoints[j]);
			CPointD footPoint = segment.GetFootPoint(*pPoint);
			CPointD ptTmp = MakePointCloser(footPoint, ptCenter);

			if(G_Math_IsPtInPts(m_arrPoints, ptTmp))
			{
				*pPoint = ptTmp;
			}
			else
			{
				float distanceI = CalcPlaneDistance(m_arrPoints[i].x - pPoint->x, m_arrPoints[i].y - pPoint->y);
				float distanceJ = CalcPlaneDistance(m_arrPoints[j].x - pPoint->x, m_arrPoints[j].y - pPoint->y);

				if(distanceI < distanceJ)
				{
					*pPoint = MakePointCloser(m_arrPoints[i], ptCenter);
				}
				else
				{
					*pPoint = MakePointCloser(m_arrPoints[j], ptCenter);
				}
			}
			return true;
		}
	}

	return false;
}

bool CPolygon::IsConvex()
{
	//叉积法判断多边形凹凸性

	size_t nSize = m_arrPoints.size();
	bool bFirst = true;
	float Cross0 = 0;

	for(int i0 = 0; i0 < nSize - 1; i0++)
	{
		int i1 = ((i0 + 1) == nSize? 0: i0 + 1);
		int i2 = ((i1 + 1) == nSize? 0: i1 + 1);

		float x0 = m_arrPoints[i1].x - m_arrPoints[i0].x;
		float x1 = m_arrPoints[i2].x - m_arrPoints[i1].x;
		float y0 = m_arrPoints[i1].y - m_arrPoints[i0].y;
		float y1 = m_arrPoints[i2].y - m_arrPoints[i1].y;

		if(bFirst)
		{
			Cross0 = x0 * y1 - x1 * y0;
			bFirst = false;
			continue;
		}

		if(Cross0 * (x0 * y1 - x1 * y0) < 0)
		{
			return false;
		}
	}

	return true;
}

CPointD CPolygon::MakePointCloser(CPointD& pOri, const CPointD& pDst)
{
#define CLOSER_THRESHOLD (0.00001f)

	float k = CLine::GetSlop(pOri, pDst);

	float deltaX = CLOSER_THRESHOLD / CalcPlaneDistance(1, k);
	float deltaY = deltaX * k;

	if(pOri.x < pDst.x)
		return CPointD(pOri.x + deltaX, pOri.y + deltaY);
	else
		return CPointD(pOri.x - deltaX, pOri.y - deltaY);
}

void CPolygon::GrahamScan()
{
	std::vector<CPointD> convex;

	size_t n = m_arrPoints.size();

	int i,j,k=0;  
	CPointD tmpSwap;  

	//找到最下且偏左的那个点  
	for(i=1;i<n;i++)  
		if ((m_arrPoints[i].y<m_arrPoints[k].y)||((m_arrPoints[i].y==m_arrPoints[k].y)&&(m_arrPoints[i].x<m_arrPoints[k].x)))  
			k=i;  
	//将这个点指定为PointSet[0]  
	tmpSwap=m_arrPoints[0];  
	m_arrPoints[0]=m_arrPoints[k];  
	m_arrPoints[k]=tmpSwap;  

	//按极角从小到大,距离偏短进行排序  
	for (i=1;i<n-1;i++)  
	{
		k=i;
		for (j=i+1;j<n;j++)  
			if( (CPointD::Cross(m_arrPoints[j] - m_arrPoints[0], m_arrPoints[k] - m_arrPoints[0])>0)  
				||((CPointD::Cross(m_arrPoints[j] - m_arrPoints[0], m_arrPoints[k] - m_arrPoints[0])==0)  
				&&(CPointD::Distance(m_arrPoints[0],m_arrPoints[j])<CPointD::Distance(m_arrPoints[0],m_arrPoints[k]))) )  
				k=j;//k 保存极角最小的那个点,或者相同距离原点最近  
		tmpSwap=m_arrPoints[i];  
		m_arrPoints[i]=m_arrPoints[k];  
		m_arrPoints[k]=tmpSwap;  
	}  
	//第三个点先入栈  
	convex.push_back(m_arrPoints[0]);
	convex.push_back(m_arrPoints[1]);
	convex.push_back(m_arrPoints[2]);
	int top = 2;
	//判断与其余所有点的关系  
	for (i=3; i<n; i++)  
	{  
		//不满足向左转的关系,栈顶元素出栈  
		while(CPointD::Cross(m_arrPoints[i] - convex[top - 1], convex[top] - convex[top-1])>=0)
		{
			convex.erase(convex.begin() + top);
			top--;

			if (top == 1)
				break;
		}
		//当前点与栈内所有点满足向左关系,因此入栈. 
		convex.push_back(m_arrPoints[i]);
		top++;
	}  

	m_arrPoints = convex;
}
