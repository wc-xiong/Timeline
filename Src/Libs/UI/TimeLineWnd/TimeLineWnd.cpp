#include "TimeLineWnd.h"

#include "Common/SignalCenter/SignalCenterExport.h"
#include "Controll/PlayerController/PlayerControllerExport.h"

#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "Model/MediaModel/MediaModelExport.h"

#include "Common/CommonDefine.h"
#include "UICommon/UITranslate.h"
#include "UICommon/UICommonSizeDefine.h"

#include "ContentWidget.h"
#include "PathCameraClipHelper.h"

#include <QEvent>
#include <QDebug>
#include <QResizeEvent>

#include "Controll/SelectController/SelectControllerExport.h"
#include "Controll/GlobalHistoryStack/GlobalHistoryStackExport.h"
#include "Model/GlobalModel/PrjSettingData.h"
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

CTimeLineWnd::CTimeLineWnd(QWidget* parent) : CDockWidgetEx(parent)
{
	m_nWidgetType = EDWT_TimeLineWnd;
	setProperty("DockWidget", QVariant(true));

	int num = 3;
	while (num--)
	{
		auto pLayer = g_pTimeLineModel->CreateLayer();
		int newLayerIndex = g_pTimeLineModel->GetNewLayerIndex();
		pLayer->SetName(CTimeLineWndTranslate::Layer() + QString::number(newLayerIndex));
		g_pTimeLineModel->AddLayer(pLayer, newLayerIndex);
	}

	m_pContentWiget = new CContentWidget(this);
	setWidget(m_pContentWiget);
	CreateConnections();

	setMinimumSize(DockWidgetMinWidth, DockWidgetMinHeight);

	UpdateTranslateText();
	g_pConvertHelper->setFPS(g_pProjectSetting->LoadTimeLineFps());
}

CTimeLineWnd::~CTimeLineWnd()
{
	qDebug() << " ~CTimeLineWnd";
}

void CTimeLineWnd::InitWndAfterLoadPrj(bool bNewPrj)
{
	// 切片工程时间线停止
	g_pPlayerController->Stop();

	if (bNewPrj)
	{
		int num = 3;
		while (num--)
		{
			auto pLayer = g_pTimeLineModel->CreateLayer();
			int newLayerIndex = g_pTimeLineModel->GetNewLayerIndex();
			pLayer->SetName(CTimeLineWndTranslate::Layer() + QString::number(newLayerIndex));
			g_pTimeLineModel->AddLayer(pLayer, newLayerIndex);
		}
		m_pContentWiget->UpdateView();
	}

	m_pContentWiget->UpdateView(WndType::WndType_LayersView | WndType::WndType_TimeLineScale);
}

void CTimeLineWnd::CreateConnections()
{
	connect(g_pSignalCenter, &CSignalCenter::signalRefreshWindow, this, &CTimeLineWnd::slotRefreshWindow);
	// 对象列表 / 3D 视图删相机前：同步删除其全部路径相机片段
	connect(g_pSignalCenter, &CSignalCenter::signalPrepareDeleteModel, this, [](const QUuid& id) {
		PathCameraClipHelper::RemoveAllPathClipsOwnedByCamera(id);
	});
	// 本地播控状态变化：刷新游标 / 播控按钮 / 时间码
	connect(g_pPlayerController, &CPlayerController::signalPlayStateChanged, this, [this]() {
		if (m_pContentWiget)
		{
			m_pContentWiget->UpdateView(
				WndType::WndType_NoniusView | WndType::WndType_PlayControl
				| WndType::WndType_TimeLineTool | WndType::WndType_LayersTool);
		}
	});
}

void CTimeLineWnd::UpdateTranslateText()
{
	SetDockTitle(CTimeLineWndTranslate::TimeLineWnd());
}

void CTimeLineWnd::resizeEvent(QResizeEvent* event)
{
	__super::resizeEvent(event);
	if (m_pContentWiget)
		m_pContentWiget->UpdateView(WndType::WndType_All);
}

void CTimeLineWnd::slotRefreshWindow(int eWindowType, int type)
{
	if (type == SignalCenter::EDDT_All)
		type = WndType::WndType_All;

	if (eWindowType & SignalCenter::EWT_TimeLineWnd)
	{
		m_pContentWiget->UpdateView(type);
	}
}
