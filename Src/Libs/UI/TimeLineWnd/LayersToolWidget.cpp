#define _USE_MATH_DEFINES		//解决qmath.h和math.h的头文件冲突
#include "LayersToolWidget.h"
#include "GSMedia/ThreadManager/ThreadManagerExport.h"
#include "Controll/SelectController/SelectControllerExport.h"
#include "Controll/PlayerController/PlayerControllerExport.h"
#include "Common/SignalCenter/SignalCenterExport.h"
#include "Common/SignalCenter/GSNetSignalCenter.h"
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

#define LayersToolWidgetLabelColor QColor(62, 66, 72)
CLayersToolWidget::CLayersToolWidget(QWidget* pParent)
	:QWidget(pParent)
{
	InitWidget();
	CreateConnects();
}

CLayersToolWidget::~CLayersToolWidget()
{
}

void CLayersToolWidget::UpdateView()
{
	const auto vecIDs = g_pTimeLineModel->GetAllLayerIDs();
	bool bLock = true;
	bool bProcess = true;
	bool bHide = true;
	bool bMute = true;

	g_pContentViewData->ClearStateIds(0);
	g_pContentViewData->ClearStateIds(1);
	g_pContentViewData->ClearStateIds(2);
	g_pContentViewData->ClearStateIds(3);
	for (const QUuid& id : vecIDs)
	{
		auto pLayer = g_pTimeLineModel->GetLayer(id).lock();
		if (pLayer)
		{
			bLock &= pLayer->IsLock();
			bProcess &= pLayer->IsShowPlayProgress();
			bHide &= pLayer->IsHide();
			bMute &= pLayer->IsMute();

			if (pLayer->IsLock()) g_pContentViewData->AddStateIds(id, 0);
			if (pLayer->IsShowPlayProgress()) g_pContentViewData->AddStateIds(id, 1);
			if (pLayer->IsHide()) g_pContentViewData->AddStateIds(id, 2);
			if (pLayer->IsMute()) g_pContentViewData->AddStateIds(id, 3);
		}
	}

	m_pBtnLock->setChecked(bLock);
	m_pBtnProcess->setChecked(bProcess);
	m_pBtnHide->setChecked(bHide);
	m_pBtnMute->setChecked(bMute);

	m_pBtnHide->setEnabled(!bLock);
	m_pBtnMute->setEnabled(!bLock);
}

void CLayersToolWidget::InitWidget()
{
	m_pColorButton = new CTimeLineLayerLabelColor(this);
	m_pColorButton->setFixedSize(nSizeDefine_10, Common_Height);
	m_pColorButton->SetIdAndColor(QUuid(), LayersToolWidgetLabelColor);

	m_pBtnLock = new CSvgImageButton(":/dark/images/TimeLineWnd/Lock", this);
	m_pBtnLock->setFixedSize(UICommon_ImageBtnSize);
	m_pBtnLock->setProperty("transparentBkBtn", true);
	m_pBtnLock->setCheckable(true);

	m_pBtnProcess = new CSvgImageButton(":/dark/images/TimeLineWnd/Process", this);
	m_pBtnProcess->setFixedSize(UICommon_ImageBtnSize);
	m_pBtnProcess->setProperty("transparentBkBtn", true);
	m_pBtnProcess->setCheckable(true);

	m_pBtnHide = new CSvgImageButton(":/dark/images/TimeLineWnd/Hide", this);
	m_pBtnHide->setFixedSize(UICommon_ImageBtnSize);
	m_pBtnHide->setProperty("transparentBkBtn", true);
	m_pBtnHide->setCheckable(true);

	m_pBtnMute = new CSvgImageButton(":/dark/images/TimeLineWnd/Mute", this);
	m_pBtnMute->setFixedSize(UICommon_ImageBtnSize);
	m_pBtnMute->setProperty("transparentBkBtn", true);
	m_pBtnMute->setCheckable(true);

	QHBoxLayout* pLayout = new QHBoxLayout(this);
	pLayout->setMargin(0);
	pLayout->addWidget(m_pColorButton);
	pLayout->addStretch();
	pLayout->addWidget(m_pBtnProcess);
	pLayout->addWidget(m_pBtnLock);
	pLayout->addWidget(m_pBtnHide);
	pLayout->addWidget(m_pBtnMute);
	pLayout->addSpacing(nSizeDefine_6);
}

void CLayersToolWidget::CreateConnects()
{
	connect(m_pBtnLock, &CSvgImageButton::clicked, this, [&]() {
		bool bClear = false;

		const auto vecIDs = g_pTimeLineModel->GetAllLayerIDs();
		for (const QUuid& id : vecIDs)
		{
			auto pLayer = g_pTimeLineModel->GetLayer(id).lock();
			if (pLayer)
			{
				if (m_pBtnLock->isChecked())
					pLayer->SetLock(true);
				else
				{
					if (g_pContentViewData->GetStateIdsNum(0) == (int)vecIDs.size())
					{
						pLayer->SetLock(false);
						bClear = true;
					}
					else if (!g_pContentViewData->IsStateIds(id, 0))
						pLayer->SetLock(false);
				}
			}
		}

		if (bClear) g_pContentViewData->ClearStateIds(0);

		m_pBtnHide->setEnabled(!m_pBtnLock->isChecked());
		m_pBtnMute->setEnabled(!m_pBtnLock->isChecked());

		emit signalUpdateView(WndType::WndType_LayerLabels |
			WndType::WndType_LayersView);

		g_pSelectController->ClearTimeLineObjectSelect();

		emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
		emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_MediaNodeSelect);

		HistoryStackSave();

		LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndViewLeftTop(),
			CTimeLineWndTranslate::LockUnlock(), CCommonTranslate::Click());
	});
	connect(m_pBtnProcess, &CSvgImageButton::clicked, this, [&]() {
		bool bClear = false;

		const auto vecIDs = g_pTimeLineModel->GetAllLayerIDs();
		for (const QUuid& id : vecIDs)
		{
			auto pLayer = g_pTimeLineModel->GetLayer(id).lock();
			if (pLayer)
			{
				if (m_pBtnProcess->isChecked())
					pLayer->SetShowPlayProgress(true);
				else
				{
					if (g_pContentViewData->GetStateIdsNum(1) == (int)vecIDs.size())
					{
						pLayer->SetShowPlayProgress(false);
						bClear = true;
					}
					else if (!g_pContentViewData->IsStateIds(id, 1))
						pLayer->SetShowPlayProgress(false);
				}
			}
		}

		if (bClear) g_pContentViewData->ClearStateIds(1);

		emit signalUpdateView(WndType::WndType_LayerLabels |
			WndType::WndType_LayersView);

		HistoryStackSave();

		LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndViewLeftTop(),
			CPropertyTranslate::ShowPlayProgress(), CCommonTranslate::Click());
	});
	connect(m_pBtnHide, &CSvgImageButton::clicked, this, [&]() {
		bool bClear = false;

		const auto vecIDs = g_pTimeLineModel->GetAllLayerIDs();
		for (const QUuid& id : vecIDs)
		{
			auto pLayer = g_pTimeLineModel->GetLayer(id).lock();
			if (pLayer)
			{
				if (m_pBtnHide->isChecked())
				{
					pLayer->SetHide(true);
					pLayer->SetMute(true);
				}
				else
				{
					if (g_pContentViewData->GetStateIdsNum(2) == (int)vecIDs.size())
					{
						pLayer->SetHide(false);
						bClear = true;
					}
					else if (!g_pContentViewData->IsStateIds(id, 2))
						pLayer->SetHide(false);

					if (!g_pContentViewData->IsStateIds(id, 3))
						pLayer->SetMute(false);
				}
			}
		}

		if (bClear) g_pContentViewData->ClearStateIds(2);

		if (m_pBtnHide->isChecked())
			m_pBtnMute->setChecked(true);
		else
			m_pBtnMute->setChecked(g_pContentViewData->GetStateIdsNum(3) == (int)vecIDs.size());

		emit signalUpdateView(WndType::WndType_LayerLabels |
			WndType::WndType_LayersView);

		HistoryStackSave();
		if (g_pGSNetSignal)
		{
			QList<QUuid> layerIds;
			layerIds.reserve(static_cast<int>(vecIDs.size()));
			for (const QUuid& id : vecIDs)
				layerIds.append(id);
			emit g_pGSNetSignal->signalModifyTimelineLayerHideMute(layerIds);
		}
		LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndViewLeftTop(),
			CTimeLineWndTranslate::HideShow(), CCommonTranslate::Click());
	});
	connect(m_pBtnMute, &CSvgImageButton::clicked, this, [&]() {
		bool bClear = false;

		const auto vecIDs = g_pTimeLineModel->GetAllLayerIDs();
		for (const QUuid& id : vecIDs)
		{
			auto pLayer = g_pTimeLineModel->GetLayer(id).lock();
			if (pLayer)
			{
				if (m_pBtnMute->isChecked())
					pLayer->SetMute(true);
				else
				{
					if (g_pContentViewData->GetStateIdsNum(3) == (int)vecIDs.size())
					{
						pLayer->SetMute(false);
						bClear = true;
					}
					else if (!g_pContentViewData->IsStateIds(id, 3))
						pLayer->SetMute(false);
				}
			}
		}

		if (bClear) g_pContentViewData->ClearStateIds(3);

		emit signalUpdateView(WndType::WndType_LayerLabels);

		HistoryStackSave();
		if (g_pGSNetSignal)
		{
			QList<QUuid> layerIds;
			layerIds.reserve(static_cast<int>(vecIDs.size()));
			for (const QUuid& id : vecIDs)
				layerIds.append(id);
			emit g_pGSNetSignal->signalModifyTimelineLayerHideMute(layerIds);
		}

		LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndViewLeftTop(),
			CTimeLineWndTranslate::MuteUnmute(), CCommonTranslate::Click());
	});
	//connect(g_pSignalCenter, &CSignalCenter::signalStopTimeLindPlay, this, [&]() {
	//	slotStopPlay(false);
	//});
}

void CLayersToolWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	QRect rc = rect();

	painter.fillRect(rc, QColor(18, 20, 22));

	__super::paintEvent(event);
}

void CLayersToolWidget::UpdateTranslateText()
{
	m_pBtnLock->setToolTip(CTimeLineWndTranslate::LockUnlock());
	m_pBtnProcess->setToolTip(CPropertyTranslate::ShowPlayProgress());
	m_pBtnHide->setToolTip(CTimeLineWndTranslate::HideShow());
	m_pBtnMute->setToolTip(CTimeLineWndTranslate::MuteUnmute());
}

void CLayersToolWidget::slotStopPlay(bool bBtn)
{
	g_pPlayerController->Stop();

	emit signalUpdateView(WndType_NoniusView);

	if (bBtn)
	{
		LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndViewLeftTop(),
			CTimeLineWndTranslate::Stop(), CCommonTranslate::Click());
	}
	else
	{
		LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndViewLeftTop(),
			CTimeLineWndTranslate::Stop(), CTimeLineWndTranslate::StopByStageChange());
	}
}
