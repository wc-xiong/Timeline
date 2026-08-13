#include "CRadioButton.h"

#include "UICommonSizeDefine.h"


//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CRadioButton::CRadioButton(QWidget *parent)
	: QRadioButton(parent)
{
	
}

CRadioButton::~CRadioButton()
{
}

void CRadioButton::SetText(const QString& text, int nIndicatorSize)
{
	m_strText = text;
	setToolTip(text);
	const int nWidth = width() - nIndicatorSize;
	// 布局前 width 常过小，强行 elide 会得到空串；宽度不足时直接显示全文
	if (nWidth <= 0)
	{
		setText(text);
		return;
	}
	auto str = fontMetrics().elidedText(text, Qt::ElideRight, nWidth);
	setText(str.isEmpty() ? text : str);
}

QString CRadioButton::GetText()
{
	return m_strText;
}
