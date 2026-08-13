#pragma once
#ifndef __LINE_H__
#define __LINE_H__

#include "../CommonGlobal.h"
#include "Point.h"

class COMMON_EXPORT CLine
{
public:
	CLine(float a = FLT_MAX,float b = FLT_MAX,float c = FLT_MAX){m_a = a; m_b = b; m_c = c;}
	CLine(CPointD &p1, CPointD &p2);
	CLine(CPointD& p1, float slop);

	~CLine(void);
public:	
	CPointD operator&(const CLine &other);

	void	Relocate(CPointD &p1, CPointD &p2);
	float	GetXbyY(float y);
	float	GetYbyX(float x);
	float	GetSlop();
	static float GetSlop(const CPointD &p1,const CPointD &p2);
	void	CreateLine( float slop, CPointD &ptPoint );
	float	Distance2(CPointD &p);
	bool	IsSameSide(CPointD &p1, CPointD &p2);
	CPointD GetFootPoint(CPointD &p);
	void  FitLine(float Xs[], float Ys[], int n);

public:
	static	bool IsCoincident(const CPointD &p1, const CPointD &p2,
		const CPointD &q1, const CPointD &q2)
	{return ((p1 == q1 && p2 == q2) || (p1 == q2 && p2 == q1));	};
	static	bool IsCross(const CPointD &p1, const CPointD &p2,
		const CPointD &q1,const CPointD &q2)
	{return !IsCoincident(p1, p2, q1, q2) && IsRectCross(p1, p2, q1, q2) && IsLineSegmentCross(p1, p2, q1, q2);};
	static	bool IsRectCross(const CPointD &p1, const CPointD &p2,
		const CPointD &q1, const CPointD &q2);
	static	bool IsLineSegmentCross(const CPointD &p1, const CPointD &p2,
		const CPointD &q1, const CPointD &q2);
	static	bool GetSegmentCross(const CPointD &p1, const CPointD &p2,
		const CPointD &q1, const CPointD &q2,
		float &x, float &y);

	float m_a;
	float m_b;
	float m_c;
};






inline  bool  LinesCrossPoint(CLine cLine1,CLine cLine2,CPointD &CrossPoint)
{
	if ( cLine2.m_a / cLine1.m_a == cLine2.m_b / cLine1.m_b )
	{
		return false;
	}
	float a1 = cLine1.m_a;
	float b1 = cLine1.m_b;
	float c1 = cLine1.m_c;
	float a2 = cLine2.m_a;
	float b2 = cLine2.m_b;
	float c2 = cLine2.m_c;
	CrossPoint.x = ( b1 * c2 - b2 * c1 )/( a1 * b2 - a2 * b1 );
	CrossPoint.y = ( a2 * c1 - a1 * c2 )/( a1 * b2 - a2 * b1 );
	return true;
}
inline float	CalcPlaneDistance(float x, float y)
{
	return sqrtf(x * x + y * y);
}
//计算角平分线
inline CLine AngleBisector(CPointD A, CPointD B, CPointD C)
{
	double ba = CalcPlaneDistance(B.x - A.x, B.y - A.y);
	double bc = CalcPlaneDistance(C.x - B.x, C.y - B.y);
	// 利用三角形相似，求点D
	double k = ba / bc;
	CPointD D;
	D.x = float((C.x - B.x) * k + B.x);
	D.y = float((C.y - B.y) * k + B.y);
	// 线段AD的中点E
	CPointD E;
	E.x = (A.x + D.x)/2;
	E.y = (A.y + D.y)/2;
	return CLine(B, E);
}

/*
	判断三个点是否近似在一条直线上
	P1: ( X1,Y1);
	P2: ( X2,Y2);
	P3: ( X3,Y3);
	叉积 = (X1-X2)*(Y3-Y2)-(X3-X2)*(Y1-Y2) = 0    这是平行的判定
	但是理论上，灯点不一定完全平行，因而会有小偏差，这里我们采用阈值
*/
#define  POINTS_PARALLEL_THRESHOLD   5.0f
inline bool  IsPointsParallel( CPointD &pt1, CPointD &pt2,CPointD &pt3 )
{

	float crossProduct = ( pt1.x - pt2.x) * ( pt3.y - pt2.y ) - ( pt3.x - pt2.x ) * ( pt1.y - pt2.y );
	
	if ( crossProduct > POINTS_PARALLEL_THRESHOLD || crossProduct < -1 * POINTS_PARALLEL_THRESHOLD )
	{
		return false;
	}
	return true;
}

class CSegment: public CLine
{
public:
	CSegment(CPointD &p1,CPointD &p2):
	CLine(p1, p2)
	{
		m_p1.x = p1.x;
		m_p1.y = p1.y;
		m_p2.x = p2.x;
		m_p2.y = p2.y;
	}
	~CSegment()
	{

	}

	CPointD m_p1, m_p2;
};

#endif