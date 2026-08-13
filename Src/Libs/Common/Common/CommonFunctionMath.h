#pragma once
#define _USE_MATH_DEFINES		//解决qmath.h和math.h的头文件冲突
#include <math.h>
#include <algorithm>
#include <vector>
#include <set>
#include "Maths/Point.h"

inline int	Round(double mvalue)
{
	if (mvalue >= 0)
		return int(mvalue + 0.5);
	else
		return int(mvalue - 0.5);
}

template <class T> 
auto G_Math_CalcLength(T p1, T p2)
{
	auto res = (p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y);
	return std::sqrt(res);
}

template <class T> 
T G_Math_GetCross(T x0, T y0, T x1, T y1)			//计算向量叉积
{
	return x0 * y1 - x1 * y0;
}

template <class T>
bool G_Math_PointValueIsSame(T a, T b)
{
	double e = 0.1;//此处取值绝对值低于0.1是为了旋转时float精度不满足要求时，确保界面绘制正确
	return  (a - b < e && b - a < e);
}

// 在误差范围内，两点是否相等
template <class T> 
bool G_Math_IsPointSame(const T& p1, const T& p2, double fThresholdValue)
{
	double offsetX = p1.x - p2.x;
	double offsetY = p1.y - p2.y;
	if (offsetX * offsetX + offsetY * offsetY > fThresholdValue * fThresholdValue)
		return false;
	return true;
}

//判断两线段相交
inline bool G_Math_IsIntersect(double px1, double py1, double px2, double py2, double px3, double py3, double px4, double py4)
{
	bool flag = false;
	double d = (px2 - px1) * (py4 - py3) - (py2 - py1) * (px4 - px3);
	if (d != 0)
	{
		double r = ((py1 - py3) * (px4 - px3) - (px1 - px3) * (py4 - py3)) / d;
		double s = ((py1 - py3) * (px2 - px1) - (px1 - px3) * (py2 - py1)) / d;
		if ((r >= 0) && (r <= 1) && (s >= 0) && (s <= 1))
		{
			flag = true;
		}
	}
	return flag;
}

//判断点是否在多边形内
template<class T> 
bool G_Math_IsPtInPts(const std::vector<T>& polygon, const T& pt)
{
	int i, j;
	bool c = false;
	int nPolygon = (int)polygon.size();
	for (i = 0; i < nPolygon; i++)
	{
		if (pt == polygon[i])
			return true;
	}

	for (i = 0, j = nPolygon - 1; i < nPolygon; j = i++)
	{
		if ((((polygon[i].y < pt.y) && (pt.y <= polygon[j].y))
			|| ((polygon[j].y < pt.y) && (pt.y <= polygon[i].y)))
			&& (polygon[i].x <= pt.x || polygon[j].x <= pt.x))
		{
			if (polygon[i].x + (pt.y - polygon[i].y) / (polygon[j].y - polygon[i].y)*(polygon[j].x - polygon[i].x) < pt.x)
				c = !c;
		}
	}
	return c;
}

template <class T> 
void G_Math_GetIntersectionPoint(T center, T source, T cur, T& A, T& B, T& C)
{
	auto typeId = center.x;
	typeId = 0.0f;
	auto ax = typeId;
	auto ay = typeId;
	auto bx = typeId;
	auto by = typeId;
	auto cx = typeId;
	auto cy = typeId;
	// (x-center.x)^2 + (y-center.y)^2 = 10000 圆
	// y = ax + b 直线
	// 两条直线过中心相交，并且相互垂直，一条过起点交A，另一条的交点为B，C为当前点与中心点连线与圆的交点
	if (!G_Math_PointValueIsSame(source.x, center.x)) // 直线斜率必须有效不能无穷大
	{
		auto n1 = (source.y - center.y) * 1.0f / (source.x - center.x);
		auto m1 = center.y - n1 * center.x;
		auto a1 = 1 + n1 * n1;
		auto b1 = -2 * center.x + 2 * n1 * (m1 - center.y);
		auto c1 = center.x * center.x + (m1 - center.y) * (m1 - center.y) - 10000;
		{
			auto x1 = (-1 * b1 + std::sqrt(b1 *b1 - 4 * a1*c1)) / (2 * a1);
			auto y1 = n1 * x1 + m1;
			auto x2 = (-1 * b1 - std::sqrt(b1 *b1 - 4 * a1*c1)) / (2 * a1);
			auto y2 = n1 * x2 + m1;
			auto lenght1 = G_Math_CalcLength(source, T(x1, y1));
			auto lenght2 = G_Math_CalcLength(source, T(x2, y2));
			ax = lenght1 < lenght2 ? x1 : x2;
			ay = lenght1 < lenght2 ? y1 : y2;
		}
		// 计算得到 ax, ay ， 然后计算 bx, by
		auto n2 = -1 / n1;
		auto m2 = center.y - n2 * center.x;
		auto a2 = 1 + n2 * n2;
		auto b2 = -2 * center.x + 2 * n2 * (m2 - center.y);
		auto c2 = center.x * center.x + (m2 - center.y) * (m2 - center.y) - 10000;
		{
			auto x1 = (-1 * b2 + std::sqrt(b2*b2 - 4 * a2*c2)) / (2 * a2);
			auto y1 = n2 * x1 + m2;
			auto x2 = (-1 * b2 - std::sqrt(b2*b2 - 4 * a2*c2)) / (2 * a2);
			auto y2 = n2 * x2 + m2;
			if (ax > center.x && ay > center.y)
			{
				if (x1 > center.x && y1 < center.y)
				{
					bx = x1;
					by = y1;
				}
				else if (x2 > center.x && y2 < center.y)
				{
					bx = x2;
					by = y2;
				}
			}
			else if (ax > center.x && ay < center.y)
			{
				if (x1 < center.x && y1 < center.y)
				{
					bx = x1;
					by = y1;
				}
				else if (x2 < center.x && y2 < center.y)
				{
					bx = x2;
					by = y2;
				}
			}
			else if (ax < center.x && ay < center.y)
			{
				if (x1 < center.x && y1 > center.y)
				{
					bx = x1;
					by = y1;
				}
				else if (x2 < center.x && y2 > center.y)
				{
					bx = x2;
					by = y2;
				}
			}
			else if (ax < center.x && ay > center.y)
			{
				if (x1 > center.x && y1 > center.y)
				{
					bx = x1;
					by = y1;
				}
				else if (x2 > center.x && y2 > center.y)
				{
					bx = x2;
					by = y2;
				}
			}
		}
	}
	else
	{
		if (source.y > center.y)
		{
			ax = center.x;
			ay = center.y + 100;
			bx = center.x + 100;
			by = center.y;
		}
		else if (source.y < center.y)
		{
			ax = center.x;
			ay = center.y - 100;
			bx = center.x - 100;
			by = center.y;
		}
		else
		{
			ax = center.x;
			ay = center.y;
			bx = center.x;
			by = center.y;
		}
	}

	if (!G_Math_PointValueIsSame(cur.x, center.x))
	{
		auto n1 = (cur.y - center.y) * 1.0f / (cur.x - center.x);
		auto m1 = center.y - n1 * center.x;
		auto a1 = 1 + n1 * n1;
		auto b1 = -2 * center.x + 2 * n1 * (m1 - center.y);
		auto c1 = center.x * center.x + (m1 - center.y) * (m1 - center.y) - 10000;
		{
			auto x1 = (-1 * b1 + std::sqrt(b1 *b1 - 4 * a1*c1)) / (2 * a1);
			auto y1 = n1 * x1 + m1;
			auto x2 = (-1 * b1 - std::sqrt(b1 *b1 - 4 * a1*c1)) / (2 * a1);
			auto y2 = n1 * x2 + m1;
			auto lenght1 = G_Math_CalcLength(cur, T(x1, y1));
			auto lenght2 = G_Math_CalcLength(cur, T(x2, y2));
			cx = lenght1 < lenght2 ? x1 : x2;
			cy = lenght1 < lenght2 ? y1 : y2;
		}
	}
	else
	{
		if (cur.y > center.y)
		{
			cx = center.x;
			cy = center.y + 100;
		}
		else if (cur.y < center.y)
		{
			cx = center.x;
			cy = center.y - 100;
		}
		else
		{
			cx = center.x;
			cy = center.y;
		}
	}

	A.x = ax; A.y = ay;
	B.x = bx; B.y = by;
	C.x = cx; C.y = cy;
}

template <class T> 
double G_Math_CalcAngle(T center, T source, T curpos)
{
	CPointD centerCpy = CPointD(center.x, center.y);
	CPointD sourceCpy = CPointD(source.x, source.y);
	CPointD curposCpy = CPointD(curpos.x, curpos.y);

	if (G_Math_PointValueIsSame(centerCpy.x, sourceCpy.x) && G_Math_PointValueIsSame(centerCpy.y, sourceCpy.y))
		return 0;
	if (G_Math_PointValueIsSame(centerCpy.x, curposCpy.x) && G_Math_PointValueIsSame(centerCpy.y, curposCpy.y))
		return 0;
	if (G_Math_PointValueIsSame(curposCpy.x, sourceCpy.x) && G_Math_PointValueIsSame(curposCpy.y, sourceCpy.y))
		return 0;

	double squareCenterSource = (sourceCpy.x - centerCpy.x) * (sourceCpy.x - centerCpy.x) + (sourceCpy.y - centerCpy.y) * (sourceCpy.y - centerCpy.y);
	double squareCenterCurpos = (curposCpy.x - centerCpy.x) * (curposCpy.x - centerCpy.x) + (curposCpy.y - centerCpy.y) * (curposCpy.y - centerCpy.y);
	double squareSourceCurpos = (curposCpy.x - sourceCpy.x) * (curposCpy.x - sourceCpy.x) + (curposCpy.y - sourceCpy.y) * (curposCpy.y - sourceCpy.y);

	double cs = std::sqrt(squareCenterSource);
	double cc = std::sqrt(squareCenterCurpos);

	double cosSourceCenterCurpos = (squareCenterSource + squareCenterCurpos - squareSourceCurpos) / (2 * cs * cc);
	if (cosSourceCenterCurpos < -1)
		cosSourceCenterCurpos = -1;
	if (cosSourceCenterCurpos > 1)
		cosSourceCenterCurpos = 1;
	// 以center-source 为基准轴垂直于center-source 以center 为中心的坐标系，计算curpos所在象限
	// 1.计算得到以center为中心的圆与 以center和source确定的直线 两者的交点且与source更近的点的坐标A();
	double ax = 0.0, ay = 0.0, bx = 0.0, by = 0.0, cx = 0.0, cy = 0.0;
	// 计算得到以center为中心的圆与 以center和source确定的直线 两者的交点且与source更近的点的坐标
	// 顺时针方向的垂直点
	// 计算得到以center为中心的圆与 以center和curpos确定的直线 两者的交点且与source更近的点的坐标
	CPointD A, B, C;
	G_Math_GetIntersectionPoint(centerCpy, sourceCpy, curposCpy, A, B, C);
	ax = A.x; ay = A.y;
	bx = B.x; by = B.y;
	cx = C.x; cy = C.y;
	double length1 = G_Math_CalcLength(A, C);
	double length2 = G_Math_CalcLength(B, C);

	// 根据连线长度判断象限
	int position = 0;
	double tmp = std::sqrt(20000.0);
	if (length1 <= tmp && length2 <= tmp)
		position = 1;
	else if (length1 >= tmp && length2 <= tmp)
		position = 2;
	else if (length1 >= tmp && length2 >= tmp)
		position = 3;
	else if (length1 <= tmp && length2 >= tmp)
		position = 4;
	if (position == 1 || position == 2)
		return std::acos(cosSourceCenterCurpos) * 180 / M_PI;
	else
		return 360.0 - std::acos(cosSourceCenterCurpos) * 180 / M_PI;
}

// 计算点到直线的垂直映射点
template <class T> 
T G_Math_CalcLineMapPt(T srcPt, T pt1, T pt2)
{
	T res;
	if (G_Math_PointValueIsSame(pt1.x, pt2.x)) //x相同，与X轴垂直
	{
		res.x = pt1.x;
		res.y = srcPt.y;
	}
	else if (G_Math_PointValueIsSame(pt1.y, pt2.y)) //y相同, 与y轴垂直
	{
		res.x = srcPt.x;
		res.y = pt1.y;
	}
	else
	{
		auto lineSlope = (pt2.y - pt1.y) / (pt2.x - pt1.x); //线段斜率
		res.x = (pt1.x*lineSlope + srcPt.x / lineSlope + srcPt.y - pt1.y) / (lineSlope + 1 / lineSlope);
		res.y = lineSlope * (res.x - pt1.x) + pt1.y;
	}
	return res;
}

// 点到线段的最小距离(^2)
template <class T> 
double G_Math_PointToSegDist(const T& p0, const T& p1, const T& p2)
{
	double cross = (p2.x - p1.x) * (p0.x - p1.x) + (p2.y - p1.y) * (p0.y - p1.y);
	if (cross <= 0) return pow((p0.x - p1.x), 2) + pow((p0.y - p1.y), 2);

	double d2 = (p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y);
	if (cross >= d2) return pow((p0.x - p2.x), 2) + pow((p0.y - p2.y), 2);

	double r = cross / d2;
	double px = p1.x + (p2.x - p1.x) * r;
	double py = p1.y + (p2.y - p1.y) * r;
	return sqrt(pow((p0.x - px), 2) + pow((p0.y - py), 2));
}

// 计算点到直线的垂直距离
template <class T> 
double	G_Math_CalcPointToLineDistance(T Res, T point1, T point2)
{
	// 两点重合,无法计算
	if (G_Math_PointValueIsSame(point1.x, point2.x) && G_Math_PointValueIsSame(point1.y, point2.y))
	{
		double x = (point1.x + point2.x) / 2;
		double y = (point1.y + point2.y) / 2;
		return sqrt((Res.x - x)*(Res.x - x) + (Res.y - y)*(Res.y - y));
	}
	if (G_Math_PointValueIsSame(point1.x, point2.x))
		return Res.x - point1.x > 0 ? Res.x - point1.x : point1.x - Res.x;
	if (G_Math_PointValueIsSame(point1.y, point2.y))
		return Res.y - point1.y > 0 ? Res.y - point1.y : point1.y - Res.y;
	// 直线不是垂直于x轴的线
	if (!G_Math_PointValueIsSame(point1.x, point2.x))
	{
		double a = 0.0, b = 0.0;
		a = (point2.y - point1.y) / (point2.x - point1.x);
		b = point1.y - a * point1.x;
		// 计算3点围成的三角形面积
		double size = (Res.x * point1.y + point1.x * point2.y + point2.x * Res.y - Res.x * point2.y - point1.x * Res.y - point2.x * point1.y) / 2.0;
		if (size < 0)
			size = 0 - size;
		// 计算point1 point2 的距离
		double point12Length = G_Math_CalcLength(point1, point2);
		return size * 2 / point12Length; // S*2/bottom = Height
	}
	return 0;
}

//判断两条线段是否相交
//充要条件：1.不是平行线, 2.线段所在直线的交点位于两条线段上
template <class T,class V> 
bool G_Math_IsSegmentCross(T p1, T p2, T q1, T q2, V x, V y)
{
	BOOL isCoincident = ((p1 == q1 && p2 == q2) || (p1 == q2 && p2 == q1));
	BOOL isRectCross = 
		std::min(p1.x, p2.x) <= std::max(q1.x, q2.x) && 
		std::min(q1.x, q2.x) <= std::max(p1.x, p2.x) && 
		std::min(p1.y, p2.y) <= std::max(q1.y, q2.y) && 
		std::min(q1.y, q2.y) <= std::max(p1.y, p2.y);
	BOOL isLineSegmentCross = true;
	double line1, line2;
	line1 = p1.x * (q1.y - p2.y) + p2.x * (p1.y - q1.y) + q1.x * (p2.y - p1.y);
	line2 = p1.x * (q2.y - p2.y) + p2.x * (p1.y - q2.y) + q2.x * (p2.y - p1.y);
	if ((line1 * line2 >= 0) && !(std::abs(line1) <= DBL_EPSILON && std::abs(line2) <= DBL_EPSILON))
		isLineSegmentCross = false;

	line1 = q1.x * (p1.y - q2.y) + q2.x * (q1.y - p1.y) + p1.x * (q2.y - q1.y);
	line2 = q1.x * (p2.y - q2.y) + q2.x * (q1.y - p2.y) + p2.x * (q2.y - q1.y);
	if ((line1 * line2 >= 0) && !(std::abs(line1) <= DBL_EPSILON && std::abs(line2) <= DBL_EPSILON))
		isLineSegmentCross = false;

	if (!isCoincident && isRectCross && isLineSegmentCross)
	{
		double tmpLeft, tmpRight;
		tmpLeft = (q2.x - q1.x) * (p1.y - p2.y) - (p2.x - p1.x) * (q1.y - q2.y);
		tmpRight = (p1.y - q1.y) * (p2.x - p1.x) * (q2.x - q1.x) + q1.x * (q2.y - q1.y) * (p2.x - p1.x) - p1.x * (p2.y - p1.y) * (q2.x - q1.x);

		if (std::abs(tmpLeft) < 1e-6)
		{
			return false;
		}
		x = tmpRight / tmpLeft;

		tmpLeft = (p1.x - p2.x) * (q2.y - q1.y) - (p2.y - p1.y) * (q1.x - q2.x);
		tmpRight = p2.y * (p1.x - p2.x) * (q2.y - q1.y) + (q2.x - p2.x) * (q2.y - q1.y) * (p1.y - p2.y) - q2.y * (q1.x - q2.x) * (p2.y - p1.y);

		if (std::abs(tmpLeft) < 1e-6)
		{
			return false;
		}
		y = tmpRight / tmpLeft;

		return true;
	}
	return false;
}

template<class T> 
bool G_Math_IsPointInTriangle(T ptx, T pty, T p0x, T p0y, T p1x, T p1y, T p2x, T p2y)
{
	p0x = ptx - p0x;
	p1x = ptx - p1x;
	p2x = ptx - p2x;
	p0y = pty - p0y;
	p1y = pty - p1y;
	p2y = pty - p2y;
	T t1 = G_Math_GetCross(p0x, p0y, p1x, p1y);
	T t2 = G_Math_GetCross(p1x, p1y, p2x, p2y);
	T t3 = G_Math_GetCross(p2x, p2y, p0x, p0y);
	return t1 * t2 >= 0 && t2 * t3 >= 0 && t3 * t1 >= 0;
}

inline bool G_Math_PtOfTowLineIntersect(CPointD &p1, CPointD &p2, CPointD &p3, CPointD &p4, CPointD& p)
{
	double a1 = p2.y - p1.y;
	double b1 = p1.x - p2.x;
	double c1 = p2.x*p1.y - p2.y*p1.x;

	double a2 = p4.y - p3.y;
	double b2 = p3.x - p4.x;
	double c2 = p4.x*p3.y - p4.y*p3.x;

	double d = a1 * b2 - b1 * a2;
	if (!(abs(d) < DBL_MIN))
	{
		p.x = (b1*c2 - b2 * c1) / d;
		p.y = (c1*a2 - c2 * a1) / d;
		return true;
	}
	return false;
}

inline double G_Math_RadianToAngle(double radian)
{
	return radian / M_PI * 180;
}

inline double G_Math_AngleToRadian(double angle)
{
	return angle / 180 * M_PI;
}

//多边形重叠判断
#include <vector>
#include <limits>

template<class T>//宽松判断
bool G_Math_IsPolygonsOverlap(const std::vector<T>& polygon1, const std::vector<T>& polygon2)
{
	if (polygon1.empty() || polygon2.empty()) return false;

	// 1. 边界框检测 AABB
	float min1_x = std::numeric_limits<float>::max();
	float max1_x = -std::numeric_limits<float>::max();
	float min1_y = std::numeric_limits<float>::max();
	float max1_y = -std::numeric_limits<float>::max();

	for (const auto& pt : polygon1) {
		min1_x = std::min(min1_x, static_cast<float>(pt.x));
		max1_x = std::max(max1_x, static_cast<float>(pt.x));
		min1_y = std::min(min1_y, static_cast<float>(pt.y));
		max1_y = std::max(max1_y, static_cast<float>(pt.y));
	}

	float min2_x = std::numeric_limits<float>::max();
	float max2_x = -std::numeric_limits<float>::max();
	float min2_y = std::numeric_limits<float>::max();
	float max2_y = -std::numeric_limits<float>::max();

	for (const auto& pt : polygon2) {
		min2_x = std::min(min2_x, static_cast<float>(pt.x));
		max2_x = std::max(max2_x, static_cast<float>(pt.x));
		min2_y = std::min(min2_y, static_cast<float>(pt.y));
		max2_y = std::max(max2_y, static_cast<float>(pt.y));
	}

	// 判断 AABB 是否相交
	if (max1_x < min2_x || max2_x < min1_x || max1_y < min2_y || max2_y < min1_y) {
		return false; // AABB 不重叠，直接返回 false
	}

	// 2. 点在多边形内检测
	for (const auto& pt : polygon1) {
		if (G_Math_IsPtInPts(polygon2, pt)) {
			return true;
		}
	}
	for (const auto& pt : polygon2) {
		if (G_Math_IsPtInPts(polygon1, pt)) {
			return true;
		}
	}

	// 3. 边与边相交检测
	// 遍历 polygon1 的每条边
	int n1 = polygon1.size();
	for (int i = 0; i < n1; ++i) {
		int j = (i + 1) % n1;
		const T& p1 = polygon1[i];
		const T& p2 = polygon1[j];

		// 对比 polygon2 的每条边
		int n2 = polygon2.size();
		for (int k = 0; k < n2; ++k) {
			int l = (k + 1) % n2;
			const T& q1 = polygon2[k];
			const T& q2 = polygon2[l];

			float x = 0, y = 0;
			if (G_Math_IsSegmentCross(p1, p2, q1, q2, x, y)) {
				return true; // 存在边相交
			}
		}
	}

	// 所有条件都不满足，认为不重叠
	return false;
}

template<class T>//严格判断
bool G_Math_IsPolygonsOverlapStrictly(const std::vector<T>& polygon1, const std::vector<T>& polygon2)
{
	if (polygon1.empty() || polygon2.empty()) return false;

	// 1. 边界框检测 AABB
	float min1_x = std::numeric_limits<float>::max();
	float max1_x = -std::numeric_limits<float>::max();
	float min1_y = std::numeric_limits<float>::max();
	float max1_y = -std::numeric_limits<float>::max();

	for (const auto& pt : polygon1) {
		min1_x = std::min(min1_x, static_cast<float>(pt.x));
		max1_x = std::max(max1_x, static_cast<float>(pt.x));
		min1_y = std::min(min1_y, static_cast<float>(pt.y));
		max1_y = std::max(max1_y, static_cast<float>(pt.y));
	}

	float min2_x = std::numeric_limits<float>::max();
	float max2_x = -std::numeric_limits<float>::max();
	float min2_y = std::numeric_limits<float>::max();
	float max2_y = -std::numeric_limits<float>::max();

	for (const auto& pt : polygon2) {
		min2_x = std::min(min2_x, static_cast<float>(pt.x));
		max2_x = std::max(max2_x, static_cast<float>(pt.x));
		min2_y = std::min(min2_y, static_cast<float>(pt.y));
		max2_y = std::max(max2_y, static_cast<float>(pt.y));
	}

	// 判断 AABB 是否相交
	if (max1_x <= min2_x || max2_x <= min1_x || max1_y <= min2_y || max2_y <= min1_y) {
		return false; // AABB 不重叠，直接返回 false
	}

	// 2. 点在多边形内检测
	for (const auto& pt : polygon1) {
		if (G_Math_IsPtInPts(polygon2, pt)) {
			return true;
		}
	}
	for (const auto& pt : polygon2) {
		if (G_Math_IsPtInPts(polygon1, pt)) {
			return true;
		}
	}

	// 3. 边与边相交检测
	// 遍历 polygon1 的每条边
	int n1 = polygon1.size();
	for (int i = 0; i < n1; ++i) {
		int j = (i + 1) % n1;
		const T& p1 = polygon1[i];
		const T& p2 = polygon1[j];

		// 对比 polygon2 的每条边
		int n2 = polygon2.size();
		for (int k = 0; k < n2; ++k) {
			int l = (k + 1) % n2;
			const T& q1 = polygon2[k];
			const T& q2 = polygon2[l];

			float x = 0, y = 0;
			if (G_Math_IsSegmentCross(p1, p2, q1, q2, x, y)) {
				return true; // 存在边相交
			}
		}
	}

	// 所有条件都不满足，认为不重叠
	return false;
}

static bool CalcVertexAngleDeg(const CPointD& vertexPos, const CPointD& pointA, const CPointD& pointB, double& dAngleDeg)
{
	CPointD vecToA = CPointD(pointA.x - vertexPos.x, pointA.y - vertexPos.y);
	CPointD vecToB = CPointD(pointB.x - vertexPos.x, pointB.y - vertexPos.y);
	double lenToA = std::hypot(vecToA.x, vecToA.y);
	double lenToB = std::hypot(vecToB.x, vecToB.y);
	if (lenToA <= 1e-6 || lenToB <= 1e-6)
	{
		return false;
	}

	double dot = (vecToA.x * vecToB.x + vecToA.y * vecToB.y) / (lenToA * lenToB);
	if (dot > 1.0)
	{
		dot = 1.0;
	}
	else if (dot < -1.0)
	{
		dot = -1.0;
	}

	dAngleDeg = std::acos(dot) * 180.0 / M_PI;
	return true;
}

static CPointD EvalCubicBezierPoint(const CPointD& p0, const CPointD& p1, const CPointD& p2, const CPointD& p3, double t)
{
	const double u = 1.0 - t;
	const double u2 = u * u;
	const double u3 = u2 * u;
	const double t2 = t * t;
	const double t3 = t2 * t;
	return CPointD(
		p0.x * u3 + p1.x * (3.0 * u2 * t) + p2.x * (3.0 * u * t2) + p3.x * t3,
		p0.y * u3 + p1.y * (3.0 * u2 * t) + p2.y * (3.0 * u * t2) + p3.y * t3);
}

static CPointD CalcBezierArcLengthMidPoint(const CPointD& p0, const CPointD& p1, const CPointD& p2, const CPointD& p3)
{
	const int sampleCount = 1024;
	const int nSimpson = 8;

	std::vector<std::pair<double, double>> s_t_table;
	double totalLength = 0.0;
	s_t_table.emplace_back(0.0, 0.0);

	for (int i = 1; i <= sampleCount; ++i)
	{
		const double a = static_cast<double>(i - 1) / sampleCount;
		const double b = static_cast<double>(i) / sampleCount;
		const double h = (b - a) / nSimpson;
		double integral = 0.0;

		for (int k = 0; k <= nSimpson; ++k)
		{
			const double t_k = a + k * h;
			const double weight = (k == 0 || k == nSimpson) ? 1 : (k % 2 == 1) ? 4 : 2;
			const double u = 1.0 - t_k;
			const double u2 = u * u;
			const double t2 = t_k * t_k;

			const CPointD dx(
				p0.x * (-3 * u2) + p1.x * (3 * u2 - 6 * u * t_k) + p2.x * (6 * u * t_k - 3 * t2) + p3.x * 3 * t2,
				p0.y * (-3 * u2) + p1.y * (3 * u2 - 6 * u * t_k) + p2.y * (6 * u * t_k - 3 * t2) + p3.y * 3 * t2);
			const double speed = std::hypot(dx.x, dx.y);
			integral += weight * speed;
		}

		integral *= h / 3.0;
		totalLength += integral;
		s_t_table.emplace_back(totalLength, b);
	}

	if (totalLength <= 1e-12)
	{
		return p0;
	}

	for (auto& p : s_t_table)
	{
		p.first /= totalLength;
	}

	const double targetS = 0.5;
	auto it = std::lower_bound(s_t_table.begin(), s_t_table.end(),
		std::make_pair(targetS, 0.0),
		[](const std::pair<double, double>& a, const std::pair<double, double>& b)
		{
			return a.first < b.first;
		});

	if (it == s_t_table.begin())
	{
		return EvalCubicBezierPoint(p0, p1, p2, p3, 0.0);
	}
	if (it == s_t_table.end())
	{
		return EvalCubicBezierPoint(p0, p1, p2, p3, 1.0);
	}

	--it;
	const auto nextIt = it + 1;
	const double s0 = it->first;
	const double s1 = nextIt->first;
	const double t0 = it->second;
	const double t1 = nextIt->second;
	const double ratio = (targetS - s0) / (s1 - s0);
	const double t = t0 + ratio * (t1 - t0);
	return EvalCubicBezierPoint(p0, p1, p2, p3, t);
}

static CPointD CalcEdgeMidPoint(const CPointD& beginPos, const CPointD& beginRightHandle,
	const CPointD& endLeftHandle, const CPointD& endPos, bool bCurveEdge)
{
	if (bCurveEdge)
	{
		return CalcBezierArcLengthMidPoint(beginPos, beginRightHandle, endLeftHandle, endPos);
	}

	return CPointD(
		(beginPos.x + endPos.x) * 0.5,
		(beginPos.y + endPos.y) * 0.5);
}

static bool IsConvexPolygon(const std::vector<CPointD>& vecPoint, std::set<int> exceptIndexSet)
{
	const int n = static_cast<int>(vecPoint.size());
	if (n < 3)
	{
		return false;
	}

	int nSign = 0;
	for (int i = 0; i < n; ++i)
	{
		if (exceptIndexSet.count(i))
			continue;

		const CPointD& prevPos = vecPoint[(i - 1 + n) % n];
		const CPointD& currPos = vecPoint[i];
		const CPointD& nextPos = vecPoint[(i + 1) % n];

		double dArea = (currPos.x - prevPos.x) * (nextPos.y - prevPos.y) - (currPos.y - prevPos.y) * (nextPos.x - prevPos.x);
		if (fabs(dArea) < 1e-8)
		{
			continue;
		}

		const int nCurrSign = dArea > 0 ? 1 : -1;
		if (nSign == 0)
		{
			nSign = nCurrSign;
		}
		else if (nCurrSign != nSign)
		{
			return false;
		}
	}

	return true;
}

template <class T>
static CPointD MovePointAlongDirection(const CPointD& p, T dir, double length)
{
	const double eps = 1e-9;
	double len = std::hypot(dir.x, dir.y);

	if (len < eps)
	{
		return p;
	}

	double ux = dir.x / len;
	double uy = dir.y / len;

	double newX = p.x + ux * length;
	double newY = p.y + uy * length;

	return CPointD(newX, newY);
}

template <class T>
static CPointD CalcLineIntersect(T dir1, T dir2,
	const CPointD& basePt, T dir3, T dir4, double length)
{
	// 计算平移单位偏移量
	double lenST = std::hypot(dir3.x, dir3.y);
	double lenPQ = std::hypot(dir4.x, dir4.y);

	double dx1 = 0.0, dy1 = 0.0;
	double dx2 = 0.0, dy2 = 0.0;

	constexpr double EPS = 1e-12;

	if (lenST > EPS)
	{
		dx1 = length * dir3.x / lenST;
		dy1 = length * dir3.y / lenST;
	}
	if (lenPQ > EPS)
	{
		dx2 = length * dir4.x / lenPQ;
		dy2 = length * dir4.y / lenPQ;
	}

	// 二元一次方程组系数
	double A = dir1.x;
	double B = -dir2.x;
	double C = dir1.y;
	double D = -dir2.y;
	double E = dx2 - dx1;
	double F = dy2 - dy1;

	// 行列式判断平行
	double det = A * D - B * C;
	if (std::fabs(det) < EPS)
	{

		return MovePointAlongDirection(basePt, dir3, length);
	}

	// 克莱姆法则求解u、v
	double u = (E * D - B * F) / det;
	// double v = (A * F - E * C) / det; // 如需校验线段区间可启用

	// 代入直线3公式得到交点
	double interX = basePt.x + u * dir1.x + dx1;
	double interY = basePt.y + u * dir1.y + dy1;

	return CPointD(interX, interY);
}

template <class T1, class T2> void	CalcAfterRotatePoint(T1 center, T2 angle, T1* pointsin, T1* pointsout, int count)
{
	T2 dAngle = -angle / 180.0f * M_PI; // 反向旋转
	for (int i = 0; i < count; i++)
	{
		pointsout[i].x = ((pointsin[i].x - center.x) * cos(dAngle) - (pointsin[i].y - center.y) * sin(dAngle)) + center.x;
		pointsout[i].y = ((pointsin[i].y - center.y) * cos(dAngle) + (pointsin[i].x - center.x) * sin(dAngle)) + center.y;
		//此处补充极近似处理
		int x = int(pointsout[i].x);
		int xpp = x + 1;
		int y = int(pointsout[i].y);
		int ypp = y + 1;
		if (pointsout[i].x - x < 0.000001)
			pointsout[i].x = x * 1.0f;
		else if (xpp - pointsout[i].x < 0.000001)
			pointsout[i].x = xpp * 1.0f;

		if (pointsout[i].y - y < 0.000001)
			pointsout[i].y = y * 1.0f;
		else if (ypp - pointsout[i].y < 0.000001)
			pointsout[i].y = ypp * 1.0f;
	}
}