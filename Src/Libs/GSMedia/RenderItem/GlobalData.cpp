#include "RenderItemExport.h"
#include "Common/Common/ResourceDefine.h"
#include "GSMedia/RenderDevice/DeviceResource.h"

#include "BaseRenderItem.h"
#include "FxRenderItem.h"
#include "MediaScrollRenderItem.h"

#//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

IBaseRenderItem* CreateRenderItem(std::shared_ptr<SMediaPty> pMediaPty, CD3DResourceChild* pd3dRes, bool bUseCopyTexRender)
{
	assert(pMediaPty);

	if (!pMediaPty)
		return nullptr;

	switch (pMediaPty->pBasePty->GetResType())
	{
	case ERT_Video:
	case ERT_Picture:
	case ERT_VideoCapture:
	case ERT_WebMedia:
	case ERT_Ndi:
		return new CFxRenderItem(pMediaPty, pd3dRes, bUseCopyTexRender);
	case ERT_Audio:
	case ERT_AudioCapture:
	case ERT_TestChart:
	case ERT_Teleprompter:
	case ERT_Subtitle:
		return new CBaseRenderItem(pMediaPty, pd3dRes, bUseCopyTexRender);
	case ERT_Text:
		return new CMediaScrollRenderItem(pMediaPty, pd3dRes, bUseCopyTexRender);
	default:
		return new CBaseRenderItem(pMediaPty, pd3dRes, bUseCopyTexRender);
	}

	return nullptr;
}