#include "CommonFunction.h"
#include "Common/Common/CommonFunctionMath.h"
#include "Common/Common/Maths/Polygon.h"
#include <vector>

D2D1_POINT_2F CommonFunction::CPointD_2_D2DPoint2F(const CPointD& ptF)
{
	D2D1_POINT_2F pt2F;
	pt2F.x = (float)ptF.x;
	pt2F.y = (float)ptF.y;
	return pt2F;
}

