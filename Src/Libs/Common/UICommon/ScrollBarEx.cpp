#include "ScrollBarEx.h"
#include <QGuiApplication>
#include <QWheelEvent>
#include "UICommonSizeDefine.h"

#include <QDebug>
#include <QMouseEvent>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CSrollBarEx::CSrollBarEx(Qt::Orientation direct, QWidget* parent) 
	: QScrollBar(direct, parent)
{
	this->setRange(0, 0);
	this->hide();
}

CSrollBarEx::~CSrollBarEx()
{
}

void CSrollBarEx::slotValueChange(int val)
{
	//qDebug() << "set value" << val;
	this->setValue(val);
}

void CSrollBarEx::slotRangeChange(int min, int max)
{
	//qDebug() << "set range " << min << max;
	this->setMinimum(min);
	this->setRange(0, max);
	this->setPageStep(0.75* (this->height() + max));
	this->setVisible(max > 0);
}

void CSrollBarEx::mouseMoveEvent(QMouseEvent *event)
{
	auto curPos = geometry();
	//qDebug() << "--------------------:curSlder pos is:" << curPos;

	if (value() == maximum() && event->pos().x() >= rect().width())
		return;
	else if (value() == minimum() && event->pos().x() < 0)
		return;

	__super::mouseMoveEvent(event);

	//qDebug() << "--------------------:scrollBar mouse pos is:" << event->pos();
}