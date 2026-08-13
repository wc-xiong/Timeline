#include "TimeLinePropertyFrame.h"

#include "Common/LanguageDefine.h"
#include "Common/Common/CommonFunctionMath.h"

#include "UICommon/UITranslate.h"
#include "UICommon/ComboBoxWidget.h"
#include "UICommon/UICommonSizeDefine.h"
#include "UICommon/TimeEditWidget.h"
#include "UICommon/PushButtonEx.h"

#include "Model/ModelModel/ModelModelExport.h"
#include "Model/GlobalModel/GlobalModelExport.h"
#include "Model/GlobalModel/PrjSettingData.h"
#include "Model/MediaModel/MediaModelExport.h"
#include "Model/ResourceModel/ResourceModelExport.h"
#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "PropertyTimeHelper.h"

#include "Controll/ViewDataManager/ViewDataManagerExport.h"
#include "Controll/SelectController/SelectControllerExport.h"
#include "Controll/ViewDataManager/ViewDataManagerExport.h"
#include "Common/SignalCenter/SignalCenterExport.h"
#include "Controll/GlobalHistoryStack/GlobalHistoryStackExport.h"
#include "Controll/PlayerController/PlayerControllerExport.h"

#include <QDebug>
#include <QBoxLayout>
#include<vector>


//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CTimeLinePropertyFrame::CTimeLinePropertyFrame(QWidget* pParent /*=nullptr*/)
	: CPropertyFrame(pParent)
{
	InitWidget();
	CreateConnects();
	UpdateTranslateText();
	UpdateProperty();
}

CTimeLinePropertyFrame::~CTimeLinePropertyFrame()
{

}

void CTimeLinePropertyFrame::UpdateProperty()
{
	m_pPlayDuration->SetFrameEdit(true);

	PropertyTimeHelper::DisplayTimelineMs(m_pPlayDuration, g_pTimeLineModel->GetDuration());
	m_pPlayDuration->SetMinTime(std::max(g_pTimeLineModel->GetMaxUsefulDuration().count(), 10000LL));

	m_pEndOperateType->SetSelectIndex(g_pTimeLineModel->GetPlayEndActionType());
}

void CTimeLinePropertyFrame::InitWidget()
{
	m_pPlayDuration = new CTimeEditWidget(this);
	m_pPlayDuration->SetLabelConfig(SysDpiZoom(78), Qt::AlignLeft | Qt::AlignVCenter, nSizeDefine_8);
	m_pPlayDuration->SetLabelProperty("BoldName", true);
	m_pPlayDuration->SetEditConfig(Qt::AlignCenter);
	m_pPlayDuration->SetFps(g_pProjectSetting->LoadTimeLineFps());

	m_pEndOperateType = new CComboBoxWidget(this);
	m_pEndOperateType->SetLabelConfig(SysDpiZoom(78), Qt::AlignLeft | Qt::AlignVCenter, nSizeDefine_8);
	m_pEndOperateType->SetLabelProperty("BoldName", true);
	m_pEndOperateType->SetLimitLabelWidth(true);
	m_pEndOperateType->SetComboTextAlignment(Qt::AlignCenter);

	m_pDeleteTail = new CPushButton(this);
	m_pDeleteTail->setFixedHeight(UICommon_CtrlHeight);
	m_pDeleteTail->setMinimumWidth(SysDpiZoom(192));

	QHBoxLayout* p = new QHBoxLayout;
	p->setMargin(0);
	p->setSpacing(0);
	p->addSpacing(SysDpiZoom(88));
	p->addWidget(m_pDeleteTail);

	QVBoxLayout* pLayout = new QVBoxLayout(this);
	pLayout->setMargin(0);
	pLayout->setSpacing(0);
	pLayout->addWidget(m_pPlayDuration);
	pLayout->addSpacing(nSizeDefine_8);
	pLayout->addWidget(m_pEndOperateType);
	pLayout->addSpacing(nSizeDefine_8);
	pLayout->addLayout(p);
}

void CTimeLinePropertyFrame::CreateConnects()
{
	connect(m_pPlayDuration, &CTimeEditWidget::signalTimeEditFinish, this, [&]() {
		const auto duration = PropertyTimeHelper::CommitTimelineMs(m_pPlayDuration);
		g_pTimeLineModel->SetDuration(duration);
		PropertyTimeHelper::DisplayTimelineMs(m_pPlayDuration, g_pTimeLineModel->GetDuration());
		emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_TimeLineWnd, SignalCenter::EDDT_All);

		HistoryStackSave();
	});

	connect(m_pEndOperateType, &CComboBoxWidget::signalSelectChange, this, [&](int nIndex) {
		g_pTimeLineModel->SetPlayEndActionType(nIndex);

		HistoryStackSave();
	});

	connect(m_pDeleteTail, &CPushButton::clicked, this, [&]() {
		const auto curTime = g_pPlayerController->GetCurTime();
		g_pTimeLineModel->SetMaxUsefulDuration();
		const auto newDuration = g_pTimeLineModel->GetDuration();
		// 游标落在被裁掉的无素材尾段时，Seek 同步时间线游标与播放器（不可用 Stop，否则不更新游标位置）
		if (curTime > newDuration)
			emit g_pSignalCenter->signalSeekTimelineAt(newDuration.count());
		UpdateProperty();
		// EDDT_All → TimeLineWnd 映射为 WndType_All；默认 EDDT_None 会导致 UpdateView(0) 不刷新
		emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_TimeLineWnd, SignalCenter::EDDT_All);

		HistoryStackSave();
	});
}

void CTimeLinePropertyFrame::UpdateTranslateText()
{
	m_pDeleteTail->SetText(CPropertyTranslate::DeleteTail());
	m_pPlayDuration->SetName(CPropertyTranslate::PlayDuration());
	m_pEndOperateType->SetName(CPropertyTranslate::PlayEndAcction());

	QStringList list;
	list << QString(CPropertyTranslate::CycleToPlay()) << QString(CPropertyTranslate::StopToPlay()) << QString(CPropertyTranslate::PauseAtLastFrame());
	m_pEndOperateType->AddItems(list, true);
}
