#include "TimelinePropertyHost.h"
#include "MediaPropertyFrame.h"
#include "TimeLinePropertyFrame.h"
#include "TimeLineTransFxPropertyFrame.h"

#include "Controll/SelectController/SelectControllerExport.h"
#include "Model/MediaModel/MediaModelExport.h"
#include "Common/SignalCenter/SignalCenterExport.h"
#include "UICommon/UICommonSizeDefine.h"

#include <QVBoxLayout>
#include <QScrollBar>
#include <QPalette>
#include <QVariant>
#include <QFrame>

CTimelinePropertyHost::CTimelinePropertyHost(QWidget* parent)
	: QScrollArea(parent)
{
	setProperty("DockWidget", QVariant(true));
	setProperty("TimelinePropertyHost", QVariant(true));
	setWidgetResizable(true);
	setFrameShape(QFrame::NoFrame);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	if (verticalScrollBar())
		verticalScrollBar()->setProperty("ResourceScrollBar", QVariant(true));

	// 暗色底：避免 AttributeName/BoldName 浅色字贴在浅底上
	const QColor bg(31, 33, 36);
	QPalette pal = palette();
	pal.setColor(QPalette::Window, bg);
	pal.setColor(QPalette::Base, bg);
	setPalette(pal);
	setAutoFillBackground(true);
	if (viewport())
	{
		viewport()->setPalette(pal);
		viewport()->setAutoFillBackground(true);
	}

	auto* pContent = new QWidget(this);
	pContent->setProperty("TransparentWnd", QVariant(true));
	pContent->setAutoFillBackground(false);
	pContent->setPalette(pal);

	m_pMediaFrame = new CMediaPropertyFrame(pContent);
	m_pTimeLineFrame = new CTimeLinePropertyFrame(pContent);
	m_pTransFxFrame = new CTimeLineTransFxPropertyFrame(pContent);
	m_pMediaFrame->setAttribute(Qt::WA_StyledBackground, true);
	m_pTimeLineFrame->setAttribute(Qt::WA_StyledBackground, true);
	m_pTransFxFrame->setAttribute(Qt::WA_StyledBackground, true);
	m_pMediaFrame->setAutoFillBackground(false);
	m_pTimeLineFrame->setAutoFillBackground(false);
	m_pTransFxFrame->setAutoFillBackground(false);

	auto* pLayout = new QVBoxLayout(pContent);
	pLayout->setContentsMargins(nSizeDefine_16, nSizeDefine_15, nSizeDefine_16, 0);
	pLayout->setSpacing(0);
	pLayout->addWidget(m_pMediaFrame);
	pLayout->addWidget(m_pTimeLineFrame);
	pLayout->addWidget(m_pTransFxFrame);
	pLayout->addSpacing(nSizeDefine_72);
	pLayout->addStretch();

	setWidget(pContent);

	connect(g_pSelectController, &CSelectController::signalSelectionChanged,
		this, &CTimelinePropertyHost::RefreshBySelection);
	connect(g_pSignalCenter, &CSignalCenter::signalRefreshWindow, this,
		[this](int eWindowType, int) {
			if (eWindowType & SignalCenter::EWT_Property)
				RefreshBySelection();
		});
	connect(g_pSignalCenter, &CSignalCenter::signalTransFxSelectChanged, this,
		[this](QUuid& id) {
			if (m_pTransFxFrame)
				m_pTransFxFrame->SetTransFxID(id);
			RefreshBySelection();
		});

	// 默认：未选中媒体 → 时间线属性
	ShowFrame(EFrameKind::TimeLine);
	m_pTimeLineFrame->UpdateProperty();
}

void CTimelinePropertyHost::RefreshBySelection()
{
	// 优先级：转场 > 媒体片段 > 时间线
	if (g_pSelectController && g_pSelectController->HasTransFxSelected())
	{
		const auto ids = g_pSelectController->GetTransFxSelectIds();
		if (!ids.empty() && m_pTransFxFrame)
		{
			m_pTransFxFrame->SetTransFxID(*ids.begin());
			ShowFrame(EFrameKind::TransFx);
			m_pTransFxFrame->UpdateProperty();
			return;
		}
	}

	bool bShowMedia = false;
	if (g_pSelectController && g_pSelectController->HasTimeLineObjectSelected() && g_pMediaModel)
	{
		for (const auto& id : g_pSelectController->GetTLObjectSelectIds())
		{
			if (g_pMediaModel->GetMediaNode(id).lock())
			{
				bShowMedia = true;
				break;
			}
		}
	}

	if (bShowMedia)
	{
		ShowFrame(EFrameKind::Media);
		m_pMediaFrame->UpdateProperty();
	}
	else
	{
		ShowFrame(EFrameKind::TimeLine);
		m_pTimeLineFrame->UpdateProperty();
	}
}

void CTimelinePropertyHost::ShowFrame(EFrameKind kind)
{
	m_curFrame = kind;
	if (m_pMediaFrame)
		m_pMediaFrame->setVisible(kind == EFrameKind::Media);
	if (m_pTimeLineFrame)
		m_pTimeLineFrame->setVisible(kind == EFrameKind::TimeLine);
	if (m_pTransFxFrame)
		m_pTransFxFrame->setVisible(kind == EFrameKind::TransFx);
}
