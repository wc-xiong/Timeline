#include "MediaPropertyFrame.h"

#include "Common/LanguageDefine.h"
#include "Common/Common/CommonFunctionMath.h"
#include "Common/Common/ResourceDefine.h"
#include "Common/Common/MediaCommon.h"
#include "Common/Common/ChannelMap.h"

#include "UICommon/UITranslate.h"
#include "UICommon/LineEditWidget.h"
#include "UICommon/GroupWidgetBox.h"
#include "UICommon/ComboBoxWidget.h"
#include "UICommon/UICommonSizeDefine.h"
#include "UICommon/DoubleSpinBoxWidget.h"
#include "UICommon/TimeEditWidget.h"
#include "UICommon/SliderWidget.h"
#include "UICommon/LogTranslate.h"
#include "UICommon/CheckBoxEx.h"
#include "UICommon/LabelSpinbox.h"
#include "UICommon/PropertyResetBtn.h"
#include "UICommon/PropertyKeyFrameBar.h"
#include "UICommon/SvgImageButton.h"
#include "MediaKeyframeHelper.h"
#include "Model/ModelModel/ModelModelExport.h"
#include "Model/GlobalModel/GlobalModelExport.h"
#include "Model/GlobalModel/LogModel.h"
#include "Model/GlobalModel/PrjSettingData.h"
#include "Model/MediaModel/MediaModelExport.h"
#include "Model/ResourceModel/ResourceModelExport.h"
#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "PropertyTimeHelper.h"

#include "Controll/ViewDataManager/ViewDataManagerExport.h"
#include "Controll/SelectController/SelectControllerExport.h"
#include "Common/SignalCenter/SignalCenterExport.h"
#include "Common/SignalCenter/GSNetSignalCenter.h"
#include "Controll/GlobalHistoryStack/GlobalHistoryStackExport.h"
#include "Controll/PlayerController/PlayerControllerExport.h"
#include "Controll/RenderController/RenderControllerExport.h"
#include "Controll/PlayCommandCenter/PlayCommandCenterExport.h"

#include <QDebug>
#include <QBoxLayout>
#include <QLabel>
#include <QResizeEvent>
#include <QSignalBlocker>
#include <vector>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

namespace
{
	static bool IsShowStyleProperty(int nType)
	{
		return IsSupportFillMode(nType) || IsSupportScroll(nType);
	}

	static bool IsTextLike(int nType)
	{
		return nType == ERT_Text || nType == ERT_Subtitle || nType == ERT_Teleprompter;
	}

	static void EmitModifyTimelineLayerObject(const QUuid& objectId)
	{
		if (!g_pTimeLineModel || !g_pGSNetSignal)
			return;

		auto pLayer = g_pTimeLineModel->GetLayer(objectId).lock();
		if (!pLayer)
			return;

		emit g_pGSNetSignal->signalModifyTimelineLayerObject(
			g_pTimeLineModel->GetId(), pLayer->GetId(), objectId);
	}

	QWidget* WrapRowWithKeyFrame(QWidget* parent, QWidget* pRow, CPropertyKeyFrameBar** ppBar)
	{
		QWidget* w = new QWidget(parent);
		QHBoxLayout* l = new QHBoxLayout(w);
		l->setMargin(0);
		l->setSpacing(0);
		l->addWidget(pRow, 1);
		l->addStretch();
		*ppBar = new CPropertyKeyFrameBar(w);
		l->addWidget(*ppBar);
		return w;
	}

	/** 位置 / 缩放 / 旋转 / 裁剪：暂时不通过属性编辑写入关键帧 */
	// 阶段3 GPU 预览已接 Position/Scale/Rotation/Crop：允许关键帧
	bool IsTemporarilyNoKeyframeProperty(EMediaPropertyId)
	{
		return false;
	}

	/** 缩放组容器：W/H 行与 XYZ 同宽对齐，锁定钮浮层叠在左侧并垂直居中 */
	class CScaleRowsWidget : public QWidget
	{
	public:
		explicit CScaleRowsWidget(QWidget* parent = nullptr)
			: QWidget(parent)
		{
		}

		void SetAspectLockButton(CSvgImageButton* pBtn)
		{
			m_pAspectLockBtn = pBtn;
			if (m_pAspectLockBtn)
				m_pAspectLockBtn->setParent(this);
			UpdateLockButtonGeometry();
		}

	protected:
		void resizeEvent(QResizeEvent* event) override
		{
			QWidget::resizeEvent(event);
			UpdateLockButtonGeometry();
		}

	private:
		void UpdateLockButtonGeometry()
		{
			if (!m_pAspectLockBtn)
				return;
			const int nBtnW = m_pAspectLockBtn->width();
			const int nBtnH = m_pAspectLockBtn->height();
			const int nY = (height() - nBtnH) / 2;
			m_pAspectLockBtn->setGeometry(0, nY, nBtnW, nBtnH);
			m_pAspectLockBtn->raise();
		}

		CSvgImageButton* m_pAspectLockBtn = nullptr;
	};
}

CMediaPropertyFrame::CMediaPropertyFrame(QWidget* pParent /*=nullptr*/)
	: CPropertyFrame(pParent)
{
	InitWidget();
	CreateConnects();
	UpdateTranslateText();
	UpdateProperty();
	CMediaKeyframeHelper::RegisterMediaPropertyFrame(this);
}

CMediaPropertyFrame::~CMediaPropertyFrame()
{
	if (CMediaKeyframeHelper::GetRegisteredMediaPropertyFrame() == this)
		CMediaKeyframeHelper::RegisterMediaPropertyFrame(nullptr);
}

// 选中变化或外部刷新：加载基础/入出点信息，先写默认值再按游标插值，最后更新 enable 状态
void CMediaPropertyFrame::UpdateProperty()
{
	m_pPlayStart->SetFrameEdit(true);
	m_pPlayDuration->SetFrameEdit(true);
	m_pClipBegin->SetFrameEdit(true);
	m_pClipEnd->SetFrameEdit(true);

	m_pMutilMediaPlayStart->SetFrameEdit(true);
	m_pMutilMediaPlayDuration->SetFrameEdit(true);

	m_setMediaIDs = g_pSelectController->GetTLObjectSelectIds();
	if (m_setMediaIDs.size() == 1)
	{
		m_pOneMediaFrame->setVisible(true);
		m_pMutilMediaFrame->setVisible(false);

		EnableEdit();

		bool bLock = false;
		auto pLayer = g_pTimeLineModel->GetLayer(*m_setMediaIDs.begin()).lock();
		if (pLayer)
			bLock = pLayer->IsLock();

		auto pObject = g_pTimeLineModel->GetTimeLineObject(*m_setMediaIDs.begin()).lock();
		if (pObject)
		{
			m_msOldStartTime = pObject->GetStartTime();
			m_msOldDuration = pObject->GetDuration();

			PropertyTimeHelper::DisplayTimelineMs(m_pPlayStart, pObject->GetStartTime());
			PropertyTimeHelper::DisplayTimelineMs(m_pPlayDuration, pObject->GetDuration());

			m_pPlayStart->setEnabled(!bLock);
			m_pPlayDuration->setEnabled(!bLock);
		}

		auto mediaNode = g_pMediaModel->GetMediaNode(*m_setMediaIDs.begin()).lock();
		if (mediaNode)
		{
			auto name = mediaNode->GetMediaName();

			int resType = mediaNode->GetResourceType();
			auto pRes = g_pResourceModel->GetResourceNode(mediaNode->GetResourceId());
			if (pRes)
				resType = pRes->GetType();

			m_pFrameStyleProperty->setVisible(IsShowStyleProperty(resType));
			m_pComboFillType->setVisible(IsSupportFillMode(resType));

			m_pNameLineEditWidget->SetText(name);
			m_pComboFillType->SetSelectIndex(mediaNode->GetBaseProperty()->GetFillType());

			if (pRes)
			{
				m_pClipBegin->SetMinMaxTime(0, mediaNode->GetClipEndTime().count() - 1000);
				m_pClipEnd->SetMinMaxTime(mediaNode->GetClipStartTime().count() + 1000, pRes->GetDuration());
			}

			PropertyTimeHelper::DisplayClipMs(m_pClipBegin, mediaNode->GetClipStartTime());
			PropertyTimeHelper::DisplayClipMs(m_pClipEnd, mediaNode->GetClipEndTime());

			m_pClipBegin->setEnabled(!bLock);
			m_pClipEnd->setEnabled(!bLock);
			m_pComboFillType->setEnabled(!bLock);

			// 1) 默认值打底，避免切换 clip 残留上一素材的 Z/Rot/Crop 等 UI 缓存
			CMediaKeyframeHelper::LoadFxPropertyDisplayDefaults(this, *m_setMediaIDs.begin());

			const auto playheadMs = g_pPlayerController
				? std::optional<std::chrono::milliseconds>(g_pPlayerController->GetCurTime())
				: std::nullopt;
			// 2) 游标在 clip 内时用关键帧插值覆盖显示
			CMediaKeyframeHelper::SyncKeyframeEvaluatedValuesAtPlayhead(this, *m_setMediaIDs.begin(), playheadMs);
			// 3) 图层未锁且游标在 clip 内时才允许编辑滑条/SpinBox/关键帧栏
			SetPropertyActionBarsEnabled(!bLock, playheadMs);

			// 缩放等比例锁定：从媒体 KeepAspectRatio 恢复；锁定且基准无效时用当前 W/H 补基准
			if (m_pScaleAspectLockBtn && mediaNode->GetBaseProperty())
			{
				const bool bKeep = mediaNode->GetBaseProperty()->GetKeepAspectRatio();
				{
					QSignalBlocker blocker(m_pScaleAspectLockBtn);
					m_pScaleAspectLockBtn->setChecked(bKeep);
				}
				if (bKeep)
				{
					const QSizeF szKeep = mediaNode->GetBaseProperty()->GetSizeKeepAspectRatio();
					if (szKeep.width() <= 0.0 || szKeep.height() <= 0.0)
					{
						mediaNode->GetBaseProperty()->SetSizeKeepAspectRatio(
							QSizeF(mediaNode->GetMediaWidth(), mediaNode->GetMediaHeight()));
					}
				}
				m_pScaleAspectLockBtn->setToolTip(bKeep
					? CPropertyTranslate::ScaleAspectUnlock()
					: CPropertyTranslate::ScaleAspectLock());
			}

			m_pFrameVideoProperty->setVisible(resType == ERT_Video);
			m_pFrameVideoProperty->setEnabled(!bLock);

			UpdateGroupVisibility(resType);
			UpdateAudioPropertyUI();

			switch (resType)
			{
			case ERT_Text:
			{
				auto pParam = mediaNode->GetShareProperty()->GetScrollParam();
				if (!pParam)
					break;

				m_pComboTextScrollType->SetSelectIndex(pParam->nScrollType);
				if (pParam->nScrollType == EST_Stop)
				{
					m_pScrollPtyContainer->hide();
					m_pFrameScrollSpeed->setVisible(false);
				}
				else
				{
					m_pScrollPtyContainer->show();
					m_pFrameScrollSpeed->setVisible(true);
				}

				m_pEteBtn->setChecked(pParam->bEndToEnd);
				m_pEndToEndInterval->setEnabled(pParam->bEndToEnd);
				m_pEndToEndInterval->SetValue(pParam->nEndSpace);
				m_pEditTextScrollSpeed->SetValue(pParam->fScrollSpeed);

				if (auto pText = std::dynamic_pointer_cast<ITextResource>(pRes))
				{
					bool bIsAnimationText = pText->IsAnimationText();
					int nAnimationType = pText->GetBackgroundType();

					if (bIsAnimationText || (1 <= nAnimationType && nAnimationType <= 6))
					{
						m_pComboTextScrollType->setEnabled(false);
						pParam->bIsDisableScrollType = !(m_pComboTextScrollType->isEnabled());
						UpdateTurnPageScrollCtrl(false);
						m_pOneMediaFrame->setEnabled(!bLock);
						RefreshPropertyKeyframeToggleStates();
						return;
					}
					m_pComboTextScrollType->setEnabled(true);
				}
			}
			break;
			default:
				break;
			}
		}

		// 图层锁定：禁用属性页全部控件
		m_pOneMediaFrame->setEnabled(!bLock);

		UpdateTurnPageScrollCtrl(false);
		RefreshPropertyKeyframeToggleStates();
	}
	else if (m_setMediaIDs.size() > 1)
	{
		m_pOneMediaFrame->setVisible(false);
		m_pMutilMediaFrame->setVisible(true);
		m_pEffectPropertyGroup->setVisible(false);
		m_pVolumePropertyGroup->setVisible(false);
		m_pPositionPropertyGroup->setVisible(false);
		m_pScalePropertyGroup->setVisible(false);
		m_pRotationPropertyGroup->setVisible(false);
		m_pCropPropertyGroup->setVisible(false);
		m_pFeatherPropertyGroup->setVisible(false);
		m_pKeyingPropertyGroup->setVisible(false);
		m_pAudioPropertyGroup->setVisible(false);

		std::chrono::milliseconds minTime = g_pTimeLineModel->GetDuration();
		std::chrono::milliseconds maxTime = std::chrono::milliseconds(0);
		for (const auto& id : m_setMediaIDs)
		{
			auto pObject = g_pTimeLineModel->GetTimeLineObject(id).lock();
			if (pObject->GetStartTime() < minTime)
				minTime = pObject->GetStartTime();
			if (pObject->GetStartTime() + pObject->GetDuration() > maxTime)
				maxTime = pObject->GetStartTime() + pObject->GetDuration();
		}

		PropertyTimeHelper::DisplayTimelineMs(m_pMutilMediaPlayStart, minTime);
		PropertyTimeHelper::DisplayTimelineMs(m_pMutilMediaPlayDuration, maxTime - minTime);

		bool bAnyLayerLocked = false;
		for (const auto& id : m_setMediaIDs)
		{
			auto pLayer = g_pTimeLineModel->GetLayer(id).lock();
			if (pLayer && pLayer->IsLock())
			{
				bAnyLayerLocked = true;
				break;
			}
		}
		m_pMutilMediaFrame->setEnabled(!bAnyLayerLocked);
	}
	else
	{
		ClearEdit();
	}
}

void CMediaPropertyFrame::UpdateGroupVisibility(int resType)
{
	const bool bTextLike = IsTextLike(resType);
	const bool bVolume = IsSupportVolum(resType);
	const bool bVisualFx = IsHasRealThumb(resType);
	const bool bAudioGroup = IsSupportVolum(resType) || IsNeedShowAudioChannelMap(resType);

	m_pAudioPropertyGroup->setVisible(bAudioGroup);
	m_pVolumePropertyGroup->setVisible(bVolume);

	m_pPositionPropertyGroup->setVisible(bVisualFx);
	m_pScalePropertyGroup->setVisible(bVisualFx);
	m_pRotationPropertyGroup->setVisible(bVisualFx);
	m_pCropPropertyGroup->setVisible(bVisualFx && !bTextLike);
	m_pFeatherPropertyGroup->setVisible(bVisualFx && !bTextLike);
	m_pKeyingPropertyGroup->setVisible(bVisualFx && !bTextLike);
	m_pEffectPropertyGroup->setVisible(bVisualFx);

	const bool bShowEffectRgb = bVisualFx && !bTextLike;
	if (m_pWrapEffectRed)
		m_pWrapEffectRed->setVisible(bShowEffectRgb);
	if (m_pWrapEffectGreen)
		m_pWrapEffectGreen->setVisible(bShowEffectRgb);
	if (m_pWrapEffectBlue)
		m_pWrapEffectBlue->setVisible(bShowEffectRgb);

	m_pFrameVideoProperty->setVisible(resType == ERT_Video);
	UpdateGroupSeparatorVisibility();
}

void CMediaPropertyFrame::UpdateAudioPropertyUI()
{
	if (m_setMediaIDs.size() != 1)
		return;

	auto mediaNode = g_pMediaModel->GetMediaNode(*m_setMediaIDs.begin()).lock();
	if (!mediaNode)
		return;

	auto pShare = mediaNode->GetShareProperty();
	if (!pShare)
		return;

	int nChannels = 0;
	if (auto pRes = g_pResourceModel->GetResourceNode(mediaNode->GetResourceId()))
		nChannels = pRes->GetAudioChannels();

	int resType = mediaNode->GetResourceType();
	if (auto pRes = g_pResourceModel->GetResourceNode(mediaNode->GetResourceId()))
		resType = pRes->GetType();

	// 音频格式始终以当前素材资源声道数为准（勿沿用上一素材的 channelMap.type）
	const unsigned nChannelType = (unsigned)qBound(0, nChannels, (int)CHANNEL_SYSTEM_MAX);
	m_nAudioChannelType = nChannelType;
	m_pLabelAudioTypeValue->setText(BuildAudioTypeText(resType, (int)nChannelType));

	const bool bShowChannelMap = IsNeedShowAudioChannelMap(resType) && nChannelType > 0;
	m_pWrapAudioChannelMap->setVisible(bShowChannelMap);

	if (!bShowChannelMap)
	{
		if (nChannelType == 0)
		{
			pShare->SetUsedChannelMap(false);
			mediaNode->SetUsedChannelMap(false);
		}
		m_pChkAudioChannelMap->blockSignals(true);
		m_pChkAudioChannelMap->setChecked(false);
		m_pChkAudioChannelMap->blockSignals(false);
		for (auto* pCombo : m_vecChannelCombobox)
		{
			if (pCombo)
				pCombo->hide();
		}
		return;
	}

	// 映射表 type 与资源声道不一致时强制按资源重置
	SChannelMap channelMap = mediaNode->GetAudioChannelMap();
	if (channelMap.type == 0)
		channelMap = pShare->GetAudioChannelMap();
	if (channelMap.type != nChannelType)
	{
		channelMap = SChannelMap::Default((int)nChannelType);
		mediaNode->SetUsedChannelMap(false);
		pShare->SetUsedChannelMap(false);
	}
	mediaNode->SetAudioChannelMap(channelMap);
	pShare->SetAudioChannelMap(channelMap);

	const bool bChecked = mediaNode->GetUsedChannelMap() || pShare->GetUsedChannelMap();
	mediaNode->SetUsedChannelMap(bChecked);
	pShare->SetUsedChannelMap(bChecked);

	m_pChkAudioChannelMap->blockSignals(true);
	m_pChkAudioChannelMap->setChecked(bChecked);
	m_pChkAudioChannelMap->blockSignals(false);

	UpdateChannelMapComboItems((int)nChannelType);

	const QStringList names = CPropertyTranslate::AudioChannelName();
	for (int idx = 0; idx < (int)m_vecChannelCombobox.size(); ++idx)
	{
		auto* pCombo = m_vecChannelCombobox[idx];
		if (!pCombo)
			continue;
		if (idx < names.size())
			pCombo->SetName(names[idx]);
		pCombo->blockSignals(true);
		if (idx < (int)nChannelType)
		{
			pCombo->show();
			pCombo->setEnabled(bChecked);
			const unsigned sel = channelMap.map[idx];
			if (sel < (unsigned)pCombo->Count())
				pCombo->SetSelectIndex((int)sel);
		}
		else
		{
			pCombo->hide();
		}
		pCombo->blockSignals(false);
	}

	if (m_pAudioPropertyGroup)
	{
		m_pAudioPropertyGroup->updateGeometry();
		m_pAudioPropertyGroup->update();
	}
}

QString CMediaPropertyFrame::BuildAudioTypeText(int nResType, int nChannels) const
{
	Q_UNUSED(nResType);
	if (nChannels <= 0)
		return QString("-");

	switch (nChannels)
	{
	case 1: return CHANNEL_SYSTEM_1_NAME;
	case 2: return CHANNEL_SYSTEM_2_NAME;
	case 3: return CHANNEL_SYSTEM_2_1_NAME;
	case 4: return CHANNEL_SYSTEM_4_NAME;
	case 5: return CHANNEL_SYSTEM_4_1_NAME;
	case 6: return CHANNEL_SYSTEM_5_1_NAME;
	case 7: return CHANNEL_SYSTEM_6_1_NAME;
	case 8: return CHANNEL_SYSTEM_7_1_NAME;
	default:
		return QString::number(nChannels);
	}
}

void CMediaPropertyFrame::UpdateChannelMapComboItems(int nInputChannels)
{
	const QStringList names = CPropertyTranslate::AudioChannelName();
	const QStringList items = names.mid(0, qBound(0, nInputChannels, names.size()));
	for (auto* pCombo : m_vecChannelCombobox)
	{
		if (!pCombo)
			continue;
		pCombo->blockSignals(true);
		pCombo->Clear();
		pCombo->AddItems(items, false);
		pCombo->blockSignals(false);
	}
}

void CMediaPropertyFrame::InitWidget()
{
	#define WIDGET_LEFT_MARGIN SysDpiZoom(20)

	m_pOneMediaFrame = new QFrame(this);

	m_pNameLineEditWidget = new CLineEditWidget(m_pOneMediaFrame);
	m_pNameLineEditWidget->setMinimumWidth(SysDpiZoom(258));
	m_pNameLineEditWidget->SetLabelConfig(SysDpiZoom(78), Qt::AlignLeft | Qt::AlignVCenter, nSizeDefine_8);
	m_pNameLineEditWidget->SetLabelProperty("AttributeName", true);
	m_pNameLineEditWidget->SetEditConfig(Qt::AlignCenter);
	m_pNameLineEditWidget->setContentsMargins(WIDGET_LEFT_MARGIN, 0, 0, 0);

	m_pPlayStart = new CTimeEditWidget(m_pOneMediaFrame);
	m_pPlayStart->setMinimumWidth(SysDpiZoom(258));
	m_pPlayStart->SetLabelConfig(SysDpiZoom(78), Qt::AlignLeft | Qt::AlignVCenter, nSizeDefine_8);
	m_pPlayStart->SetLabelProperty("AttributeName", true);
	m_pPlayStart->SetEditConfig(Qt::AlignCenter);
	m_pPlayStart->SetReadOnly(false);
	m_pPlayStart->SetMinTime(0);
	m_pPlayStart->setContentsMargins(WIDGET_LEFT_MARGIN, 0, 0, 0);
	m_pPlayStart->SetFps(g_pProjectSetting->LoadTimeLineFps());

	m_pPlayDuration = new CTimeEditWidget(m_pOneMediaFrame);
	m_pPlayDuration->setMinimumWidth(SysDpiZoom(258));
	m_pPlayDuration->SetLabelConfig(SysDpiZoom(78), Qt::AlignLeft | Qt::AlignVCenter, nSizeDefine_8);
	m_pPlayDuration->SetLabelProperty("AttributeName", true);
	m_pPlayDuration->SetEditConfig(Qt::AlignCenter);
	m_pPlayDuration->SetReadOnly(false);
	m_pPlayDuration->SetFps(g_pProjectSetting->LoadTimeLineFps());
	m_pPlayStart->SetMinTime(0);
	m_pPlayDuration->SetMinTime(1000);
	m_pPlayDuration->setContentsMargins(WIDGET_LEFT_MARGIN, 0, 0, 0);

	m_pFrameVideoProperty = new QFrame(m_pOneMediaFrame);
	m_pFrameVideoProperty->setContentsMargins(WIDGET_LEFT_MARGIN, 0, 0, 0);
	QVBoxLayout* pLayoutVideoPreperty = new QVBoxLayout(m_pFrameVideoProperty);
	pLayoutVideoPreperty->setMargin(0);
	pLayoutVideoPreperty->setSpacing(0);

	m_pClipBegin = new CTimeEditWidget(m_pFrameVideoProperty);
	m_pClipBegin->setMinimumWidth(SysDpiZoom(258));
	m_pClipBegin->SetLabelConfig(SysDpiZoom(78), Qt::AlignLeft | Qt::AlignVCenter, nSizeDefine_8);
	m_pClipBegin->SetLabelProperty("AttributeName", true);
	m_pClipBegin->SetEditConfig(Qt::AlignCenter);
	m_pClipBegin->SetReadOnly(false);
	pLayoutVideoPreperty->addWidget(m_pClipBegin);
	pLayoutVideoPreperty->addSpacing(nSizeDefine_8);

	m_pClipEnd = new CTimeEditWidget(m_pFrameVideoProperty);
	m_pClipEnd->setMinimumWidth(SysDpiZoom(258));
	m_pClipEnd->SetLabelConfig(SysDpiZoom(78), Qt::AlignLeft | Qt::AlignVCenter, nSizeDefine_8);
	m_pClipEnd->SetLabelProperty("AttributeName", true);
	m_pClipEnd->SetEditConfig(Qt::AlignCenter);
	m_pClipEnd->SetReadOnly(false);
	m_pClipEnd->SetFps(g_pProjectSetting->LoadTimeLineFps());
	pLayoutVideoPreperty->addWidget(m_pClipEnd);

	auto CreatePropertySpinRow = [&](CDoubleSpinBoxWidget** ppSpin, const QString& strName,
		double fMin, double fMax, int nDecimals = 3, bool bRotate = false)
	{
		*ppSpin = new CDoubleSpinBoxWidget(m_pOneMediaFrame, bRotate);
		(*ppSpin)->SetDecimals(nDecimals);
		(*ppSpin)->SetSingleStep(1);
		(*ppSpin)->setMinimumWidth(SysDpiZoom(258));
		(*ppSpin)->SetLabelConfig(SysDpiZoom(78), Qt::AlignLeft | Qt::AlignVCenter, nSizeDefine_8);
		(*ppSpin)->SetMaxandMinValue(fMax, fMin);
		(*ppSpin)->SetName(strName);
		(*ppSpin)->SetSpinBoxAlignment(Qt::AlignCenter);
		(*ppSpin)->SetLabelProperty("AttributeName", true);
		(*ppSpin)->setContentsMargins(WIDGET_LEFT_MARGIN, 0, 0, 0);
		(*ppSpin)->ShowHoverValue(true);
	};

	CreatePropertySpinRow(&m_pX, QString("X"), -20000, 20000);
	CreatePropertySpinRow(&m_pY, QString("Y"), -20000, 20000);
	CreatePropertySpinRow(&m_pZ, QString("Z"), -20000, 20000);
	CreatePropertySpinRow(&m_pWidth, QString("W"), 1, 100000000);
	CreatePropertySpinRow(&m_pHeight, QString("H"), 1, 100000000);
	CreatePropertySpinRow(&m_pRotX, QString("X"), 0, 360, 3, true);
	CreatePropertySpinRow(&m_pRotY, QString("Y"), 0, 360, 3, true);
	CreatePropertySpinRow(&m_pRotZ, QString("Z"), 0, 360, 3, true);
	m_pRotZ->SetSuffix(u8"°");

	m_pFrameStyleProperty = new QFrame(m_pOneMediaFrame);
	m_pComboFillType = new CComboBoxWidget(m_pFrameStyleProperty);
	m_pComboFillType->setMinimumWidth(SysDpiZoom(258));
	m_pComboFillType->SetLabelConfig(SysDpiZoom(78), Qt::AlignLeft | Qt::AlignVCenter, nSizeDefine_8);
	m_pComboFillType->SetLabelProperty("AttributeName", true);
	m_pComboFillType->SetWidgetContentMargin(0, 0, 0, SysDpiZoom(8));
	m_pComboFillType->SetComboTextAlignment(Qt::AlignCenter);
	m_pComboFillType->setContentsMargins(WIDGET_LEFT_MARGIN, 0, 0, 0);

	m_pFrameScrollType = new QFrame(m_pFrameStyleProperty);
	m_pComboTextScrollType = new CComboBoxWidget(m_pFrameScrollType);
	m_pComboTextScrollType->setMinimumWidth(SysDpiZoom(258));
	m_pComboTextScrollType->SetLabelConfig(SysDpiZoom(78), Qt::AlignLeft | Qt::AlignVCenter, nSizeDefine_8);
	m_pComboTextScrollType->SetLabelProperty("AttributeName", true);
	m_pComboTextScrollType->SetComboTextAlignment(Qt::AlignCenter);
	m_pComboTextScrollType->SetLimitLabelWidth(true);
	m_pComboTextScrollType->SetElideComboDisplayText(true);
	m_pComboTextScrollType->setContentsMargins(WIDGET_LEFT_MARGIN, 0, 0, 0);
	QVBoxLayout* pLayoutTurnType = new QVBoxLayout(m_pFrameScrollType);
	pLayoutTurnType->setMargin(0);
	pLayoutTurnType->setSpacing(0);
	pLayoutTurnType->addWidget(m_pComboTextScrollType);
	pLayoutTurnType->addSpacing(SysDpiZoom(8));

	m_pFrameScrollSpeed = new QFrame(m_pFrameStyleProperty);
	m_pEditTextScrollSpeed = new CLabelSpinbox(m_pFrameScrollSpeed, QString(), 1);
	m_pEditTextScrollSpeed->SetMinimum(1);
	m_pEditTextScrollSpeed->SetMaximum(20);
	m_pEditTextScrollSpeed->setMinimumWidth(SysDpiZoom(258));
	m_pEditTextScrollSpeed->SetLabelConfig(SysDpiZoom(78), Qt::AlignLeft | Qt::AlignVCenter, nSizeDefine_8);
	m_pEditTextScrollSpeed->SetLabelProperty("AttributeName", true);
	m_pEditTextScrollSpeed->setContentsMargins(WIDGET_LEFT_MARGIN, 0, 0, 0);
	QVBoxLayout* pLayoutInterval = new QVBoxLayout(m_pFrameScrollSpeed);
	pLayoutInterval->setMargin(0);
	pLayoutInterval->setSpacing(0);
	pLayoutInterval->addWidget(m_pEditTextScrollSpeed);
	pLayoutInterval->addSpacing(SysDpiZoom(8));

	m_pScrollPtyContainer = new QFrame(m_pFrameStyleProperty);
	m_pEteBtn = new CCheckBoxEx(m_pScrollPtyContainer);
	m_pEteBtn->setProperty("AttributeNameCheckBox", true);
	m_pEteBtn->setAutoExclusive(false);
	m_pEteBtn->setMinimumWidth(SysDpiZoom(74));
	m_pEndToEndInterval = new CLabelSpinbox(m_pScrollPtyContainer, QString(), 16);
	m_pEndToEndInterval->SetMinMaximum(0, 8192);
	m_pEndToEndInterval->SetLabelConfig(SysDpiZoom(24), Qt::AlignLeft | Qt::AlignVCenter, SysDpiZoom(4));
	m_pEndToEndInterval->setEnabled(m_pEteBtn->isChecked());
	m_pEndToEndInterval->SetLabelProperty("AttributeName", true);
	m_pEndToEndInterval->setMinimumWidth(SysDpiZoom(90));
	QHBoxLayout* pEteLayout = new QHBoxLayout(m_pScrollPtyContainer);
	pEteLayout->setContentsMargins(0, 0, 0, nSizeDefine_8);
	pEteLayout->setSpacing(0);
	pEteLayout->addSpacing(SysDpiZoom(88 + WIDGET_LEFT_MARGIN));
	pEteLayout->addWidget(m_pEteBtn);
	pEteLayout->addSpacing(SysDpiZoom(10));
	pEteLayout->addWidget(m_pEndToEndInterval);

	QVBoxLayout* pLayoutBasePreperty = new QVBoxLayout(m_pFrameStyleProperty);
	pLayoutBasePreperty->setMargin(0);
	pLayoutBasePreperty->setSpacing(0);
	pLayoutBasePreperty->addWidget(m_pComboFillType);
	pLayoutBasePreperty->addWidget(m_pFrameScrollType);
	pLayoutBasePreperty->addWidget(m_pFrameScrollSpeed);
	pLayoutBasePreperty->addWidget(m_pScrollPtyContainer);

	auto CreateSliderWidget = [&](QWidget* parent, const QString& strName, float fMin, float fMax, float fDefault, bool bEffect = true)
	{
		CSliderWidget* pNewObj = new CSliderWidget(parent, false);
		if (bEffect)
		{
			connect(pNewObj, SIGNAL(signalValueChange(double)), this, SLOT(slotEffectPropertyValueChange(double)));
			connect(pNewObj, &CSliderWidget::signalValueChangedMouseRelease, this, &CMediaPropertyFrame::slotEffectPropertyValueChangeMouseRelease);
		}
		pNewObj->SetName(strName);
		pNewObj->SetSliderConfig(0, fMin, fMax, fDefault, "%");
		pNewObj->SetLabelConfig(SysDpiZoom(76), Qt::AlignLeft | Qt::AlignVCenter, SysDpiZoom(12));
		pNewObj->SetSpinBoxWidth(SysDpiZoom(60));
		pNewObj->SetLabelProperty("AttributeName", true);
		pNewObj->SetSliderProperty("GridSlider", true);
		pNewObj->SetSpinBoxProperty("BtnVisible", false);
		pNewObj->setContentsMargins(WIDGET_LEFT_MARGIN, 0, 0, 0);
		return pNewObj;
	};

	auto AddGroupSlitLine = [&](CGroupWidgetBox* pGroup) -> QFrame* {
		auto pLine = new QFrame(m_pOneMediaFrame);
		pLine->setFixedHeight(2);
		pLine->setFrameShape(QFrame::HLine);
		pLine->setFrameShadow(QFrame::Sunken);
		pLine->setProperty("LineWnd", true);
		pGroup->AddSpacing(nSizeDefine_4);
		pGroup->AddContainWidget(pLine);
		pGroup->AddSpacing(nSizeDefine_4);
		return pLine;
	};

	auto SetupGroupHeaderActions = [&](CGroupWidgetBox* pGroup, EMediaPropertyGroupId /*groupId*/,
		CPropertyResetBtn** ppResetBtn, CPropertyKeyFrameBar** ppKeyFrameBar)
	{
		AttachPropertyGroupHeaderWidgets(pGroup, ppResetBtn, ppKeyFrameBar);
	};

	// 基础属性
	m_pBasePropertyGroup = new CGroupWidgetBox(m_pOneMediaFrame);
	m_pBasePropertyGroup->SetSpacing(nSizeDefine_8);
	m_pBasePropertyGroup->SetContainContentsMargins(0, 0, 0, 0);
	m_pBasePropertyGroup->AddContainWidget(m_pNameLineEditWidget);
	m_pBasePropertyGroup->AddContainWidget(m_pPlayStart);
	m_pBasePropertyGroup->AddContainWidget(m_pPlayDuration);
	m_pBasePropertyGroup->AddContainWidget(m_pFrameVideoProperty);
	m_pStyleSplitLine = AddGroupSlitLine(m_pBasePropertyGroup);
	m_pBasePropertyGroup->AddContainWidget(m_pFrameStyleProperty);

	// 音频属性
	m_pAudioPropertyGroup = new CGroupWidgetBox(m_pOneMediaFrame);
	m_pAudioPropertyGroup->SetSpacing(nSizeDefine_8);
	m_pAudioPropertyGroup->SetContainContentsMargins(0, 0, 0, 0);

	m_pWrapAudioType = new QWidget(m_pOneMediaFrame);
	QHBoxLayout* pAudioTypeLayout = new QHBoxLayout(m_pWrapAudioType);
	pAudioTypeLayout->setMargin(0);
	pAudioTypeLayout->setSpacing(0);
	pAudioTypeLayout->addSpacing(WIDGET_LEFT_MARGIN);
	m_pLabelAudioTypeName = new QLabel(m_pWrapAudioType);
	m_pLabelAudioTypeName->setFixedSize(SysDpiZoom(78), SysDpiZoom(24));
	m_pLabelAudioTypeName->setProperty("AttributeName", true);
	m_pLabelAudioTypeValue = new QLabel(m_pWrapAudioType);
	m_pLabelAudioTypeValue->setProperty("AttributeValue", true);
	pAudioTypeLayout->addWidget(m_pLabelAudioTypeName);
	pAudioTypeLayout->addSpacing(nSizeDefine_8);
	pAudioTypeLayout->addWidget(m_pLabelAudioTypeValue, 1);
	pAudioTypeLayout->addStretch();

	m_pWrapAudioChannelMap = new QWidget(m_pOneMediaFrame);
	QHBoxLayout* pChannelMapLayout = new QHBoxLayout(m_pWrapAudioChannelMap);
	pChannelMapLayout->setMargin(0);
	pChannelMapLayout->setSpacing(0);
	pChannelMapLayout->addSpacing(WIDGET_LEFT_MARGIN);
	m_pLabelAudioChannelMapName = new QLabel(m_pWrapAudioChannelMap);
	m_pLabelAudioChannelMapName->setFixedSize(SysDpiZoom(78), SysDpiZoom(24));
	m_pLabelAudioChannelMapName->setProperty("AttributeName", true);
	m_pChkAudioChannelMap = new CCheckBoxEx(m_pWrapAudioChannelMap);
	m_pChkAudioChannelMap->setText(QString());
	pChannelMapLayout->addWidget(m_pLabelAudioChannelMapName);
	pChannelMapLayout->addSpacing(nSizeDefine_8);
	pChannelMapLayout->addWidget(m_pChkAudioChannelMap);
	pChannelMapLayout->addStretch();
	m_pAudioPropertyGroup->AddContainWidget(m_pWrapAudioType);
	m_pAudioPropertyGroup->AddContainWidget(m_pWrapAudioChannelMap);

	// 预创建最大声道路数，刷新时按素材声道数显示/隐藏
	m_vecChannelCombobox.clear();
	for (unsigned int i = 0; i < CHANNEL_SYSTEM_MAX; ++i)
	{
		CComboBoxWidget* pChannelBox = new CComboBoxWidget(m_pOneMediaFrame);
		pChannelBox->setMinimumWidth(SysDpiZoom(258));
		pChannelBox->SetLabelConfig(SysDpiZoom(78), Qt::AlignLeft | Qt::AlignVCenter, nSizeDefine_8);
		pChannelBox->SetLabelProperty("AttributeName", true);
		pChannelBox->SetComboTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		pChannelBox->SetElideComboDisplayText(true);
		pChannelBox->setContentsMargins(WIDGET_LEFT_MARGIN, 0, 0, 0);
		pChannelBox->setProperty("ChannelSystem", (int)i);
		const QStringList names = CPropertyTranslate::AudioChannelName();
		if (i < (unsigned)names.size())
			pChannelBox->SetName(names[(int)i]);
		m_vecChannelCombobox.push_back(pChannelBox);
		m_pAudioPropertyGroup->AddContainWidget(pChannelBox);
	}

	// 音量
	m_pVolumePropertyGroup = new CGroupWidgetBox(m_pOneMediaFrame);
	m_pVolumePropertyGroup->SetSpacing(nSizeDefine_8);
	m_pVolume = CreateSliderWidget(m_pOneMediaFrame, QString(), 0, 1.0f, g_pProjectSetting->LoadDefaultVolume(), false);
	m_pVolumePropertyGroup->AddContainWidget(WrapRowWithKeyFrame(m_pOneMediaFrame, m_pVolume, &m_pKfVolume));
	SetupGroupHeaderActions(m_pVolumePropertyGroup, EMediaPropertyGroupId::Volume, &m_pVolumeResetBtn, &m_pVolumeGroupKeyFrameBar);

	// 位置
	m_pPositionPropertyGroup = new CGroupWidgetBox(m_pOneMediaFrame);
	m_pPositionPropertyGroup->SetSpacing(nSizeDefine_8);
	m_pPositionPropertyGroup->AddContainWidget(WrapRowWithKeyFrame(m_pOneMediaFrame, m_pX, &m_pKfPosX));
	m_pPositionPropertyGroup->AddContainWidget(WrapRowWithKeyFrame(m_pOneMediaFrame, m_pY, &m_pKfPosY));
	m_pPositionPropertyGroup->AddContainWidget(WrapRowWithKeyFrame(m_pOneMediaFrame, m_pZ, &m_pKfPosZ));
	SetupGroupHeaderActions(m_pPositionPropertyGroup, EMediaPropertyGroupId::Position, &m_pPositionResetBtn, &m_pPositionGroupKeyFrameBar);

	// 缩放：W/H 与位置 XYZ 同宽对齐；锁定钮浮于左侧 W/H 之间
	m_pScalePropertyGroup = new CGroupWidgetBox(m_pOneMediaFrame);
	m_pScalePropertyGroup->SetSpacing(nSizeDefine_8);
	{
		auto* pScaleRows = new CScaleRowsWidget(m_pOneMediaFrame);
		QVBoxLayout* pSpinLayout = new QVBoxLayout(pScaleRows);
		pSpinLayout->setMargin(0);
		pSpinLayout->setSpacing(nSizeDefine_8);
		pSpinLayout->addWidget(WrapRowWithKeyFrame(m_pOneMediaFrame, m_pWidth, &m_pKfScaleWidth));
		pSpinLayout->addWidget(WrapRowWithKeyFrame(m_pOneMediaFrame, m_pHeight, &m_pKfScaleHeight));

		m_pScaleAspectLockBtn = new CSvgImageButton(
			":/dark/images/PropertyWnd/ScaleAspect", pScaleRows);
		m_pScaleAspectLockBtn->setFixedSize(SysDpiZoom(24), SysDpiZoom(24));
		m_pScaleAspectLockBtn->setCheckable(true);
		m_pScaleAspectLockBtn->setProperty("IrregularPrismAssosiationBtn", true);
		pScaleRows->SetAspectLockButton(m_pScaleAspectLockBtn);

		m_pScalePropertyGroup->AddContainWidget(pScaleRows);
	}
	SetupGroupHeaderActions(m_pScalePropertyGroup, EMediaPropertyGroupId::Scale, &m_pScaleResetBtn, &m_pScaleGroupKeyFrameBar);

	// 旋转
	m_pRotationPropertyGroup = new CGroupWidgetBox(m_pOneMediaFrame);
	m_pRotationPropertyGroup->SetSpacing(nSizeDefine_8);
	m_pRotationPropertyGroup->AddContainWidget(WrapRowWithKeyFrame(m_pOneMediaFrame, m_pRotX, &m_pKfRotX));
	m_pRotationPropertyGroup->AddContainWidget(WrapRowWithKeyFrame(m_pOneMediaFrame, m_pRotY, &m_pKfRotY));
	m_pRotationPropertyGroup->AddContainWidget(WrapRowWithKeyFrame(m_pOneMediaFrame, m_pRotZ, &m_pKfRotZ));
	SetupGroupHeaderActions(m_pRotationPropertyGroup, EMediaPropertyGroupId::Rotation, &m_pRotationResetBtn, &m_pRotationGroupKeyFrameBar);

	// 裁剪（UI-only）
	m_pCropPropertyGroup = new CGroupWidgetBox(m_pOneMediaFrame);
	m_pCropPropertyGroup->SetSpacing(nSizeDefine_8);
	m_pWrapCropFill = new QWidget(m_pOneMediaFrame);
	QHBoxLayout* pCropFillLayout = new QHBoxLayout(m_pWrapCropFill);
	pCropFillLayout->setMargin(0);
	pCropFillLayout->setSpacing(0);
	pCropFillLayout->addSpacing(WIDGET_LEFT_MARGIN);
	m_pLabelCropFill = new QLabel(m_pWrapCropFill);
	m_pLabelCropFill->setFixedSize(SysDpiZoom(78), SysDpiZoom(24));
	m_pLabelCropFill->setProperty("AttributeName", true);
	m_pChkCropFill = new CCheckBoxEx(m_pWrapCropFill);
	m_pChkCropFill->setText(QString());
	pCropFillLayout->addWidget(m_pLabelCropFill);
	pCropFillLayout->addSpacing(nSizeDefine_8);
	pCropFillLayout->addWidget(m_pChkCropFill);
	pCropFillLayout->addStretch();
	auto CreateLocalSlider = [&](CSliderWidget** ppSlider, float fDefault)
	{
		*ppSlider = CreateSliderWidget(m_pOneMediaFrame, QString(), 0.f, 1.f, fDefault, false);
		connect(*ppSlider, &CSliderWidget::signalValueChange, this, &CMediaPropertyFrame::slotCropSliderChange);
	};

	CreateLocalSlider(&m_pCropLeft, 0.f);
	CreateLocalSlider(&m_pCropRight, 0.f);
	CreateLocalSlider(&m_pCropTop, 0.f);
	CreateLocalSlider(&m_pCropBottom, 0.f);
	m_pCropPropertyGroup->AddContainWidget(m_pWrapCropFill);
	m_pCropPropertyGroup->AddContainWidget(WrapRowWithKeyFrame(m_pOneMediaFrame, m_pCropLeft, &m_pKfCropLeft));
	m_pCropPropertyGroup->AddContainWidget(WrapRowWithKeyFrame(m_pOneMediaFrame, m_pCropRight, &m_pKfCropRight));
	m_pCropPropertyGroup->AddContainWidget(WrapRowWithKeyFrame(m_pOneMediaFrame, m_pCropTop, &m_pKfCropTop));
	m_pCropPropertyGroup->AddContainWidget(WrapRowWithKeyFrame(m_pOneMediaFrame, m_pCropBottom, &m_pKfCropBottom));
	SetupGroupHeaderActions(m_pCropPropertyGroup, EMediaPropertyGroupId::Crop, &m_pCropResetBtn, &m_pCropGroupKeyFrameBar);

	// 羽化（关键帧 → SFeatherFactor → MediaFxRender / xe_feather）
	m_pFeatherPropertyGroup = new CGroupWidgetBox(m_pOneMediaFrame);
	m_pFeatherPropertyGroup->SetSpacing(nSizeDefine_8);
	m_pFeatherRadius = CreateSliderWidget(m_pOneMediaFrame, QString(), 0.f, 1.f, 0.3f, false);
	connect(m_pFeatherRadius, &CSliderWidget::signalValueChange, this, &CMediaPropertyFrame::slotFeatherSliderChange);
	m_pFeatherPropertyGroup->AddContainWidget(WrapRowWithKeyFrame(m_pOneMediaFrame, m_pFeatherRadius, &m_pKfFeatherRadius));
	SetupGroupHeaderActions(m_pFeatherPropertyGroup, EMediaPropertyGroupId::Feather, &m_pFeatherResetBtn, &m_pFeatherGroupKeyFrameBar);

	// 抠像（UI-only）
	m_pKeyingPropertyGroup = new CGroupWidgetBox(m_pOneMediaFrame);
	m_pKeyingPropertyGroup->SetSpacing(nSizeDefine_8);
	auto CreateKeyingSlider = [&](CSliderWidget** ppSlider, float fDefault, float fMax = 1.f)
	{
		*ppSlider = CreateSliderWidget(m_pOneMediaFrame, QString(), 0.f, fMax, fDefault, false);
		connect(*ppSlider, &CSliderWidget::signalValueChange, this, &CMediaPropertyFrame::slotKeyingSliderChange);
	};
	CreateKeyingSlider(&m_pKeyingRed, 0.f);
	CreateKeyingSlider(&m_pKeyingGreen, 1.f);
	CreateKeyingSlider(&m_pKeyingBlue, 0.f);
	CreateKeyingSlider(&m_pKeyingTolerance, 0.5f);
	CreateKeyingSlider(&m_pKeyingDegree, 0.1f);
	m_pKeyingPropertyGroup->AddContainWidget(WrapRowWithKeyFrame(m_pOneMediaFrame, m_pKeyingRed, &m_pKfKeyingRed));
	m_pKeyingPropertyGroup->AddContainWidget(WrapRowWithKeyFrame(m_pOneMediaFrame, m_pKeyingGreen, &m_pKfKeyingGreen));
	m_pKeyingPropertyGroup->AddContainWidget(WrapRowWithKeyFrame(m_pOneMediaFrame, m_pKeyingBlue, &m_pKfKeyingBlue));
	m_pKeyingPropertyGroup->AddContainWidget(WrapRowWithKeyFrame(m_pOneMediaFrame, m_pKeyingTolerance, &m_pKfKeyingTolerance));
	m_pKeyingPropertyGroup->AddContainWidget(WrapRowWithKeyFrame(m_pOneMediaFrame, m_pKeyingDegree, &m_pKfKeyingDegree));
	SetupGroupHeaderActions(m_pKeyingPropertyGroup, EMediaPropertyGroupId::Keying, &m_pKeyingResetBtn, &m_pKeyingGroupKeyFrameBar);

	// 效果
	m_pEffectPropertyGroup = new CGroupWidgetBox(m_pOneMediaFrame);
	m_pEffectPropertyGroup->SetSpacing(nSizeDefine_8);
	m_pRed = CreateSliderWidget(m_pEffectPropertyGroup, QString(), 0.f, 1.f, 1.f);
	m_pGreen = CreateSliderWidget(m_pEffectPropertyGroup, QString(), 0.f, 1.f, 1.f);
	m_pBlue = CreateSliderWidget(m_pEffectPropertyGroup, QString(), 0.f, 1.f, 1.f);
	m_pAlpha = CreateSliderWidget(m_pEffectPropertyGroup, QString(), 0.f, 1.f, 1.f);
	m_pLuminance = CreateSliderWidget(m_pEffectPropertyGroup, QString(), -1.f, 1.f, 0.f);
	m_pContract = CreateSliderWidget(m_pEffectPropertyGroup, QString(), -1.f, 1.f, 0.f);
	m_pHue = CreateSliderWidget(m_pEffectPropertyGroup, QString(), -1.f, 1.f, 0.f);
	m_pSaturability = CreateSliderWidget(m_pEffectPropertyGroup, QString(), -1.f, 1.f, 0.f);
	m_pWrapEffectRed = WrapRowWithKeyFrame(m_pOneMediaFrame, m_pRed, &m_pKfEffectRed);
	m_pWrapEffectGreen = WrapRowWithKeyFrame(m_pOneMediaFrame, m_pGreen, &m_pKfEffectGreen);
	m_pWrapEffectBlue = WrapRowWithKeyFrame(m_pOneMediaFrame, m_pBlue, &m_pKfEffectBlue);
	m_pEffectPropertyGroup->AddContainWidget(m_pWrapEffectRed);
	m_pEffectPropertyGroup->AddContainWidget(m_pWrapEffectGreen);
	m_pEffectPropertyGroup->AddContainWidget(m_pWrapEffectBlue);
	m_pEffectPropertyGroup->AddContainWidget(WrapRowWithKeyFrame(m_pOneMediaFrame, m_pAlpha, &m_pKfEffectAlpha));
	m_pEffectPropertyGroup->AddContainWidget(WrapRowWithKeyFrame(m_pOneMediaFrame, m_pLuminance, &m_pKfEffectLuminance));
	m_pEffectPropertyGroup->AddContainWidget(WrapRowWithKeyFrame(m_pOneMediaFrame, m_pContract, &m_pKfEffectContrast));
	m_pEffectPropertyGroup->AddContainWidget(WrapRowWithKeyFrame(m_pOneMediaFrame, m_pHue, &m_pKfEffectHue));
	m_pEffectPropertyGroup->AddContainWidget(WrapRowWithKeyFrame(m_pOneMediaFrame, m_pSaturability, &m_pKfEffectSaturation));
	SetupGroupHeaderActions(m_pEffectPropertyGroup, EMediaPropertyGroupId::Effect, &m_pEffectResetBtn, &m_pEffectGroupKeyFrameBar);

	QVBoxLayout* pLayout = new QVBoxLayout(m_pOneMediaFrame);
	pLayout->setMargin(0);
	pLayout->setSpacing(0);
	auto AddLine = [&]() -> QFrame* {
		auto pLine = new QFrame(m_pOneMediaFrame);
		pLine->setFixedHeight(2);
		pLine->setFrameShape(QFrame::HLine);
		pLine->setFrameShadow(QFrame::Sunken);
		pLine->setProperty("LineWnd", true);
		pLayout->addSpacing(nSizeDefine_4);
		pLayout->addWidget(pLine);
		pLayout->addSpacing(nSizeDefine_4);
		return pLine;
	};

	pLayout->addWidget(m_pBasePropertyGroup);
	m_pGroupSeparators[0] = AddLine();
	pLayout->addWidget(m_pAudioPropertyGroup);
	m_pGroupSeparators[1] = AddLine();
	pLayout->addWidget(m_pVolumePropertyGroup);
	pLayout->addWidget(m_pPositionPropertyGroup);
	pLayout->addWidget(m_pScalePropertyGroup);
	pLayout->addWidget(m_pRotationPropertyGroup);
	pLayout->addWidget(m_pCropPropertyGroup);
	pLayout->addWidget(m_pFeatherPropertyGroup);
	pLayout->addWidget(m_pKeyingPropertyGroup);
	pLayout->addWidget(m_pEffectPropertyGroup);
	pLayout->addStretch();

	m_pMutilMediaFrame = new QFrame(this);
	m_pMutilMediaPlayStart = new CTimeEditWidget(m_pMutilMediaFrame);
	m_pMutilMediaPlayStart->SetLabelConfig(nSizeDefine_64, Qt::AlignLeft | Qt::AlignVCenter, nSizeDefine_8);
	m_pMutilMediaPlayStart->SetReadOnly(true);
	m_pMutilMediaPlayStart->SetFps(g_pProjectSetting->LoadTimeLineFps());
	m_pMutilMediaPlayDuration = new CTimeEditWidget(m_pMutilMediaFrame);
	m_pMutilMediaPlayDuration->SetLabelConfig(nSizeDefine_64, Qt::AlignLeft | Qt::AlignVCenter, nSizeDefine_8);
	m_pMutilMediaPlayDuration->SetReadOnly(true);
	m_pMutilMediaPlayDuration->SetFps(g_pProjectSetting->LoadTimeLineFps());
	QVBoxLayout* pLayoutMutilMedia = new QVBoxLayout(m_pMutilMediaFrame);
	pLayoutMutilMedia->setMargin(0);
	pLayoutMutilMedia->setSpacing(0);
	pLayoutMutilMedia->addWidget(m_pMutilMediaPlayStart);
	pLayoutMutilMedia->addSpacing(nSizeDefine_8);
	pLayoutMutilMedia->addWidget(m_pMutilMediaPlayDuration);

	QVBoxLayout* pMainLayout = new QVBoxLayout(this);
	pMainLayout->setMargin(0);
	pMainLayout->setSpacing(0);
	pMainLayout->addWidget(m_pOneMediaFrame);
	pMainLayout->addWidget(m_pMutilMediaFrame);

	#undef WIDGET_LEFT_MARGIN
}

// 连接基础编辑、特效滑条、分组重置/关键帧栏，以及游标变化时的实时同步
void CMediaPropertyFrame::CreateConnects()
{
	connect(m_pNameLineEditWidget, &CLineEditWidget::signalEditFinished, this, &CMediaPropertyFrame::slotLineEdit);
	connect(m_pPlayStart, &CTimeEditWidget::signalTimeChange, this, &CMediaPropertyFrame::slotTimeEdit);
	connect(m_pPlayDuration, &CTimeEditWidget::signalTimeChange, this, &CMediaPropertyFrame::slotTimeEdit);
	connect(m_pPlayStart, &CTimeEditWidget::signalTimeEditFinish, this, &CMediaPropertyFrame::slotTimeEditFinish);
	connect(m_pPlayDuration, &CTimeEditWidget::signalTimeEditFinish, this, &CMediaPropertyFrame::slotTimeEditFinish);
	connect(m_pClipBegin, &CTimeEditWidget::signalTimeChange, this, &CMediaPropertyFrame::slotTimeEdit);
	connect(m_pClipEnd, &CTimeEditWidget::signalTimeChange, this, &CMediaPropertyFrame::slotTimeEdit);
	connect(m_pClipBegin, &CTimeEditWidget::signalTimeEditFinish, this, &CMediaPropertyFrame::slotTimeEditFinish);
	connect(m_pClipEnd, &CTimeEditWidget::signalTimeEditFinish, this, &CMediaPropertyFrame::slotTimeEditFinish);
	connect(m_pComboFillType, &CComboBoxWidget::signalSelectChange, this, &CMediaPropertyFrame::slotFillComboSelectChange);
	connect(m_pComboTextScrollType, &CComboBoxWidget::signalSelectChange, this, &CMediaPropertyFrame::slotComboTurnPageScrollTypeChange);
	connect(m_pEditTextScrollSpeed, &CLabelSpinbox::valueChanged, this, [this](int nValue)
	{
		if (m_setMediaIDs.size() != 1) return;
		auto mediaNode = g_pMediaModel->GetMediaNode(*m_setMediaIDs.begin()).lock();
		if (!mediaNode) return;
		if (auto pParam = mediaNode->GetShareProperty()->GetScrollParam())
			pParam->fScrollSpeed = (float)nValue;
	});
	connect(m_pEteBtn, &CCheckBoxEx::clicked, this, &CMediaPropertyFrame::slotEteBtnClicked);
	connect(m_pEndToEndInterval, &CLabelSpinbox::valueChanged, this, &CMediaPropertyFrame::slotEndToEndIntervalChange);
	connect(m_pEndToEndInterval, &CLabelSpinbox::signalEditingFinished, this, [this]() { HistoryStackSave(); });
	connect(m_pMutilMediaPlayStart, &CTimeEditWidget::signalTimeEditFinish, this, &CMediaPropertyFrame::slotTimeEditFinish);
	connect(m_pMutilMediaPlayDuration, &CTimeEditWidget::signalTimeEditFinish, this, &CMediaPropertyFrame::slotTimeEditFinish);
	connect(m_pChkAudioChannelMap, &CCheckBoxEx::clicked, this, &CMediaPropertyFrame::slotAudioChannelMapCheckChanged);
	for (auto* pCombo : m_vecChannelCombobox)
		connect(pCombo, &CComboBoxWidget::signalSelectChange, this, &CMediaPropertyFrame::slotAudioChannelChanged);
	connect(m_pVolume, &CSliderWidget::signalValueChange, this, [this](double fValue)
	{
		if (m_setMediaIDs.size() != 1) return;
		const QUuid mediaId = *m_setMediaIDs.begin();
		if (auto mediaNode = g_pMediaModel->GetMediaNode(mediaId).lock())
			mediaNode->SetVolumn((float)fValue);
		ApplyPropertyEditAtPlayhead(EMediaPropertyId::VolumeIntensity, (float)fValue);
		NotifyMediaVolumeChanged(mediaId);
	});
	connect(m_pVolume, &CSliderWidget::signalValueChangedMouseRelease, this, [this](double) { HistoryStackSave(); });
	connect(m_pX, &CDoubleSpinBoxWidget::signalValueChange, this, &CMediaPropertyFrame::slotDoubleSpinBoxEdit);
	connect(m_pY, &CDoubleSpinBoxWidget::signalValueChange, this, &CMediaPropertyFrame::slotDoubleSpinBoxEdit);
	connect(m_pZ, &CDoubleSpinBoxWidget::signalValueChange, this, &CMediaPropertyFrame::slotDoubleSpinBoxEdit);
	connect(m_pWidth, &CDoubleSpinBoxWidget::signalValueChange, this, &CMediaPropertyFrame::slotDoubleSpinBoxEdit);
	connect(m_pHeight, &CDoubleSpinBoxWidget::signalValueChange, this, &CMediaPropertyFrame::slotDoubleSpinBoxEdit);
	connect(m_pScaleAspectLockBtn, &CSvgImageButton::clicked, this, &CMediaPropertyFrame::slotScaleAspectLockClicked);
	connect(m_pRotX, &CDoubleSpinBoxWidget::signalValueChange, this, &CMediaPropertyFrame::slotDoubleSpinBoxEdit);
	connect(m_pRotY, &CDoubleSpinBoxWidget::signalValueChange, this, &CMediaPropertyFrame::slotDoubleSpinBoxEdit);
	connect(m_pRotZ, &CDoubleSpinBoxWidget::signalValueChange, this, &CMediaPropertyFrame::slotDoubleSpinBoxEdit);
	connect(m_pLuminance, SIGNAL(signalValueChange(double)), this, SLOT(slotEffectPropertyValueChange(double)));
	connect(m_pRed, SIGNAL(signalValueChange(double)), this, SLOT(slotEffectPropertyValueChange(double)));
	connect(m_pGreen, SIGNAL(signalValueChange(double)), this, SLOT(slotEffectPropertyValueChange(double)));
	connect(m_pBlue, SIGNAL(signalValueChange(double)), this, SLOT(slotEffectPropertyValueChange(double)));
	connect(m_pAlpha, SIGNAL(signalValueChange(double)), this, SLOT(slotEffectPropertyValueChange(double)));
	connect(m_pContract, SIGNAL(signalValueChange(double)), this, SLOT(slotEffectPropertyValueChange(double)));
	connect(m_pHue, SIGNAL(signalValueChange(double)), this, SLOT(slotEffectPropertyValueChange(double)));
	connect(m_pSaturability, SIGNAL(signalValueChange(double)), this, SLOT(slotEffectPropertyValueChange(double)));
	connect(m_pLuminance, &CSliderWidget::signalValueChangedMouseRelease, this, &CMediaPropertyFrame::slotEffectPropertyValueChangeMouseRelease);
	connect(m_pRed, &CSliderWidget::signalValueChangedMouseRelease, this, &CMediaPropertyFrame::slotEffectPropertyValueChangeMouseRelease);
	connect(m_pGreen, &CSliderWidget::signalValueChangedMouseRelease, this, &CMediaPropertyFrame::slotEffectPropertyValueChangeMouseRelease);
	connect(m_pBlue, &CSliderWidget::signalValueChangedMouseRelease, this, &CMediaPropertyFrame::slotEffectPropertyValueChangeMouseRelease);
	connect(m_pAlpha, &CSliderWidget::signalValueChangedMouseRelease, this, &CMediaPropertyFrame::slotEffectPropertyValueChangeMouseRelease);
	connect(m_pContract, &CSliderWidget::signalValueChangedMouseRelease, this, &CMediaPropertyFrame::slotEffectPropertyValueChangeMouseRelease);
	connect(m_pHue, &CSliderWidget::signalValueChangedMouseRelease, this, &CMediaPropertyFrame::slotEffectPropertyValueChangeMouseRelease);
	connect(m_pSaturability, &CSliderWidget::signalValueChangedMouseRelease, this, &CMediaPropertyFrame::slotEffectPropertyValueChangeMouseRelease);
	connect(m_pChkCropFill, &CCheckBoxEx::clicked, this, [this]()
	{
		m_bCropFill = m_pChkCropFill->isChecked();
		ApplyPropertyEditAtPlayhead(EMediaPropertyId::CropFill, m_bCropFill ? 1.f : 0.f);
		HistoryStackSave();
	});
	connect(m_pCropLeft, &CSliderWidget::signalValueChange, this, &CMediaPropertyFrame::slotCropSliderChange);
	connect(m_pCropRight, &CSliderWidget::signalValueChange, this, &CMediaPropertyFrame::slotCropSliderChange);
	connect(m_pCropTop, &CSliderWidget::signalValueChange, this, &CMediaPropertyFrame::slotCropSliderChange);
	connect(m_pCropBottom, &CSliderWidget::signalValueChange, this, &CMediaPropertyFrame::slotCropSliderChange);
	connect(m_pFeatherRadius, &CSliderWidget::signalValueChange, this, &CMediaPropertyFrame::slotFeatherSliderChange);
	connect(m_pKeyingRed, &CSliderWidget::signalValueChange, this, &CMediaPropertyFrame::slotKeyingSliderChange);
	connect(m_pKeyingGreen, &CSliderWidget::signalValueChange, this, &CMediaPropertyFrame::slotKeyingSliderChange);
	connect(m_pKeyingBlue, &CSliderWidget::signalValueChange, this, &CMediaPropertyFrame::slotKeyingSliderChange);
	connect(m_pKeyingTolerance, &CSliderWidget::signalValueChange, this, &CMediaPropertyFrame::slotKeyingSliderChange);
	connect(m_pKeyingDegree, &CSliderWidget::signalValueChange, this, &CMediaPropertyFrame::slotKeyingSliderChange);
	connect(m_pCropLeft, &CSliderWidget::signalValueChangedMouseRelease, this, &CMediaPropertyFrame::slotEffectPropertyValueChangeMouseRelease);
	connect(m_pCropRight, &CSliderWidget::signalValueChangedMouseRelease, this, &CMediaPropertyFrame::slotEffectPropertyValueChangeMouseRelease);
	connect(m_pCropTop, &CSliderWidget::signalValueChangedMouseRelease, this, &CMediaPropertyFrame::slotEffectPropertyValueChangeMouseRelease);
	connect(m_pCropBottom, &CSliderWidget::signalValueChangedMouseRelease, this, &CMediaPropertyFrame::slotEffectPropertyValueChangeMouseRelease);
	connect(m_pFeatherRadius, &CSliderWidget::signalValueChangedMouseRelease, this, &CMediaPropertyFrame::slotEffectPropertyValueChangeMouseRelease);
	connect(m_pKeyingRed, &CSliderWidget::signalValueChangedMouseRelease, this, &CMediaPropertyFrame::slotEffectPropertyValueChangeMouseRelease);
	connect(m_pKeyingGreen, &CSliderWidget::signalValueChangedMouseRelease, this, &CMediaPropertyFrame::slotEffectPropertyValueChangeMouseRelease);
	connect(m_pKeyingBlue, &CSliderWidget::signalValueChangedMouseRelease, this, &CMediaPropertyFrame::slotEffectPropertyValueChangeMouseRelease);
	connect(m_pKeyingTolerance, &CSliderWidget::signalValueChangedMouseRelease, this, &CMediaPropertyFrame::slotEffectPropertyValueChangeMouseRelease);
	connect(m_pKeyingDegree, &CSliderWidget::signalValueChangedMouseRelease, this, &CMediaPropertyFrame::slotEffectPropertyValueChangeMouseRelease);
	ConnectGroupHeaderActions(EMediaPropertyGroupId::Volume, m_pVolumeResetBtn, m_pVolumeGroupKeyFrameBar);
	ConnectGroupHeaderActions(EMediaPropertyGroupId::Position, m_pPositionResetBtn, m_pPositionGroupKeyFrameBar);
	ConnectGroupHeaderActions(EMediaPropertyGroupId::Scale, m_pScaleResetBtn, m_pScaleGroupKeyFrameBar);
	ConnectGroupHeaderActions(EMediaPropertyGroupId::Rotation, m_pRotationResetBtn, m_pRotationGroupKeyFrameBar);
	ConnectGroupHeaderActions(EMediaPropertyGroupId::Crop, m_pCropResetBtn, m_pCropGroupKeyFrameBar);
	ConnectGroupHeaderActions(EMediaPropertyGroupId::Feather, m_pFeatherResetBtn, m_pFeatherGroupKeyFrameBar);
	ConnectGroupHeaderActions(EMediaPropertyGroupId::Keying, m_pKeyingResetBtn, m_pKeyingGroupKeyFrameBar);
	ConnectGroupHeaderActions(EMediaPropertyGroupId::Effect, m_pEffectResetBtn, m_pEffectGroupKeyFrameBar);
	ConnectPropertyKeyFrameBar(m_pKfVolume, EMediaPropertyId::VolumeIntensity);
	ConnectPropertyKeyFrameBar(m_pKfPosX, EMediaPropertyId::PosX);
	ConnectPropertyKeyFrameBar(m_pKfPosY, EMediaPropertyId::PosY);
	ConnectPropertyKeyFrameBar(m_pKfPosZ, EMediaPropertyId::PosZ);
	ConnectPropertyKeyFrameBar(m_pKfScaleWidth, EMediaPropertyId::ScaleWidth);
	ConnectPropertyKeyFrameBar(m_pKfScaleHeight, EMediaPropertyId::ScaleHeight);
	ConnectPropertyKeyFrameBar(m_pKfRotX, EMediaPropertyId::RotX);
	ConnectPropertyKeyFrameBar(m_pKfRotY, EMediaPropertyId::RotY);
	ConnectPropertyKeyFrameBar(m_pKfRotZ, EMediaPropertyId::RotZ);
	ConnectPropertyKeyFrameBar(m_pKfCropLeft, EMediaPropertyId::CropLeft);
	ConnectPropertyKeyFrameBar(m_pKfCropRight, EMediaPropertyId::CropRight);
	ConnectPropertyKeyFrameBar(m_pKfCropTop, EMediaPropertyId::CropTop);
	ConnectPropertyKeyFrameBar(m_pKfCropBottom, EMediaPropertyId::CropBottom);
	ConnectPropertyKeyFrameBar(m_pKfFeatherRadius, EMediaPropertyId::FeatherRadius);
	ConnectPropertyKeyFrameBar(m_pKfKeyingRed, EMediaPropertyId::KeyingRed);
	ConnectPropertyKeyFrameBar(m_pKfKeyingGreen, EMediaPropertyId::KeyingGreen);
	ConnectPropertyKeyFrameBar(m_pKfKeyingBlue, EMediaPropertyId::KeyingBlue);
	ConnectPropertyKeyFrameBar(m_pKfKeyingTolerance, EMediaPropertyId::KeyingTolerance);
	ConnectPropertyKeyFrameBar(m_pKfKeyingDegree, EMediaPropertyId::KeyingDegree);
	ConnectPropertyKeyFrameBar(m_pKfEffectRed, EMediaPropertyId::EffectRed);
	ConnectPropertyKeyFrameBar(m_pKfEffectGreen, EMediaPropertyId::EffectGreen);
	ConnectPropertyKeyFrameBar(m_pKfEffectBlue, EMediaPropertyId::EffectBlue);
	ConnectPropertyKeyFrameBar(m_pKfEffectAlpha, EMediaPropertyId::EffectAlpha);
	ConnectPropertyKeyFrameBar(m_pKfEffectLuminance, EMediaPropertyId::EffectLuminance);
	ConnectPropertyKeyFrameBar(m_pKfEffectContrast, EMediaPropertyId::EffectContrast);
	ConnectPropertyKeyFrameBar(m_pKfEffectHue, EMediaPropertyId::EffectHue);
	ConnectPropertyKeyFrameBar(m_pKfEffectSaturation, EMediaPropertyId::EffectSaturation);
	connect(g_pSignalCenter, &CSignalCenter::signalUpdateMediaProperty, this, [this](const QUuid& mediaid)
	{
		if (m_setMediaIDs.empty() || *m_setMediaIDs.begin() != mediaid) return;
		UpdateProperty();
	});
	// 游标移动：实时刷新 enable 状态、关键帧 Toggle 与插值后的属性值
	if (g_pSignalCenter)
	{
		connect(g_pSignalCenter, &CSignalCenter::signalNoniusTimeChanged, this, [this](qint64 timelineMs)
		{
			if (m_setMediaIDs.size() != 1 || !m_pOneMediaFrame->isVisible()) return;
			// 播放中降频刷属性面板，避免每帧 enable/插值拖垮 UI
			if (g_pPlayerController && g_pPlayerController->IsPlaying())
			{
				static qint64 s_lastUiMs = -1;
				if (s_lastUiMs >= 0 && qAbs(timelineMs - s_lastUiMs) < 100)
					return;
				s_lastUiMs = timelineMs;
			}
			const auto playheadMs = std::chrono::milliseconds(timelineMs);
			SetPropertyActionBarsEnabled(m_pOneMediaFrame->isEnabled(), playheadMs);
			RefreshPropertyKeyframeToggleStates(playheadMs);
			SyncKeyframeEvaluatedValuesAtPlayhead(playheadMs);
		});
		connect(g_pSignalCenter, &CSignalCenter::signalSyncClipFxActiveProperty, this,
			[this](const QUuid& mediaId, int nPropId, qint64 clipLocalMs)
			{
				if (m_setMediaIDs.size() != 1 || *m_setMediaIDs.begin() != mediaId)
					return;
				if (!m_pOneMediaFrame || !m_pOneMediaFrame->isVisible())
					return;
				CMediaKeyframeHelper::SyncActivePropertyFrame(mediaId,
					static_cast<EMediaPropertyId>(nPropId),
					std::chrono::milliseconds(clipLocalMs));
				RefreshPropertyKeyframeToggleStates();
			});
	}
}

void CMediaPropertyFrame::ConnectGroupHeaderActions(EMediaPropertyGroupId groupId,
	CPropertyResetBtn* pResetBtn, CPropertyKeyFrameBar* pKeyFrameBar)
{
	if (!pResetBtn || !pKeyFrameBar) return;
	connect(pResetBtn, &CPropertyResetBtn::signalResetClicked, this, [this, groupId]() { ResetPropertyGroup(groupId); });
	connect(pKeyFrameBar, &CPropertyKeyFrameBar::signalPreKeyFrameClicked, this, [this, groupId]() { slotGroupKeyFrameStub(groupId, EKeyFrameOp::Pre); });
	connect(pKeyFrameBar, &CPropertyKeyFrameBar::signalToggleKeyFrameClicked, this, [this, groupId]() { slotGroupKeyFrameStub(groupId, EKeyFrameOp::Toggle); });
	connect(pKeyFrameBar, &CPropertyKeyFrameBar::signalNextKeyFrameClicked, this, [this, groupId]() { slotGroupKeyFrameStub(groupId, EKeyFrameOp::Next); });
}

void CMediaPropertyFrame::ConnectPropertyKeyFrameBar(CPropertyKeyFrameBar* pBar, EMediaPropertyId propId)
{
	if (!pBar) return;
	connect(pBar, &CPropertyKeyFrameBar::signalPreKeyFrameClicked, this, [this, propId]() { slotPropertyKeyFrameStub(propId, EKeyFrameOp::Pre); });
	connect(pBar, &CPropertyKeyFrameBar::signalToggleKeyFrameClicked, this, [this, propId]() { slotPropertyKeyFrameStub(propId, EKeyFrameOp::Toggle); });
	connect(pBar, &CPropertyKeyFrameBar::signalNextKeyFrameClicked, this, [this, propId]() { slotPropertyKeyFrameStub(propId, EKeyFrameOp::Next); });
}

void CMediaPropertyFrame::UpdateTranslateText()
{
	m_pBasePropertyGroup->SetName(CPropertyTranslate::BaseProperty());
	m_pNameLineEditWidget->SetName(CCommonTranslate::Name());
	m_pPlayStart->SetName(CPropertyTranslate::PlayStart());
	m_pPlayDuration->SetName(CPropertyTranslate::PlayDuration());
	m_pClipBegin->SetName(CPropertyTranslate::ClipBegin());
	m_pClipEnd->SetName(CPropertyTranslate::ClipEnd());
	m_pComboFillType->SetName(CPropertyTranslate::FillType());
	m_pComboTextScrollType->SetName(CTextPropertyFrameTranslate::ScrollGroupName());
	m_pEditTextScrollSpeed->SetText(CTextPropertyFrameTranslate::ScrollSpeedText());
	m_pEteBtn->setText(CTextPropertyFrameTranslate::EteBtnTip());
	m_pEndToEndInterval->SetText(CTextPropertyFrameTranslate::InterValText());

	m_pAudioPropertyGroup->SetName(CPropertyTranslate::AudioProperty());
	m_pLabelAudioTypeName->setText(CCommonTranslate::AudioFormat());
	if (m_pLabelAudioChannelMapName)
		m_pLabelAudioChannelMapName->setText(CPropertyTranslate::UsedChannelMapTip());
	m_pChkAudioChannelMap->setText(QString());
	{
		const QStringList names = CPropertyTranslate::AudioChannelName();
		for (int idx = 0; idx < (int)m_vecChannelCombobox.size(); ++idx)
		{
			if (idx < names.size())
				m_vecChannelCombobox[idx]->SetName(names[idx]);
			m_vecChannelCombobox[idx]->AddItems(names.mid(0, (int)m_nAudioChannelType), true);
		}
	}

	m_pVolumePropertyGroup->SetName(CPropertyTranslate::Volumn());
	m_pVolume->SetName(CPropertyTranslate::Intensity());

	m_pPositionPropertyGroup->SetName(CCommonTranslate::Position());
	m_pScalePropertyGroup->SetName(CSliceViewTranslate::Zoom());
	m_pRotationPropertyGroup->SetName(CSliceViewTranslate::Rotate());

	m_pCropPropertyGroup->SetName(CFxPropertyTranslate::Cut());
	if (m_pLabelCropFill)
		m_pLabelCropFill->setText(CFxPropertyTranslate::Complement());
	m_pChkCropFill->setText(QString());
	m_pCropLeft->SetName(CFxPropertyTranslate::Left());
	m_pCropRight->SetName(CFxPropertyTranslate::Right());
	m_pCropTop->SetName(CFxPropertyTranslate::Top());
	m_pCropBottom->SetName(CFxPropertyTranslate::Bottom());

	m_pFeatherPropertyGroup->SetName(CFxPropertyTranslate::Feather());
	m_pFeatherRadius->SetName(CFxPropertyTranslate::FeatherRadius());

	m_pKeyingPropertyGroup->SetName(CFxPropertyTranslate::Keying());
	m_pKeyingRed->SetName(CCommonTranslate::Red());
	m_pKeyingGreen->SetName(CCommonTranslate::Green());
	m_pKeyingBlue->SetName(CCommonTranslate::Blue());
	m_pKeyingTolerance->SetName(CFxPropertyTranslate::Threshold());
	m_pKeyingDegree->SetName(CFxPropertyTranslate::Degree());

	m_pEffectPropertyGroup->SetName(CPropertyTranslate::EffectProperty());
	m_pLuminance->SetName(CCommonTranslate::Brightness());
	m_pRed->SetName(CCommonTranslate::Red());
	m_pGreen->SetName(CCommonTranslate::Green());
	m_pBlue->SetName(CCommonTranslate::Blue());
	m_pAlpha->SetName(CCommonTranslate::Transparent());
	m_pContract->SetName(CCommonTranslate::Contrast());
	m_pHue->SetName(CCommonTranslate::Hue());
	m_pSaturability->SetName(CCommonTranslate::Saturation());

	m_pMutilMediaPlayStart->SetName(CPropertyTranslate::PlayStart());
	m_pMutilMediaPlayDuration->SetName(CPropertyTranslate::PlayDuration());

	if (m_pScaleAspectLockBtn)
	{
		m_pScaleAspectLockBtn->setToolTip(m_pScaleAspectLockBtn->isChecked()
			? CPropertyTranslate::ScaleAspectUnlock()
			: CPropertyTranslate::ScaleAspectLock());
	}
}

void CMediaPropertyFrame::ClearEdit()
{
	m_pOneMediaFrame->setVisible(false);
	m_pMutilMediaFrame->setVisible(false);
	SetPropertyActionBarsEnabled(false);
}

void CMediaPropertyFrame::EnableEdit()
{
	m_pOneMediaFrame->setEnabled(true);
	m_pMutilMediaFrame->setEnabled(true);
}

/**
 * 统一控制特效编辑区 enable：
 * - 重置按钮：仅受 bEnabled（图层 lock）影响
 * - 关键帧栏 / 滑条 / SpinBox：需 bEnabled 且游标落在当前 clip 内
 */
void CMediaPropertyFrame::SetPropertyActionBarsEnabled(bool bEnabled, std::optional<std::chrono::milliseconds> playheadMs)
{
	bool bKeyframeEnabled = bEnabled;
	if (bEnabled && m_setMediaIDs.size() == 1)
		bKeyframeEnabled = CMediaKeyframeHelper::IsPlayheadOnObject(*m_setMediaIDs.begin(), playheadMs);

	auto setGroupHeader = [bEnabled, bKeyframeEnabled](CPropertyResetBtn* pReset, CPropertyKeyFrameBar* pGroupKf)
	{
		if (pReset) pReset->SetEnabled(bEnabled);
		if (pGroupKf) pGroupKf->SetEnabled(bKeyframeEnabled);
	};
	auto setRowKf = [bKeyframeEnabled](CPropertyKeyFrameBar* pBar)
	{
		if (pBar) pBar->SetEnabled(bKeyframeEnabled);
	};
	auto setEditorEnabled = [bKeyframeEnabled](QWidget* pWidget) { if (pWidget) pWidget->setEnabled(bKeyframeEnabled); };
	auto setSliderEnabled = [bKeyframeEnabled](CSliderWidget* pSlider) { if (pSlider) pSlider->setEnabled(bKeyframeEnabled); };
	auto setSpinEnabled = [bKeyframeEnabled](CDoubleSpinBoxWidget* pSpin) { if (pSpin) pSpin->setEnabled(bKeyframeEnabled); };

	setGroupHeader(m_pVolumeResetBtn, m_pVolumeGroupKeyFrameBar);
	setGroupHeader(m_pPositionResetBtn, m_pPositionGroupKeyFrameBar);
	setGroupHeader(m_pScaleResetBtn, m_pScaleGroupKeyFrameBar);
	setGroupHeader(m_pRotationResetBtn, m_pRotationGroupKeyFrameBar);
	setGroupHeader(m_pCropResetBtn, m_pCropGroupKeyFrameBar);
	setGroupHeader(m_pFeatherResetBtn, m_pFeatherGroupKeyFrameBar);
	setGroupHeader(m_pKeyingResetBtn, m_pKeyingGroupKeyFrameBar);
	setGroupHeader(m_pEffectResetBtn, m_pEffectGroupKeyFrameBar);
	setRowKf(m_pKfVolume);
	setRowKf(m_pKfPosX); setRowKf(m_pKfPosY); setRowKf(m_pKfPosZ);
	setRowKf(m_pKfScaleWidth); setRowKf(m_pKfScaleHeight);
	setRowKf(m_pKfRotX); setRowKf(m_pKfRotY); setRowKf(m_pKfRotZ);
	setRowKf(m_pKfCropLeft); setRowKf(m_pKfCropRight);
	setRowKf(m_pKfCropTop); setRowKf(m_pKfCropBottom);
	setRowKf(m_pKfFeatherRadius);
	setRowKf(m_pKfKeyingRed); setRowKf(m_pKfKeyingGreen); setRowKf(m_pKfKeyingBlue);
	setRowKf(m_pKfKeyingTolerance); setRowKf(m_pKfKeyingDegree);
	setRowKf(m_pKfEffectRed); setRowKf(m_pKfEffectGreen); setRowKf(m_pKfEffectBlue);
	setRowKf(m_pKfEffectAlpha); setRowKf(m_pKfEffectLuminance); setRowKf(m_pKfEffectContrast);
	setRowKf(m_pKfEffectHue); setRowKf(m_pKfEffectSaturation);
	setSliderEnabled(m_pVolume);
	setSpinEnabled(m_pX); setSpinEnabled(m_pY); setSpinEnabled(m_pZ);
	setSpinEnabled(m_pWidth); setSpinEnabled(m_pHeight);
	if (m_pScaleAspectLockBtn)
		m_pScaleAspectLockBtn->setEnabled(bEnabled);
	setSpinEnabled(m_pRotX); setSpinEnabled(m_pRotY); setSpinEnabled(m_pRotZ);
	setEditorEnabled(m_pChkCropFill);
	setSliderEnabled(m_pCropLeft); setSliderEnabled(m_pCropRight);
	setSliderEnabled(m_pCropTop); setSliderEnabled(m_pCropBottom);
	setSliderEnabled(m_pFeatherRadius);
	setSliderEnabled(m_pKeyingRed); setSliderEnabled(m_pKeyingGreen); setSliderEnabled(m_pKeyingBlue);
	setSliderEnabled(m_pKeyingTolerance); setSliderEnabled(m_pKeyingDegree);
	setSliderEnabled(m_pLuminance); setSliderEnabled(m_pRed); setSliderEnabled(m_pGreen);
	setSliderEnabled(m_pBlue); setSliderEnabled(m_pAlpha); setSliderEnabled(m_pContract);
	setSliderEnabled(m_pHue); setSliderEnabled(m_pSaturability);

	// 位置 / 缩放 / 旋转 / 裁剪：隐藏关键帧栏，滑条与编辑框可编辑但不打关键帧
	DisableTemporarilyUnsupportedPropertyGroups();
}

void CMediaPropertyFrame::DisableTemporarilyUnsupportedPropertyGroups()
{
	// 阶段3：位置/缩放/旋转/裁剪关键帧已恢复，保留空实现以免改调用点
}

void CMediaPropertyFrame::ApplyPropertyEditAtPlayhead(EMediaPropertyId propId, float value)
{
	if (m_setMediaIDs.size() != 1) return;
	const QUuid mediaId = *m_setMediaIDs.begin();

	// 位置 / 缩放 / 旋转 / 裁剪：只写模型（裁剪写 FX 因子），不新增/更新关键帧
	if (IsTemporarilyNoKeyframeProperty(propId))
	{
		CMediaKeyframeHelper::EnsureFxFactorForProperty(mediaId, propId, value, this);
		emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_TimeLineWnd, SignalCenter::EDDT_MediaNodeDataChange);
		emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_SliceWnd, SignalCenter::EDDT_MediaNodeDataChange);
		emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
		return;
	}

	const auto playheadMs = g_pPlayerController
		? std::optional<std::chrono::milliseconds>(g_pPlayerController->GetCurTime())
		: std::nullopt;
	// 游标在 clip 内：已有关键帧则更新，否则在游标处新增；并通知 ClipFx Plot 刷新
	if (!CMediaKeyframeHelper::ApplyPropertyValueAtPlayhead(mediaId, propId, value, this, playheadMs)) return;
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, propId, FindPropertyKeyFrameBar(propId), playheadMs);
}

CPropertyKeyFrameBar* CMediaPropertyFrame::FindPropertyKeyFrameBar(EMediaPropertyId propId) const
{
	switch (propId)
	{
	case EMediaPropertyId::VolumeIntensity: return m_pKfVolume;
	case EMediaPropertyId::PosX: return m_pKfPosX;
	case EMediaPropertyId::PosY: return m_pKfPosY;
	case EMediaPropertyId::PosZ: return m_pKfPosZ;
	case EMediaPropertyId::ScaleWidth: return m_pKfScaleWidth;
	case EMediaPropertyId::ScaleHeight: return m_pKfScaleHeight;
	case EMediaPropertyId::RotX: return m_pKfRotX;
	case EMediaPropertyId::RotY: return m_pKfRotY;
	case EMediaPropertyId::RotZ: return m_pKfRotZ;
	case EMediaPropertyId::CropFill: return nullptr;
	case EMediaPropertyId::CropLeft: return m_pKfCropLeft;
	case EMediaPropertyId::CropRight: return m_pKfCropRight;
	case EMediaPropertyId::CropTop: return m_pKfCropTop;
	case EMediaPropertyId::CropBottom: return m_pKfCropBottom;
	case EMediaPropertyId::FeatherRadius: return m_pKfFeatherRadius;
	case EMediaPropertyId::KeyingRed: return m_pKfKeyingRed;
	case EMediaPropertyId::KeyingGreen: return m_pKfKeyingGreen;
	case EMediaPropertyId::KeyingBlue: return m_pKfKeyingBlue;
	case EMediaPropertyId::KeyingTolerance: return m_pKfKeyingTolerance;
	case EMediaPropertyId::KeyingDegree: return m_pKfKeyingDegree;
	case EMediaPropertyId::EffectRed: return m_pKfEffectRed;
	case EMediaPropertyId::EffectGreen: return m_pKfEffectGreen;
	case EMediaPropertyId::EffectBlue: return m_pKfEffectBlue;
	case EMediaPropertyId::EffectAlpha: return m_pKfEffectAlpha;
	case EMediaPropertyId::EffectLuminance: return m_pKfEffectLuminance;
	case EMediaPropertyId::EffectContrast: return m_pKfEffectContrast;
	case EMediaPropertyId::EffectHue: return m_pKfEffectHue;
	case EMediaPropertyId::EffectSaturation: return m_pKfEffectSaturation;
	default: return nullptr;
	}
}

float CMediaPropertyFrame::GetPropertyKeyframeValue(EMediaPropertyId propId) const
{
	switch (propId)
	{
	case EMediaPropertyId::VolumeIntensity: return (float)m_pVolume->GetValue();
	case EMediaPropertyId::PosX: return (float)m_pX->GetValue();
	case EMediaPropertyId::PosY: return (float)m_pY->GetValue();
	case EMediaPropertyId::PosZ: return m_fPosZ;
	case EMediaPropertyId::ScaleWidth: return (float)m_pWidth->GetValue();
	case EMediaPropertyId::ScaleHeight: return (float)m_pHeight->GetValue();
	case EMediaPropertyId::RotX: return m_fRotX;
	case EMediaPropertyId::RotY: return m_fRotY;
	case EMediaPropertyId::RotZ: return (float)m_pRotZ->GetValue();
	case EMediaPropertyId::CropFill: return m_bCropFill ? 1.f : 0.f;
	case EMediaPropertyId::CropLeft: return m_fCropLeft;
	case EMediaPropertyId::CropRight: return m_fCropRight;
	case EMediaPropertyId::CropTop: return m_fCropTop;
	case EMediaPropertyId::CropBottom: return m_fCropBottom;
	case EMediaPropertyId::FeatherRadius: return m_fFeatherRadius;
	case EMediaPropertyId::KeyingRed: return m_fKeyingRed;
	case EMediaPropertyId::KeyingGreen: return m_fKeyingGreen;
	case EMediaPropertyId::KeyingBlue: return m_fKeyingBlue;
	case EMediaPropertyId::KeyingTolerance: return m_fKeyingTolerance;
	case EMediaPropertyId::KeyingDegree: return m_fKeyingDegree;
	case EMediaPropertyId::EffectRed: return (float)m_pRed->GetValue();
	case EMediaPropertyId::EffectGreen: return (float)m_pGreen->GetValue();
	case EMediaPropertyId::EffectBlue: return (float)m_pBlue->GetValue();
	case EMediaPropertyId::EffectAlpha: return (float)m_pAlpha->GetValue();
	case EMediaPropertyId::EffectLuminance: return (float)m_pLuminance->GetValue();
	case EMediaPropertyId::EffectContrast: return (float)m_pContract->GetValue();
	case EMediaPropertyId::EffectHue: return (float)m_pHue->GetValue();
	case EMediaPropertyId::EffectSaturation: return (float)m_pSaturability->GetValue();
	default: return 0.f;
	}
}

void CMediaPropertyFrame::SetPropertyDisplayValue(EMediaPropertyId propId, float value)
{
	switch (propId)
	{
	case EMediaPropertyId::VolumeIntensity: { QSignalBlocker b(m_pVolume); m_pVolume->SetValue(value); break; }
	case EMediaPropertyId::PosX: { QSignalBlocker b(m_pX); m_pX->SetValue(value); break; }
	case EMediaPropertyId::PosY: { QSignalBlocker b(m_pY); m_pY->SetValue(value); break; }
	case EMediaPropertyId::PosZ: m_fPosZ = value; { QSignalBlocker b(m_pZ); m_pZ->SetValue(value); break; }
	case EMediaPropertyId::ScaleWidth: { QSignalBlocker b(m_pWidth); m_pWidth->SetValue(value); break; }
	case EMediaPropertyId::ScaleHeight: { QSignalBlocker b(m_pHeight); m_pHeight->SetValue(value); break; }
	case EMediaPropertyId::RotX: m_fRotX = value; { QSignalBlocker b(m_pRotX); m_pRotX->SetValue(value); break; }
	case EMediaPropertyId::RotY: m_fRotY = value; { QSignalBlocker b(m_pRotY); m_pRotY->SetValue(value); break; }
	case EMediaPropertyId::RotZ: { QSignalBlocker b(m_pRotZ); m_pRotZ->SetValue(value); break; }
	case EMediaPropertyId::CropFill: m_bCropFill = value >= 0.5f; { QSignalBlocker b(m_pChkCropFill); m_pChkCropFill->setChecked(m_bCropFill); break; }
	case EMediaPropertyId::CropLeft: m_fCropLeft = value; { QSignalBlocker b(m_pCropLeft); m_pCropLeft->SetValue(value); break; }
	case EMediaPropertyId::CropRight: m_fCropRight = value; { QSignalBlocker b(m_pCropRight); m_pCropRight->SetValue(value); break; }
	case EMediaPropertyId::CropTop: m_fCropTop = value; { QSignalBlocker b(m_pCropTop); m_pCropTop->SetValue(value); break; }
	case EMediaPropertyId::CropBottom: m_fCropBottom = value; { QSignalBlocker b(m_pCropBottom); m_pCropBottom->SetValue(value); break; }
	case EMediaPropertyId::FeatherRadius: m_fFeatherRadius = value; { QSignalBlocker b(m_pFeatherRadius); m_pFeatherRadius->SetValue(value); break; }
	case EMediaPropertyId::KeyingRed: m_fKeyingRed = value; { QSignalBlocker b(m_pKeyingRed); m_pKeyingRed->SetValue(value); break; }
	case EMediaPropertyId::KeyingGreen: m_fKeyingGreen = value; { QSignalBlocker b(m_pKeyingGreen); m_pKeyingGreen->SetValue(value); break; }
	case EMediaPropertyId::KeyingBlue: m_fKeyingBlue = value; { QSignalBlocker b(m_pKeyingBlue); m_pKeyingBlue->SetValue(value); break; }
	case EMediaPropertyId::KeyingTolerance: m_fKeyingTolerance = value; { QSignalBlocker b(m_pKeyingTolerance); m_pKeyingTolerance->SetValue(value); break; }
	case EMediaPropertyId::KeyingDegree: m_fKeyingDegree = value; { QSignalBlocker b(m_pKeyingDegree); m_pKeyingDegree->SetValue(value); break; }
	case EMediaPropertyId::EffectRed: { QSignalBlocker b(m_pRed); m_pRed->SetValue(value); break; }
	case EMediaPropertyId::EffectGreen: { QSignalBlocker b(m_pGreen); m_pGreen->SetValue(value); break; }
	case EMediaPropertyId::EffectBlue: { QSignalBlocker b(m_pBlue); m_pBlue->SetValue(value); break; }
	case EMediaPropertyId::EffectAlpha: { QSignalBlocker b(m_pAlpha); m_pAlpha->SetValue(value); break; }
	case EMediaPropertyId::EffectLuminance: { QSignalBlocker b(m_pLuminance); m_pLuminance->SetValue(value); break; }
	case EMediaPropertyId::EffectContrast: { QSignalBlocker b(m_pContract); m_pContract->SetValue(value); break; }
	case EMediaPropertyId::EffectHue: { QSignalBlocker b(m_pHue); m_pHue->SetValue(value); break; }
	case EMediaPropertyId::EffectSaturation: { QSignalBlocker b(m_pSaturability); m_pSaturability->SetValue(value); break; }
	default: break;
	}
}

void CMediaPropertyFrame::RefreshPropertyKeyframeToggleStates(std::optional<std::chrono::milliseconds> playheadMs)
{
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, EMediaPropertyId::VolumeIntensity, m_pKfVolume, playheadMs);
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, EMediaPropertyId::PosX, m_pKfPosX, playheadMs);
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, EMediaPropertyId::PosY, m_pKfPosY, playheadMs);
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, EMediaPropertyId::PosZ, m_pKfPosZ, playheadMs);
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, EMediaPropertyId::ScaleWidth, m_pKfScaleWidth, playheadMs);
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, EMediaPropertyId::ScaleHeight, m_pKfScaleHeight, playheadMs);
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, EMediaPropertyId::RotX, m_pKfRotX, playheadMs);
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, EMediaPropertyId::RotY, m_pKfRotY, playheadMs);
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, EMediaPropertyId::RotZ, m_pKfRotZ, playheadMs);
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, EMediaPropertyId::CropLeft, m_pKfCropLeft, playheadMs);
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, EMediaPropertyId::CropRight, m_pKfCropRight, playheadMs);
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, EMediaPropertyId::CropTop, m_pKfCropTop, playheadMs);
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, EMediaPropertyId::CropBottom, m_pKfCropBottom, playheadMs);
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, EMediaPropertyId::FeatherRadius, m_pKfFeatherRadius, playheadMs);
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, EMediaPropertyId::KeyingRed, m_pKfKeyingRed, playheadMs);
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, EMediaPropertyId::KeyingGreen, m_pKfKeyingGreen, playheadMs);
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, EMediaPropertyId::KeyingBlue, m_pKfKeyingBlue, playheadMs);
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, EMediaPropertyId::KeyingTolerance, m_pKfKeyingTolerance, playheadMs);
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, EMediaPropertyId::KeyingDegree, m_pKfKeyingDegree, playheadMs);
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, EMediaPropertyId::EffectRed, m_pKfEffectRed, playheadMs);
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, EMediaPropertyId::EffectGreen, m_pKfEffectGreen, playheadMs);
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, EMediaPropertyId::EffectBlue, m_pKfEffectBlue, playheadMs);
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, EMediaPropertyId::EffectAlpha, m_pKfEffectAlpha, playheadMs);
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, EMediaPropertyId::EffectLuminance, m_pKfEffectLuminance, playheadMs);
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, EMediaPropertyId::EffectContrast, m_pKfEffectContrast, playheadMs);
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, EMediaPropertyId::EffectHue, m_pKfEffectHue, playheadMs);
	CMediaKeyframeHelper::RefreshKeyframeToggleState(this, EMediaPropertyId::EffectSaturation, m_pKfEffectSaturation, playheadMs);

	// 分组标题行关键帧栏（Toggle 表示组内任一属性在当前游标处有关键帧）
	CMediaKeyframeHelper::RefreshGroupKeyframeToggleState(this, EMediaPropertyGroupId::Volume, m_pVolumeGroupKeyFrameBar, playheadMs);
	CMediaKeyframeHelper::RefreshGroupKeyframeToggleState(this, EMediaPropertyGroupId::Position, m_pPositionGroupKeyFrameBar, playheadMs);
	CMediaKeyframeHelper::RefreshGroupKeyframeToggleState(this, EMediaPropertyGroupId::Scale, m_pScaleGroupKeyFrameBar, playheadMs);
	CMediaKeyframeHelper::RefreshGroupKeyframeToggleState(this, EMediaPropertyGroupId::Rotation, m_pRotationGroupKeyFrameBar, playheadMs);
	CMediaKeyframeHelper::RefreshGroupKeyframeToggleState(this, EMediaPropertyGroupId::Crop, m_pCropGroupKeyFrameBar, playheadMs);
	CMediaKeyframeHelper::RefreshGroupKeyframeToggleState(this, EMediaPropertyGroupId::Feather, m_pFeatherGroupKeyFrameBar, playheadMs);
	CMediaKeyframeHelper::RefreshGroupKeyframeToggleState(this, EMediaPropertyGroupId::Keying, m_pKeyingGroupKeyFrameBar, playheadMs);
	CMediaKeyframeHelper::RefreshGroupKeyframeToggleState(this, EMediaPropertyGroupId::Effect, m_pEffectGroupKeyFrameBar, playheadMs);

	// RefreshKeyframeToggleState 可能把禁用组 KF 栏重新打开，此处再压一次
	DisableTemporarilyUnsupportedPropertyGroups();
}

void CMediaPropertyFrame::SyncKeyframeEvaluatedValuesAtPlayhead(std::optional<std::chrono::milliseconds> playheadMs)
{
	if (m_setMediaIDs.size() != 1) return;
	CMediaKeyframeHelper::SyncKeyframeEvaluatedValuesAtPlayhead(this, *m_setMediaIDs.begin(), playheadMs);
}

void CMediaPropertyFrame::ResetPropertyGroup(EMediaPropertyGroupId groupId)
{
	if (m_setMediaIDs.size() != 1)
		return;
	const QUuid mediaId = *m_setMediaIDs.begin();
	if (!CMediaKeyframeHelper::ResetPropertyGroup(mediaId, groupId, this))
		return;
	HistoryStackSave();
	// UpdateProperty refreshes enable/toggle/sync from model; keep
	UpdateProperty();
}
void CMediaPropertyFrame::UpdateTurnPageScrollCtrl(bool bSelectPreIndex)
{
	Q_UNUSED(bSelectPreIndex);
	if (m_setMediaIDs.size() != 1)
		return;

	auto mediaNode = g_pMediaModel->GetMediaNode(*m_setMediaIDs.begin()).lock();
	if (!mediaNode)
		return;

	int resType = mediaNode->GetResourceType();
	if (auto pRes = g_pResourceModel->GetResourceNode(mediaNode->GetResourceId()))
		resType = pRes->GetType();

	const bool bFill = IsSupportFillMode(resType);
	const bool bScroll = IsSupportScroll(resType);

	if (m_pComboFillType && bFill && m_pComboFillType->Count() == 0)
	{
		QStringList fillItems;
		fillItems << CPropertyTranslate::FullWindow()
			<< CPropertyTranslate::KeepRatio()
			<< CPropertyTranslate::NoScale();
		m_pComboFillType->AddItems(fillItems, false);
	}

	if (m_pFrameScrollType)
		m_pFrameScrollType->setVisible(bScroll);

	if (bScroll && m_pComboTextScrollType && m_pComboTextScrollType->Count() == 0)
		m_pComboTextScrollType->AddItems(CTextPropertyFrameTranslate::ScrollModelItems(), false);

	int nScrollType = EST_Stop;
	if (auto pShare = mediaNode->GetShareProperty())
		if (auto pParam = pShare->GetScrollParam())
			nScrollType = pParam->nScrollType;

	const bool bScrollMoving = bScroll && nScrollType != EST_Stop;
	if (m_pFrameScrollSpeed)
		m_pFrameScrollSpeed->setVisible(bScrollMoving);
	if (m_pScrollPtyContainer)
	{
		if (bScrollMoving)
			m_pScrollPtyContainer->show();
		else
			m_pScrollPtyContainer->hide();
	}

	UpdateGroupSeparatorVisibility();
}

void CMediaPropertyFrame::UpdateGroupSeparatorVisibility()
{
	auto isVisibleGroup = [](CGroupWidgetBox* pGroup) {
		return pGroup && pGroup->isVisible();
	};

	auto hasAnyVisible = [&](std::initializer_list<CGroupWidgetBox*> groups) {
		for (auto* pGroup : groups)
		{
			if (isVisibleGroup(pGroup))
				return true;
		}
		return false;
	};

	if (m_pStyleSplitLine)
	{
		bool bStyleVisible = m_pFrameStyleProperty && m_pFrameStyleProperty->isVisible();
		if (bStyleVisible)
		{
			bStyleVisible = (m_pComboFillType && m_pComboFillType->isVisible())
				|| (m_pFrameScrollType && m_pFrameScrollType->isVisible())
				|| (m_pFrameScrollSpeed && m_pFrameScrollSpeed->isVisible())
				|| (m_pScrollPtyContainer && m_pScrollPtyContainer->isVisible());
		}
		m_pStyleSplitLine->setVisible(bStyleVisible);
	}

	if (m_pGroupSeparators[0])
	{
		m_pGroupSeparators[0]->setVisible(isVisibleGroup(m_pBasePropertyGroup)
			&& isVisibleGroup(m_pAudioPropertyGroup));
	}

	if (m_pGroupSeparators[1])
	{
		const bool bHasFxBelowAudio = hasAnyVisible({
			m_pVolumePropertyGroup, m_pPositionPropertyGroup, m_pScalePropertyGroup,
			m_pRotationPropertyGroup, m_pCropPropertyGroup, m_pFeatherPropertyGroup,
			m_pKeyingPropertyGroup, m_pEffectPropertyGroup });
		m_pGroupSeparators[1]->setVisible(isVisibleGroup(m_pAudioPropertyGroup) && bHasFxBelowAudio);
	}
}

void CMediaPropertyFrame::slotDoubleSpinBoxEdit(float fValue)
{
	if (m_setMediaIDs.size() != 1) return;
	const QUuid mediaId = *m_setMediaIDs.begin();
	auto mediaNode = g_pMediaModel->GetMediaNode(mediaId).lock();
	if (!mediaNode) return;
	QObject* pSpin = sender();
	EMediaPropertyId propId = EMediaPropertyId::PosX;
	if (pSpin == m_pX) { mediaNode->SetMediaPos(fValue, mediaNode->GetMediaPos().y); propId = EMediaPropertyId::PosX; }
	else if (pSpin == m_pY) { mediaNode->SetMediaPos(mediaNode->GetMediaPos().x, fValue); propId = EMediaPropertyId::PosY; }
	else if (pSpin == m_pZ) { m_fPosZ = fValue; propId = EMediaPropertyId::PosZ; }
	else if (pSpin == m_pWidth || pSpin == m_pHeight)
	{
		const bool bWidth = (pSpin == m_pWidth);
		propId = bWidth ? EMediaPropertyId::ScaleWidth : EMediaPropertyId::ScaleHeight;
		if (bWidth)
			mediaNode->SetMediaWidth(fValue);
		else
			mediaNode->SetMediaHeight(fValue);

		if (CMediaKeyframeHelper::IsScaleAspectLocked(mediaId))
		{
			const float pairedVal = CMediaKeyframeHelper::ComputePairedScaleValue(mediaId, propId, fValue);
			const EMediaPropertyId pairedId = CMediaKeyframeHelper::GetPairedScalePropertyId(propId);
			if (pairedId == EMediaPropertyId::ScaleWidth)
			{
				mediaNode->SetMediaWidth(pairedVal);
				QSignalBlocker b(m_pWidth);
				m_pWidth->SetValue(pairedVal);
			}
			else
			{
				mediaNode->SetMediaHeight(pairedVal);
				QSignalBlocker b(m_pHeight);
				m_pHeight->SetValue(pairedVal);
			}
			ApplyPropertyEditAtPlayhead(propId, fValue);
			ApplyPropertyEditAtPlayhead(pairedId, pairedVal);
			HistoryStackSave();
			emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_SliceWnd, SignalCenter::EDDT_MediaNodeDataChange);
			return;
		}
	}
	else if (pSpin == m_pRotX) { m_fRotX = fValue; propId = EMediaPropertyId::RotX; }
	else if (pSpin == m_pRotY) { m_fRotY = fValue; propId = EMediaPropertyId::RotY; }
	else if (pSpin == m_pRotZ) { mediaNode->SetMediaRotateAngle(fValue); propId = EMediaPropertyId::RotZ; }
	else return;
	ApplyPropertyEditAtPlayhead(propId, fValue);
	HistoryStackSave();
	emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_SliceWnd, SignalCenter::EDDT_MediaNodeDataChange);
}

void CMediaPropertyFrame::slotScaleAspectLockClicked()
{
	if (m_setMediaIDs.size() != 1 || !m_pScaleAspectLockBtn)
		return;
	auto mediaNode = g_pMediaModel->GetMediaNode(*m_setMediaIDs.begin()).lock();
	if (!mediaNode || !mediaNode->GetBaseProperty())
		return;

	const bool bLocked = m_pScaleAspectLockBtn->isChecked();
	mediaNode->GetBaseProperty()->SetKeepAspectRatio(bLocked);
	// 锁定按下时用当前宽高写入比例基准；解锁不改当前 W/H
	if (bLocked)
	{
		mediaNode->GetBaseProperty()->SetSizeKeepAspectRatio(
			QSizeF(mediaNode->GetMediaWidth(), mediaNode->GetMediaHeight()));
	}
	m_pScaleAspectLockBtn->setToolTip(bLocked
		? CPropertyTranslate::ScaleAspectUnlock()
		: CPropertyTranslate::ScaleAspectLock());
	HistoryStackSave();
}

void CMediaPropertyFrame::slotLineEdit(const QString& text) { Q_UNUSED(text); }
void CMediaPropertyFrame::slotTimeEdit() {}
void CMediaPropertyFrame::slotTimeEditFinish()
{
	QObject* pSender = sender();
	if (m_setMediaIDs.empty())
	{
		HistoryStackSave();
		UpdateProperty();
		return;
	}

	if (pSender == m_pPlayStart || pSender == m_pPlayDuration)
	{
		auto pObject = g_pTimeLineModel->GetTimeLineObject(*m_setMediaIDs.begin()).lock();
		if (pObject)
		{
			auto newStart = m_msOldStartTime;
			auto newDuration = m_msOldDuration;
			if (pSender == m_pPlayStart)
			{
				newStart = PropertyTimeHelper::CommitTimelineMs(m_pPlayStart);
				newDuration = m_msOldDuration;
			}
			else
			{
				newStart = m_msOldStartTime;
				newDuration = PropertyTimeHelper::CommitTimelineMs(m_pPlayDuration);
			}
			if (newStart < std::chrono::milliseconds(0))
				newStart = std::chrono::milliseconds(0);
			if (newDuration < std::chrono::milliseconds(1))
				newDuration = std::chrono::milliseconds(1);
			if (newStart != m_msOldStartTime)
				pObject->SetStartTime(newStart);
			if (newDuration != m_msOldDuration)
				pObject->SetDuration(newDuration);
			emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_TimeLineWnd, SignalCenter::EDDT_All);
		}
	}
	else if (pSender == m_pClipBegin || pSender == m_pClipEnd)
	{
		auto mediaNode = g_pMediaModel->GetMediaNode(*m_setMediaIDs.begin()).lock();
		if (mediaNode)
		{
			if (pSender == m_pClipBegin)
			{
				auto clipBegin = PropertyTimeHelper::CommitClipMs(m_pClipBegin);
				mediaNode->SetClipStartTime(clipBegin);
			}
			else
			{
				auto clipEnd = PropertyTimeHelper::CommitClipMs(m_pClipEnd);
				mediaNode->SetClipEndTime(clipEnd);
			}
			emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_TimeLineWnd, SignalCenter::EDDT_All);
		}
	}
	else if (pSender == m_pMutilMediaPlayStart || pSender == m_pMutilMediaPlayDuration)
	{
		std::chrono::milliseconds minTime = g_pTimeLineModel->GetDuration();
		std::chrono::milliseconds maxTime = std::chrono::milliseconds(0);
		for (const auto& id : m_setMediaIDs)
		{
			auto pObject = g_pTimeLineModel->GetTimeLineObject(id).lock();
			if (!pObject)
				continue;
			if (pObject->GetStartTime() < minTime)
				minTime = pObject->GetStartTime();
			if (pObject->GetStartTime() + pObject->GetDuration() > maxTime)
				maxTime = pObject->GetStartTime() + pObject->GetDuration();
		}
		const auto oldSpan = maxTime - minTime;
		if (pSender == m_pMutilMediaPlayStart)
		{
			const auto newMin = PropertyTimeHelper::CommitTimelineMs(m_pMutilMediaPlayStart);
			const auto delta = newMin - minTime;
			for (const auto& id : m_setMediaIDs)
			{
				auto pObject = g_pTimeLineModel->GetTimeLineObject(id).lock();
				if (!pObject)
					continue;
				auto layer = g_pTimeLineModel->GetLayer(id).lock();
				if (layer && layer->IsLock())
					continue;
				auto t = pObject->GetStartTime() + delta;
				if (t < std::chrono::milliseconds(0))
					t = std::chrono::milliseconds(0);
				pObject->SetStartTime(t);
			}
		}
		else if (oldSpan.count() > 0)
		{
			const auto newSpan = PropertyTimeHelper::CommitTimelineMs(m_pMutilMediaPlayDuration);
			const double scale = static_cast<double>(newSpan.count()) / static_cast<double>(oldSpan.count());
			for (const auto& id : m_setMediaIDs)
			{
				auto pObject = g_pTimeLineModel->GetTimeLineObject(id).lock();
				if (!pObject)
					continue;
				auto layer = g_pTimeLineModel->GetLayer(id).lock();
				if (layer && layer->IsLock())
					continue;
				const auto rel = pObject->GetStartTime() - minTime;
				const auto newStart = minTime + std::chrono::milliseconds(static_cast<long long>(rel.count() * scale));
				const auto newDur = std::chrono::milliseconds(static_cast<long long>(pObject->GetDuration().count() * scale));
				pObject->SetStartTime(newStart);
				if (newDur.count() > 0)
					pObject->SetDuration(newDur);
			}
		}
		emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_TimeLineWnd, SignalCenter::EDDT_All);
	}

	HistoryStackSave();
	UpdateProperty();
}

void CMediaPropertyFrame::slotEffectPropertyValueChange(double fValue)
{
	if (m_setMediaIDs.size() != 1) return;
	auto mediaNode = g_pMediaModel->GetMediaNode(*m_setMediaIDs.begin()).lock();
	if (!mediaNode || !mediaNode->GetEffectProperty()) return;
	QObject* pSendObj = sender();
	EMediaPropertyId propId = EMediaPropertyId::EffectLuminance;
	auto pEffect = mediaNode->GetEffectProperty();
	if (pSendObj == m_pLuminance) { pEffect->SetLuminance(fValue); propId = EMediaPropertyId::EffectLuminance; }
	else if (pSendObj == m_pRed) { pEffect->SetRed(fValue); propId = EMediaPropertyId::EffectRed; }
	else if (pSendObj == m_pGreen) { pEffect->SetGreen(fValue); propId = EMediaPropertyId::EffectGreen; }
	else if (pSendObj == m_pBlue) { pEffect->SetBlue(fValue); propId = EMediaPropertyId::EffectBlue; }
	else if (pSendObj == m_pAlpha) { pEffect->SetAlpha(fValue); propId = EMediaPropertyId::EffectAlpha; }
	else if (pSendObj == m_pContract) { pEffect->SetContrast(fValue); propId = EMediaPropertyId::EffectContrast; }
	else if (pSendObj == m_pHue) { pEffect->SetHue(fValue); propId = EMediaPropertyId::EffectHue; }
	else if (pSendObj == m_pSaturability) { pEffect->SetSaturability(fValue); propId = EMediaPropertyId::EffectSaturation; }
	else return;
	ApplyPropertyEditAtPlayhead(propId, (float)fValue);
	emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_SliceWnd, SignalCenter::EDDT_MediaNodeDataChange);
	emit g_pSignalCenter->signalUpdateMediaProperty(*m_setMediaIDs.begin());
}
void CMediaPropertyFrame::slotEffectPropertyValueChangeMouseRelease(double fValue) { Q_UNUSED(fValue); HistoryStackSave(); }
void CMediaPropertyFrame::slotFillComboSelectChange(int nIndex) { Q_UNUSED(nIndex); }
void CMediaPropertyFrame::slotComboTurnPageScrollTypeChange(int nIndex) { Q_UNUSED(nIndex); }
void CMediaPropertyFrame::slotTurnPageIntervalChange(int nValue) { Q_UNUSED(nValue); }
void CMediaPropertyFrame::slotEndToEndIntervalChange(int nValue) { Q_UNUSED(nValue); }
void CMediaPropertyFrame::slotEteBtnClicked() {}
void CMediaPropertyFrame::slotUpdateScrollProperty(bool b) { Q_UNUSED(b); }
void CMediaPropertyFrame::NotifyMediaAudioChannelMapChanged(const QUuid& mediaId)
{
	if (mediaId.isNull() || !g_pGSNetSignal)
		return;
	emit g_pGSNetSignal->signalModifyMediaAudioChannelMap(mediaId);
}

void CMediaPropertyFrame::NotifyMediaPropertyKeyframesChanged(const QUuid& mediaId)
{
	if (mediaId.isNull() || !g_pGSNetSignal)
		return;
	emit g_pGSNetSignal->signalModifyMediaPropertyKeyframes(mediaId);
}

void CMediaPropertyFrame::NotifyMediaVolumeChanged(const QUuid& mediaId)
{
	if (mediaId.isNull() || !g_pGSNetSignal)
		return;
	emit g_pGSNetSignal->signalModifyMediaVolume(mediaId);
}

void CMediaPropertyFrame::slotAudioChannelMapCheckChanged()
{
	if (m_setMediaIDs.size() != 1)
		return;
	auto mediaNode = g_pMediaModel->GetMediaNode(*m_setMediaIDs.begin()).lock();
	if (!mediaNode)
		return;
	auto pShare = mediaNode->GetShareProperty();
	if (!pShare)
		return;

	const bool bChecked = m_pChkAudioChannelMap->isChecked();
	pShare->SetUsedChannelMap(bChecked);
	mediaNode->SetUsedChannelMap(bChecked);
	for (auto* pCombo : m_vecChannelCombobox)
	{
		if (pCombo && pCombo->isVisible())
			pCombo->setEnabled(bChecked);
	}
	NotifyMediaAudioChannelMapChanged(mediaNode->GetMediaId());
	HistoryStackSave();
}

void CMediaPropertyFrame::slotAudioChannelChanged(int nIndex)
{
	CComboBoxWidget* pSender = dynamic_cast<CComboBoxWidget*>(sender());
	if (!pSender)
		return;
	if (m_setMediaIDs.size() != 1)
		return;
	auto mediaNode = g_pMediaModel->GetMediaNode(*m_setMediaIDs.begin()).lock();
	if (!mediaNode)
		return;
	auto pShare = mediaNode->GetShareProperty();
	if (!pShare)
		return;

	const int outCh = pSender->property("ChannelSystem").toInt();
	if (outCh < 0 || outCh >= CHANNEL_SYSTEM_MAX)
		return;

	SChannelMap channelMap = mediaNode->GetAudioChannelMap();
	channelMap.map[outCh] = static_cast<unsigned>(nIndex);
	mediaNode->SetAudioChannelMap(channelMap);
	pShare->SetAudioChannelMap(channelMap);
	NotifyMediaAudioChannelMapChanged(mediaNode->GetMediaId());
	HistoryStackSave();
}

void CMediaPropertyFrame::slotCropSliderChange(double fValue)
{
	QObject* pSender = sender();
	EMediaPropertyId propId = EMediaPropertyId::CropLeft;
	if (pSender == m_pCropLeft) { m_fCropLeft = (float)fValue; propId = EMediaPropertyId::CropLeft; }
	else if (pSender == m_pCropRight) { m_fCropRight = (float)fValue; propId = EMediaPropertyId::CropRight; }
	else if (pSender == m_pCropTop) { m_fCropTop = (float)fValue; propId = EMediaPropertyId::CropTop; }
	else if (pSender == m_pCropBottom) { m_fCropBottom = (float)fValue; propId = EMediaPropertyId::CropBottom; }
	else return;
	ApplyPropertyEditAtPlayhead(propId, (float)fValue);
}
void CMediaPropertyFrame::slotFeatherSliderChange(double fValue)
{
	m_fFeatherRadius = (float)fValue;
	ApplyPropertyEditAtPlayhead(EMediaPropertyId::FeatherRadius, (float)fValue);
}
void CMediaPropertyFrame::slotKeyingSliderChange(double fValue)
{
	QObject* pSender = sender();
	EMediaPropertyId propId = EMediaPropertyId::KeyingRed;
	if (pSender == m_pKeyingRed) { m_fKeyingRed = (float)fValue; propId = EMediaPropertyId::KeyingRed; }
	else if (pSender == m_pKeyingGreen) { m_fKeyingGreen = (float)fValue; propId = EMediaPropertyId::KeyingGreen; }
	else if (pSender == m_pKeyingBlue) { m_fKeyingBlue = (float)fValue; propId = EMediaPropertyId::KeyingBlue; }
	else if (pSender == m_pKeyingTolerance) { m_fKeyingTolerance = (float)fValue; propId = EMediaPropertyId::KeyingTolerance; }
	else if (pSender == m_pKeyingDegree) { m_fKeyingDegree = (float)fValue; propId = EMediaPropertyId::KeyingDegree; }
	else return;
	ApplyPropertyEditAtPlayhead(propId, (float)fValue);
}
void CMediaPropertyFrame::slotPropertyKeyFrameStub(EMediaPropertyId propId, EKeyFrameOp op)
{
	if (m_setMediaIDs.size() != 1) return;
	// 位置 / 缩放 / 旋转 / 裁剪：关键帧栏已隐藏，忽略操作
	if (IsTemporarilyNoKeyframeProperty(propId))
		return;
	const QUuid mediaId = *m_setMediaIDs.begin();
	if (op == EKeyFrameOp::Toggle)
	{
		if (CMediaKeyframeHelper::TogglePropertyKeyframe(mediaId, propId, GetPropertyKeyframeValue(propId), this))
		{
			RefreshPropertyKeyframeToggleStates();
			HistoryStackSave();
		}
		return;
	}

	std::optional<std::chrono::milliseconds> targetClipLocalMs;
	if (!CMediaKeyframeHelper::NavigatePropertyKeyframe(mediaId, propId, op, &targetClipLocalMs))
		return;

	// signalNoniusTimeChanged 已同步刷新；此处用跳转目标时间再刷一次，避免 PlayerController 滞后覆盖正确状态
	if (targetClipLocalMs)
	{
		auto pObject = CMediaKeyframeHelper::GetTimelineObject(mediaId);
		if (pObject)
			RefreshPropertyKeyframeToggleStates(pObject->GetStartTime() + *targetClipLocalMs);
	}
}
void CMediaPropertyFrame::slotGroupKeyFrameStub(EMediaPropertyGroupId groupId, EKeyFrameOp op)
{
	if (m_setMediaIDs.size() != 1) return;
	// 位置 / 缩放 / 旋转 / 裁剪：分组关键帧栏已隐藏，忽略操作
	if (groupId == EMediaPropertyGroupId::Position
		|| groupId == EMediaPropertyGroupId::Scale
		|| groupId == EMediaPropertyGroupId::Rotation
		|| groupId == EMediaPropertyGroupId::Crop)
		return;
	const QUuid mediaId = *m_setMediaIDs.begin();
	const auto props = CMediaKeyframeHelper::GetGroupPropertyIds(groupId);
	if (props.empty()) return;
	if (op == EKeyFrameOp::Toggle)
	{
		// Group toggle: delete all if complete; otherwise add only missing props
		if (CMediaKeyframeHelper::ToggleGroupKeyframes(mediaId, groupId, this))
		{
			RefreshPropertyKeyframeToggleStates();
			SyncKeyframeEvaluatedValuesAtPlayhead();
			HistoryStackSave();
		}
		return;
	}

	std::optional<std::chrono::milliseconds> targetClipLocalMs;
	if (!CMediaKeyframeHelper::NavigatePropertyKeyframe(mediaId, props.front(), op, &targetClipLocalMs))
		return;

	if (targetClipLocalMs)
	{
		auto pObject = CMediaKeyframeHelper::GetTimelineObject(mediaId);
		if (pObject)
			RefreshPropertyKeyframeToggleStates(pObject->GetStartTime() + *targetClipLocalMs);
	}
}