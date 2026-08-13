#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <d2d1.h>
#include "Point.h"
#include <vector>
#include <QRect>

/* --------------------------------------------------------------------------*/
/**
 * double类型的3x2矩阵。
 * 由于仿射变换最后一列始终为(0,0,1)，因此3x2的矩阵就足够了
 * 平移，旋转，放缩
 */
 /* ----------------------------------------------------------------------------*/
class CMatrix3x2D
{
public:
	CMatrix3x2D(
		double m11,
		double m12,
		double m21,
		double m22,
		double m31,
		double m32)
	{
		this->m11 = m11;
		this->m12 = m12;
		this->m21 = m21;
		this->m22 = m22;
		this->m31 = m31;
		this->m32 = m32;
	}
	CMatrix3x2D() 
	{
		this->m11 = 0;
		this->m12 = 0;
		this->m21 = 0;
		this->m22 = 0;
		this->m31 = 0;
		this->m32 = 0;
	};

	//单位矩阵
	static	CMatrix3x2D	Identity()
	{
		CMatrix3x2D identity;

		identity.m11 = 1.0;
		identity.m12 = 0.0;
		identity.m21 = 0.0;
		identity.m22 = 1.0;
		identity.m31 = 0.0;
		identity.m32 = 0.0;

		return identity;
	}

	//平移矩阵
	// x，横向平移；y，纵向平移
	static	CMatrix3x2D	Translation(double x,double y)
	{
		CMatrix3x2D translation;

		translation.m11 = 1.0; translation.m12 = 0.0;
		translation.m21 = 0.0; translation.m22 = 1.0;
		translation.m31 = x; translation.m32 = y;

		return translation;
	}

	//平移矩阵
	// size.x，横向平移；size.y，纵向平移
	static	CMatrix3x2D	Translation(CPointD size)
	{
		return Translation(size.x, size.y);
	}

	//放缩矩阵
	// x，横向放缩；y，纵向放缩；center，放缩中心
	static	CMatrix3x2D	Scale(double x, double y,CPointD center = CPointD())
	{
		CMatrix3x2D scale;

		scale.m11 = x; scale.m12 = 0.0;
		scale.m21 = 0.0; scale.m22 = y;
		scale.m31 = center.x - x * center.x;
		scale.m32 = center.y - y * center.y;

		return scale;
	}

	//放缩矩阵
	// size.x，横向平移；size.y，纵向平移；center，放缩中心
	static	CMatrix3x2D	Scale(CPointD size,	CPointD center = CPointD())
	{
		return Scale(size.x, size.y, center);
	}

	//旋转矩阵
	// angle，旋转角度，顺时针顺序；center，旋转中心
	static	CMatrix3x2D	Rotation(double angle,	CPointD center = CPointD())
	{
		CMatrix3x2D rotation;

		angle = angle / 180 * M_PI;

		rotation.m11 = cos(angle);
		rotation.m12 = sin(angle);
		rotation.m21 = -sin(angle);
		rotation.m22 = cos(angle);
		rotation.m31 = 0;
		rotation.m32 = 0;

		return Translation(-center.x, -center.y)*rotation*Translation(center.x, center.y);
	}

	//行列式
	double Determinant() const
	{
		return (m11 * m22) - (m12 * m21);
	}

	//逆矩阵
	CMatrix3x2D Inverse() const 
	{
		double k = 1 / (m11*m22 - m21 * m12);

		return CMatrix3x2D(
			m22*k, -m12 * k,
			-m21 * k, m11*k,
			m21*m32*k - m22 * m31*k, m12*m32*k - m11 * m32*k
		);
	}

	//是否为单位矩阵
	bool IsIdentity() const
	{
		return abs(m11 - 1.0) < DBL_MIN && abs(m12 - 0.0) < DBL_MIN
			&& abs(m21 - 0.0) < DBL_MIN && abs(m22 - 1.0) < DBL_MIN
			&& abs(m31 - 0.0) < DBL_MIN && abs(m32 - 0.0) < DBL_MIN;
	}

	//矩阵相乘
	void SetProduct(const CMatrix3x2D &a, const CMatrix3x2D &b)
	{
		m11 = a.m11 * b.m11 + a.m12 * b.m21;
		m12 = a.m11 * b.m12 + a.m12 * b.m22;
		m21 = a.m21 * b.m11 + a.m22 * b.m21;
		m22 = a.m21 * b.m12 + a.m22 * b.m22;
		m31 = a.m31 * b.m11 + a.m32 * b.m21 + b.m31;
		m32 = a.m31 * b.m12 + a.m32 * b.m22 + b.m32;
	}

	//矩阵乘法
	CMatrix3x2D operator*(const CMatrix3x2D &matrix) const
	{
		CMatrix3x2D result;
		result.SetProduct(*this, matrix);
		return result;
	}

	//变换矩阵作用于点上
	CPointD TransformPoint(CPointD point) const
	{
		return CPointD(
			point.x * m11 + point.y * m21 + m31,
			point.x * m12 + point.y * m22 + m32
		);
	}

	//变换矩阵作用于点上
	CPointF TransformPoint(CPointF point) const
	{
		return CPointF(
			float(point.x * m11 + point.y * m21 + m31),
			float(point.x * m12 + point.y * m22 + m32)
		);
	}

	//转换成d2d矩阵
	D2D1::Matrix3x2F ToD2D1Matrix3x2F()
	{
		return D2D1::Matrix3x2F(
			(float)m11, (float)m12,
			(float)m21, (float)m22,
			(float)m31, (float)m32
		);
	}
private:
	double m11;
	double m12;
	double m21;
	double m22;
	double m31;
	double m32;
};


struct SRectD
{
	double left;
	double right;
	double top;
	double bottom;
	SRectD()
	{
		left = right = top = bottom = 0;
	}
	SRectD(double l, double t, double r, double b)
	{
		left = l;
		right = r;
		top = t;
		bottom = b;
	}
	SRectD(const D2D1_RECT_F& rc)
	{
		left = rc.left;
		right = rc.right;
		top = rc.top;
		bottom = rc.bottom;
	}

	SRectD(const QRect& rc)
	{
		left = rc.left();
		right = rc.left() + rc.width();
		top = rc.top();
		bottom = rc.top() + rc.height();
	}

	SRectD(const QRectF& rc)
	{
		left = rc.left();
		right = rc.left() + rc.width();
		top = rc.top();
		bottom = rc.top() + rc.height();
	}

	double Width()const
	{
		return right - left;
	}
	double Height()const
	{
		return bottom - top;
	}
	D2D1_RECT_F ToD2D1_RECT_F()
	{
		D2D1_RECT_F rc;
		rc.left = float(left);
		rc.right = float(right);
		rc.top = float(top);
		rc.bottom = float(bottom);
		return rc;
	}
	CPointD Center()
	{
		return CPointD((left + right) / 2, (top + bottom) / 2);
	}
	bool operator!=(const SRectD& other)
	{
		return left != other.left ||
			right != other.right ||
			top != other.top ||
			bottom != other.bottom;
	}

	bool operator==(const SRectD& other)
	{
		return left == other.left &&
			right == other.right &&
			top == other.top &&
			bottom == other.bottom;
	}

	void InitRect()
	{
		left = DBL_MAX;
		top = DBL_MAX;
		right = -DBL_MAX;
		bottom = -DBL_MAX;
	}

	void MakeRect(const CPointD& pt)
	{
		left = pt.x > left ? left : pt.x;
		right = pt.x < right ? right : pt.x;
		top = pt.y > top ? top : pt.y;
		bottom = pt.y < bottom ? bottom : pt.y;
	}

	void MakeRect(const std::vector<CPointD>& vecPts)
	{
		for (const auto& point : vecPts)
		{
			MakeRect(point);
		}
	}

	void MakeRect(const SRectD& rc)
	{
		left = std::min(left, rc.left);
		top = std::min(top, rc.top);
		right = std::max(right, rc.right);
		bottom = std::max(bottom, rc.bottom);
	}

	void ApplyRect(CPointD& pt,const SRectD& rc)
	{
		pt.x = (pt.x - left) / Width() * rc.Width() + rc.left;
		pt.y = (pt.y - top) / Height() * rc.Height() + rc.top;
	}

	bool isStrongIntersect(const SRectD& rectB) 
	{
		// 先校验矩形自身坐标的合法性（避免传入无效矩形）
		if (left >= right || top >= bottom ||
			rectB.left >= rectB.right || rectB.top >= rectB.bottom) 
		{
			return false;
		}

		// 判断是否分离（满足任一条件则不相交）
		bool isSeparated = (right <= rectB.left)   // A在B左侧
			|| (left >= rectB.right)  // A在B右侧
			|| (bottom <= rectB.top)  // A在B上方
			|| (top >= rectB.bottom); // A在B下方

		// 非分离状态即为强相交
		return !isSeparated;
	}
};

typedef SRectD D2D1_RECT_D;