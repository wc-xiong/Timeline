#define _USE_MATH_DEFINES
#include "LayersViewWidget.h"
#include "LayerLabelsWidget.h"
#include "LayersWidget.h"
#include "LayersControlWidget.h"
#include "ContentViewData.h"
#include "Model/TimeLineModel/TimeLineModelExport.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QLabel>
#include <QFrame>
#include <algorithm>
#include "TimelineWinMacros.h"
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif

CLayersViewWidget::CLayersViewWidget(QWidget* pParent /*= 0*/)
	:QWidget(pParent)
{
	InitWidget();
	CreateConnects();
}

CLayersViewWidget::~CLayersViewWidget()
{
}

void CLayersViewWidget::UpdateTranslateText()
{
	if (m_pLayersControl)
		m_pLayersControl->UpdateTranslateText();
	if (m_pLayersLabel)
		m_pLayersLabel->UpdateTranslateText();
	if (m_pLayersView)
		m_pLayersView->UpdateTranslateText();
}

void CLayersViewWidget::UpdateView(uint32_t type)
{
	// 先刷新右侧图层行，左侧 Label 才能拿到正确行高（插入/删除图层时）
	if (m_pLayersView && type & WndType_LayersView)
		m_pLayersView->UpdateView();

	if (m_pLayersLabel && type & WndType_LayerLabels)
	{
		// ClipFx 高度变化只 emit LayerLabels|VerScroll：仅同步行高，避免 UpdateView 全量刷按钮/重绘闪动
		// 必须同时含 LayerLabels 与 VerScroll；单独 LayerLabels（如总静音）需走 UpdateView 刷新按钮状态
		const uint32_t kHeightOnlyBits = WndType_LayerLabels | WndType_VerScroll;
		if ((type & kHeightOnlyBits) == kHeightOnlyBits
			&& (type & ~kHeightOnlyBits) == 0
			&& m_pLayersView)
			m_pLayersLabel->RelayoutLayersGeometry(m_pLayersView->GetLayerHeights());
		else
			m_pLayersLabel->UpdateView();
	}
	if (m_pLayersControl && type & WndType_LayersControl)
		m_pLayersControl->UpdateView();
	if (m_pVerScrollbar && type & WndType_VerScroll && m_pLayersLabel)
	{
		// 滚动条状态未变时勿反复 setVisible/setMaximum，避免带动左侧 Label 区域布局闪动
		const int scrollMax = m_pLayersLabel->GetScrollMax();
		const int scrollCur = m_pLayersLabel->GetScrollCur();
		const bool bCanScroll = m_pLayersLabel->IsCanScroll();
		if (scrollCur > scrollMax)
			m_pVerScrollbar->setValue(scrollMax);

		if (m_pVerScrollbar->isVisible() != bCanScroll)
			m_pVerScrollbar->setVisible(bCanScroll);
		m_pVerScrollbar->blockSignals(true);
		if (m_pVerScrollbar->maximum() != scrollMax)
			m_pVerScrollbar->setMaximum(scrollMax);
		if (m_pVerScrollbar->value() != m_pLayersLabel->GetScrollCur())
			m_pVerScrollbar->setValue(m_pLayersLabel->GetScrollCur());
		const int pageStep = m_pLayersLabel->height();
		if (m_pVerScrollbar->pageStep() != pageStep)
			m_pVerScrollbar->setPageStep(pageStep);
		m_pVerScrollbar->blockSignals(false);
	}
	if (m_pHorScrollbar && type & WndType_HorScroll)
	{
		//修改水平滚动条
		int timeLength = g_pContentViewData->TimeToPosition(g_pTimeLineModel->GetDuration());
		int windowWidth = m_pLayersView->width();
		g_pContentViewData->SetTimelineViewWidth(windowWidth);

		if (timeLength + g_pContentViewData->GetXOffset() < windowWidth)
		{
			g_pContentViewData->SetXOffset(std::min(0, windowWidth - timeLength));
		}

		m_pHorScrollbar->setVisible(timeLength > windowWidth);
		m_pHorScrollbar->blockSignals(true);
		m_pHorScrollbar->setMaximum(std::max(timeLength - windowWidth, 0));
		m_pHorScrollbar->setValue(abs(g_pContentViewData->GetXOffset()));
		m_pHorScrollbar->setPageStep(windowWidth);
		m_pHorScrollbar->blockSignals(false);
	}
}

void CLayersViewWidget::SetHorScrollbarVal(int val)
{
	m_pHorScrollbar->setValue(val);
}

void CLayersViewWidget::SetVerScrollbarVal(int val)
{
	m_pVerScrollbar->setValue(val);
}

void CLayersViewWidget::SetScrollCur(int val)
{
	m_pLayersLabel->SetScrollCur(val);
}

void CLayersViewWidget::InitWidget()
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

	// 创建子部件
	m_pLayersLabel = new CLayerLabelsWidget(this);
	m_pLayersControl = new CLayersControlWidget(this);
	m_pLayersView = new CLayersWidget(this);

	m_pLayersLabel->setFixedWidth(Left_Wnd_Width);
	m_pLayersControl->setFixedWidth(Left_Wnd_Width);
	m_pLayersControl->setFixedHeight(Common_Height);
	// 左侧垂直布局：图层列表 + 控制按钮
	QVBoxLayout* pLeftLayout = new QVBoxLayout;
	pLeftLayout->setContentsMargins(0, 0, 0, 0);
	pLeftLayout->setSpacing(0);
	pLeftLayout->addWidget(m_pLayersLabel);
	pLeftLayout->addSpacing(nSizeDefine_16);
	pLeftLayout->addWidget(CreateLine(Left_Wnd_Width, nSizeDefine_1));
	pLeftLayout->addWidget(m_pLayersControl);
	pLeftLayout->addWidget(CreateLine(Left_Wnd_Width, nSizeDefine_1));
	pLeftLayout->addSpacing(nSizeDefine_4);

	// 右侧垂直布局：单独的图层列表
	QFrame* pFrameHorScorllbar = new QFrame(this);
	pFrameHorScorllbar->setFixedHeight(Common_Height);
	QVBoxLayout* pFrameLayout = new QVBoxLayout(pFrameHorScorllbar);
	pFrameLayout->setMargin(0);
	pFrameLayout->setSpacing(0);
	m_pHorScrollbar = new QScrollBar(Qt::Horizontal, pFrameHorScorllbar);
	m_pHorScrollbar->setFixedHeight(SysDpiZoom(12));
	m_pHorScrollbar->setProperty("TimeLineWndScrollBar", true);
	pFrameLayout->addStretch();
	pFrameLayout->addWidget(m_pHorScrollbar);
	pFrameLayout->addStretch();

	QFrame* pFrameVerScorllbar = new QFrame(this);
	pFrameVerScorllbar->setFixedWidth(SysDpiZoom(12));
	QVBoxLayout* pViewVerSliderLayout = new QVBoxLayout(pFrameVerScorllbar);
	pViewVerSliderLayout->setMargin(0);
	pViewVerSliderLayout->setSpacing(0);
	m_pVerScrollbar = new QScrollBar(Qt::Vertical, pFrameVerScorllbar);
	m_pVerScrollbar->setFixedWidth(SysDpiZoom(12));
	m_pVerScrollbar->setProperty("TimeLineWndScrollBar", true);
	pViewVerSliderLayout->addWidget(m_pVerScrollbar);
	pViewVerSliderLayout->addSpacing(Common_Height);

	QVBoxLayout* pRightLayout = new QVBoxLayout;
	pRightLayout->setMargin(0);
	pRightLayout->setContentsMargins(0, 0, 0, 0);
	pRightLayout->setSpacing(0);
	pRightLayout->addWidget(m_pLayersView);
	pRightLayout->addSpacing(nSizeDefine_16);
	pRightLayout->addWidget(CreateLine(-1, nSizeDefine_1));
	pRightLayout->addWidget(pFrameHorScorllbar);
	pRightLayout->addWidget(CreateLine(-1, nSizeDefine_1));
	pRightLayout->addSpacing(nSizeDefine_4);

	// 水平布局：左右并排
	QHBoxLayout* pMainLayout = new QHBoxLayout(this);
	pMainLayout->setMargin(0);
	pMainLayout->setSpacing(0);
	pMainLayout->addLayout(pLeftLayout);
	pMainLayout->addWidget(CreateLine(nSizeDefine_1, -1));
	pMainLayout->addLayout(pRightLayout);
	pMainLayout->addWidget(CreateLine(nSizeDefine_1, -1));
	pMainLayout->addWidget(pFrameVerScorllbar);

}

void CLayersViewWidget::CreateConnects()
{
	connect(m_pLayersLabel, &CLayerLabelsWidget::signalUpdateView, this, [&](int type) {
		UpdateView(type);
	});
	connect(m_pLayersLabel, &CLayerLabelsWidget::signalWheel, this, [&](int tmp) {
		if (m_pVerScrollbar)m_pVerScrollbar->setValue(m_pVerScrollbar->value() + tmp);
	});
	connect(m_pLayersLabel, &CLayerLabelsWidget::signalUpdateDragLine, this, [&](int index) {
		emit signalUpdateDragLine(index);
	});
	connect(m_pLayersView, &CLayersWidget::signalWheel, this, [&](int x, int y) {
		if (m_pHorScrollbar) m_pHorScrollbar->setValue(m_pHorScrollbar->value() + x);
		if (m_pVerScrollbar) m_pVerScrollbar->setValue(m_pVerScrollbar->value() + y);
		emit signalUpdateView(WndType::WndType_TimeLineTool);
	});
	connect(m_pLayersView, &CLayersWidget::signalUpdateView, this, &CLayersViewWidget::signalUpdateView);
	connect(m_pHorScrollbar, &QScrollBar::valueChanged, this, [&](int v) {
		emit signalHorScrollValueChanged(v);
	});
	connect(m_pVerScrollbar, &QScrollBar::valueChanged, this, [&](int v) {
		g_pContentViewData->SetYOffset(-v);
		if(m_pLayersLabel) m_pLayersLabel->SetScrollCur(v);
		UpdateView(WndType::WndType_LayersView);
	});
	connect(m_pLayersControl, &CLayersControlWidget::signalUpdateView, this, [&](int type) {
		emit signalUpdateView(type);
	});
}
