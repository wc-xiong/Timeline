#pragma once
#define VIEWDATAMANAGER_STATIC_LIB
#include "ViewDataManagerGlobal.h"
#include <chrono>

class VIEWDATAMANAGER_EXPORT CViewDataManager
{
public:
	std::chrono::milliseconds GetTimeClipDurationOffset() const { return std::chrono::milliseconds(0); }
};

VIEWDATAMANAGER_EXPORT void InitViewDataManager();
VIEWDATAMANAGER_EXPORT CViewDataManager* GetViewDataManager();
VIEWDATAMANAGER_EXPORT void UninitViewDataManager();
#define g_pViewDataManager GetViewDataManager()
