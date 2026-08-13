#include "ResourceWnd.h"

#include "UICommon/BaseWidget.h"
#include "ResContentWidget.h"
#include "Common/CommonDefine.h"
#include "UICommon/UITranslate.h"
#include "UICommon/UICommonSizeDefine.h"
#include <QEvent>
#include <QDebug>
#include "Model/ResourceModel/ResourceModel.h"
#include "Common/SignalCenter/SignalCenterExport.h"
#include "ResWndUISizeDefine.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CResourceWnd::CResourceWnd( QWidget *parent, QWidget* pMainWnd) : CDockWidgetEx(parent)
{
	m_nWidgetType = EDWT_ResourceWnd;

    m_pContentWidget = new CResContentWidget(this, pMainWnd);
	setWidget(m_pContentWidget);

	CreateConnects();
	UpdateTranslateText();

	setMinimumSize(DockWidgetMinWidth, DockWidgetMinHeight);
}

CResourceWnd::~CResourceWnd()
{
	qDebug() << "~CResourceWnd";
}

void CResourceWnd::InitWndAfterLoadPrj(bool bNewPrj)
{
	if (!bNewPrj)
	{
		InitResourceWndAfterLoadPrj();
	}
}

void CResourceWnd::InitResourceWndAfterLoadPrj()
{
	m_pContentWidget->InitResourceWndAfterLoadPrj();
}

void CResourceWnd::CreateConnects()
{
	connect(g_pSignalCenter, &CSignalCenter::signalRefreshWindow, this, &CResourceWnd::slotRefreshWindow);

	connect(m_pContentWidget, &CResContentWidget::signalMutedByAudioCapture, this, &CResourceWnd::signalMutedByAudioCapture);
	connect(m_pContentWidget, &CResContentWidget::signalPrjDemuxerFinish, this, &CResourceWnd::signalPrjDemuxerFinish);
	connect(m_pContentWidget, &CResContentWidget::signalPictureTrans, this, &CResourceWnd::signalPictureTrans); 
	connect(m_pContentWidget, &CResContentWidget::signalResDemuxerProgress, this, &CResourceWnd::signalResDemuxerProgress);
	connect(m_pContentWidget, &CResContentWidget::signalResFileLostRecoveryState, this, &CResourceWnd::signalResFileLostRecoveryState);
	
	connect(g_pSignalCenter, &CSignalCenter::signalLocateResNode, m_pContentWidget, &CResContentWidget::slotMediaResLocate);
	
	connect(this, &CResourceWnd::signalToolsWndCreatMedia, m_pContentWidget, &CResContentWidget::signalToolsWndCreatMedia);
	connect(m_pContentWidget, &CResContentWidget::signalToolsWndAddMedia, this, &CResourceWnd::signalToolsWndAddMedia);
	connect(m_pContentWidget, &CResContentWidget::signalToolsWndAddMedia, this, [&]() {
		SetDockWidgetShowState(true,true);
		});

	connect(m_pContentWidget, &CResContentWidget::signalUpdateColorTextResWnd, this, &CResourceWnd::signalUpdateColorTextResWnd);

	connect(m_pContentWidget, &CResContentWidget::signalUpdateResToMedia, this, &CResourceWnd::signalUpdateResToMedia);
	connect(m_pContentWidget, &CResContentWidget::signalPreviewMedia, this, &CResourceWnd::signalPreviewMedia);
}

void CResourceWnd::InitResWndAndModelBeforeLoadPrj()
{
	m_pContentWidget->InitResWndAndModelBeforeLoadPrj();
}

void CResourceWnd::UpdateTranslateText()
{
	SetDockTitle(CResourceTranslate::ResourceWnd());
}

void CResourceWnd::slotRefreshWindow(int eWindowType, int eOperatorType)
{
	if (eWindowType&SignalCenter::EWT_ResourceWnd)
	{
	}
}