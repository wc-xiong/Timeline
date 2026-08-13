#include "BaseWidget.h"
#include <QListView>
#include <QWheelEvent>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CBaseWidget::CBaseWidget(QWidget* pParent):
	QWidget(pParent)
{
}

CBaseWidget::~CBaseWidget()
{
}

void CBaseWidget::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		UpdateTranslateText();
	}
	__super::changeEvent(event);
}