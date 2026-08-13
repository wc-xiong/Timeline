#include "SignalCenterExport.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CSignalCenter* pSignalCenter = nullptr;
SIGNALCENTER_EXPORT void InitSignalCenter()
{
	pSignalCenter = new CSignalCenter;
}
SIGNALCENTER_EXPORT CSignalCenter* GetSignalCenter()
{
	return pSignalCenter;
}
SIGNALCENTER_EXPORT void UninitSignalCenter()
{
	delete pSignalCenter;
}
