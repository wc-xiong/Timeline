#define _USE_MATH_DEFINES		//解决qmath.h和math.h的头文件冲突
#include "LayersControlWidget.h"
#include "GSMedia/ThreadManager/ThreadManagerExport.h"
#include "Controll/SelectController/SelectControllerExport.h"
#include "Controll/PlayerController/PlayerControllerExport.h"
#include "Common/SignalCenter/SignalCenterExport.h"
#include "Controll/ViewDataManager/ViewDataManagerExport.h"
#include "Controll/GlobalHistoryStack/GlobalHistoryStackExport.h"

#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "Model/ResourceModel/ResourceModelExport.h"
#include "Model/GlobalModel/GlobalModelExport.h"
#include "Model/GlobalModel/LogModel.h"

#include "UICommon/menuex.h"
#include "UICommon/Messagebox.h"
#include "UICommon/RenameDialog.h"
#include "UICommon/Colordialog.h"
#include "UICommon/UITranslate.h"

#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QPainter>
#include "TimelineWinMacros.h"


//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CLayersControlWidget::CLayersControlWidget(QWidget* pParent)
	:QWidget(pParent)
{
	InitWidget();
	CreateConnects();
}

CLayersControlWidget::~CLayersControlWidget()
{
}

void CLayersControlWidget::UpdateView()
{
	if (!m_pLabelCtrlAlignSection)
		return;
	if (g_pContentViewData->HasObjectDragPreview())
	{
		// Ctrl 吸附提示始终显示；Alt 新建图层仅单选素材移动时显示
		QString tip = CTimeLineWndTranslate::CtrlAlignSection();
		if (g_pContentViewData->IsObjectDragAltNewLayerTipVisible())
			tip += QLatin1Char(' ') + CTimeLineWndTranslate::AltNewLayerTip();
		m_pLabelCtrlAlignSection->setText(tip);
		m_pLabelCtrlAlignSection->show();
		m_pLabelCtrlAlignSection->update();
	}
	else
	{
		m_pLabelCtrlAlignSection->hide();
		m_pLabelCtrlAlignSection->update();
	}
}

void CLayersControlWidget::InitWidget()
{
	m_pBtnAddLayer = new CSvgImageButton(":/dark/images/TimeLineWnd/Add", this);
	m_pBtnAddLayer->setFixedSize(QSize(nSizeDefine_20, nSizeDefine_20));
	m_pBtnAddLayer->setProperty("transparentBkBtn", true);

	m_pLabelCtrlAlignSection = new QLabel(this);
	m_pLabelCtrlAlignSection->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_pLabelCtrlAlignSection->hide();
	m_pLabelCtrlAlignSection->setProperty("SliceWndEmptyTips2", true);

	QHBoxLayout* pLayout = new QHBoxLayout(this);
	pLayout->setMargin(0);
	pLayout->addSpacing(SysDpiZoom(3));
	pLayout->addWidget(m_pLabelCtrlAlignSection);
	pLayout->addStretch();
	pLayout->addWidget(m_pBtnAddLayer);
	pLayout->addSpacing(SysDpiZoom(3));
}

void CLayersControlWidget::CreateConnects()
{
	connect(m_pBtnAddLayer, &CSvgImageButton::clicked, this, [&]() {

		std::set<QUuid> setIds = g_pSelectController->GetTLLayerSelectIds();
		if (setIds.size() > 0)
		{
			int minIndex = INT_MAX;
			int maxIndex = INT_MIN;
			std::for_each(setIds.begin(), setIds.end(), [&minIndex, &maxIndex](const auto& it) {
				int index = g_pTimeLineModel->GetLayerIndex(it);
				minIndex = std::min(index, minIndex);
				maxIndex = std::max(index, maxIndex);
			});

			//右键菜单
			CMenuEx* pRBtnMenu = new CMenuEx(this);
			pRBtnMenu->setFixedWidth(nSizeDefine_148);

			auto pAddPre = pRBtnMenu->addAction(CTimeLineWndTranslate::InsertLastLayer());
			auto pAddNext = pRBtnMenu->addAction(CTimeLineWndTranslate::InsertNextLayer());

			QAction* pSelect = pRBtnMenu->exec(cursor().pos());

			if (pSelect == pAddPre)
			{
				auto p = g_pTimeLineModel->CreateLayer();
				p->SetName(CTimeLineWndTranslate::Layer() + QString::number(g_pTimeLineModel->GetNewLayerIndex()));
				g_pTimeLineModel->AddLayer(p, minIndex);
				g_pSelectController->ClearTimeLineObjectSelect();
				g_pSelectController->AddTimeLineLayerSelect(p->GetId(), true);

				emit signalUpdateView(WndType::WndType_LayerLabels | WndType::WndType_VerScroll |
					WndType::WndType_LayersView | WndType::WndType_TimeLineTool);

				emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
				emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_MediaNodeSelect);

				HistoryStackSave();
				LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndViewLeftTop(),
					CTimeLineWndTranslate::InsertLastLayer(), QString::number(minIndex));
			}
			else if (pSelect == pAddNext)
			{
				auto p = g_pTimeLineModel->CreateLayer();
				p->SetName(CTimeLineWndTranslate::Layer() + QString::number(g_pTimeLineModel->GetNewLayerIndex()));
				g_pTimeLineModel->AddLayer(p, maxIndex + 1);
				g_pSelectController->ClearTimeLineObjectSelect();
				g_pSelectController->AddTimeLineLayerSelect(p->GetId(), true);

				emit signalUpdateView(WndType::WndType_LayerLabels | WndType::WndType_VerScroll |
					WndType::WndType_LayersView | WndType::WndType_TimeLineTool);

				emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
				emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_MediaNodeSelect);

				HistoryStackSave();
				LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndViewLeftTop(),
					CTimeLineWndTranslate::InsertLastLayer(), QString::number(maxIndex));
			}

			delete pRBtnMenu;
		}
		else
		{
			// 创建一个新时间线图层
			auto p = g_pTimeLineModel->CreateLayer();
			p->SetName(CTimeLineWndTranslate::Layer() + QString::number(g_pTimeLineModel->GetNewLayerIndex()));
			g_pTimeLineModel->AddLayer(p);
			g_pSelectController->ClearTimeLineObjectSelect();
			g_pSelectController->AddTimeLineLayerSelect(p->GetId(), true);

			emit signalUpdateView(WndType::WndType_LayerLabels | WndType::WndType_VerScroll |
				WndType::WndType_LayersView | WndType::WndType_TimeLineTool);

			emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
			emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_MediaNodeSelect);

			HistoryStackSave();
			LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndViewLeftTop(),
				CTimeLineWndTranslate::AddLayer());
		}
	});
}

void CLayersControlWidget::UpdateTranslateText()
{
	if (m_pLabelCtrlAlignSection)
	{
		QString tip = CTimeLineWndTranslate::CtrlAlignSection();
		if (g_pContentViewData && g_pContentViewData->IsObjectDragAltNewLayerTipVisible())
			tip += QLatin1Char(' ') + CTimeLineWndTranslate::AltNewLayerTip();
		m_pLabelCtrlAlignSection->setText(tip);
	}
	if(m_pBtnAddLayer)
		m_pBtnAddLayer->setToolTip(CTimeLineWndTranslate::AddLayer());
}
