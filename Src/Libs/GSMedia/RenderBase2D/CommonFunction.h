#pragma once
#include "RenderBase2DGlobal.h"
#include "Common/Common/Maths/Matrix3x2.h"
#include "Common/Common/ICustomDataInterface.h"

namespace CommonFunction {
	D2D1_POINT_2F RENDERBASE2D_API CPointD_2_D2DPoint2F(const CPointD& ptF);
}