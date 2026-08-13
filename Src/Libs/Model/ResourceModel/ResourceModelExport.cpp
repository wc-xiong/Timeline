#include "ResourceModelExport.h"
#include "ResourceModel.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

RESOURCEMODEL_EXPORT CResourceModel* CreateResourceModel()
{
	return new CResourceModel;
}

CResourceModel* pResourceModel = nullptr;
RESOURCEMODEL_EXPORT void InitResourceModel()
{
	pResourceModel = new CResourceModel;
}
RESOURCEMODEL_EXPORT CResourceModel* GetResourceModel()
{
	return pResourceModel;
}
RESOURCEMODEL_EXPORT void UninitResourceModel()
{
	delete pResourceModel;
}
