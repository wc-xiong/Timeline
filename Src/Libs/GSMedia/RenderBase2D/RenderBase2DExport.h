#pragma once
#include "RenderBase2DGlobal.h"
#include "IRenderBase2DInterface.h"
#include "BasicEffect.h"
#include <memory>

RENDERBASE2D_API std::unique_ptr<IGridDrawObject> CreateInputGridDrawObject(int nAdapterIndex, BasicEffect* pBasicEffect);
RENDERBASE2D_API std::unique_ptr<ICustom2DDrawHelper> CreateCustom2DDrawHelper();




