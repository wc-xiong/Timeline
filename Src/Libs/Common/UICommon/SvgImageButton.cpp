#include "SvgImageButton.h"
#include <QPainter>
#include <QPaintEvent>
#include <QEvent>
#include "UICommonSizeDefine.h"
#include <QApplication>
#include <QStyleOptionButton>
#include <QFile>
#include "UICommonApi.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CSvgImageButton::CSvgImageButton(const QString &strImagePath, QWidget* pParent, bool bSendMouseEventToParent) :
	QPushButton(pParent),
	m_strImagePath(strImagePath),
	m_bSendMouseEventToParent(bSendMouseEventToParent)
{
}

CSvgImageButton::~CSvgImageButton()
{
}

bool CSvgImageButton::event(QEvent* event)
{
	if (event->type() == QEvent::ToolTip && m_bHideToolTipWhenDisabled && !isEnabled())
		return true;

	return QPushButton::event(event);
}

void CSvgImageButton::SetHideToolTipWhenDisabled(bool bHide)
{
	m_bHideToolTipWhenDisabled = bHide;
}

void CSvgImageButton::SetImagePath(const QString &strImagePath) {
	m_strImagePath = strImagePath;
}

void CSvgImageButton::mousePressEvent(QMouseEvent *e)
{
	if (m_bSendMouseEventToParent)
		QApplication::sendEvent(parentWidget(), e);

	__super::mousePressEvent(e);
}

void CSvgImageButton::mouseReleaseEvent(QMouseEvent *e)
{
	if (m_bSendMouseEventToParent)
		QApplication::sendEvent(parentWidget(), e);

	__super::mouseReleaseEvent(e);
}

void CSvgImageButton::paintEvent(QPaintEvent *event)
{
	__super::paintEvent(event);

	// 获取当前按钮状态
	QStyleOptionButton option;
	option.initFrom(this);

	QString strCurImagePath = m_strImagePath;
	if(isCheckable())
		strCurImagePath += (isChecked() ? "/Checked" : "/UnChecked");

	auto strPrePath = strCurImagePath;
	if(!isEnabled())
		strCurImagePath += "/Disable.svg";
	else if (isDown())
		strCurImagePath += "/Down.svg";
	else if (option.state & QStyle::State_MouseOver)
		strCurImagePath += "/Hover.svg";
	else
		strCurImagePath += "/Normal.svg";

	if(!QFile::exists(strCurImagePath))
		strCurImagePath = strPrePath + "/Normal.svg";

	QPainter painter(this);
	painter.drawPixmap(this->rect(), GetPixmapFromSvg(strCurImagePath, this->rect().size()));
}
