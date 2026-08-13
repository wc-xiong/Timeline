#include "Line.h"
#include <float.h>
#include <algorithm>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end


CLine::CLine(CPointD& p1, CPointD &p2)
{
	Relocate(p1, p2);
}

CLine::CLine(CPointD& p1, float slop)
{
	m_a = -slop;
	m_b = 1;
	m_c = -(m_a * p1.x + m_b * p1.y);
}

CLine::~CLine( void )
{

}

CPointD CLine::operator&( const CLine &other )
{
	float a1 = m_a;
	float b1 = m_b;
	float c1 = m_c;
	float a2 = other.m_a;
	float b2 = other.m_b;
	float c2 = other.m_c;
	float denominator = a1 * b2 - a2 * b1;
	if(abs(denominator) < FLT_EPSILON)
	{
		return CPointD(FLT_MAX, FLT_MAX);
	}
	float x = (c2 * b1 - c1 * b2)/denominator;
	float y = (c1 * a2 - c2 * a1)/denominator;
	return CPointD(x, y);
}

float CLine::GetXbyY( float y )
{
	return -1*(m_b * y + m_c)/m_a;
}

float CLine::GetYbyX( float x )
{
	return -1*(m_a * x + m_c)/m_b;
}

bool CLine::IsRectCross( const CPointD &p1, const CPointD &p2, const CPointD &q1, const CPointD &q2 )
{
	return	std::min(p1.x,p2.x) <= std::max(q1.x,q2.x)
	     && std::min(q1.x,q2.x) <= std::max(p1.x,p2.x)
		 && std::min(p1.y,p2.y) <= std::max(q1.y,q2.y)
		 && std::min(q1.y,q2.y) <= std::max(p1.y,p2.y);
}

bool CLine::IsLineSegmentCross( const CPointD &p1, const CPointD &p2, const CPointD &q1, const CPointD &q2 )
{
	double line1,line2;
	line1 = p1.x * (q1.y - p2.y) +
		p2.x * (p1.y - q1.y) +
		q1.x * (p2.y - p1.y);
	line2 = p1.x * (q2.y - p2.y) +
		p2.x * (p1.y - q2.y) + 
		q2.x * (p2.y - p1.y);
	if ((line1 * line2 >= 0) && !(abs(line1) <= FLT_EPSILON && abs(line2) <= FLT_EPSILON))
		return false;

	line1 = q1.x * (p1.y - q2.y) +
		q2.x * (q1.y - p1.y) +
		p1.x * (q2.y - q1.y);
	line2 = q1.x * (p2.y - q2.y) + 
		q2.x * (q1.y - p2.y) +
		p2.x * (q2.y - q1.y);
	if ((line1 * line2 >= 0) && !(abs(line1) <= FLT_EPSILON && abs(line2) <= FLT_EPSILON))
		return false;
	return true;
}

bool CLine::GetSegmentCross( const CPointD &p1, const CPointD &p2,
							const CPointD &q1, const CPointD &q2,
							float &x, float &y )
{
	if(IsCross(p1, p2, q1, q2))
	{
		float tmpLeft,tmpRight;
		tmpLeft = (q2.x - q1.x) * (p1.y - p2.y) - (p2.x - p1.x) * (q1.y - q2.y);
		tmpRight = (p1.y - q1.y) * (p2.x - p1.x) * (q2.x - q1.x) + q1.x * (q2.y - q1.y) * (p2.x - p1.x) - p1.x * (p2.y - p1.y) * (q2.x - q1.x);

		if(tmpLeft < FLT_EPSILON && tmpLeft > -FLT_EPSILON)
		{
			return false;
		}
		x = tmpRight/tmpLeft;

		tmpLeft = (p1.x - p2.x) * (q2.y - q1.y) - (p2.y - p1.y) * (q1.x - q2.x);
		tmpRight = p2.y * (p1.x - p2.x) * (q2.y - q1.y) + (q2.x- p2.x) * (q2.y - q1.y) * (p1.y - p2.y) - q2.y * (q1.x - q2.x) * (p2.y - p1.y); 

		if(tmpLeft < FLT_EPSILON && tmpLeft > -FLT_EPSILON)
		{
			return false;
		}
		y = tmpRight/tmpLeft;

		return true;
	}
	return false;
}

float CLine::GetSlop()
{
	if ( m_b == 0 )
	{
		return FLT_MAX;     //垂直
	}
	return ( -1.0f * m_a / m_b );
}

float CLine::GetSlop(const CPointD &p1,const CPointD &p2)
{
	if ( p1.x == p2.x )
	{
		return FLT_MAX;
	}
	return ( (p2.y - p1.y)/(p2.x - p1.x) );
}

void CLine::Relocate( CPointD &p1, CPointD &p2 )
{
	float x1 = p1.x;
	float y1 = p1.y;
	float x2 = p2.x;
	float y2 = p2.y;
	m_a = y2 - y1;
	m_b = x1 - x2;
	m_c = x2 * y1 - x1 * y2;
}


void CLine::CreateLine( float slop, CPointD &ptPoint )
{
	if ( slop == FLT_MAX )
	{
		m_a  = 1;
		m_b  = 0;
		m_c  = - ptPoint.x;
	}else{
		m_a = - slop;
		m_b = 1;
		m_c = -( m_a * ptPoint.x + ptPoint.y );
	}
}

float CLine::Distance2( CPointD &p )
{
	return abs(m_a * p.x + m_b * p.y + m_c) / sqrtf(m_a * m_a + m_b * m_b);
}

bool CLine::IsSameSide( CPointD &p1, CPointD &p2 )
{
	return (m_a * p1.x + m_b * p1.y + m_c) * (m_a * p2.x + m_b * p2.y + m_c) > 0;
}

CPointD CLine::GetFootPoint(CPointD &p)
{
	return CPointD((m_b * m_b * p.x - m_a * m_b * p.y - m_a * m_c) / (m_a * m_a + m_b * m_b)
				 ,(-m_a * m_b * p.x + m_a * m_a * p.y - m_b * m_c) / (m_a * m_a + m_b * m_b));

}


void CLine::FitLine(float Xs[], float Ys[], int n)
{
	double k = 0;
	double b = 0;

	double sumX = 0;
	double sumY = 0;
	double sumXY = 0;
	double sumXX = 0;

	for(int i = 0; i < n; i++)
	{
		sumX += Xs[i];
		sumY += Ys[i];
		sumXY += Xs[i] * Ys[i];
		sumXX += Xs[i] * Xs[i];
	}

	double avgX = sumX / n;
	double avgY = sumY / n;

	k = (sumXY - n * avgX * avgY) / (sumXX - n * avgX * avgX);
	b = avgY - k * avgX;

	CPointD pt;
	pt.x = 0;
	pt.y = (float)b;
	CreateLine((float)k , pt);
}
