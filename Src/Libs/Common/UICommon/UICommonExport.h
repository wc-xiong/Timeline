#pragma once
#include "UICommonGlobal.h"

class QWidget;
class CDockManager;

// 传入pWidget为所有DockWidget父窗口
UICOMMON_EXPORT void InitDockManager(QWidget* pWidget);

UICOMMON_EXPORT void UnInitDockManager();

UICOMMON_EXPORT CDockManager* GetDockManager();

#define g_pDockWidgetMgr GetDockManager()