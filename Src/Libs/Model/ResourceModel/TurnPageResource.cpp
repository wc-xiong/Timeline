#include "TurnPageResource.h"
#include "common/MediaCommon.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CTurnPageResource::CTurnPageResource()
{

}

CTurnPageResource::~CTurnPageResource()
{

}


CTurnPageResource::CTurnPageResource(const CTurnPageResource& other)
{
}

CTurnPageResource& CTurnPageResource::operator=(const CTurnPageResource& other)
{
	return *this;
}

void CTurnPageResource::SetCurPage(int nPage)
{
	m_nCurPage.store(nPage);
}

int CTurnPageResource::GetCurPage()
{
	return m_nCurPage.load();
}