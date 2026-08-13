#include "SuspendedScrollBar.h"
#include "UICommonSizeDefine.h"

#include <QVariant>
#include <QAbstractItemView>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CSuspendedScrollBar::CSuspendedScrollBar(Qt::Orientation t, QWidget *parent)
	: QScrollBar(parent)
{
	setOrientation(t);

	setProperty("SuspendedScrollBar", true);
	setContextMenuPolicy(Qt::NoContextMenu);
	setRange(0, 0);
	hide();
	m_nScrollMode = QAbstractItemView::ScrollMode::ScrollPerPixel;	//默认按照像素滚动
}

CSuspendedScrollBar::~CSuspendedScrollBar()
{

}

void CSuspendedScrollBar::SetScrollMode(int mode)
{
	m_nScrollMode = mode;
}

void CSuspendedScrollBar::slotRangeChanged(int nMin, int nMax)
{
	setRange(nMin,nMax);
	//
	if(this->orientation() == Qt::Vertical && m_nScrollMode == QAbstractItemView::ScrollMode::ScrollPerPixel)
		setPageStep(0.2 * (this->height() + nMax) );
	else if(this->orientation() == Qt::Horizontal)
		setPageStep(0.75 * (this->width() + nMax));

	if (nMax <= 0)
		hide();
}

void CSuspendedScrollBar::slotValueChange(int nValue)
{
	setValue(nValue);
}