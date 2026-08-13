#include "TimeLineTransFxPropertyFrame.h"

#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "Model/GlobalModel/GlobalModelExport.h"
#include "Model/GlobalModel/PrjSettingData.h"
#include "Common/SignalCenter/SignalCenterExport.h"
#include "Controll/GlobalHistoryStack/GlobalHistoryStackExport.h"

#include "UICommon/TimeEditWidget.h"
#include "UICommon/ComboBoxWidget.h"
#include "UICommon/UITranslate.h"
#include "UICommon/UICommonSizeDefine.h"
#include "Common/Common/TransationTypeDefine.h"
#include "PropertyTimeHelper.h"

#include <QVBoxLayout>
#include <QFrame>

#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif

namespace
{
	QString TransTypeDisplayName(int type)
	{
		switch (type)
		{
		case ETFT_Fade: return CPropertyTranslate::ETFT_Fade();
		case ETFT_LeftMove: return CPropertyTranslate::ETFT_LeftMove();
		case ETFT_RightMove: return CPropertyTranslate::ETFT_RightMove();
		case ETFT_UpMove: return CPropertyTranslate::ETFT_UpMove();
		case ETFT_DownMove: return CPropertyTranslate::ETFT_DownMove();
		case ETFT_LeftTopMove: return CPropertyTranslate::ETFT_LeftTopMove();
		case ETFT_LeftBottomMove: return CPropertyTranslate::ETFT_LeftBottomMove();
		case ETFT_RightTopMove: return CPropertyTranslate::ETFT_RightTopMove();
		case ETFT_RightBottomMove: return CPropertyTranslate::ETFT_RightBottomMove();
		case ETFT_LeftCover: return CPropertyTranslate::ETFT_LeftCover();
		case ETFT_TopCover: return CPropertyTranslate::ETFT_TopCover();
		case ETFT_RightCover: return CPropertyTranslate::ETFT_RightCover();
		case ETFT_BottomCover: return CPropertyTranslate::ETFT_BottomCover();
		case ETFT_LeftTopCover: return CPropertyTranslate::ETFT_LeftTopCover();
		case ETFT_RightTopCover: return CPropertyTranslate::ETFT_RightTopCover();
		case ETFT_LeftBottomCover: return CPropertyTranslate::ETFT_LeftBottomCover();
		case ETFT_RightBottomCover: return CPropertyTranslate::ETFT_RightBottomCover();
		case ETFT_LeftTopObliqueCover: return CPropertyTranslate::ETFT_LeftTopObliqueCover();
		case ETFT_RightTopObliqueCover: return CPropertyTranslate::ETFT_RightTopObliqueCover();
		case ETFT_LeftBottomObliqueCover: return CPropertyTranslate::ETFT_LeftBottomObliqueCover();
		case ETFT_RightBottomObliqueCover: return CPropertyTranslate::ETFT_RightBottomObliqueCover();
		case ETFT_GrillesHorizontal: return CPropertyTranslate::ETFT_GrillesHorizontal();
		case ETFT_GrillesVertical: return CPropertyTranslate::ETFT_GrillesVertical();
		case ETFT_LeftRightOpen: return CPropertyTranslate::ETFT_LeftRightOpen();
		case ETFT_TopBottomOpen: return CPropertyTranslate::ETFT_TopBottomOpen();
		case ETFT_LeftRightClose: return CPropertyTranslate::ETFT_LeftRightClose();
		case ETFT_TopBottomClose: return CPropertyTranslate::ETFT_TopBottomClose();
		case ETFT_MosaicLarge: return CPropertyTranslate::ETFT_MosaicLarge();
		case ETFT_MosaicMiddle: return CPropertyTranslate::ETFT_MosaicMiddle();
		case ETFT_MosaicSmall: return CPropertyTranslate::ETFT_MosaicSmall();
		case ETFT_RightRotFull: return CPropertyTranslate::ETFT_RightRotFull();
		case ETFT_RightRotHalf: return CPropertyTranslate::ETFT_RightRotHalf();
		case ETFT_RightRotQuarter: return CPropertyTranslate::ETFT_RightRotQuarter();
		case ETFT_LeftRotFull: return CPropertyTranslate::ETFT_LeftRotFull();
		case ETFT_LeftRotHalf: return CPropertyTranslate::ETFT_LeftRotHalf();
		case ETFT_LeftRotQuarter: return CPropertyTranslate::ETFT_LeftRotQuarter();
		case ETFT_SmallToLargeCenter: return CPropertyTranslate::ETFT_SmallToLargeCenter();
		case ETFT_SmallToLargeLeftTop: return CPropertyTranslate::ETFT_SmallToLargeLeftTop();
		case ETFT_SmallToLargeLeftBottom: return CPropertyTranslate::ETFT_SmallToLargeLeftBottom();
		case ETFT_SmallToLargeRightTop: return CPropertyTranslate::ETFT_SmallToLargeRightTop();
		case ETFT_SmallToLargeRightBottom: return CPropertyTranslate::ETFT_SmallToLargeRightBottom();
		case ETFT_CenterToBorderRect: return CPropertyTranslate::ETFT_CenterToBorderRect();
		case ETFT_BorderToCenterRect: return CPropertyTranslate::ETFT_BorderToCenterRect();
		case ETFT_CenterToBorderLozenge: return CPropertyTranslate::ETFT_CenterToBorderLozenge();
		case ETFT_BorderToCenterLozenge: return CPropertyTranslate::ETFT_BorderToCenterLozenge();
		case ETFT_CenterToBorderCross: return CPropertyTranslate::ETFT_CenterToBorderCross();
		case ETFT_BorderToCenterCross: return CPropertyTranslate::ETFT_BorderToCenterCross();
		case ETFT_PerspectiveRotationLeft: return CPropertyTranslate::ETFT_PerspectiveRotationLeft();
		case ETFT_PerspectiveRotationRight: return CPropertyTranslate::ETFT_PerspectiveRotationRight();
		case ETFT_PerspectiveRotationTop: return CPropertyTranslate::ETFT_PerspectiveRotationTop();
		case ETFT_PerspectiveRotationBottom: return CPropertyTranslate::ETFT_PerspectiveRotationBottom();
		default: return QStringLiteral("TransFx %1").arg(type);
		}
	}
}

CTimeLineTransFxPropertyFrame::CTimeLineTransFxPropertyFrame(QWidget* pParent)
	: CPropertyFrame(pParent)
{
	InitWidget();
	RefreshTransTypeCombo();
	CreateConnects();
	UpdateTranslateText();
}

CTimeLineTransFxPropertyFrame::~CTimeLineTransFxPropertyFrame()
{
}

void CTimeLineTransFxPropertyFrame::InitWidget()
{
	m_pTransFxFrame = new QFrame(this);
	m_pTransFxFrame->setObjectName("TransFxPropertyFrame");

	QVBoxLayout* pMainLayout = new QVBoxLayout(m_pTransFxFrame);
	pMainLayout->setContentsMargins(10, 10, 10, 10);
	pMainLayout->setSpacing(8);

	m_pComboTransType = new CComboBoxWidget(m_pTransFxFrame);
	m_pComboTransType->setMinimumWidth(SysDpiZoom(258));
	m_pComboTransType->SetLabelConfig(SysDpiZoom(78), Qt::AlignLeft | Qt::AlignVCenter, nSizeDefine_8);
	m_pComboTransType->SetLabelProperty("AttributeName", true);
	m_pComboTransType->SetComboTextAlignment(Qt::AlignCenter);
	pMainLayout->addWidget(m_pComboTransType);

	m_pDurationTimeEdit = new CTimeEditWidget(m_pTransFxFrame);
	m_pDurationTimeEdit->setMinimumWidth(SysDpiZoom(258));
	m_pDurationTimeEdit->SetLabelConfig(SysDpiZoom(78), Qt::AlignLeft | Qt::AlignVCenter, nSizeDefine_8);
	m_pDurationTimeEdit->SetLabelProperty("AttributeName", true);
	m_pDurationTimeEdit->SetEditConfig(Qt::AlignCenter);
	m_pDurationTimeEdit->SetFps(g_pProjectSetting->LoadTimeLineFps());
	m_pDurationTimeEdit->SetFrameEdit(true);
	pMainLayout->addWidget(m_pDurationTimeEdit);

	QVBoxLayout* pThisLayout = qobject_cast<QVBoxLayout*>(layout());
	if (!pThisLayout)
	{
		pThisLayout = new QVBoxLayout(this);
		setLayout(pThisLayout);
	}
	pThisLayout->addWidget(m_pTransFxFrame);
	pThisLayout->addStretch();
}

void CTimeLineTransFxPropertyFrame::CreateConnects()
{
	connect(m_pComboTransType, &CComboBoxWidget::signalSelectChange, this, &CTimeLineTransFxPropertyFrame::slotTransTypeChange);
	connect(m_pDurationTimeEdit, &CTimeEditWidget::signalTimeEditFinish, this, &CTimeLineTransFxPropertyFrame::slotDurationFinish);
}

void CTimeLineTransFxPropertyFrame::RefreshTransTypeCombo()
{
	m_pComboTransType->Clear();
	// 阶段4：开放 Fade + 全部顶点转场
	for (int t = ETFT_Fade; t < ETFT_Count; ++t)
		m_pComboTransType->AddItem(TransTypeDisplayName(t));
}

int CTimeLineTransFxPropertyFrame::TransTypeToComboIndex(int transType) const
{
	if (transType < ETFT_Fade || transType >= ETFT_Count)
		return -1;
	return transType - ETFT_Fade;
}

int CTimeLineTransFxPropertyFrame::ComboIndexToTransType(int index) const
{
	return ETFT_Fade + index;
}

void CTimeLineTransFxPropertyFrame::UpdateProperty()
{
	auto pTransFx = g_pTimeLineModel->GetTransFx(m_transFxID).lock();
	if (!pTransFx)
	{
		ClearEdit();
		return;
	}

	const int comboIndex = TransTypeToComboIndex(pTransFx->GetTransType());
	if (comboIndex >= 0)
		m_pComboTransType->SetSelectIndex(comboIndex);
	PropertyTimeHelper::DisplayTimelineMs(m_pDurationTimeEdit, pTransFx->GetDuration());

	bool bLock = false;
	if (auto pLayer = g_pTimeLineModel->GetLayer(pTransFx->GetLeftObjectId()).lock())
		bLock = pLayer->IsLock();
	if (bLock)
		m_pTransFxFrame->setEnabled(false);
	else
		EnableEdit();
}

void CTimeLineTransFxPropertyFrame::UpdateTranslateText()
{
	m_pComboTransType->SetName(CCommonTranslate::Type());
	m_pDurationTimeEdit->SetName(CCommonTranslate::Duration());
}

void CTimeLineTransFxPropertyFrame::ClearEdit()
{
	m_pComboTransType->SetSelectIndex(-1);
	PropertyTimeHelper::DisplayTimelineMs(m_pDurationTimeEdit, std::chrono::milliseconds(0));
	m_pTransFxFrame->setEnabled(false);
}

void CTimeLineTransFxPropertyFrame::EnableEdit()
{
	m_pTransFxFrame->setEnabled(true);
}

void CTimeLineTransFxPropertyFrame::slotTransTypeChange(int index)
{
	auto pTransFx = g_pTimeLineModel->GetTransFx(m_transFxID).lock();
	if (!pTransFx || index < 0)
		return;

	const int newType = ComboIndexToTransType(index);
	if (pTransFx->GetTransType() == newType)
		return;

	pTransFx->SetTransType(newType);
	HistoryStackSave();
	UpdateProperty();
	emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_TransFxDataChange);
	emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_TimeLineWnd, (1 << 13));
	emit g_pSignalCenter->signalUpdateMediaProperty(pTransFx->GetLeftObjectId());
	emit g_pSignalCenter->signalUpdateMediaProperty(pTransFx->GetRightObjectId());
}

void CTimeLineTransFxPropertyFrame::slotDurationFinish()
{
	auto pTransFx = g_pTimeLineModel->GetTransFx(m_transFxID).lock();
	if (!pTransFx)
		return;

	auto pLeftObj = g_pTimeLineModel->GetTimeLineObject(pTransFx->GetLeftObjectId()).lock();
	auto pRightObj = g_pTimeLineModel->GetTimeLineObject(pTransFx->GetRightObjectId()).lock();
	if (!pLeftObj || !pRightObj)
		return;

	std::chrono::milliseconds newDuration = PropertyTimeHelper::CommitTimelineMs(m_pDurationTimeEdit);
	if (newDuration == pTransFx->GetDuration())
		return;

	if (newDuration < std::chrono::milliseconds(1))
		newDuration = std::chrono::milliseconds(1);

	const auto leftStart = pLeftObj->GetStartTime();
	const auto endLeft = leftStart + pLeftObj->GetDuration();
	const auto maxDuration = endLeft - leftStart;
	if (newDuration > maxDuration)
		newDuration = maxDuration;

	auto newRightStart = endLeft - newDuration;
	if (newRightStart < leftStart)
		newRightStart = leftStart;

	const auto endRight = newRightStart + pRightObj->GetDuration();
	const auto overlapStart = std::max(leftStart, newRightStart);
	const auto overlapEnd = std::min(endLeft, endRight);
	const auto actualDuration = (overlapEnd > overlapStart)
		? (overlapEnd - overlapStart) : std::chrono::milliseconds(0);
	if (actualDuration.count() <= 0)
		return;

	pRightObj->SetStartTime(newRightStart);
	pTransFx->SetDuration(actualDuration);
	PropertyTimeHelper::DisplayTimelineMs(m_pDurationTimeEdit, actualDuration);
	HistoryStackSave();
	UpdateProperty();
	emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_TransFxDataChange);
	emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_TimeLineWnd, (1 << 13));
}
