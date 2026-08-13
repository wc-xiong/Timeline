#pragma once
#include "UICommonGlobal.h"
#include "DockWidget/DockWidget.h"

class UICOMMON_EXPORT CDockWidgetEx : public CDockWidget
{
	Q_OBJECT
public:
	CDockWidgetEx(QWidget* pParent = 0):CDockWidget(pParent){}
	virtual ~CDockWidgetEx() = default;

	virtual void InitWndAfterLoadPrj(bool bNewPrj) = 0;
};
