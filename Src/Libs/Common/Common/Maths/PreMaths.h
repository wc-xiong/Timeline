#pragma once

template <class T> T G_PreMaths_GetCross(T x0, T y0, T x1, T y1)			//计算向量叉积
{
	return x0 * y1 - x1 * y0;
}

inline float	PreMaths_CalcPlaneDistance(float x, float y)
{
	return sqrt(x * x + y * y);
}