#pragma once
#include "RenderItemGlobal.h"
#include "Common/Common/IRenderItemInterface.h"
#include "Common/Common/IMediaPropertyInterface.h"

class CD3DResourceChild;
RENDERITEM_API IBaseRenderItem* CreateRenderItem(std::shared_ptr<SMediaPty> pMediaPty, CD3DResourceChild* pd3dRes, bool bUseCopyTexRender = true);