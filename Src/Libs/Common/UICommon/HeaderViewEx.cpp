#include "HeaderViewEx.h"
#include <QPainter>
#include <QPaintEvent>
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

CHeaderViewEx::CHeaderViewEx(QWidget* pParent) :
	QHeaderView(Qt::Horizontal, pParent)
	, m_textLeftInterval(8)
{
	setProperty("HeaderViewEx", true);
	setFixedHeight(SysDpiZoom(30));
}

CHeaderViewEx::~CHeaderViewEx()
{
}

void CHeaderViewEx::paintEvent(QPaintEvent *event)
{
	return QHeaderView::paintEvent(event);
}

void CHeaderViewEx::paintSection(QPainter *painter, const QRect &rect, int nCol) const
{
	painter->save();
	QHeaderView::paintSection(painter, rect, nCol);
	painter->restore();

	const int nIntervalHeight = SysDpiZoom(18);
	const int nTextInterval = SysDpiZoom(m_textLeftInterval);
	const int nTextTop = SysDpiZoom(7);
	const int nDevideLineTop = SysDpiZoom(6);

	int nLeft = rect.left();
	int nRight = rect.right();

	if (nCol != count() - 1)
	{
		painter->setPen(QColor(Qt::black));
		painter->drawLine(QPoint(nRight - nSizeDefine_1, nDevideLineTop), QPoint(nRight - nSizeDefine_1, nDevideLineTop + nIntervalHeight));
		painter->setPen(QColor(255, 255, 255, 100));
		painter->drawLine(QPoint(nRight, nDevideLineTop), QPoint(nRight, nDevideLineTop + nIntervalHeight));
	}

	// 写界面文字
	QFont font("Microsoft Yahei");
	font.setPixelSize(SysDpiZoom(12));
	font.setBold(m_bBold);
	painter->setFont(font);
	painter->setPen(QColor(Qt::white));
	QFontMetrics fm = painter->fontMetrics();
	
	if (nCol >= 0 && nCol < m_lstHeadColName.size())
	{
		auto strText = m_lstHeadColName[nCol];
		int nTextWidth = fm.width(strText);

		int nPos = nLeft + ((nCol >= 0 && nCol < m_vecColLeftInterval.size()) ? m_vecColLeftInterval[nCol] : nTextInterval);
		QRect rcText = QRect(QPoint(nPos, nTextTop), QPoint(rect.right(), nTextTop + nIntervalHeight));

		// 判断文字是否显示不全减少偏移量
		if (nTextWidth > rcText.width())
		{
			int nSubWidth = nTextWidth - rcText.width();
			rcText.setLeft(std::max(nLeft + nSizeDefine_2, rcText.left() - nSubWidth));
		}
		painter->drawText(rcText,strText);
	}
}
