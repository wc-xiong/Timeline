#include "DockSplitter.h"
#include "ads_globals.h"

#include "../UICommonExport.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CDockManager* g_pDockManager = nullptr;

void InitDockManager(QWidget* pWidget)
{
	if (!g_pDockManager)
		g_pDockManager = new CDockManager(pWidget);
}

void UnInitDockManager()
{
	if (g_pDockManager)
	{
		delete g_pDockManager;
		g_pDockManager = nullptr;
	}
}

CDockManager* GetDockManager()
{
	return g_pDockManager;
}

namespace internal
{
	//============================================================================
	void hideEmptyParentSplitters(CDockSplitter* Splitter)
	{
		while (Splitter && Splitter->isVisible())
		{
			if (!Splitter->hasVisibleContent())
			{
				Splitter->hide();
			}
			Splitter = internal::findParent<CDockSplitter*>(Splitter);
		}
	}

} // namespace internal

//---------------------------------------------------------------------------
// EOF ads_globals.cpp
