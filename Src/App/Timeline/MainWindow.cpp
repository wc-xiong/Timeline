#include "MainWindow.h"
#include "MediaPreviewWnd.h"
#include "PreviewPlayBar.h"
#include "TimelinePreviewBridge.h"
#include "UI/PropertyWnd/TimelinePropertyHost.h"
#include "UI/TimeLineWnd/TimeLineWnd.h"
#include "UI/ResourceWnd/ResourceWnd.h"

#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "Common/SignalCenter/SignalCenterExport.h"
#include "Common/SignalCenter/GSNetSignalCenter.h"
#include "Controll/SelectController/SelectControllerExport.h"
#include "Controll/PlayerController/PlayerControllerExport.h"
#include "Controll/PlayCommandCenter/PlayCommandCenterExport.h"
#include "Controll/GlobalHistoryStack/GlobalHistoryStackExport.h"
#include "Controll/ViewDataManager/ViewDataManagerExport.h"
#include "Controll/PlayStateCenter/PlayStateCenterExport.h"
#include "Model/MediaModel/MediaModelExport.h"
#include "Model/ResourceModel/ResourceModelExport.h"
#include "GSMedia/TimeServer/TimeServerExport.h"
#include "Model/GlobalModel/PrjSettingData.h"
#include "Model/TimeLineModel/ConvertHelper.h"

#include <QVBoxLayout>
#include <QTimer>
#include <QSizePolicy>
#include <QPalette>
#include <QIcon>
#include <QShowEvent>

#ifdef Q_OS_WIN
#include <Windows.h>
#include <dwmapi.h>
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1
#define DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1 19
#endif
#ifndef DWMWA_CAPTION_COLOR
#define DWMWA_CAPTION_COLOR 35
#endif
#ifndef DWMWA_TEXT_COLOR
#define DWMWA_TEXT_COLOR 36
#endif
#endif

CMainWindow::CMainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	// 暗色主窗样式（style.css: QWidget[MainWindow=true]）
	setProperty("MainWindow", true);

	// 时间线相关单例初始化（桩实现）
	InitSignalCenter();
	InitTimeLineModel();
	InitSelectController();
	InitPlayerController();
	PlayCommandCenter::InitPlayCommandController();
	InitHistoryStack();
	InitViewDataManager();
	PlayStateCenter::InitPlayStateCenter();
	InitMediaModel();
	InitResourceModel();
	TimeServer_Init();
	g_pConvertHelper->setFPS(g_pProjectSetting->LoadTimeLineFps());
	g_pConvertHelper->setDropMode(false);

	setWindowTitle(QStringLiteral("Timeline"));
	setWindowIcon(QIcon(QStringLiteral(":/dark/images/App/Timeline.svg")));
	resize(1440, 900);

	m_pRootSplitter = new QSplitter(Qt::Vertical, this);
	m_pRootSplitter->setProperty("MainWindowSplitter", true);
	m_pRootSplitter->setChildrenCollapsible(false);
	m_pTopSplitter = new QSplitter(Qt::Horizontal, m_pRootSplitter);
	m_pTopSplitter->setProperty("MainWindowSplitter", true);
	m_pTopSplitter->setChildrenCollapsible(false);

	// 素材库：仅视频/音频（ResourceWnd）
	// CResourceWnd 构造里已设 DockWidgetMinWidth（缩不动的下限）；默认宽度与之对齐
	auto* pMediaLib = new CResourceWnd(this, this);
	pMediaLib->InitResWndAndModelBeforeLoadPrj();
	pMediaLib->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	const int mediaLibW = pMediaLib->minimumWidth();

	auto* pPreviewPanel = new QWidget;
	pPreviewPanel->setMinimumSize(160, 120);
	pPreviewPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	pPreviewPanel->setAutoFillBackground(true);
	{
		QPalette pal = pPreviewPanel->palette();
		const QColor bg(13, 13, 15);
		pal.setColor(QPalette::Window, bg);
		pal.setColor(QPalette::Base, bg);
		pPreviewPanel->setPalette(pal);
	}
	auto* pPreviewLayout = new QVBoxLayout(pPreviewPanel);
	pPreviewLayout->setContentsMargins(0, 0, 0, 0);
	pPreviewLayout->setSpacing(0);

	auto* pPreview = new CMediaPreviewWnd(pPreviewPanel);
	pPreview->setMinimumSize(160, 90);
	pPreview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	auto* pPlayBar = new CPreviewPlayBar(pPreviewPanel);
	pPreviewLayout->addWidget(pPreview, 1);
	pPreviewLayout->addWidget(pPlayBar);

	// 属性区：媒体属性 / 时间线属性 按选中切换
	auto* pProps = new CTimelinePropertyHost;
	pProps->setMinimumWidth(260);
	pProps->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

	m_pTopSplitter->addWidget(pMediaLib);
	m_pTopSplitter->addWidget(pPreviewPanel);
	m_pTopSplitter->addWidget(pProps);
	m_pTopSplitter->setStretchFactor(0, 1);
	m_pTopSplitter->setStretchFactor(1, 3);
	m_pTopSplitter->setStretchFactor(2, 1);
	m_pTopSplitter->setSizes({ mediaLibW, 780, 340 });

	connect(pMediaLib, &CResourceWnd::signalPreviewMedia, pPreview, &CMediaPreviewWnd::OpenFile);
	connect(g_pGSNetSignal, &CGSNetSignalCenter::signalModifyMediaVolume, pPreview,
		[pPreview](const QUuid& mediaId) {
			if (auto node = g_pMediaModel->GetMediaNode(mediaId).lock())
				pPreview->SetVolume(node->GetVolumn());
		});
	connect(m_pRootSplitter, &QSplitter::splitterMoved, pPreview, &CMediaPreviewWnd::RefreshSurface);
	connect(m_pTopSplitter, &QSplitter::splitterMoved, pPreview, &CMediaPreviewWnd::RefreshSurface);

	// 完整迁移的 TimeLineWnd（CDockWidgetEx）
	auto* pTimeLine = new CTimeLineWnd(this);
	pTimeLine->setMinimumHeight(180);
	pTimeLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	// 时间线播控 → 预览窗（Play/Pause/Seek/游标）
	auto* pPreviewBridge = new CTimelinePreviewBridge(pPreview, this);
	connect(g_pPlayerController, &CPlayerController::signalPlayStateChanged,
		pPreviewBridge, &CTimelinePreviewBridge::OnPlayStateChanged);
	connect(g_pSignalCenter, &CSignalCenter::signalNoniusTimeChanged,
		pPreviewBridge, &CTimelinePreviewBridge::OnNoniusTimeChanged);

	m_pRootSplitter->addWidget(m_pTopSplitter);
	m_pRootSplitter->addWidget(pTimeLine);
	m_pRootSplitter->setStretchFactor(0, 3);
	m_pRootSplitter->setStretchFactor(1, 2);
	m_pRootSplitter->setSizes({ 560, 340 });
	setCentralWidget(m_pRootSplitter);

	// 分割条拖动：DX 预览(WA_PaintOnScreen)会导致邻接区域残留，强制整窗刷新
	connect(m_pRootSplitter, &QSplitter::splitterMoved, this, [this](int, int) { ForceUiRefresh(); });
	connect(m_pTopSplitter, &QSplitter::splitterMoved, this, [this](int, int) { ForceUiRefresh(); });

	// 启动后补一次清屏与邻接刷新，消除初始白底/边缘残留
	QTimer::singleShot(0, this, [this, pPreview]() {
		if (pPreview)
			pPreview->RefreshSurface();
		ForceUiRefresh();
	});
}

void CMainWindow::showEvent(QShowEvent* event)
{
	QMainWindow::showEvent(event);
	ApplyWindowChrome();
}

void CMainWindow::ApplyWindowChrome()
{
#ifdef Q_OS_WIN
	const HWND hwnd = reinterpret_cast<HWND>(winId());
	if (!hwnd)
		return;

	BOOL dark = TRUE;
	if (FAILED(DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark))))
		DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1, &dark, sizeof(dark));

	// Win11：标题栏纯黑 + 浅色文字
	const COLORREF caption = RGB(0, 0, 0);
	const COLORREF text = RGB(235, 235, 235);
	DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &caption, sizeof(caption));
	DwmSetWindowAttribute(hwnd, DWMWA_TEXT_COLOR, &text, sizeof(text));
#else
	Q_UNUSED(this);
#endif
}

void CMainWindow::resizeEvent(QResizeEvent* event)
{
	QMainWindow::resizeEvent(event);
	ForceUiRefresh();
}

void CMainWindow::ForceUiRefresh()
{
	// 延后一帧：等 layout 完成后再刷，避免拉伸时残影
	QTimer::singleShot(0, this, [this]() {
		if (centralWidget())
		{
			centralWidget()->update();
			const auto kids = centralWidget()->findChildren<QWidget*>();
			for (QWidget* w : kids)
			{
				if (!w || w->testAttribute(Qt::WA_PaintOnScreen))
					continue;
				w->update();
			}
		}
		update();
	});
}
