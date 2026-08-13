#define _USE_MATH_DEFINES
#include "BarWidget.h"
#include "LayersToolWidget.h"
#include "ScaleWidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPainter>
#include "TimelineWinMacros.h"

#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif

CBarWidget::CBarWidget(QWidget* pParent /*= 0*/)
	:QWidget(pParent)
{
	InitWidget();
	CreateConnects();
}

CBarWidget::~CBarWidget()
{
}

void CBarWidget::UpdateTranslateText()
{
	if (m_pLayersTool) m_pLayersTool->UpdateTranslateText();
}

void CBarWidget::UpdateView(uint32_t type)
{
	if (m_pLayersTool && type & WndType::WndType_LayersTool) m_pLayersTool->UpdateView();
	if (m_pScale && type & WndType::WndType_TimeLineScale) m_pScale->UpdateView();
}

void CBarWidget::Clear()
{
	m_pScale->Clear();
}

void CBarWidget::InitWidget()
{
	auto CreateLine = [&](int w, int h) {
		QLabel* pLine = new QLabel(this);
		if (w != -1)
			pLine->setFixedWidth(w);
		if (h != -1)
			pLine->setFixedHeight(h);
		pLine->setStyleSheet("background-color: black;");
		return pLine;
	};

	m_pLayersTool = new CLayersToolWidget(this);
	m_pScale = new CScaleWidget(this);

	m_pLayersTool->setFixedWidth(Left_Wnd_Width);
	m_pLayersTool->setFixedHeight(Common_Height);

	// 水平布局：图层操作工具 + 时间刻度条
	QHBoxLayout* pLayout1 = new QHBoxLayout;
	pLayout1->setContentsMargins(0, 0, 0, 0);
	pLayout1->setSpacing(0);
	pLayout1->addWidget(m_pLayersTool);
	pLayout1->addWidget(CreateLine(nSizeDefine_1, -1));
	pLayout1->addWidget(m_pScale);
	pLayout1->addWidget(CreateLine(nSizeDefine_1, Common_Height));

	QVBoxLayout* pMainLayout = new QVBoxLayout(this);
	pMainLayout->setSpacing(0);
	pMainLayout->setContentsMargins(0, 0, 0, 0);
	pMainLayout->addLayout(pLayout1);
}

void CBarWidget::CreateConnects()
{
	connect(m_pLayersTool, &CLayersToolWidget::signalUpdateView, this, [&](int type) {
		emit signalUpdateView(type);
	});
	connect(m_pScale, &CScaleWidget::signalUpdateView, this, [&](int type) {
		emit signalUpdateView(type);
	});
}
