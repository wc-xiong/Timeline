#pragma once
#ifndef __POINT_H__
#define __POINT_H__

#include <float.h>
#include <cmath>

template<class T>  class CPointEx
{
public:
	CPointEx();
	CPointEx(T X,T Y);
	~CPointEx(){};

	bool	     operator==  (const CPointEx<T> &ptf) const;
	CPointEx<T> &operator=   (const CPointEx<T> &ptf);

	CPointEx<T>  operator+   (const CPointEx<T> &ptf);
	CPointEx<T>  operator-   (const CPointEx<T> &ptf);
	CPointEx<T>  operator*   (T f);
	CPointEx<T>  operator/   (T f);
	bool		 operator<   (const CPointEx<T> &ptf) const;

static	T	Cross(const CPointEx<T> &pt0, const CPointEx<T> &pt1);
static	T	Distance(const CPointEx<T> &pt0, const CPointEx<T> &pt1);

	void        SetPoint(T X,T Y);
	void		Normalize();

public:
	T x , y;
};
/*
       下面是实现；
*/
template<class T>				  CPointEx<T>::CPointEx() : x(T(0)),y( T(0))			{						}
template<class T>				  CPointEx<T>::CPointEx( T X,T Y ) : x(X), y(Y)			{						}
template<class T>      void       CPointEx<T>::SetPoint( T X,T Y )							{  x = Y;   y = Y; }
template<class T>      void       CPointEx<T>::Normalize() 
{ 
	if (0.0 == x && 0.0 == y) return;
	T t = 1 / sqrt(x * x + y * y);
	x = x * t;
	y = y * t;
}

template<class T>   CPointEx<T> & CPointEx<T>::operator=( const CPointEx<T> &ptf )			{ 	x = ptf.x; 	y = ptf.y; return *this; }
template<class T>   CPointEx<T>   CPointEx<T>::operator-( const CPointEx<T> &ptf )			{ 	return CPointEx<T>(x - ptf.x, 	y - ptf.y);}
template<class T>   CPointEx<T>   CPointEx<T>::operator+( const CPointEx<T> &ptf )			{ 	return CPointEx<T>(x + ptf.x, 	y + ptf.y);}
template<class T>   CPointEx<T>   CPointEx<T>::operator*(T f) { return CPointEx<T>(x * f, y * f); }
template<class T>   CPointEx<T>   CPointEx<T>::operator/(T f) { return CPointEx<T>(x / f, y / f); }

template<class T>  bool	  CPointEx<T>::operator<( const CPointEx<T> &ptf) const { return x < ptf.x || (x == ptf.x && y < ptf.y); }

template<class T>  bool	  CPointEx<T>::operator==( const CPointEx<T> &ptf )	const		{ 	return ( x == ptf.x && y == ptf.y ); }

template<class T>  T	  CPointEx<T>::Cross(const CPointEx<T> &pt0, const CPointEx<T> &pt1)		{ 	return G_Math_GetCross(pt0.x, pt0.y, pt1.x, pt1.y); }
template<class T>  T	  CPointEx<T>::Distance(const CPointEx<T> &p1, const CPointEx<T> &p2)		{ 	return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y)); }

template<> inline bool	CPointEx<double>::operator==( const CPointEx<double> &ptf )	const		{ 	return (abs(x - ptf.x) < DBL_EPSILON && abs(y - ptf.y) < DBL_EPSILON); }
template<> inline bool	CPointEx<float>::operator==( const CPointEx<float> &ptf )	const		{ 	return (abs(x - ptf.x) < FLT_EPSILON && abs(y - ptf.y) < FLT_EPSILON); }

typedef  CPointEx<double>	CPointD;
typedef  CPointEx<float>	CPointF;

inline CPointD G_CPointF2D(const CPointF& pt) { return CPointD((double)pt.x, (double)pt.y); }
inline CPointF G_CPointD2F(const CPointD& pt) { return CPointF((float)pt.x, (float)pt.y); }
inline bool G_CPointIsNull(const CPointD& pt) { return pt.x == DBL_MAX || pt.y == DBL_MAX; }

//		额外的

template<class T>  CPointEx<T>       cPointEx( T X , T Y)									{ CPointEx<T>  pt;  pt.x = X;  pt.y = Y; return pt; }


template<class T>  class CPoint3Ex
{
public:
	CPoint3Ex();
	CPoint3Ex(T X, T Y, T Z);
	~CPoint3Ex() {};
	CPoint3Ex<T> &operator= (const CPoint3Ex<T> &ptf);
	CPoint3Ex<T> operator^ (const CPoint3Ex<T> &ptf);
	T			 operator* (const CPoint3Ex<T> &ptf);
	CPoint3Ex<T> operator+ (const CPoint3Ex<T> &ptf);
	CPoint3Ex<T> operator- (const CPoint3Ex<T> &ptf);

public:
	T x, y, z;
};
/*
	   下面是实现；
*/
template<class T>				  CPoint3Ex<T>::CPoint3Ex() : x(T(0)), y(T(0)) {}
template<class T>				  CPoint3Ex<T>::CPoint3Ex(T X, T Y, T Z) : x(X), y(Y), z(Z) {}
template<class T>   CPoint3Ex<T> & CPoint3Ex<T>::operator=(const CPoint3Ex<T> &ptf) { x = ptf.x; y = ptf.y; z = ptf.z; return *this; }
template<class T>   CPoint3Ex<T>  CPoint3Ex<T>::operator+(const CPoint3Ex<T> &ptf) { return CPoint3Ex(x + ptf.x, y + ptf.y, z + ptf.z); }
template<class T>   CPoint3Ex<T>  CPoint3Ex<T>::operator-(const CPoint3Ex<T> &ptf) { return CPoint3Ex(x - ptf.x, y - ptf.y, z - ptf.z); }
template<class T>   T			  CPoint3Ex<T>::operator*(const CPoint3Ex<T> &ptf) { return x * ptf.x + y * ptf.y + z * ptf.z; }
template<class T>   CPoint3Ex<T>  CPoint3Ex<T>::operator^(const CPoint3Ex<T> &ptf) { return CPoint3Ex(y*ptf.z - ptf.y*z, ptf.x*z - ptf.z*x, x*ptf.y - y * ptf.x); }

typedef  CPoint3Ex<double>	CPoint3D;
typedef  CPoint3Ex<float>	CPoint3F;

#endif