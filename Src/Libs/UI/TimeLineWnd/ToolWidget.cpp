#define _USE_MATH_DEFINES		//解决qmath.h和math.h的头文件冲突
#include "ToolWidget.h"
#include <algorithm>
#include "GSMedia/ThreadManager/ThreadManagerExport.h"
#include "Controll/SelectController/SelectControllerExport.h"
#include "Controll/PlayerController/PlayerControllerExport.h"
#include "Common/SignalCenter/SignalCenterExport.h"
#include "Controll/ViewDataManager/ViewDataManagerExport.h"
#include "Controll/GlobalHistoryStack/GlobalHistoryStackExport.h"

#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "PathCameraClipHelper.h"
#include "Model/TimeLineModel/TimeLineObject.h"
#include "Model/ResourceModel/ResourceModelExport.h"
#include "Model/GlobalModel/GlobalModelExport.h"
#include "Model/GlobalModel/LogModel.h"
#include "Model/GlobalModel/PrjSettingData.h"
#include "Model/MediaModel/MediaModelExport.h"
#include "Common/CommonFunctionApi.h"
#include "UICommon/menuex.h"
#include "UICommon/Messagebox.h"
#include "UICommon/RenameDialog.h"
#include "UICommon/Colordialog.h"
#include "UICommon/UITranslate.h"
#include "UICommon/ShowMessageWidget.h"
#include "Model/TimeLineModel/ConvertHelper.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QDebug>
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QAbstractSpinBox>
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

namespace
{
	constexpr uint32_t kSeekRefreshViewMask = WndType::WndType_TimeLineTool
		| WndType::WndType_TimeLineScale | WndType::WndType_LayersView
		| WndType::WndType_HorScroll | WndType::WndType_NoniusView;

	constexpr uint32_t kLocateRefreshViewMask = WndType::WndType_HorScroll
		| WndType::WndType_TimeLineScale | WndType::WndType_LayersView
		| WndType::WndType_NoniusView;

	QTime MsToQTime(int nMs)
	{
		nMs = std::max(0, nMs);
		const int h = nMs / 3600000;
		const int m = (nMs % 3600000) / 60000;
		const int s = (nMs % 60000) / 1000;
		const int ms = nMs % 1000;
		return QTime(h, m, s, ms);
	}

	int QTimeToMs(const QTime& time)
	{
		return time.hour() * 3600000 + time.minute() * 60000 + time.second() * 1000 + time.msec();
	}
}

CToolWidget::CToolWidget(QWidget* pParent)
	:QWidget(pParent)
{
	InitWidget();
	CreateConnects();
}

CToolWidget::~CToolWidget()
{
}

void CToolWidget::UpdateTime()
{
	std::chrono::milliseconds msTime = g_pPlayerController->IsPlaying()
		? g_pPlayerController->GetCurTime()
		: g_pContentViewData->GetNoniusTime();
	STimeCode timeCode = g_pConvertHelper->timeToTimeCode(msTime, RoundingMode::Floor);
	QString str;
	if (g_pProjectSetting->LoadTimeLineTimeCodeStyle())
		str = QString::fromStdString(timeCode.ToString(g_pProjectSetting->LoadTimeLineDropFrame()));
	else
		str = QString("%1").arg(timeCode.ToFrames(), 7, 10, QChar('0'));

	if (m_pLabelTime && !m_pTimeEdit->isVisible() && !(m_pFrameNumberEdit && m_pFrameNumberEdit->isVisible()))
	{
		// 字符串未变不 setText，减轻播放时每帧布局开销
		if (m_pLabelTime->text() != str)
			m_pLabelTime->setText(str);
	}
}

void CToolWidget::UpdateView()
{
	bool bEnable =  g_pSelectController->HasTimeLineObjectSelected() ||
					g_pSelectController->HasTimeLineLayerSelected();

	m_pBtnDelete->setEnabled(bEnable);

	m_pSliderZoom->setValue(g_pContentViewData->GetRatioLevelIndex());
	m_pBtnZoomBig->setEnabled(g_pContentViewData->GetRatioLevelIndex() < g_pContentViewData->GetRatioLevelSize() - 1);
	m_pBtnZoomSmall->setEnabled(g_pContentViewData->GetRatioLevelIndex() > 0);

	if (m_pLabelFps)
	{
		if(FloatIsEqual(g_pProjectSetting->LoadTimeLineFps(), 29.97f) && g_pProjectSetting->LoadTimeLineDropFrame())
			m_pLabelFps->setText(QString("%1 DF").arg(g_pProjectSetting->LoadTimeLineFps(), 0, 'g', 4));
		else
			m_pLabelFps->setText(QString("%1 fps").arg(g_pProjectSetting->LoadTimeLineFps(), 0, 'g', 4));

	}

	if (g_pContentViewData->HasObjectDragPreview())
	{
		const auto previewTime = g_pContentViewData->GetObjectDragPreviewTime();
		STimeCode timeCode = g_pConvertHelper->timeToTimeCode(previewTime, RoundingMode::Floor, true);
		m_pLabelClipPos->setText(QString::fromStdString(timeCode.ToString(g_pProjectSetting->LoadTimeLineDropFrame())));
		m_pBtnClipPos->show();
		m_pLabelClipPos->show();
		m_pBtnClipPos->update();
		m_pLabelClipPos->update();
	}
	else
	{
		m_pBtnClipPos->hide();
		m_pLabelClipPos->hide();
		m_pBtnClipPos->update();
		m_pLabelClipPos->update();
	}
	UpdateKeyFrameNavButtons();
	UpdateTime();
}

void CToolWidget::InitWidget()
{
	m_pBtnClipPos = new CSvgImageButton(":/dark/images/TimeLineWnd/ToolBar/ClipPos", this);
	m_pBtnClipPos->setFixedSize(UICommon_ImageBtnSize);
	m_pBtnClipPos->setProperty("transparentBkBtn", true);
	m_pBtnClipPos->hide();

	m_pLabelClipPos = new QLabel(this);
	m_pLabelClipPos->setFixedWidth(SysDpiZoom(85));
	m_pLabelClipPos->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_pLabelClipPos->setProperty("TimeLineWndTopClipPos", true);
	m_pLabelClipPos->hide();

	m_pBtnLocateNonius = new CSvgImageButton(":/dark/images/TimeLineWnd/ToolBar/LocateNonius", this);
	m_pBtnLocateNonius->setFixedSize(UICommon_ImageBtnSize);
	m_pBtnLocateNonius->setProperty("transparentBkBtn", true);

	m_pLabelTime = new QLabel(this);
	m_pLabelTime->setFixedWidth(SysDpiZoom(120));
	m_pLabelTime->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_pLabelTime->setProperty("TimeLineWndTopTimeLabel", true);
	m_pLabelTime->installEventFilter(this);

	const int fps = static_cast<int>(std::ceil(g_pProjectSetting->LoadTimeLineFps()));
	m_pTimeEdit = new QFrameTimeEdit(this, fps);
	m_pTimeEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);
	m_pTimeEdit->setFixedWidth(SysDpiZoom(120));
	m_pTimeEdit->setFixedHeight(UICommon_CtrlHeight);
	m_pTimeEdit->setFrame(g_pProjectSetting->LoadTimeLineTimeCodeStyle());
	m_pTimeEdit->setProperty("TimeLineWndTopTimeLabel", true);
	m_pTimeEdit->hide();

	m_pFrameNumberEdit = new QLineEdit(this);
	m_pFrameNumberEdit->setFixedWidth(SysDpiZoom(120));
	m_pFrameNumberEdit->setFixedHeight(UICommon_CtrlHeight);
	m_pFrameNumberEdit->setProperty("TimeLineWndTopTimeLabel", true);
	m_pFrameNumberEdit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_pFrameNumberEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_pFrameNumberEdit->hide();

	m_pLabelFps = new QLabel(this);
	m_pLabelFps->setFixedWidth(SysDpiZoom(52));
	m_pLabelFps->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_pLabelFps->setProperty("TimeLineWndTopTimeFps", true);

	m_pBtnTimeChange = new CSvgImageButton(":/dark/images/TimeLineWnd/Change", this);
	m_pBtnTimeChange->setFixedSize(UICommon_ImageBtnSize);
	m_pBtnTimeChange->setProperty("transparentBkBtn", true);

	m_pBtnDelete = new CSvgImageButton(":/dark/images/GridWnd/Delete", this);
	m_pBtnDelete->setFixedSize(UICommon_ImageBtnSize);
	m_pBtnDelete->setProperty("transparentBkBtn", true);

	m_pBtnPreKeyFrame = new CSvgImageButton(":/dark/images/TimeLineWnd/ToolBar/PreKeyFrame", this);
	m_pBtnPreKeyFrame->setFixedSize(UICommon_ImageBtnSize);
	m_pBtnPreKeyFrame->setProperty("transparentBkBtn", true);

	m_pBtnNextKeyFrame = new CSvgImageButton(":/dark/images/TimeLineWnd/ToolBar/NextKeyFrame", this);
	m_pBtnNextKeyFrame->setFixedSize(UICommon_ImageBtnSize);
	m_pBtnNextKeyFrame->setProperty("transparentBkBtn", true);

	m_pBtnZoomBig = new CSvgImageButton(":/dark/images/TimeLineWnd/ToolBar/ZoomBig", this);
	m_pBtnZoomBig->setFixedSize(UICommon_ImageBtnSize);
	m_pBtnZoomBig->setProperty("transparentBkBtn", true);

	m_pBtnZoomSmall = new CSvgImageButton(":/dark/images/TimeLineWnd/ToolBar/ZoomSmall", this);
	m_pBtnZoomSmall->setFixedSize(UICommon_ImageBtnSize);
	m_pBtnZoomSmall->setProperty("transparentBkBtn", true);

	m_pSliderZoom = new CSliderEx(this);
	m_pSliderZoom->setFixedSize(QSize(SysDpiZoom(90), UICommon_CtrlHeight));
	m_pSliderZoom->SetRange(0, g_pContentViewData->GetRatioLevelSize() - 1, g_pContentViewData->GetRatioLevelIndex());
	m_pSliderZoom->setProperty("TimeLineZoom", true);

	auto CreateLine = [&]() {
		QLabel* pLine = new QLabel(this);
		pLine->setFixedSize(nSizeDefine_1, nSizeDefine_16);
		pLine->setStyleSheet("background-color: black;");

		return pLine;
	};

	QWidget* pTimeContainer = new QWidget(this);
	QGridLayout* pTimeLayout = new QGridLayout(pTimeContainer);
	pTimeLayout->setMargin(0);
	pTimeLayout->setSpacing(0);
	pTimeLayout->addWidget(m_pLabelTime, 0, 0);
	pTimeLayout->addWidget(m_pTimeEdit, 0, 0);
	pTimeLayout->addWidget(m_pFrameNumberEdit, 0, 0);
	pTimeContainer->setFixedWidth(SysDpiZoom(120));

	QHBoxLayout* pLayout = new QHBoxLayout(this);
	pLayout->setMargin(0);
	pLayout->addStretch();
	pLayout->addWidget(m_pBtnClipPos);
	pLayout->addWidget(m_pLabelClipPos);
	pLayout->addSpacing(nSizeDefine_4);
	pLayout->addWidget(CreateLine());
	pLayout->addWidget(m_pBtnLocateNonius);
	pLayout->addSpacing(nSizeDefine_4);
	pLayout->addWidget(pTimeContainer);
	//pLayout->addSpacing(nSizeDefine_4);
	pLayout->addWidget(m_pLabelFps);
	pLayout->addSpacing(nSizeDefine_8);
	pLayout->addWidget(CreateLine());
	m_pBtnTimeChange->setVisible(false);
	pLayout->addSpacing(nSizeDefine_8);
	pLayout->addWidget(m_pBtnDelete);
	pLayout->addSpacing(nSizeDefine_4);
	pLayout->addWidget(m_pBtnPreKeyFrame);
	pLayout->addSpacing(nSizeDefine_4);
	pLayout->addWidget(m_pBtnNextKeyFrame);
	pLayout->addSpacing(nSizeDefine_8);
	pLayout->addWidget(CreateLine());
	pLayout->addSpacing(nSizeDefine_8);
	pLayout->addWidget(m_pBtnZoomSmall);
	pLayout->addSpacing(nSizeDefine_8);
	pLayout->addWidget(m_pSliderZoom);
	pLayout->addSpacing(nSizeDefine_8);
	pLayout->addWidget(m_pBtnZoomBig);
	pLayout->addSpacing(nSizeDefine_12);
	UpdateKeyFrameNavButtons();
}

void CToolWidget::BeginTimeEdit()
{
	const auto msTime = g_pContentViewData->GetNoniusTime();
	STimeCode timeCode = g_pConvertHelper->timeToTimeCode(msTime, RoundingMode::Floor, true);
	m_bEditingAsTimeCode = g_pProjectSetting->LoadTimeLineTimeCodeStyle();

	m_pLabelTime->hide();
	if (m_bEditingAsTimeCode)
	{
		if (m_pFrameNumberEdit)
			m_pFrameNumberEdit->hide();
		m_pTimeEdit->blockSignals(true);
		m_pTimeEdit->setFps(static_cast<int>(std::ceil(g_pProjectSetting->LoadTimeLineFps())));
		m_pTimeEdit->setFrame(true);
		m_pTimeEdit->setTime(MsToQTime(static_cast<int>(msTime.count())));
		m_pTimeEdit->setEditorText(QString::fromStdString(timeCode.ToString(g_pProjectSetting->LoadTimeLineDropFrame())));
		m_pTimeEdit->blockSignals(false);
		m_pTimeEdit->show();
		m_pTimeEdit->setFocus();
		m_pTimeEdit->selectAll();
	}
	else
	{
		m_pTimeEdit->hide();
		m_pFrameNumberEdit->blockSignals(true);
		m_pFrameNumberEdit->setText(QString("%1").arg(timeCode.ToFrames(), 7, 10, QChar('0')));
		m_pFrameNumberEdit->blockSignals(false);
		m_pFrameNumberEdit->show();
		m_pFrameNumberEdit->setFocus();
		m_pFrameNumberEdit->selectAll();
	}
}

void CToolWidget::FinishTimeEdit()
{
	const bool bTimeEditVisible = m_pTimeEdit && m_pTimeEdit->isVisible();
	const bool bFrameEditVisible = m_pFrameNumberEdit && m_pFrameNumberEdit->isVisible();
	if (!bTimeEditVisible && !bFrameEditVisible)
		return;

	if (g_pContentViewData->IsTimelineRecordLocked())
	{
		CShowMessageWidget::ShowMessage(3000, CCameraRecordTranslate::TimelineRecordLockedTip(), nullptr, ESMT_Warning);
		m_pTimeEdit->hide();
		if (m_pFrameNumberEdit)
			m_pFrameNumberEdit->hide();
		m_pLabelTime->show();
		UpdateTime();
		return;
	}

	std::chrono::milliseconds newMs;
	if (m_bEditingAsTimeCode)
	{
		newMs = g_pConvertHelper->textToTime(m_pTimeEdit->editorText().trimmed());
	}
	else
	{
		bool ok = false;
		const int frames = m_pFrameNumberEdit->text().trimmed().toInt(&ok);
		if (ok)
			newMs = g_pConvertHelper->timeCodeToTime(g_pConvertHelper->framesToTimecode(frames));
		else
			newMs = g_pContentViewData->GetNoniusTime();
	}
	if (newMs < std::chrono::milliseconds(0))
		newMs = std::chrono::milliseconds(0);
	if (newMs > g_pTimeLineModel->GetDuration())
		newMs = g_pTimeLineModel->GetDuration();

	if (!g_pContentViewData->SeekTimelineAt(newMs))
	{
		CShowMessageWidget::ShowMessage(3000, CCameraRecordTranslate::TimelineRecordLockedTip(), nullptr, ESMT_Warning);
		m_pTimeEdit->hide();
		if (m_pFrameNumberEdit)
			m_pFrameNumberEdit->hide();
		m_pLabelTime->show();
		UpdateTime();
		return;
	}
	m_pTimeEdit->hide();
	if (m_pFrameNumberEdit)
		m_pFrameNumberEdit->hide();
	m_pLabelTime->show();
	UpdateTime();
	emit signalUpdateView(kSeekRefreshViewMask);
	emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
	emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_None, SignalCenter::EDDT_SectionSelect);
}

bool CToolWidget::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == m_pLabelTime && event->type() == QEvent::MouseButtonDblClick)
	{
		BeginTimeEdit();
		return true;
	}
	return QWidget::eventFilter(watched, event);
}

void CToolWidget::CreateConnects()
{
	connect(m_pTimeEdit, &QFrameTimeEdit::editingFinished, this, &CToolWidget::FinishTimeEdit);
	connect(m_pFrameNumberEdit, &QLineEdit::editingFinished, this, &CToolWidget::FinishTimeEdit);

	connect(m_pBtnLocateNonius, &CSvgImageButton::clicked, this, [&]() {
		g_pContentViewData->ScrollToKeepNoniusAtThird();
		emit signalUpdateView(kLocateRefreshViewMask);
	});

	connect(m_pBtnPreKeyFrame, &CSvgImageButton::clicked, this, [&]() {
		NavigateSelectedObjectKeyframe(true);
	});

	connect(m_pBtnNextKeyFrame, &CSvgImageButton::clicked, this, [&]() {
		NavigateSelectedObjectKeyframe(false);
	});

	connect(g_pSignalCenter, &CSignalCenter::signalNoniusTimeChanged, this, [this](qint64) {
		// 播放中关键帧导航按钮无需每帧重算
		if (g_pPlayerController && g_pPlayerController->IsPlaying())
			return;
		UpdateKeyFrameNavButtons();
	});

	connect(m_pBtnTimeChange, &CSvgImageButton::clicked, this, [&]() {
		//更新时间类型
		UpdateTime();
		emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_MediaNodeSelect);

		LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndTop(), 
			CTimeLineWndTranslate::TimeType(), CCommonTranslate::Click());
	});

	connect(m_pBtnDelete, &CSvgImageButton::clicked, this, [&]() {
		
		std::set<QUuid> setObjects = g_pSelectController->GetTLObjectSelectIds();
		if (!setObjects.empty())
		{
			//对象
			QString names;
			for (const auto& id : setObjects)
			{
				auto pLayer = g_pTimeLineModel->GetLayer(id).lock();
				if (pLayer)
				{
					auto pObject = pLayer->GetObject(id).lock();
					if (pObject->GetType() == ITimeLineObject::ObjectType_Media)
					{
						auto pMedia = g_pMediaModel->GetMediaNode(id).lock();
						if (pMedia)
						{
							names += pMedia->GetMediaName() + ";";
						}

						g_pPlayerController->DelMediaAndFreePlayer(id);
					}

					PathCameraClipHelper::RemoveClipFromOwnerCamera(id);
					pLayer->DeleteObject(g_pTimeLineModel->GetId(), id);
				}
			}

			g_pSelectController->ClearTimeLineLayerSelect();
			g_pSelectController->ClearTimeLineObjectSelect();
			m_pBtnDelete->setEnabled(false);
			emit signalUpdateView(WndType::WndType_LayerLabels | WndType::WndType_VerScroll |
				WndType::WndType_LayersView);

			emit g_pSignalCenter->signalLocateResNode(QUuid());
			emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
			emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_MediaNodeSelect);
			emit g_pSignalCenter->signalUpdatePropertyShowType();

			if (g_pPlayerController->IsPlaying())
				g_pPlayerController->Pause();
			HistoryStackSave();

			LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndTop(), CCommonTranslate::Delete(), names);
		}
		else
		{
			//删除对象和图层
			std::set<QUuid> setLayers = g_pSelectController->GetTLLayerSelectIds();
			if (!setLayers.empty())
			{
				auto allLayerIds = g_pTimeLineModel->GetAllLayerIDs();
				int maxindex = INT_MIN;
				int minindex = INT_MAX;
				bool bDelete = false;

				for (const auto& id : setLayers)
				{
					int index = g_pTimeLineModel->GetLayerIndex(id);
					if (index > maxindex) maxindex = index;
					if (index < minindex) minindex = index;

					auto pLayer = g_pTimeLineModel->GetLayer(id).lock();
					if (pLayer && 0 != pLayer->GetObjectCount())
					{
						auto res = UICommon::CMessageBox::warning(this, "GrandMapping", CTimeLineWndTranslate::DeleteTip(),
							QMessageBox::Ok | QMessageBox::Cancel);
						if (QMessageBox::Ok == res)
						{
							auto objectIds = pLayer->GetAllObjectIDs();
							for (const auto& objectId : objectIds)
							{
								auto pObject = pLayer->GetObject(objectId).lock();
								if (pObject->GetType() == ITimeLineObject::ObjectType_Media)
								{
									g_pPlayerController->DelMediaAndFreePlayer(objectId);
								}
							}
							emit g_pSignalCenter->signalLocateResNode(QUuid());
							emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
							emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_MediaNodeSelect);

							g_pTimeLineModel->DeleteLayer(id);
							g_pSelectController->DeleteTimeLineLayerSelect(id);
							bDelete = true;

							if (g_pPlayerController->IsPlaying())
								g_pPlayerController->Pause();
							HistoryStackSave();

							LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndTop(), CCommonTranslate::Delete(), pLayer->GetName());
						}
					}
					else
					{
						g_pTimeLineModel->DeleteLayer(id);
						g_pSelectController->DeleteTimeLineLayerSelect(id);
						bDelete = true;

						HistoryStackSave();
						LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndTop(), CCommonTranslate::Delete(), pLayer->GetName());
					}
				}

				if (bDelete)
				{
					//删除图层后修改图层选中
					maxindex = maxindex + 1;
					minindex = minindex - 1;
					if (maxindex >= 0 && maxindex < (int)allLayerIds.size())
						g_pSelectController->AddTimeLineLayerSelect(allLayerIds[maxindex]);
					else if (minindex >= 0 && minindex < (int)allLayerIds.size())
						g_pSelectController->AddTimeLineLayerSelect(allLayerIds[minindex]);
					else
						m_pBtnDelete->setEnabled(false);
				}

				emit signalUpdateView(WndType::WndType_LayerLabels | WndType::WndType_VerScroll |
					WndType::WndType_LayersView);
			}
		}
	});

	connect(m_pBtnZoomSmall, &CSvgImageButton::clicked, this, [&]() {
		if (g_pContentViewData->DoInterfaceZoom(false, g_pContentViewData->GetNoniusTime()))
		{
			emit signalUpdateView(WndType::WndType_TimeLineTool | WndType::WndType_TimeLineScale |
				WndType::WndType_LayersView | WndType::WndType_HorScroll | WndType::WndType_NoniusView);
		}
		LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndTop(), CTimeLineWndTranslate::ZoomSamll(), CCommonTranslate::Click());
	});

	connect(m_pBtnZoomBig, &CSvgImageButton::clicked, this, [&]() {
		if (g_pContentViewData->DoInterfaceZoom(true, g_pContentViewData->GetNoniusTime()))
		{
			emit signalUpdateView(WndType::WndType_TimeLineTool | WndType::WndType_TimeLineScale |
				WndType::WndType_LayersView | WndType::WndType_HorScroll | WndType::WndType_NoniusView);
		}
		LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndTop(), CTimeLineWndTranslate::ZoomBig(), CCommonTranslate::Click());
	});

	connect(m_pSliderZoom, &CSliderEx::signalvalueChanged, this, [&](double v) {
		g_pContentViewData->SetZoomRatioLevelIndex(static_cast<int>(v), g_pContentViewData->GetNoniusTime());
		emit signalUpdateView(WndType::WndType_TimeLineTool | WndType::WndType_TimeLineScale |
			WndType::WndType_LayersView | WndType::WndType_HorScroll | WndType_NoniusView);
	});
}

void CToolWidget::UpdateTranslateText()
{
	m_pBtnTimeChange->setToolTip(CTimeLineWndTranslate::TimeType());
	m_pBtnDelete->setToolTip(CCommonTranslate::Delete());
	m_pBtnLocateNonius->setToolTip(CTimeLineWndTranslate::LocateNonius());
	m_pBtnPreKeyFrame->setToolTip(CTimeLineWndTranslate::PreKeyFrame());
	m_pBtnNextKeyFrame->setToolTip(CTimeLineWndTranslate::NextKeyFrame());
	m_pBtnZoomSmall->setToolTip(CTimeLineWndTranslate::ZoomSamll());
	m_pBtnZoomBig->setToolTip(CTimeLineWndTranslate::ZoomBig());
}

void CToolWidget::slotDelete()
{
	m_pBtnDelete->click();
}

void CToolWidget::UpdateKeyFrameNavButtons()
{
	bool bPre = false;
	bool bNext = false;
	const auto setObjects = g_pSelectController->GetTLObjectSelectIds();
	// 仅单选素材/路径片段时可跳转；多选/未选则双按钮置灰
	if (setObjects.size() == 1)
	{
		const QUuid objectId = *setObjects.begin();
		auto pBase = g_pTimeLineModel->GetTimeLineObject(objectId).lock();
		auto* pObject = dynamic_cast<CTimeLineObject*>(pBase.get());
		if (pObject)
		{
			// 路径相机片段：经典 Fx 轨；媒体：ClipFx 属性关键帧
			const auto times = PathCameraClipHelper::IsPathCameraClip(objectId)
				? PathCameraClipHelper::CollectPathCameraKeyframeLocalTimes(objectId)
				: pObject->GetAllPropertyKeyframeTimes();
			if (!times.empty())
			{
				auto playhead = g_pContentViewData
					? g_pContentViewData->GetNoniusTime()
					: g_pPlayerController->GetCurTime();
				auto curLocal = playhead - pObject->GetStartTime();
				if (g_pConvertHelper)
					curLocal = g_pConvertHelper->formatTimeWithFrame(curLocal);

				auto itPre = std::lower_bound(times.begin(), times.end(), curLocal);
				bPre = (itPre != times.begin());
				auto itNext = std::upper_bound(times.begin(), times.end(), curLocal);
				bNext = (itNext != times.end());
			}
		}
	}
	if (m_pBtnPreKeyFrame)
		m_pBtnPreKeyFrame->setEnabled(bPre);
	if (m_pBtnNextKeyFrame)
		m_pBtnNextKeyFrame->setEnabled(bNext);
}

void CToolWidget::NavigateSelectedObjectKeyframe(bool bPre)
{
	const auto setObjects = g_pSelectController->GetTLObjectSelectIds();
	if (setObjects.size() != 1)
		return;

	const QUuid objectId = *setObjects.begin();
	auto pBase = g_pTimeLineModel->GetTimeLineObject(objectId).lock();
	auto* pObject = dynamic_cast<CTimeLineObject*>(pBase.get());
	if (!pObject || !g_pContentViewData)
		return;

	const auto times = PathCameraClipHelper::IsPathCameraClip(objectId)
		? PathCameraClipHelper::CollectPathCameraKeyframeLocalTimes(objectId)
		: pObject->GetAllPropertyKeyframeTimes();
	if (times.empty())
		return;

	auto playhead = g_pContentViewData->GetNoniusTime();
	auto curLocal = playhead - pObject->GetStartTime();
	if (g_pConvertHelper)
		curLocal = g_pConvertHelper->formatTimeWithFrame(curLocal);

	std::chrono::milliseconds target = curLocal;
	if (bPre)
	{
		auto it = std::lower_bound(times.begin(), times.end(), curLocal);
		if (it == times.begin())
			return;
		target = *(--it);
	}
	else
	{
		auto it = std::upper_bound(times.begin(), times.end(), curLocal);
		if (it == times.end())
			return;
		target = *it;
	}

	const auto absTime = pObject->GetStartTime() + target;
	if (!g_pContentViewData->SeekTimelineAt(absTime))
		return;

	UpdateTime();
	emit signalUpdateView(kSeekRefreshViewMask);
	emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
	// CMediaPropertyFrame / CClipFxWidget / 路径菱形 通过 signalNoniusTimeChanged 与 LayersView 刷新同步
	UpdateKeyFrameNavButtons();
}
