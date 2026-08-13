#include "RenderBase2DExport.h"
#include "GridDrawObject.h"
#include "Custom2DDrawHelper.h"

#//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

std::unique_ptr<IGridDrawObject> CreateInputGridDrawObject(int nAdapterIndex, BasicEffect* pBasicEffect)
{
	return std::make_unique<CGridDrawObject>(nAdapterIndex,pBasicEffect);
}

std::unique_ptr<ICustom2DDrawHelper> CreateCustom2DDrawHelper()
{
	return std::make_unique<CCustom2DDrawHelper>();
}
