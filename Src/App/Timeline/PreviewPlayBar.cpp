#include "PreviewPlayBar.h"

#include "UICommon/SvgImageButton.h"
#include "UICommon/UICommonSizeDefine.h"
#include "UI/TimeLineWnd/ContentViewData.h"
#include "Controll/PlayerController/PlayerControllerExport.h"
#include "Controll/PlayCommandCenter/PlayCommandCenterExport.h"
#include "Controll/PlayStateCenter/PlayStateCenterExport.h"
#include "Common/CommonDefine.h"

#include <QHBoxLayout>

namespace
{
	const QString kPlayIconPath = QStringLiteral(":/dark/images/TimeLineWnd/PlayControl/Play");
	const QString kPauseIconPath = QStringLiteral(":/dark/images/TimeLineWnd/PlayControl/Pause");
}

CPreviewPlayBar::CPreviewPlayBar(QWidget* parent)
	: QWidget(parent)
{
	setFixedHeight(SysDpiZoom(36));
	setStyleSheet(QStringLiteral("background-color: #1a1a1c;"));
	InitWidget();
	CreateConnects();
	SyncPlayState();
}

void CPreviewPlayBar::InitWidget()
{
	m_pBtnPlayPause = new CSvgImageButton(kPlayIconPath, this, false);
	m_pBtnPlayPause->setFixedSize(UICommon_ImageBtnSize);
	m_pBtnPlayPause->setProperty("transparentBkBtn", true);
	m_pBtnPlayPause->setToolTip(QStringLiteral("Play"));
	m_bShowingPauseIcon = false;

	auto* pLayout = new QHBoxLayout(this);
	pLayout->setContentsMargins(SysDpiZoom(8), SysDpiZoom(4), SysDpiZoom(8), SysDpiZoom(4));
	pLayout->setSpacing(SysDpiZoom(8));
	pLayout->addStretch(1);
	pLayout->addWidget(m_pBtnPlayPause);
	pLayout->addStretch(1);
}

void CPreviewPlayBar::CreateConnects()
{
	connect(m_pBtnPlayPause, &QPushButton::clicked, this, &CPreviewPlayBar::OnPlayPauseClicked);
	if (g_pPlayerController)
	{
		connect(g_pPlayerController, &CPlayerController::signalPlayStateChanged,
			this, &CPreviewPlayBar::SyncPlayState);
	}
}

void CPreviewPlayBar::OnPlayPauseClicked()
{
	if (!g_pPlayerController || !g_pContentViewData)
		return;
	if (g_pContentViewData->IsTimelineRecordLocked())
		return;

	SCellPlayState state;
	state.idCell = g_pContentViewData->GetTimeLineId();
	state.seek = g_pContentViewData->GetNoniusTime().count();

	if (g_pPlayerController->IsPlaying())
	{
		PlayCommandCenter::OnControllerPause(state);
	}
	else
	{
		g_pPlayerController->SetPlayRangeMode(TimelinePlayRangeMode_Normal);
		PlayCommandCenter::OnControllerPlay(state);
	}
	SyncPlayState();
}

void CPreviewPlayBar::SyncPlayState()
{
	if (!m_pBtnPlayPause)
		return;

	const bool bPlaying = g_pPlayerController && g_pPlayerController->IsPlaying();
	if (bPlaying == m_bShowingPauseIcon)
		return;

	m_bShowingPauseIcon = bPlaying;
	m_pBtnPlayPause->SetImagePath(bPlaying ? kPauseIconPath : kPlayIconPath);
	m_pBtnPlayPause->setToolTip(bPlaying ? QStringLiteral("Pause") : QStringLiteral("Play"));
	m_pBtnPlayPause->update();
}
