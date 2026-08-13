#include "CheckBoxEx.h"

#include <QApplication>
#include <QMouseEvent>

#include "UICommonSizeDefine.h"

#include <QResizeEvent>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CCheckBoxEx::CCheckBoxEx(QWidget* pParent):
	QCheckBox(pParent)
{
	m_nBoxAndSpaceWidth = SysDpiZoom(18);
	setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
}

CCheckBoxEx::~CCheckBoxEx()
{
}

void CCheckBoxEx::SetChecked(bool bChecked)
{
	blockSignals(true);

	__super::setChecked(bChecked);

	blockSignals(false);
}

void CCheckBoxEx::SetCheckState(Qt::CheckState state)
{
	blockSignals(true);
	__super::setCheckState(state);
	blockSignals(false);
}

void CCheckBoxEx::setText(const QString &text)
{
	m_strText = text;
	QString strEndingText = this->fontMetrics().elidedText(text, Qt::ElideRight, width() - nSizeDefine_24);
	QCheckBox::setText(strEndingText);
	setToolTip(text);
}

void CCheckBoxEx::resizeEvent(QResizeEvent *event)
{
	QString strEndingText = this->fontMetrics().elidedText(m_strText, Qt::ElideRight, event->size().width() - nSizeDefine_24);
	QCheckBox::setText(strEndingText);
}

void CCheckBoxEx::mousePressEvent(QMouseEvent *event)
{
	QApplication::sendEvent(parentWidget(), event);	//让父窗口感知本窗口鼠标信息

	__super::mousePressEvent(event);
}
