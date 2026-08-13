#include "ObjectWidget.h"
#include "ClipWidget.h"
#include "ClipFxWidget.h"
#include "ClipTransFxWidget.h"
#include "LayersWidget.h"
#include "LayerWidget.h"
#include "Model/TimeLineModel/TimeLineObject.h"
#include "Model/TimeLineModel/ConvertHelper.h"
#include "ObjectDragMime.h"
#include "TimelineDragSnapCache.h"
#include "TimeLineGlobalDefine.h"
#include "TimeLineOverlapHelper.h"
#include "PathCameraClipHelper.h"
#include "ClipFxKeyframeHelper.h"
#include "ClipFxDefine.h"
#include "ContentViewData.h"
#include "Common/Common/ResourceDefine.h"

#include "Controll/SelectController/SelectControllerExport.h"
#include "Controll/ViewDataManager/ViewDataManagerExport.h"
#include "Common/SignalCenter/SignalCenterExport.h"
#include "Controll/PlayerController/PlayerControllerExport.h"
#include "Controll/PlayCommandCenter/PlayCommandCenterExport.h"
#include "Controll/GlobalHistoryStack/GlobalHistoryStackExport.h"

#include "Model/MediaModel/MediaModelExport.h"
#include "Model/ModelModel/ModelModelExport.h"
#include "Model/ResourceModel/ResourceModelExport.h"
#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "Model/GlobalModel/GlobalModelExport.h"
#include "Model/GlobalModel/SoftwareSettingData.h"

#include "UICommon/UITranslate.h"
#include "UICommon/menuex.h"
#include "UICommon/ShowMessageWidget.h"
#include "UICommon/UICommonSizeDefine.h"

#include <QRegion>
#include <QMouseEvent>
#include <QEvent>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QApplication>
#include <QCursor>
#include <algorithm>
#include <map>
#include <set>
#include "TimelineWinMacros.h"
#ifdef Q_OS_WIN
#include <Windows.h>
#ifndef IDC_PIN
#define IDC_PIN MAKEINTRESOURCE(32671)
#endif
#endif

namespace
{
	/** 路径关键帧悬停：IDC_PIN；不可用时回退手型。 */
	QCursor PathCameraKeyframeHoverCursor()
	{
		return QCursor(Qt::PointingHandCursor);
	}

	/** 路径关键帧按下/拖拽：左右拉伸（IDC_SIZEWE）。 */
	QCursor PathCameraKeyframePressCursor()
	{
		return QCursor(Qt::SizeHorCursor);
	}

	/**
	 * 计算转场交界点 J（图层坐标系下的 X）。
	 * 公式：J = leftEndLayerPx - Wr
	 * 其中 leftEndLayerPx 为左素材结束时间在图层上的像素位置，Wr 为右半场宽度（像素）。
	 * 左半场以 J 为右边界占 Wr 宽，右半场以 J 为左边界占 Wl 宽，保证 J 两侧拼接为重叠区。
	 */
	int ComputeTransFxJunctionLayerPx(const TimeLineOverlapHelper::STransFxObjectTimes& times, int wRight)
	{
		const int xOffset = g_pContentViewData->GetXOffset();
		const int leftEndLayerPx = g_pContentViewData->TimeToPosition(times.leftEnd) + xOffset;
		return leftEndLayerPx - wRight;
	}

	// 高缩放下限制 drag ghost 位图宽度，避免 grab 整条百万像素素材
	QPixmap GrabCappedDragPixmap(CObjectWidget* pWidget, const QPoint& hotSpot, QPoint& outHotSpot)
	{
		if (!pWidget)
			return QPixmap();
		outHotSpot = hotSpot;
		const int widgetW = pWidget->width();
		const int maxGrabW = SysDpiZoom(800);
		if (widgetW <= maxGrabW)
			return pWidget->grab();
		const int grabW = maxGrabW;
		const int grabX = std::max(0, std::min(hotSpot.x() - grabW / 2, widgetW - grabW));
		outHotSpot.setX(hotSpot.x() - grabX);
		return pWidget->grab(QRect(grabX, 0, grabW, pWidget->height()));
	}

	/**
	 * 按参与拖拽的素材 ID 集合，成对隐藏相关转场控件（左右半场一并隐藏）。
	 * 遍历图层上所有素材，若其转场关联的左/右素材 ID 与 objectIds 有交集，
	 * 则隐藏对应 CClipTransFxWidget（含 Front 半场与 Back 半场）。
	 * 用于拖拽/拉伸预览期间避免转场与 ghost 素材叠显。
	 */
	void HideTransFxHalvesForObjects(CLayerWidget* pLayer, const std::set<QUuid>& objectIds)
	{
		if (!pLayer || objectIds.empty())
			return;

		QRegion dirty;
		for (auto* pObj : pLayer->GetObjects())
		{
			if (!pObj)
				continue;

			for (auto* pTransFxWnd : pObj->GetClipTransFxWidgets())
			{
				if (!pTransFxWnd || !pTransFxWnd->isVisible())
					continue;

				auto pTransFx = g_pTimeLineModel->GetTransFx(pTransFxWnd->GetTransFxId()).lock();
				if (!pTransFx)
					continue;

				if (objectIds.count(pTransFx->GetLeftObjectId()) == 0
					&& objectIds.count(pTransFx->GetRightObjectId()) == 0)
					continue;

				dirty = dirty.united(pTransFxWnd->geometry());
				pTransFxWnd->hide();
			}
		}

		if (!dirty.isNull())
			pLayer->update(dirty);
	}

	/**
	 * 素材拖拽开始时的视觉处理：隐藏素材本体，并成对隐藏相关转场半边。
	 * 1. 将参与拖拽的素材设为鼠标穿透并 hide（由 QDrag pixmap 充当 ghost）
	 * 2. 按图层汇总素材 ID，调用 HideTransFxHalvesForObjects 隐藏转场
	 */
	void BeginObjectDragVisual(const std::vector<CObjectWidget*>& vecObjects)
	{
		std::map<CLayerWidget*, std::set<QUuid>> layerDragObjectIds;
		for (auto* pObj : vecObjects)
		{
			if (!pObj)
				continue;

			auto* pLayer = qobject_cast<CLayerWidget*>(pObj->parentWidget());
			if (!pLayer)
				continue;

			layerDragObjectIds[pLayer].insert(pObj->GetObjectId());
			pObj->setAttribute(Qt::WA_TransparentForMouseEvents, true);
			pObj->hide();
		}

		// 按图层批量隐藏转场半边，避免拖拽过程中 Layout 闪烁
		for (const auto& entry : layerDragObjectIds)
		{
			HideTransFxHalvesForObjects(entry.first, entry.second);
			entry.first->update();
		}
	}

	/**
	 * 素材拖拽结束时的视觉恢复：显示素材并从模型同步几何，重新布局转场。
	 * SyncGeometryFromModel 后调用 LayoutChildWidgets 恢复转场半场布局。
	 */
	void EndObjectDragVisual(const std::vector<CObjectWidget*>& vecObjects)
	{
		for (auto* pObj : vecObjects)
		{
			if (!pObj)
				continue;
			pObj->setAttribute(Qt::WA_TransparentForMouseEvents, false);
			pObj->SyncGeometryFromModel();
			pObj->LayoutChildWidgets();
			pObj->show();
		}
	}

	std::vector<CObjectWidget*> CollectDragObjectWidgets(CLayersWidget* pLayers, const STimelineObjectDragPayload& payload)
	{
		std::vector<CObjectWidget*> result;
		if (!pLayers)
			return result;
		for (const QUuid& id : payload.objectIds)
		{
			for (auto* pObj : pLayers->GetObjects())
			{
				if (pObj && pObj->GetObjectId() == id)
				{
					result.push_back(pObj);
					break;
				}
			}
		}
		return result;
	}
}

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CLayersWidget* CObjectWidget::FindLayersWidget() const
{
	QWidget* p = parentWidget();
	if (auto* pLayer = qobject_cast<CLayerWidget*>(p))
		return pLayer->GetLayersWidget();
	return qobject_cast<CLayersWidget*>(p);
}

void CObjectWidget::SyncGeometryFromModel()
{
	auto* pLayerWidget = qobject_cast<CLayerWidget*>(parentWidget());
	if (!pLayerWidget || m_objId.isNull())
		return;

	auto pLayer = g_pTimeLineModel->GetLayer(pLayerWidget->GetLayerId()).lock();
	if (!pLayer)
		return;

	auto pObject = pLayer->GetObject(m_objId).lock();
	if (!pObject)
		return;

	const int XOffset = g_pContentViewData->GetXOffset();
	const int modelX = g_pContentViewData->TimeToPosition(pObject->GetStartTime()) + XOffset;
	const int modelW = g_pContentViewData->TimeToPosition(pObject->GetDuration());
	const int objH = CalcTotalHeight();
	// 与 CLayerWidget::LayoutObjects 一致：底对齐素材条，ClipFx 向上展开。
	// 若固定 y=0，同层多选拖拽后关闭特效的素材会顶对齐到已展开素材的 ClipFx 顶部。
	const int objY = pLayerWidget->height() - objH;
	if (x() != modelX || y() != objY || width() != modelW || height() != objH)
		setGeometry(modelX, objY, modelW, objH);

	LayoutChildWidgets();
}

bool CObjectWidget::IsClickOnObjectByModel(const QPoint& localPos) const
{
	auto* pLayerWidget = qobject_cast<CLayerWidget*>(parentWidget());
	if (!pLayerWidget || m_objId.isNull())
		return true;

	auto pLayer = g_pTimeLineModel->GetLayer(pLayerWidget->GetLayerId()).lock();
	if (!pLayer)
		return false;

	auto pObject = pLayer->GetObject(m_objId).lock();
	if (!pObject)
		return false;

	const int XOffset = g_pContentViewData->GetXOffset();
	const int modelX = g_pContentViewData->TimeToPosition(pObject->GetStartTime()) + XOffset;
	const int modelW = g_pContentViewData->TimeToPosition(pObject->GetDuration());
	const QPoint posInLayer = mapTo(pLayerWidget, localPos);
	return posInLayer.x() >= modelX && posInLayer.x() < modelX + modelW;
}

void CObjectWidget::ForwardMouseEventToLayers(QMouseEvent* event)
{
	CLayersWidget* pLayers = FindLayersWidget();
	if (!pLayers)
		return;

	QMouseEvent mappedEvent(
		event->type(),
		pLayers->mapFromGlobal(event->globalPos()),
		event->globalPos(),
		event->button(),
		event->buttons(),
		event->modifiers());
	QApplication::sendEvent(pLayers, &mappedEvent);
}

void CObjectWidget::UpdateHoverCursor(const QPoint& localPos)
{
	// 路径关键帧按下/拖拽：IDC_SIZEWE；悬停命中：手型
	if (m_bPathKfInteraction)
	{
		setCursor(PathCameraKeyframePressCursor());
		return;
	}

	// 与按下一致：相机动画片段关键帧命中优先于左右缘拉伸
	if (PathCameraClipHelper::IsPathCameraClip(m_objId)
		&& PathCameraClipHelper::CanEditPathCameraKeyframes()
		&& !IsParentLayerLocked())
	{
		std::chrono::milliseconds hitLocal{ 0 };
		if (PathCameraClipHelper::HitTestPathCameraKeyframe(
			m_objId, localPos, x(), width(), height(), hitLocal))
		{
			setCursor(PathCameraKeyframeHoverCursor());
			return;
		}
	}

	// 左右缘拉伸热区
	const int edgeWidth = 10;
	const bool bThisSelected = g_pSelectController->IsTimeLineObjectSelected(m_objId);
	const int nSelectCount = g_pSelectController->GetTimeLineObjectSelectedCount();
	const bool bCanResize = !IsParentLayerLocked() && (!bThisSelected || nSelectCount <= 1);
	if (bCanResize && (localPos.x() <= edgeWidth || localPos.x() >= width() - edgeWidth))
	{
		setCursor(Qt::SizeHorCursor);
		return;
	}

	setCursor(Qt::SizeAllCursor);
}

bool CObjectWidget::GetPathKeyframeDrawState(std::chrono::milliseconds& selectedLocalMs, bool& bHasSelected,
	std::chrono::milliseconds& previewLocalMs, bool& bShowPreview,
	std::chrono::milliseconds& originLocalMs, bool& bInteraction) const
{
	bHasSelected = m_bPathKfHasSelection;
	selectedLocalMs = m_pathKfSelectedLocal;
	bShowPreview = m_bPathKfDragActive;
	previewLocalMs = m_pathKfPreviewLocal;
	originLocalMs = m_pathKfOriginalLocal;
	bInteraction = m_bPathKfInteraction;
	return bHasSelected || bShowPreview || bInteraction;
}

std::chrono::milliseconds CObjectWidget::ClampPathKeyframeLocalMs(const std::chrono::milliseconds& localMs) const
{
	auto result = localMs;
	auto pObject = g_pTimeLineModel ? g_pTimeLineModel->GetTimeLineObject(m_objId).lock() : nullptr;
	if (!pObject)
		return result;

	// 与写回模型一致：先帧对齐再钳到片段时长
	auto cullMax = pObject->GetDuration();
	if (g_pConvertHelper)
	{
		result = g_pConvertHelper->formatTimeWithFrame(result);
		cullMax = g_pConvertHelper->formatTimeWithFrame(cullMax);
	}
	if (result < std::chrono::milliseconds(0))
		result = std::chrono::milliseconds(0);
	if (result > cullMax)
		result = cullMax;
	return result;
}

bool CObjectWidget::TryBeginPathKeyframeInteraction(const QPoint& localPos)
{
	// 非路径片段 / 播放中 / 图层锁：不进入关键帧交互
	if (!PathCameraClipHelper::IsPathCameraClip(m_objId)
		|| !PathCameraClipHelper::CanEditPathCameraKeyframes()
		|| IsParentLayerLocked())
		return false;

	std::chrono::milliseconds hitLocal{ 0 };
	if (!PathCameraClipHelper::HitTestPathCameraKeyframe(
		m_objId, localPos, x(), width(), height(), hitLocal))
		return false;

	// 记录按下态并选中该时刻（尚未提交拖动）
	m_bPathKfInteraction = true;
	m_bPathKfDragActive = false;
	m_bPathKfHasSelection = true;
	m_pathKfPressPos = localPos;
	m_pathKfSelectedLocal = hitLocal;
	m_pathKfOriginalLocal = hitLocal;
	m_pathKfPreviewLocal = hitLocal;
	setCursor(PathCameraKeyframePressCursor());
	if (m_pClip)
		m_pClip->UpdateView();
	return true;
}

void CObjectWidget::UpdatePathKeyframeDragPreview(const QPoint& localPos)
{
	if (!m_bPathKfInteraction || !g_pContentViewData)
		return;

	// 首/尾关键帧禁止拖拽移动，仅允许选中
	if (PathCameraClipHelper::IsPathCameraKeyframeEndpoint(m_objId, m_pathKfOriginalLocal))
		return;

	// 未超过像素阈值：仅选中，不进入拖动预览
	if (!m_bPathKfDragActive
		&& (localPos - m_pathKfPressPos).manhattanLength() < CLIPFX_DRAG_START_THRESHOLD * 2)
		return;

	// 按水平像素位移换算本地时刻并刷新菱形预览
	m_bPathKfDragActive = true;
	setCursor(PathCameraKeyframePressCursor());

	const int totalDx = localPos.x() - m_pathKfPressPos.x();
	const auto delta = g_pContentViewData->PositionToTime(totalDx);
	m_pathKfPreviewLocal = ClampPathKeyframeLocalMs(m_pathKfOriginalLocal + delta);
	m_pathKfSelectedLocal = m_pathKfPreviewLocal;
	if (m_pClip)
		m_pClip->UpdateView();
}

void CObjectWidget::ClearPathKeyframeInteraction(bool bKeepSelection)
{
	m_bPathKfInteraction = false;
	m_bPathKfDragActive = false;
	if (!bKeepSelection)
		m_bPathKfHasSelection = false;
}

void CObjectWidget::FinishPathKeyframeInteraction(bool bCommitMove)
{
	const auto finalLocal = m_bPathKfDragActive ? m_pathKfPreviewLocal : m_pathKfOriginalLocal;
	const auto originalLocal = m_pathKfOriginalLocal;
	const bool bDidMove = bCommitMove && m_bPathKfDragActive && (finalLocal != originalLocal);

	// 拖动有效：写回多轨关键帧并通知属性/历史
	if (bDidMove)
	{
		if (PathCameraClipHelper::MovePathCameraKeyframesAtTime(m_objId, originalLocal, finalLocal))
		{
			HistoryStackSave();
			emit g_pSignalCenter->signalPathCameraClipSelectChanged(m_objId);
			emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_MediaNodeSelect);
			emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
		}
	}

	// 松手清除交互蓝；Seek 后若游标与关键帧重合则由绘制侧显示紫选中
	ClearPathKeyframeInteraction(false);

	CClipFxKeyframeHelper::SeekToClipLocalTime(m_objId, finalLocal);

	if (m_pClip)
		m_pClip->UpdateView();
	if (CLayersWidget* pParent = FindLayersWidget())
	{
		pParent->UpdateView();
		emit pParent->signalUpdateView(WndType::WndType_LayersView | WndType::WndType_NoniusView);
	}
}

bool CObjectWidget::IsParentLayerLocked() const
{
	auto pLayer = g_pTimeLineModel->GetLayer(m_objId).lock();
	return pLayer && pLayer->IsLock();
}

CObjectWidget::CObjectWidget(QWidget* pParent)
: QWidget(pParent)
, m_pClip(nullptr)
, m_pClipFx(nullptr)
{
	setMouseTracking(true);
	setAcceptDrops(true);
	InitWnd();
}

CObjectWidget::~CObjectWidget()
{
	ReleaseTransFxWidget();
}

/**
 * 释放转场控件：先 hide 再 deleteLater。
 * 转场父控件为 CLayerWidget，需在图层坐标下 update 脏区。
 */
void CObjectWidget::ReleaseTransFxWidget()
{
	for (auto* pWnd : m_transFxWidgets)
	{
		if (!pWnd)
			continue;
		if (auto* pLayer = qobject_cast<CLayerWidget*>(pWnd->parentWidget()))
			pLayer->update(pWnd->geometry());
		pWnd->hide();
		pWnd->deleteLater();
	}
	m_transFxWidgets.clear();
}

void CObjectWidget::InitWnd()
{
	m_pClipFx = new CClipFxWidget(this);
	connect(m_pClipFx, &CClipFxWidget::signalHeightChanged, this, &CObjectWidget::OnClipFxHeightChanged);
	connect(m_pClipFx, &CClipFxWidget::signalClipKeyframeMarkersChanged, this, &CObjectWidget::OnClipKeyframeMarkersChanged);
	m_pClipFx->hide();

	m_pClip = new CClipWidget(this);
	// 鼠标事件交给 CObjectWidget 处理（边缘拉伸光标、拖拽等），CClip 仅负责绘制
	m_pClip->setAttribute(Qt::WA_TransparentForMouseEvents, true);
	m_pClip->installEventFilter(this);
	m_pClip->show();
	if (m_pClipFx)
		m_pClipFx->installEventFilter(this);
}

CClipTransFxWidget* CObjectWidget::EnsureTransFxWidget()
{
	auto* pLayerWidget = qobject_cast<CLayerWidget*>(parentWidget());
	QWidget* pParent = pLayerWidget ? static_cast<QWidget*>(pLayerWidget) : this;
	auto* pWnd = new CClipTransFxWidget(pParent);
	pWnd->hide();
	m_transFxWidgets.push_back(pWnd);
	return pWnd;
}

void CObjectWidget::LayoutOneTransFxWidget(const std::shared_ptr<ITimeLineTransFx>& pTransFx,
	int leftHalfPx, int rightHalfPx, int previewJunctionLayerPx)
{
	if (!pTransFx || m_objId.isNull())
		return;

	TimeLineOverlapHelper::STransFxObjectTimes objectTimes;
	if (!TimeLineOverlapHelper::GetTransFxObjectTimes(
		pTransFx->GetLeftObjectId(), pTransFx->GetRightObjectId(), objectTimes))
		return;

	const auto overlapDuration = objectTimes.leftEnd - objectTimes.rightStart;
	if (overlapDuration.count() <= 0)
		return;

	std::chrono::milliseconds objectStartTime{ 0 };
	if (auto pObject = g_pTimeLineModel->GetTimeLineObject(m_objId).lock())
		objectStartTime = pObject->GetStartTime();

	const int overlapPx = g_pContentViewData->TimeToPosition(overlapDuration);
	const bool bIsLeftObject = (m_objId == pTransFx->GetLeftObjectId());

	int wLeft = 0;
	int wRight = 0;
	if (leftHalfPx >= 0 && rightHalfPx >= 0)
	{
		wLeft = leftHalfPx;
		wRight = rightHalfPx;
	}
	else
	{
		wLeft = overlapPx / 2;
		wRight = overlapPx - wLeft;
	}

	const int transFxPx = bIsLeftObject ? wLeft : wRight;

	auto* pLayerWidget = qobject_cast<CLayerWidget*>(parentWidget());
	const int junctionLayerPx = previewJunctionLayerPx >= 0
		? previewJunctionLayerPx
		: ComputeTransFxJunctionLayerPx(objectTimes, wRight);

	int transLayerX = 0;
	const int transW = transFxPx;
	if (bIsLeftObject)
		transLayerX = junctionLayerPx - transFxPx;
	else
		transLayerX = junctionLayerPx;

	CClipTransFxWidget* pTransFxWnd = nullptr;
	for (auto* pWnd : m_transFxWidgets)
	{
		if (pWnd && pWnd->GetTransFxId() == pTransFx->GetId())
		{
			pTransFxWnd = pWnd;
			break;
		}
	}
	if (!pTransFxWnd)
		pTransFxWnd = EnsureTransFxWidget();

	if (pLayerWidget && pTransFxWnd->parentWidget() != pLayerWidget)
		pTransFxWnd->setParent(pLayerWidget);

	pTransFxWnd->SetTransFxId(pTransFx->GetId());
	pTransFxWnd->SetDisplaySide(bIsLeftObject ? ETransFxDisplaySide::Front : ETransFxDisplaySide::Back);
	pTransFxWnd->SetLayoutContext(
		objectTimes.rightStart, overlapDuration, pTransFx->GetDuration(), objectStartTime);
	pTransFxWnd->setMinimumWidth(0);
	pTransFxWnd->setMaximumWidth(QWIDGETSIZE_MAX);
	// TransFx父级为LayerWidget；y()+fxH/clipH与媒体条对齐（不含展开的ClipFx）
	const int fxH = m_pClipFx ? m_pClipFx->CalcHeight() : 0;
	const int clipH = SingleLayerHeight;
	pTransFxWnd->setGeometry(transLayerX, y() + fxH, transW, clipH);
	pTransFxWnd->show();
	pTransFxWnd->raise();
	pTransFxWnd->UpdateView();
}

void CObjectWidget::InitSnapTimes()
{
	m_setSnapTimes.clear();
	auto layerIds = g_pTimeLineModel->GetAllLayerIDs();
	for (const auto& layerId : layerIds)
	{
		auto pLayer = g_pTimeLineModel->GetLayer(layerId).lock();
		if (!pLayer)
			continue;
		for (const auto& objId : pLayer->GetAllObjectIDs())
		{
			if (objId == m_objId)
				continue;
			auto pObj = pLayer->GetObject(objId).lock();
			if (pObj)
			{
				m_setSnapTimes.insert(pObj->GetStartTime());
				m_setSnapTimes.insert(pObj->GetStartTime() + pObj->GetDuration());
			}
		}
	}
	if (!g_pPlayerController->IsPlaying())
		m_setSnapTimes.insert(g_pPlayerController->GetCurTime());
}

void CObjectWidget::SetObjectId(const QUuid& id)
{
	m_objId = id;
	if (m_pClipFx)
		m_pClipFx->SetObjectId(id);
	LayoutChildWidgets();
}

QUuid CObjectWidget::GetObjectId() const
{
	return m_objId;
}

/**
 * 布局素材 CClip 与转场 CClipTransFxWidget。
 *
 * 转场几何（图层坐标）：
 * - 重叠区时间 [rightStart, leftEnd]，像素宽度 overlapPx
 * - 左半场宽 Wl、右半场宽 Wr，满足 Wl + Wr = overlapPx
 * - 默认 Wl = overlapPx/2，Wr = overlapPx - Wl（拉伸预览可覆盖）
 * - 交界点 J = leftEndLayerPx - Wr
 * - 左素材占 Front 半场，布局区间 [J-Wl, J]；右素材占 Back 半场，布局区间 [J, J+Wr]
 *
 * @param leftHalfPx/rightHalfPx  均 >= 0 时为拉伸预览半场宽度；默认 -1 表示等分重叠区
 * @param previewJunctionLayerPx  >= 0 时固定交界点 J 的图层 X（拉伸预览用）；默认 -1 由模型计算
 */
int CObjectWidget::CalcTotalHeight() const
{
	int fxH = 0;
	if (m_pClipFx)
		fxH = m_pClipFx->CalcHeight();
	return SingleLayerHeight + fxH;
}

void CObjectWidget::OnClipFxHeightChanged()
{
	if (auto* pLayer = qobject_cast<CLayerWidget*>(parentWidget()))
	{
		if (auto* pLayers = pLayer->GetLayersWidget())
		{
			// 内部已对每层 setGeometry + LayoutObjects + LayoutChildWidgets，
			// 并 emit LayerLabels | VerScroll
			pLayers->RelayoutLayersGeometry();
		}
		else
		{
			LayoutChildWidgets();
			pLayer->LayoutObjects();
			emit signalUpdateView(WndType::WndType_LayerLabels | WndType::WndType_VerScroll);
		}
	}
	else
	{
		LayoutChildWidgets();
	}

	InstallClipFxMouseEventForward();
}

void CObjectWidget::OnClipKeyframeMarkersChanged()
{
	if (m_pClip)
		m_pClip->UpdateView();
}

void CObjectWidget::LayoutChildWidgets(int leftHalfPx, int rightHalfPx, int previewJunctionLayerPx,
	const QUuid& previewTransFxId)
{
	if (!m_pClip)
		return;

	const int fxH = m_pClipFx ? m_pClipFx->CalcHeight() : 0;
	const int clipH = SingleLayerHeight;
	if (m_pClipFx)
	{
		if (fxH > 0)
		{
			m_pClipFx->show();
			m_pClipFx->setGeometry(0, 0, width(), fxH);
			m_pClipFx->UpdateView();
			InstallClipFxMouseEventForward();
		}
		else
			m_pClipFx->hide();
	}
	m_pClip->setGeometry(0, fxH, width(), clipH);
	m_pClip->SetObjectId(m_objId);
	if (previewTransFxId.isNull() || leftHalfPx < 0 || rightHalfPx < 0)
		m_pClip->UpdateView();

	if (m_objId.isNull())
	{
		for (auto* pWnd : m_transFxWidgets)
		{
			if (pWnd)
				pWnd->hide();
		}
		return;
	}

	const auto transFxs = g_pTimeLineModel->GetTransFxsByObject(m_objId);
	std::set<QUuid> activeTransFxIds;
	for (const auto& pTransFx : transFxs)
	{
		if (!pTransFx)
			continue;

		activeTransFxIds.insert(pTransFx->GetId());

		const bool bPreviewThis = previewTransFxId.isNull() || previewTransFxId == pTransFx->GetId();
		int previewLeft = -1;
		int previewRight = -1;
		int previewJunction = -1;
		if (bPreviewThis && leftHalfPx >= 0 && rightHalfPx >= 0)
		{
			previewLeft = leftHalfPx;
			previewRight = rightHalfPx;
			previewJunction = previewJunctionLayerPx;
		}

		LayoutOneTransFxWidget(pTransFx, previewLeft, previewRight, previewJunction);
	}

	for (size_t i = 0; i < m_transFxWidgets.size(); )
	{
		auto* pWnd = m_transFxWidgets[i];
		if (!pWnd || activeTransFxIds.count(pWnd->GetTransFxId()) == 0)
		{
			if (pWnd)
			{
				if (auto* pLayer = qobject_cast<CLayerWidget*>(pWnd->parentWidget()))
					pLayer->update(pWnd->geometry());
				pWnd->hide();
				pWnd->deleteLater();
			}
			m_transFxWidgets.erase(m_transFxWidgets.begin() + static_cast<std::ptrdiff_t>(i));
		}
		else
		{
			++i;
		}
	}
}

void CObjectWidget::paintEvent(QPaintEvent * event)
{
	QWidget::paintEvent(event);
}

bool CObjectWidget::StartObjectDrag(QMouseEvent* event)
{
	CLayersWidget* pLayers = FindLayersWidget();
	if (!pLayers)
		return false;

	QMimeData* pMime = CObjectDragMime::BuildObjectDragMime(this, pLayers, m_pressHotSpot);
	if (!pMime)
		return false;

	auto parsed = CObjectDragMime::ParseObjectDragMime(pMime);
	if (!parsed.has_value())
	{
		delete pMime;
		return false;
	}

	QPoint hotSpot;
	QPixmap pix;
	if (parsed->objectIds.size() <= 1)
		pix = GrabCappedDragPixmap(this, m_pressHotSpot, hotSpot);
	else
		pix = CObjectDragMime::CreateObjectDragPixmap(this, pLayers, *parsed, hotSpot);

	// 拖拽期间缓存吸附点，避免 dragMove 每帧全时间线扫描
	TimelineDragSnapCache::Begin(pLayers, parsed->objectIds);

	std::vector<CObjectWidget*> vecDragObjects = CollectDragObjectWidgets(pLayers, *parsed);
	// 拖拽开始：隐藏素材 ghost 及转场半边
	BeginObjectDragVisual(vecDragObjects);

	QDrag drag(this);
	drag.setMimeData(pMime);
	drag.setHotSpot(hotSpot);
	if (!pix.isNull())
		drag.setPixmap(pix);

	if (auto* pLayer = qobject_cast<CLayerWidget*>(parentWidget()))
	{
		const QPoint posInLayer = mapTo(pLayer, m_pressHotSpot);
		pLayers->PreviewTimelineObjectDrag(pMime, posInLayer);
	}

	const Qt::DropAction action = drag.exec(Qt::MoveAction);

	g_pContentViewData->SetSnap(false);
	TimelineDragSnapCache::Clear();
	pLayers->ClearObjectDragClipPosPreview();

	// 拖拽结束：恢复素材并重新布局转场
	EndObjectDragVisual(vecDragObjects);
	pLayers->UpdateView();
	for (auto* pObj : pLayers->GetObjects())
	{
		if (pObj)
			pObj->SyncGeometryFromModel();
	}
	if (action != Qt::IgnoreAction)
		pLayers->NotifyAfterObjectDrag();

	(void)event;
	return action != Qt::IgnoreAction;
}

/**
 * 将图层坐标下的转场点击转发给 CClipTransFxWidget::HandleMousePress。
 * 先经 CLayerWidget::HitTestTransFxWidgetAt（图层坐标命中测试，父控件为 CLayerWidget）。
 */
bool CObjectWidget::TryHandleTransFxMousePress(const QPoint& posInObject, QMouseEvent* event)
{
	if (!event || event->button() != Qt::LeftButton)
		return false;

	auto* pLayerWidget = qobject_cast<CLayerWidget*>(parentWidget());
	if (!pLayerWidget)
		return false;

	const QPoint posInLayer = pLayerWidget->mapFromGlobal(event->globalPos());
	auto* pTransFx = pLayerWidget->HitTestTransFxWidgetAt(posInLayer);
	if (!pTransFx)
		return false;

	const QPoint posInTransFx = pTransFx->mapFrom(pLayerWidget, posInLayer);
	QMouseEvent transFxEvent(QEvent::MouseButtonPress, posInTransFx, event->globalPos(),
		event->button(), event->buttons(), event->modifiers());
	pTransFx->HandleMousePress(&transFxEvent);
	return true;
}

/**
 * 若素材/边缘操作应让位于转场，返回 true。
 * 点击落在转场上时，素材不再处理，避免 ClearTransFxSelect 等清除转场选中。
 */
bool CObjectWidget::ShouldYieldPressToTransFx(QMouseEvent* event) const
{
	if (!event || event->button() != Qt::LeftButton)
		return false;

	// 转场已在子控件内处理过按下，素材不再重复处理
	if (CClipTransFxWidget::WasRecentPressHandledAt(event->globalPos()))
		return true;

	auto* pLayerWidget = qobject_cast<CLayerWidget*>(parentWidget());
	if (!pLayerWidget)
		return false;

	const QPoint posInLayer = pLayerWidget->mapFromGlobal(event->globalPos());
	return pLayerWidget->HitTestTransFxWidgetAt(posInLayer) != nullptr;
}

bool CObjectWidget::IsDescendantOfClipFx(QObject* watched) const
{
	if (!m_pClipFx || !watched)
		return false;
	for (QObject* p = watched; p; p = p->parent())
	{
		if (p == m_pClipFx)
			return true;
	}
	return false;
}

void CObjectWidget::InstallClipFxMouseEventForward()
{
	if (!m_pClipFx)
		return;

	m_pClipFx->installEventFilter(this);
	const auto children = m_pClipFx->findChildren<QObject*>();
	for (QObject* pChild : children)
		pChild->installEventFilter(this);
}

void CObjectWidget::ToggleClipFxTrackVisibility()
{
	auto pLayer = g_pTimeLineModel->GetLayer(m_objId).lock();
	if (!pLayer || pLayer->IsLock())
		return;
	auto pObject = pLayer->GetObject(m_objId).lock();
	if (!pObject)
		return;

	if (auto* pTlObj = dynamic_cast<CTimeLineObject*>(pObject.get()))
	{
		if (pTlObj->HasAnyPropertyKeyframes())
		{
			pTlObj->SetClipFxTrackVisible(!pTlObj->IsClipFxTrackVisible());
			// 仅改标志不够：折叠时视口可能被固定为 0，需刷新内部布局再 Relayout 图层
			if (m_pClipFx)
				m_pClipFx->RefreshAfterTrackVisibilityChanged();
			else
				OnClipFxHeightChanged();
		}
	}
}

void CObjectWidget::ShowObjectContextMenu(const QPoint& globalPos)
{
	auto pLayer = g_pTimeLineModel->GetLayer(m_objId).lock();
	auto pObject = pLayer ? pLayer->GetObject(m_objId).lock() : nullptr;
	if (!pObject)
		return;

	const QPoint localPos = mapFromGlobal(globalPos);

	// 路径片段：命中中间关键帧时独占「删除」菜单，与普通片段菜单互斥
	if (PathCameraClipHelper::IsPathCameraClip(m_objId)
		&& PathCameraClipHelper::CanEditPathCameraKeyframes()
		&& !IsParentLayerLocked())
	{
		std::chrono::milliseconds hitLocal{ 0 };
		bool bHitKf = PathCameraClipHelper::HitTestPathCameraKeyframe(
			m_objId, localPos, x(), width(), height(), hitLocal);
		if (!bHitKf && m_bPathKfHasSelection)
		{
			hitLocal = m_pathKfSelectedLocal;
			bHitKf = true;
		}
		// 与 Delete 键一致：游标与关键帧重合时也可右键删关键帧
		if (!bHitKf
			&& PathCameraClipHelper::TryGetPlayheadSelectedPathKeyframe(m_objId, hitLocal))
		{
			bHitKf = true;
		}
		if (bHitKf && !PathCameraClipHelper::IsPathCameraKeyframeEndpoint(m_objId, hitLocal))
		{
			m_bPathKfHasSelection = true;
			m_pathKfSelectedLocal = hitLocal;
			if (m_pClip)
				m_pClip->UpdateView();

			CMenuEx* pKfMenu = new CMenuEx(this);
			pKfMenu->setFixedWidth(nSizeDefine_148);
			auto pDeleteKf = pKfMenu->addAction(CCommonTranslate::Delete());
			const auto pSelect = pKfMenu->exec(globalPos);

			CLayersWidget* pLayers = FindLayersWidget();
			if (pLayers)
				pLayers->SetBlockNextLayerContextMenu(true);

			if (pSelect == pDeleteKf)
			{
				if (PathCameraClipHelper::DeletePathCameraKeyframesAtTime(m_objId, hitLocal))
				{
					ClearPathKeyframeInteraction(false);
					HistoryStackSave();
					emit g_pSignalCenter->signalPathCameraClipSelectChanged(m_objId);
					emit g_pSignalCenter->signalRefreshWindow(
						SignalCenter::EWT_Property, SignalCenter::EDDT_MediaNodeSelect);
					emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
					if (m_pClip)
						m_pClip->UpdateView();
					if (pLayers)
					{
						pLayers->UpdateView();
						emit pLayers->signalUpdateView(
							WndType::WndType_LayersView | WndType::WndType_NoniusView);
					}
				}
			}
			pKfMenu->deleteLater();
			return;
		}
	}

	CMenuEx* pRBtnMenu = new CMenuEx(this);
	pRBtnMenu->setFixedWidth(nSizeDefine_148);
	auto pResLocal = pRBtnMenu->addAction(CTimeLineWndTranslate::ResourecLoacl());
	QAction* pViewLocl = nullptr;
	if (g_pSoftwareSetting->GetAppOpType() != ESL_PlayControll)
		pViewLocl = pRBtnMenu->addAction(CTimeLineWndTranslate::ViewLoacl());
	
	QAction* pToggleFxTrack = nullptr;
	if (auto* pTlObj = dynamic_cast<CTimeLineObject*>(pObject.get()))
	{
		if (pTlObj->HasAnyPropertyKeyframes())
		{
			pToggleFxTrack = pRBtnMenu->addAction(
				pTlObj->IsClipFxTrackVisible() ? CTimeLineWndTranslate::HideKeyFrame()
				: CTimeLineWndTranslate::ShowKeyFrame());
			pToggleFxTrack->setChecked(pTlObj->IsClipFxTrackVisible());
		}
	}
	auto pCopy = pRBtnMenu->addAction(CCommonTranslate::Copy());
	auto pPaste = pRBtnMenu->addAction(CCommonTranslate::Paste());
	auto pDelete = pRBtnMenu->addAction(CCommonTranslate::Delete());
	pPaste->setEnabled(g_pContentViewData->GetTimelineCopyData().HasData());

	if (pObject->GetStartTime() > g_pPlayerController->GetCurTime() ||
		pObject->GetStartTime() + pObject->GetDuration() < g_pPlayerController->GetCurTime())
	{
		if (pViewLocl)
			pViewLocl->setEnabled(false);
	}

	// 路径相机片段：仅保留删除/复制/粘贴，其它项禁用
	if (PathCameraClipHelper::IsPathCameraClip(m_objId))
	{
		pResLocal->setEnabled(false);
		if (pViewLocl)
			pViewLocl->setEnabled(false);
		if (pToggleFxTrack)
			pToggleFxTrack->setEnabled(false);
	}

	const auto pSelect = pRBtnMenu->exec(globalPos);

	// 素材右键菜单关闭后，吞掉随后落到图层上的 contextMenu（删素材后命中为空会误弹复制/粘贴）
	CLayersWidget* pLayers = FindLayersWidget();
	if (pLayers)
		pLayers->SetBlockNextLayerContextMenu(true);

	if (pSelect == pResLocal)
	{
		if (auto pMedia = g_pMediaModel->GetMediaNode(pObject->GetId()).lock())
			emit g_pSignalCenter->signalLocateResNode(pMedia->GetResourceId());
	}
	else if (pSelect == pViewLocl)
	{
		emit g_pSignalCenter->signalFindMedia(pObject->GetId());
		emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_All);
	}
	else if (pSelect == pDelete)
	{
		const QUuid objId = pObject->GetId();
		g_pPlayerController->Pause();
		g_pSelectController->ClearTimeLineLayerSelect();
		g_pSelectController->ClearTimeLineObjectSelect();
		g_pSelectController->ClearTransFxSelect();
		g_pPlayerController->DelMediaAndFreePlayer(objId);
		g_pMediaModel->DeleteMediaNode(objId);
		PathCameraClipHelper::RemoveClipFromOwnerCamera(objId);
		pLayer->DeleteObject(g_pTimeLineModel->GetId(), objId);
		emit g_pSignalCenter->signalLocateResNode(QUuid());
		emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
		emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_MediaNodeSelect);
		emit g_pSignalCenter->signalUpdatePropertyShowType();
		// 通过 LayersWidget 刷新，避免本 ObjectWidget 即将销毁导致 signal 无效、界面不立即更新
		if (pLayers)
		{
			pLayers->UpdateView();
			emit pLayers->signalUpdateView(WndType::WndType_LayersView | WndType::WndType_LayerLabels
				| WndType::WndType_TimeLineTool | WndType::WndType_VerScroll);
		}
		else
		{
			emit signalUpdateView(WndType::WndType_LayersView | WndType::WndType_LayerLabels | WndType::WndType_TimeLineTool);
		}
		HistoryStackSave();
	}
	else if (pSelect == pCopy)
	{
		// 若当前素材未选中（右键菜单操作），先选中本素材并清除转场选择，再执行复制
		if (!g_pSelectController->IsTimeLineObjectSelected(m_objId))
		{
			g_pSelectController->ClearTimeLineLayerSelect();
			g_pSelectController->ClearTimeLineObjectSelect();
			g_pSelectController->AddTimeLineLayerSelect(m_originalLayerId, true);
			g_pSelectController->ClearTransFxSelect();
			g_pSelectController->AddTimeLineObjectSelect(m_objId, true);
		}
		g_pContentViewData->CopySelectedObjects();
	}
	else if (pSelect == pToggleFxTrack)
	{
		ToggleClipFxTrackVisibility();
	}
	else if (pSelect == pPaste)
	{
		if (auto* pLayerWidget = qobject_cast<CLayerWidget*>(parentWidget()))
		{
			const QPoint posInLayer = mapTo(pLayerWidget, localPos);
			if (!TimeLineOverlapHelper::CanPasteObjectsToLayer(posInLayer, pLayerWidget->GetLayerIndex()))
			{
				if (TimeLineOverlapHelper::IsPasteBlockedByPathClipOverlap(
					posInLayer, pLayerWidget->GetLayerIndex()))
				{
					CShowMessageWidget::ShowMessage(3000, CTimeLineWndTranslate::PathClipOverlapTip(), nullptr, ESMT_Warning);
				}
				pRBtnMenu->deleteLater();
				return;
			}
			g_pContentViewData->PasteObjectsToLayer(posInLayer, pLayerWidget->GetLayerIndex());

			if (pLayers)
			{
				pLayers->UpdateView();
				pLayers->NotifyAfterObjectDrag();
			}
		}
	}
	pRBtnMenu->deleteLater();
}

/**
 * 事件过滤器：CClip 内按下时若命中转场，转发给转场处理，
 * 避免素材选中逻辑清除转场选中状态。
 */
bool CObjectWidget::eventFilter(QObject* watched, QEvent* event)
{
	// 特效轨子控件右键：转发到素材右键菜单
	if (IsDescendantOfClipFx(watched) && event->type() == QEvent::MouseButtonRelease)
	{
		auto* mouseEvent = static_cast<QMouseEvent*>(event);
		if (mouseEvent->button() == Qt::RightButton)
		{
			ShowObjectContextMenu(mouseEvent->globalPos());
			return true;
		}
	}

	if (watched == m_pClip && event->type() == QEvent::MouseButtonPress)
	{
		auto* mouseEvent = static_cast<QMouseEvent*>(event);
		if (ShouldYieldPressToTransFx(mouseEvent))
		{
			if (!CClipTransFxWidget::WasRecentPressHandledAt(mouseEvent->globalPos()))
				TryHandleTransFxMousePress(mouseEvent->pos(), mouseEvent);
			return true;
		}
	}
	return QWidget::eventFilter(watched, event);
}

void CObjectWidget::mousePressEvent(QMouseEvent* event)
{
	// 非拉伸状态下，优先将转场区域的按下转发给转场处理
	if (!m_bResizing && event->button() == Qt::LeftButton && ShouldYieldPressToTransFx(event))
	{
		if (!CClipTransFxWidget::WasRecentPressHandledAt(event->globalPos()))
			TryHandleTransFxMousePress(event->pos(), event);
		event->accept();
		return;
	}

	if (!m_bResizing)
	{
		SyncGeometryFromModel();
		if (!IsClickOnObjectByModel(event->pos()))
		{
			ForwardMouseEventToLayers(event);
			return;
		}
	}

	if (event->button() != Qt::LeftButton)
	{
		QWidget::mousePressEvent(event);
		return;
	}

	m_bDragStarted = false;
	const int edgeWidth = 10;
	const bool bThisSelected = g_pSelectController->IsTimeLineObjectSelected(m_objId);
	const int nSelectCount = g_pSelectController->GetTimeLineObjectSelectedCount();
	const bool bCanResize = !IsParentLayerLocked() && (!bThisSelected || nSelectCount <= 1);

	// 路径关键帧命中优先于边缘拉伸 / 片段拖拽（播放中 CanEdit 为 false，不进入）
	if (TryBeginPathKeyframeInteraction(event->pos()))
	{
		g_pSelectController->ClearTransFxSelect();
		g_pSelectController->AddTimeLineObjectSelect(m_objId, true);
		emit g_pSignalCenter->signalPathCameraClipSelectChanged(m_objId);
		if (m_pClip)
			m_pClip->UpdateView();
		if (CLayersWidget* pParent = FindLayersWidget())
		{
			pParent->update();
			emit pParent->signalUpdateView(WndType::WndType_LayersView
				| WndType::WndType_LayerLabels
				| WndType::WndType_TimeLineTool);
		}
		event->accept();
		return;
	}
	// 未命中关键帧：清除选中高亮
	if (m_bPathKfHasSelection && PathCameraClipHelper::IsPathCameraClip(m_objId))
	{
		ClearPathKeyframeInteraction(false);
		if (m_pClip)
			m_pClip->UpdateView();
	}

	EResizeSide side = RS_None;
	if (bCanResize)
	{
		if (event->pos().x() <= edgeWidth)
			side = RS_Left;
		else if (event->pos().x() >= width() - edgeWidth)
			side = RS_Right;
	}

	if (side != RS_None)
	{
		// 边缘拉伸区域若落在转场上，不进入拉伸流程
		if (ShouldYieldPressToTransFx(event))
		{
			event->accept();
			return;
		}
		if (!bThisSelected || nSelectCount > 1)
		{
		g_pSelectController->ClearTransFxSelect();
		g_pSelectController->AddTimeLineObjectSelect(m_objId, true);
			if (PathCameraClipHelper::IsPathCameraClip(m_objId))
			{
				emit g_pSignalCenter->signalPathCameraClipSelectChanged(m_objId);
			}
			else
			{
				emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_MediaNodeSelect);
			}
			if (m_pClip)
				m_pClip->UpdateView();
			if (CLayersWidget* pParent = FindLayersWidget())
			{
				pParent->update();
				emit pParent->signalUpdateView(WndType::WndType_LayersView
					| WndType::WndType_LayerLabels
					| WndType::WndType_TimeLineTool);
			}
		}
		m_bResizing = true;
		m_resizeSide = side;
		m_resizeStartPos = event->pos();
		m_resizeStartGlobalPos = event->globalPos();
		m_resizeStartWidth = width();
		m_originalX = x();
		if (auto pLayer = g_pTimeLineModel->GetLayer(m_objId).lock())
		{
			m_originalLayerId = pLayer->GetId();
			if (auto pObject = pLayer->GetObject(m_objId).lock())
			{
				m_originalStartTime = pObject->GetStartTime();
				m_originalDuration = pObject->GetDuration();
			}
		}
		// 拉伸开始：成对隐藏相关转场半边，避免拉伸预览与转场叠显
		if (auto* pLayerWidget = qobject_cast<CLayerWidget*>(parentWidget()))
		{
			std::set<QUuid> resizeObjectIds;
			resizeObjectIds.insert(m_objId);
			HideTransFxHalvesForObjects(pLayerWidget, resizeObjectIds);
		}
		setCursor(Qt::SizeHorCursor);
		return;
	}

	if (!bThisSelected || nSelectCount <= 1)
	{
		if (ShouldYieldPressToTransFx(event))
		{
			event->accept();
			return;
		}
		g_pSelectController->ClearTransFxSelect();
		g_pSelectController->AddTimeLineObjectSelect(m_objId, true);
		if (PathCameraClipHelper::IsPathCameraClip(m_objId))
		{
			emit g_pSignalCenter->signalPathCameraClipSelectChanged(m_objId);
		}
		else
		{
			emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_MediaNodeSelect);
		}
		if (m_pClip)
			m_pClip->UpdateView();
		if (CLayersWidget* pParent = FindLayersWidget())
		{
			pParent->update();
			emit pParent->signalUpdateView(WndType::WndType_LayersView
				| WndType::WndType_LayerLabels
				| WndType::WndType_TimeLineTool);
		}
	}

	m_bResizing = false;
	m_dragStartPos = event->pos();
	m_pressHotSpot = event->pos();
	m_originalX = x();

	InitSnapTimes();

	auto pLayer = g_pTimeLineModel->GetLayer(m_objId).lock();
	m_originalLayerId = pLayer ? pLayer->GetId() : QUuid();
	if (pLayer)
	{
		if (auto pObject = pLayer->GetObject(m_objId).lock())
			m_originalStartTime = pObject->GetStartTime();
	}
	setCursor(Qt::SizeAllCursor);
	raise();
	// 按下素材时：将图层上所有转场控件置顶，避免被素材遮挡
	if (auto* pLayerWidget = qobject_cast<CLayerWidget*>(parentWidget()))
		pLayerWidget->RaiseTransFxWidgets();
}

void CObjectWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (m_bPathKfInteraction)
	{
		if (event->buttons() & Qt::LeftButton)
			UpdatePathKeyframeDragPreview(event->pos());
		else
			UpdateHoverCursor(event->pos());
		event->accept();
		return;
	}

	if (!m_bResizing && !IsClickOnObjectByModel(event->pos()))
	{
		SyncGeometryFromModel();
		ForwardMouseEventToLayers(event);
		return;
	}

	if (m_bResizing)
	{
		int dx = event->globalPos().x() - m_resizeStartGlobalPos.x();
		int newWidth = m_resizeStartWidth;
		if (m_resizeSide == RS_Right)
			newWidth = m_resizeStartWidth + dx;
		else if (m_resizeSide == RS_Left)
			newWidth = m_resizeStartWidth - dx;
		if (newWidth < nSizeDefine_4)
			newWidth = nSizeDefine_4;

		auto* pLayerWidget = qobject_cast<CLayerWidget*>(parentWidget());
		const QRect oldRect(x(), y(), width(), height());

		if (m_resizeSide == RS_Left)
		{
			const int offset = newWidth - m_resizeStartWidth;
			move(m_originalX - offset, y());
		}
		resize(newWidth, height());
		if (m_pClip)
		{
			// 拉伸预览仅同步几何，避免高缩放下每帧全宽重绘 CClipWidget
			m_pClip->setGeometry(0, 0, width(), height());
		}
		QRect newRect(x(), y(), width(), height());
		if (pLayerWidget)
		{
			// 路径相机左缘拉伸：关键帧按绝对时间重绘，需刷新新旧几何并集，不能只刷 XOR 边带
			if (m_resizeSide == RS_Left && PathCameraClipHelper::IsPathCameraClip(m_objId))
			{
				pLayerWidget->update(oldRect.united(newRect));
				if (m_pClip)
					m_pClip->update();
			}
			else
			{
				// 仅刷新新旧几何差集，避免高缩放下每帧 invalidate 整条素材宽度
				QRegion dirtyRegion(oldRect);
				dirtyRegion ^= QRegion(newRect);
				if (!dirtyRegion.isEmpty())
					pLayerWidget->update(dirtyRegion.boundingRect());
			}
		}
		return;
	}

	if ((event->buttons() & Qt::LeftButton) && !m_bDragStarted)
	{
		if ((event->pos() - m_dragStartPos).manhattanLength() >= QApplication::startDragDistance())
		{
			m_bDragStarted = true;
			StartObjectDrag(event);
			m_bDragStarted = false;
		}
		return;
	}

	UpdateHoverCursor(event->pos());
}

void CObjectWidget::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasFormat(g_TimelineObjectDragFormat))
	{
		if (auto* pLayer = qobject_cast<CLayerWidget*>(parentWidget()))
		{
			const QPoint posInLayer = mapTo(pLayer, event->pos());
			if (CLayersWidget* pLayers = FindLayersWidget())
				pLayers->PreviewTimelineObjectDrag(event->mimeData(), posInLayer);
		}
		event->setDropAction(Qt::MoveAction);
		event->accept();
		return;
	}
	event->ignore();
}

void CObjectWidget::dragMoveEvent(QDragMoveEvent* event)
{
	if (event->mimeData()->hasFormat(g_TimelineObjectDragFormat))
	{
		if (auto* pLayer = qobject_cast<CLayerWidget*>(parentWidget()))
		{
			const QPoint posInLayer = mapTo(pLayer, event->pos());
			if (CLayersWidget* pLayers = FindLayersWidget())
				pLayers->PreviewTimelineObjectDrag(event->mimeData(), posInLayer);
		}
		event->acceptProposedAction();
	}
	else
		event->ignore();
}

void CObjectWidget::dropEvent(QDropEvent* event)
{
	if (event->mimeData()->hasFormat(g_TimelineObjectDragFormat))
	{
		auto* pLayer = qobject_cast<CLayerWidget*>(parentWidget());
		if (!pLayer)
			return;

		const QPoint posInLayer = mapTo(pLayer, event->pos());
		if (pLayer->TryCommitObjectDrop(event->mimeData(), posInLayer))
			event->acceptProposedAction();
		return;
	}

	if (auto* pLayer = qobject_cast<CLayerWidget*>(parentWidget()))
	{
		const QPoint posInLayer = mapTo(pLayer, event->pos());
		pLayer->HandleResourceDropAt(event, posInLayer);
		return;
	}

	QWidget::dropEvent(event);
}

void CObjectWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
	if (m_bResizing || m_bDragStarted || ShouldYieldPressToTransFx(event) || IsParentLayerLocked())
		return;

	SyncGeometryFromModel();
	if (!IsClickOnObjectByModel(event->pos()))
		return;

	if (event->button() != Qt::LeftButton)
		return;

	ToggleClipFxTrackVisibility();
	event->accept();
}

void CObjectWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (m_bPathKfInteraction && event->button() == Qt::LeftButton)
	{
		FinishPathKeyframeInteraction(true);
		UpdateHoverCursor(event->pos());
		event->accept();
		return;
	}

	// 释放时若落在转场上，由转场处理，素材不再响应
	if (ShouldYieldPressToTransFx(event))
	{
		event->accept();
		return;
	}

	if (m_bResizing)
	{
		m_bResizing = false;
		UpdateHoverCursor(event->pos());

		std::chrono::milliseconds newDuration = g_pContentViewData->PositionToTime(width());
		if (newDuration < std::chrono::milliseconds(1))
			newDuration = std::chrono::milliseconds(1);

		std::chrono::milliseconds newStartTime = m_originalStartTime;
		if (m_resizeSide == RS_Left)
		{
			int dx = x() - m_originalX;
			newStartTime = m_originalStartTime - g_pContentViewData->PositionToTime(-dx);
			if (newStartTime < std::chrono::milliseconds(0))
				newStartTime = std::chrono::milliseconds(0);

			// 左缘拉伸钉住右边界：先帧对齐 end/start，再用 end - start，避免属性区 format(S)+format(D) 抖一帧
			auto oldEnd = m_originalStartTime + m_originalDuration;
			if (g_pConvertHelper)
			{
				oldEnd = g_pConvertHelper->formatTimeWithFrame(oldEnd);
				newStartTime = g_pConvertHelper->formatTimeWithFrame(newStartTime);
			}
			if (newStartTime >= oldEnd)
			{
				newStartTime = g_pConvertHelper
					? (oldEnd - g_pConvertHelper->frameToTimes(1))
					: (oldEnd - std::chrono::milliseconds(1));
			}
			if (newStartTime < std::chrono::milliseconds(0))
				newStartTime = std::chrono::milliseconds(0);
			if (g_pConvertHelper)
				newStartTime = g_pConvertHelper->formatTimeWithFrame(newStartTime);
			newDuration = oldEnd - newStartTime;
			if (newDuration < std::chrono::milliseconds(1))
				newDuration = std::chrono::milliseconds(1);
		}

		if (auto pLayer = g_pTimeLineModel->GetLayer(m_originalLayerId).lock())
		{
			if (auto pObject = pLayer->GetObject(m_objId).lock())
			{
				// 拉伸提交前：新区间不得完全覆盖同图层其它素材
				if (!TimeLineOverlapHelper::CanPlaceObjectOnLayer(
					m_originalLayerId, m_objId, newStartTime, newDuration))
				{
					SyncGeometryFromModel();
					LayoutChildWidgets();
					if (auto* pLayerWidget = qobject_cast<CLayerWidget*>(parentWidget()))
						pLayerWidget->RefreshTransFxForLayer();
					return;
				}
				// 路径相机片段禁止与同一相机其它路径片段时间重叠
				if (PathCameraClipHelper::IsPathCameraClip(m_objId)
					&& PathCameraClipHelper::IsPathClipRangeOverlap(newStartTime, newDuration, m_objId))
				{
					SyncGeometryFromModel();
					LayoutChildWidgets();
					if (auto* pLayerWidget = qobject_cast<CLayerWidget*>(parentWidget()))
						pLayerWidget->RefreshTransFxForLayer();
					CShowMessageWidget::ShowMessage(3000, CTimeLineWndTranslate::PathClipOverlapTip(), nullptr, ESMT_Warning);
					return;
				}

				pObject->SetStartTime(newStartTime);
				pObject->SetDuration(newDuration);
				// 路径相机：拉伸保绝对位置 / 平移保相对位置，写回前帧对齐并裁剪
				if (PathCameraClipHelper::IsPathCameraClip(m_objId))
				{
					if (m_originalStartTime != newStartTime)
						PathCameraClipHelper::SyncClipStartTimeToOwnerCamera(m_objId, newStartTime);
					if (m_originalStartTime != newStartTime || m_originalDuration != newDuration)
					{
						PathCameraClipHelper::AdjustPathCameraKeyframesForRangeChange(
							m_objId, m_originalStartTime, newStartTime,
							m_originalDuration, newDuration);
					}
				}
			}
		}
		HistoryStackSave();
		SyncGeometryFromModel();
		LayoutChildWidgets();
		if (m_pClip)
			m_pClip->UpdateView();
		// 拉伸结束：按最新模型数据刷新图层转场布局（含成对半场恢复）
		if (auto* pLayerWidget = qobject_cast<CLayerWidget*>(parentWidget()))
			pLayerWidget->RefreshTransFxForLayer();
		if (CLayersWidget* pParent = FindLayersWidget())
		{
			pParent->UpdateView();
			emit pParent->signalUpdateView(WndType::WndType_LayersView | WndType::WndType_LayerLabels | WndType::WndType_NoniusView);
		}
		emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
		emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_MediaNodeSelect);
		return;
	}

	if (event->button() == Qt::RightButton)
	{
		if (rect().contains(event->pos()))
		{
			ShowObjectContextMenu(event->globalPos());
			event->accept();
			return;
		}
	}
	if (!IsClickOnObjectByModel(event->pos()))
	{
		SyncGeometryFromModel();
		ForwardMouseEventToLayers(event);
		return;
	}
}
