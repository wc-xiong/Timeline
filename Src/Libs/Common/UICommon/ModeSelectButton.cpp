#include "ModeSelectButton.h"
#include "UICommonSizeDefine.h"
#include "UICommonApi.h"

#include <QPainter>
#include <QStyleOptionButton>
#include <QPainterPath>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CModeSelectBtn::CModeSelectBtn(const QString & strPath, const int borderWidth, QWidget *parent)
	: QPushButton(parent)
	, m_strImagePath(strPath)
	, m_nBorderWidth(borderWidth)
{
	m_pix = QPixmap(m_strImagePath);
}

void CModeSelectBtn::SetPixmap(QPixmap pix)
{
	m_pix = pix;
}

void CModeSelectBtn::SetUsed(bool bUsed)
{
	m_bUsed = bUsed;
}

void CModeSelectBtn::SetHasBackgroundColor(bool bHasBackGroundColor)
{
	m_bHasBackGroundColor = bHasBackGroundColor;
}

void CModeSelectBtn::mouseDoubleClickEvent(QMouseEvent *event)
{
	__super::mouseDoubleClickEvent(event);

	emit signalDoubleClicked();
}

void CModeSelectBtn::paintEvent(QPaintEvent * event)
{
	// 获取当前按钮状态
	QStyleOptionButton option;
	option.initFrom(this);

	QPainter painter(this);
	QPainterPath path;
	painter.setRenderHint(QPainter::Antialiasing);

	QRect paintRect = this->rect().adjusted(nSizeDefine_1, nSizeDefine_1, -nSizeDefine_1, -nSizeDefine_1);

	// 浅底色绘制
	if (m_bHasBackGroundColor)
	{
		paintRect = this->rect().adjusted(nSizeDefine_5, nSizeDefine_5, -nSizeDefine_5, -nSizeDefine_5);
		painter.save();
		painter.setPen(QPen(QBrush(QColor(0, 0, 0)), nSizeDefine_1));
		path.addRoundedRect(this->rect().adjusted(nSizeDefine_1, nSizeDefine_1, -nSizeDefine_1, -nSizeDefine_1), nSizeDefine_4, nSizeDefine_4);
		painter.fillPath(path, QColor(75, 78, 82));
		painter.restore();

		painter.fillRect(paintRect, QColor(0, 0, 0));
	}


	painter.setPen(QPen(QColor(0, 0, 0), nSizeDefine_1));
	painter.drawRoundedRect(this->rect(), nSizeDefine_4, nSizeDefine_4);

	painter.save();
	if (!m_pix.isNull())
		painter.drawPixmap(paintRect, m_pix.scaled(paintRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

	painter.restore();
	if (!isChecked() && (option.state & QStyle::State_MouseOver) && !m_bUsed &&isEnabled())
	{
		painter.save();
		painter.setPen(QPen(QBrush(QColor(169, 100, 32)), m_nBorderWidth));
		painter.drawRoundedRect(this->rect().adjusted(nSizeDefine_1, nSizeDefine_1, -nSizeDefine_1, -nSizeDefine_1), nSizeDefine_4, nSizeDefine_4);
		painter.restore();
	}
	else if (isChecked())
	{
		painter.save();
		painter.setPen(QPen(QBrush(QColor(255, 139, 24)), m_nBorderWidth));
		painter.drawRoundedRect(this->rect().adjusted(nSizeDefine_1, nSizeDefine_1, -nSizeDefine_1, -nSizeDefine_1), nSizeDefine_4, nSizeDefine_4);
		painter.restore();
	}
}