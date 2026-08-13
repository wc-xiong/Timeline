#define _USE_MATH_DEFINES
#include "ClipWidget.h"

#include "LayerWidget.h"
#include "ContentViewData.h"
#include "Common/Common/ResourceDefine.h"
#include "Common/Common/ITimelineInterface.h"

#include "Controll/SelectController/SelectControllerExport.h"
#include "Controll/ViewDataManager/ViewDataManagerExport.h"

#include "Model/MediaModel/MediaModelExport.h"
#include "Model/ModelModel/ModelModelExport.h"
#include "Model/ResourceModel/ResourceModelExport.h"
#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "Model/TimeLineModel/TimeLineObject.h"
#include "Common/Common/IModelDataInterface.h"
#include "PathCameraClipHelper.h"
#include "ObjectWidget.h"

#include "UICommon/UICommonSizeDefine.h"

#include <QPainter>
#include <QFontMetrics>
#include <QPaintEvent>
#include <algorithm>
#include <QPolygonF>
#include "TimelineWinMacros.h"
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif

namespace
{
	QColor GetTimelineLayerBackground(const QWidget* pWidget)
	{
		for (const QWidget* p = pWidget; p; p = p->parentWidget())
		{
			if (auto* pLayer = qobject_cast<const CLayerWidget*>(p))
			{
				return g_pSelectController->IsTimeLineLayerSelected(pLayer->GetLayerId())
					? QColor("#3E4044") : QColor("#303236");
			}
		}
		return QColor("#303236");
	}
	CTimeLineObject* GetTimelineObject(const QUuid& objectId)
	{
		if (objectId.isNull() || !g_pTimeLineModel)
			return nullptr;
		auto pObj = g_pTimeLineModel->GetTimeLineObject(objectId).lock();
		return dynamic_cast<CTimeLineObject*>(pObj.get());
	}

	int ClipLocalTimeToX(const std::chrono::milliseconds& clipLocalMs,
		const std::chrono::milliseconds& objectStart)
	{
		if (!g_pContentViewData)
			return 0;
		return static_cast<int>(g_pContentViewData->TimeToPosition(objectStart + clipLocalMs)
			- g_pContentViewData->TimeToPosition(objectStart));
	}

	/**
	 * @brief 路径相机关键帧：按时间线绝对时刻定位到片段控件内 X
	 * 拉伸过程中模型尚未改 start，用当前 ObjectWidget 几何作临时起点，菱形不随左缘平移
	 */
	int PathCameraKeyframeToClipX(const std::chrono::milliseconds& clipLocalMs,
		const std::chrono::milliseconds& modelStart, const QWidget* pClipWidget)
	{
		if (!g_pContentViewData || !pClipWidget)
			return ClipLocalTimeToX(clipLocalMs, modelStart);

		const auto* pObjectWidget = qobject_cast<const CObjectWidget*>(pClipWidget->parentWidget());
		if (!pObjectWidget)
			return ClipLocalTimeToX(clipLocalMs, modelStart);

		const auto absTime = modelStart + clipLocalMs;
		const int absLayerX = g_pContentViewData->TimeToPosition(absTime)
			+ g_pContentViewData->GetXOffset();
		return absLayerX - pObjectWidget->x();
	}

	void DrawPropertyKeyframeDiamond(QPainter& painter, int centerX, int centerY, int halfSize,
		bool bDragging = false, bool bPlayheadSelected = false)
	{
		const qreal cx = centerX;
		const qreal cy = centerY;
		const qreal hs = halfSize;
		QPolygonF poly;
		poly << QPointF(cx, cy - hs) << QPointF(cx + hs, cy)
			<< QPointF(cx, cy + hs) << QPointF(cx - hs, cy);
		// 拖拽蓝优先于游标紫选中
		if (bDragging)
		{
			painter.setBrush(QColor("#3D8BFF"));
			painter.setPen(QPen(Qt::black, nSizeDefine_1));
		}
		else if (bPlayheadSelected)
		{
			painter.setBrush(QColor("#EB66FF"));
			painter.setPen(QPen(Qt::black, nSizeDefine_1));
		}
		else
		{
			painter.setBrush(Qt::white);
			painter.setPen(QPen(Qt::black, nSizeDefine_1));
		}
		painter.drawPolygon(poly);
	}

	void DrawPropertyKeyframeMarkers(QPainter& painter, const QRect& paintRc, const QRect& rc,
		const QUuid& objectId, const QWidget* pClipWidget)
	{
		auto* pObject = GetTimelineObject(objectId);
		if (!pObject)
			return;

		const auto objectStart = pObject->GetStartTime();
		const int markerY = rc.y() + nSizeDefine_13 + nSizeDefine_10;
		const int halfSize = SysDpiZoom(6);

		bool bPathCameraKeyframes = false;
		auto AllLocalMs = pObject->GetAllPropertyKeyframeTimes();		//一般媒体
		if (AllLocalMs.empty())//路径相机
		{
			bPathCameraKeyframes = true;
			AllLocalMs = PathCameraClipHelper::CollectPathCameraKeyframeLocalTimes(objectId);
		}

		std::chrono::milliseconds selectedLocal{ 0 };
		std::chrono::milliseconds previewLocal{ 0 };
		std::chrono::milliseconds originLocal{ 0 };
		std::chrono::milliseconds playheadSelectedLocal{ 0 };
		bool bHasSelected = false;
		bool bShowPreview = false;
		bool bInteraction = false;
		bool bPlayheadHasSelected = false;
		if (bPathCameraKeyframes)
		{
			if (auto* pObjectWidget = qobject_cast<const CObjectWidget*>(
				pClipWidget ? pClipWidget->parentWidget() : nullptr))
			{
				pObjectWidget->GetPathKeyframeDrawState(
					selectedLocal, bHasSelected, previewLocal, bShowPreview, originLocal, bInteraction);
			}
			bPlayheadHasSelected = PathCameraClipHelper::TryGetPlayheadSelectedPathKeyframe(
				objectId, playheadSelectedLocal);
		}

		auto drawAtLocal = [&](const std::chrono::milliseconds& clipLocalMs,
			bool bDragging, bool bPlayheadSelected)
		{
			int x = rc.x() + (bPathCameraKeyframes
				? PathCameraKeyframeToClipX(clipLocalMs, objectStart, pClipWidget)
				: ClipLocalTimeToX(clipLocalMs, objectStart));
			if (x + halfSize < paintRc.left() || x - halfSize > paintRc.right())
				return;
			if (bPathCameraKeyframes)
			{
				const int left = rc.x();
				const int right = rc.x() + rc.width();
				if (x < left)
					x = left;
				else if (x > right)
					x = right;
			}
			DrawPropertyKeyframeDiamond(painter, x, markerY, halfSize, bDragging, bPlayheadSelected);
		};

		for (const auto& clipLocalMs : AllLocalMs)
		{
			// 拖动中：隐藏尚未提交的原位置菱形
			if (bPathCameraKeyframes && bShowPreview && clipLocalMs == originLocal
				&& originLocal != previewLocal)
				continue;

			bool bDragging = false;
			if (bPathCameraKeyframes && bInteraction)
			{
				if (bShowPreview)
					bDragging = (clipLocalMs == previewLocal);
				else if (bHasSelected)
					bDragging = (clipLocalMs == selectedLocal);
			}
			const bool bPlayheadSelected = bPathCameraKeyframes && bPlayheadHasSelected
				&& !bDragging && clipLocalMs == playheadSelectedLocal;
			drawAtLocal(clipLocalMs, bDragging, bPlayheadSelected);
		}

		// 拖动预览：preview 不在模型列表时额外绘制（蓝）
		if (bPathCameraKeyframes && bShowPreview)
		{
			const bool bInList = std::find(AllLocalMs.begin(), AllLocalMs.end(), previewLocal)
				!= AllLocalMs.end();
			if (!bInList)
				drawAtLocal(previewLocal, true, false);
		}
	}

	void DrawPathCameraKeyframeDiamond(QPainter& painter, int centerX, int centerY, int halfSize)
	{
		// 默认样式：实心白菱形，无描边（选中白框由片段通用选中态绘制）
		QPolygonF poly;
		poly << QPointF(centerX, centerY - halfSize)
			<< QPointF(centerX + halfSize, centerY)
			<< QPointF(centerX, centerY + halfSize)
			<< QPointF(centerX - halfSize, centerY);
		painter.setBrush(Qt::white);
		painter.setPen(Qt::NoPen);
		painter.drawPolygon(poly);
	}

	void DrawPathCameraClipContent(QPainter& painter, const QRect& paintRc, const QRect& rc,
		const QUuid& objectId, const QString& name)
	{
		auto* pObject = GetTimelineObject(objectId);
		if (!pObject)
			return;

		// 标题（左上）
		painter.setPen(QColor("#DCDCDC"));
		QFont f = painter.font();
		f.setPixelSize(nSizeDefine_10);
		painter.setFont(f);
		const int titleH = nSizeDefine_10;
		const int titleLeft = rc.x() + nSizeDefine_4;
		const int titleW = std::max(0, rc.width() - nSizeDefine_8);
		if (titleW > 0)
			painter.drawText(titleLeft, rc.y() + nSizeDefine_1, titleW, titleH, Qt::AlignLeft | Qt::AlignVCenter, name);

		// 中部深色轨道（默认无白框）
		const int trackH = nSizeDefine_20;
		const int trackY = rc.y() + nSizeDefine_13;
		const int trackLeft = rc.x() + nSizeDefine_4;
		const int trackW = std::max(0, rc.width() - nSizeDefine_8);
		if (trackW <= 0)
			return;
		const QRect trackRc(trackLeft, trackY, trackW, trackH);
		painter.setPen(Qt::NoPen);
		painter.setBrush(QColor("#1F2124"));
		painter.drawRect(trackRc);

		const auto durationMs = pObject->GetDuration().count();
		/*const auto times = CollectPathCameraKeyframeTimes(pObject);
		const int halfSize = SysDpiZoom(5);
		const int markerY = trackY + trackH / 2;

		auto TimeToTrackX = [&](const std::chrono::milliseconds& clipLocalMs) -> int
		{
			if (durationMs <= 0)
				return trackLeft;
			const qint64 t = std::clamp<qint64>(clipLocalMs.count(), 0, durationMs);
			return trackLeft + static_cast<int>(t * trackW / durationMs);
		};

		if (times.size() >= 2)
		{
			const int x0 = TimeToTrackX(times.front());
			const int x1 = TimeToTrackX(times.back());
			painter.setPen(QPen(QColor("#AAAEB3"), nSizeDefine_1));
			painter.drawLine(x0, markerY, x1, markerY);
		}

		for (const auto& clipLocalMs : times)
		{
			const int x = TimeToTrackX(clipLocalMs);
			if (x + halfSize < paintRc.left() || x - halfSize > paintRc.right())
				continue;
			DrawPathCameraKeyframeDiamond(painter, x, markerY, halfSize);
		}*/
	}
}

CClipWidget::CClipWidget(QWidget* pParent /*= 0*/)
	: QWidget(pParent)
{
	setAttribute(Qt::WA_OpaquePaintEvent);
}

CClipWidget::~CClipWidget()
{
}

void CClipWidget::SetObjectId(const QUuid& id)
{
	m_objId = id;
	update();
}

void CClipWidget::UpdateView()
{
	update();
}

void CClipWidget::paintEvent(QPaintEvent* event)
{
	const QRect rc = rect();
	const QRect paintRc = event->rect().intersected(rc);
	if (paintRc.isEmpty())
		return;

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setClipRect(paintRc);
	painter.fillRect(paintRc, GetTimelineLayerBackground(this));

	if (m_objId.isNull())
	{
		QWidget::paintEvent(event);
		return;
	}

	const bool bSelect = g_pSelectController->IsTimeLineObjectSelected(m_objId);
	painter.setBrush(QColor(61, 139, 255, 102));
	painter.setPen(Qt::NoPen);
	QRect r = rc;
	r.setLeft(std::max(rc.x(), -nSizeDefine_4));
	r.setRight(std::min(rc.x() + rc.width(), width() + nSizeDefine_4) - nSizeDefine_1);
	painter.drawRoundedRect(r, nSizeDefine_4, nSizeDefine_4);

	QString name;
	int nMediaTime = rc.width();
	bool bPathCameraClip = false;

	auto pMedia = g_pMediaModel->GetMediaNode(m_objId).lock();
	if (pMedia)
	{
		name = pMedia->GetMediaName();
		nMediaTime = g_pContentViewData->TimeToPosition(pMedia->GetClipEndTime() - pMedia->GetClipStartTime());
		auto pResource = g_pResourceModel->GetResourceNode(pMedia->GetResourceId());
		if (pResource)
		{
			auto pixmap = pResource->GetThumbnail(true, false);
			pixmap = pixmap.scaled(QSize(nSizeDefine_38, nSizeDefine_20));
			int x = rc.x() + nSizeDefine_4;
			int w = 0;
			if (x < 0)
			{
				w = abs(x) / nSizeDefine_38 * nSizeDefine_38;
				x = x + w;
			}
			w = rc.width() - nSizeDefine_8 - w;
			if (x + w > width())
			{
				int n = (width() - x) / nSizeDefine_38 + 1;
				w = n * nSizeDefine_38;
			}
			const int drawLeft = std::max(x, paintRc.left());
			const int drawRight = std::min(x + w, paintRc.right() + 1);
			if (drawRight > drawLeft)
				painter.drawTiledPixmap(drawLeft, rc.y() + nSizeDefine_13, drawRight - drawLeft, nSizeDefine_20, pixmap);
		}
	}
	else if (PathCameraClipHelper::FindCameraByPathClipId(m_objId))
	{
		bPathCameraClip = true;
		if (auto pTlObj = g_pTimeLineModel->GetTimeLineObject(m_objId).lock())
			name = pTlObj->GetDisplayName();
		
		// 中部深色轨道（默认无白框）
		const int trackH = nSizeDefine_20;
		const int trackY = rc.y() + nSizeDefine_14;
		const int trackLeft = rc.x() + nSizeDefine_1;
		const int trackW = std::max(0, rc.width() - nSizeDefine_2);
		if (trackW <= 0)
			return;
		const QRect trackRc(trackLeft, trackY, trackW, trackH);
		painter.setPen(Qt::NoPen);
		painter.setBrush(QColor("#1F2124"));
		painter.drawRect(trackRc);

		const int markerY = trackY + trackH / 2;
		painter.setPen(QPen(QColor("#CCD0D7"), nSizeDefine_1));
		painter.drawLine(trackLeft + nSizeDefine_1, markerY, trackW - nSizeDefine_1, markerY);
		

	}
	else if (auto pModel = g_pModelDataModel->GetModel(m_objId).lock())
	{
		name = pModel->GetName();
		QString path;
		switch (pModel->GetModelType())
		{
		case EMTNT_Sphere: path = ":/dark/images/GridWnd/Model/Sphere.svg"; break;
		case EMTNT_Camera: path = ":/dark/images/GridWnd/Model/Camera.svg"; break;
		case EMTNT_DirectionalLight: path = ":/dark/images/GridWnd/Model/DirectionalLight.svg"; break;
		case EMTNT_PointLight: path = ":/dark/images/GridWnd/Model/PointLight.svg"; break;
		case EMTNT_SpotLight: path = ":/dark/images/GridWnd/Model/SpotLight.svg"; break;
		case EMTNT_IrregularPrism: path = ":/dark/images/GridWnd/Model/IrregularPrism.svg"; break;
		case EMTNT_RotationSolid: path = ":/dark/images/GridWnd/Model/RotationSolid.svg"; break;
		case EMTNT_ImmersiveSpace: path = ":/dark/images/GridWnd/Model/ImmersiveSpace.svg"; break;
		case EMTNT_Plane: path = ":/dark/images/GridWnd/Model/Plane.svg"; break;
		case EMTNT_ModuleInference: path = ":/dark/images/GridWnd/Model/ModuleInference.svg"; break;
		case EMTNT_AIPanorama: path = ":/dark/images/GridWnd/Model/AIPanorama.svg"; break;
		case EMTNT_VRCamera: path = ":/dark/images/GridWnd/Model/VRCamera.svg"; break;
		default: path = ":/dark/images/GridWnd/Model/Custom.svg"; break;
		}
		QPixmap pixmap(path);
		pixmap = pixmap.scaled(QSize(nSizeDefine_20, nSizeDefine_20));
		int x = rc.x() + nSizeDefine_4;
		int w = rc.width() - nSizeDefine_8;
		const int drawLeft = std::max(x, paintRc.left());
		const int drawRight = std::min(x + w, paintRc.right() + 1);
		if (drawRight > drawLeft)
			painter.drawTiledPixmap(drawLeft, rc.y() + nSizeDefine_13, drawRight - drawLeft, nSizeDefine_20, pixmap);
	}

	painter.setPen(Qt::white);
	QFont f = painter.font();
	f.setPixelSize(nSizeDefine_10);
	painter.setFont(f);
	int wName = QFontMetrics(f).horizontalAdvance(name);
	const int textStep = std::max(1, wName + nSizeDefine_48);
	int pos = nSizeDefine_4;
	if (paintRc.left() > nSizeDefine_4)
		pos = nSizeDefine_4 + ((paintRc.left() - nSizeDefine_4) / textStep) * textStep;
	for (; pos < rc.width() && pos <= paintRc.right(); pos += textStep)
	{
		int wTemp = wName;
		if (rc.width() - pos - nSizeDefine_4 < wTemp)
			wTemp = rc.width() - pos - nSizeDefine_4;
		if (wTemp > 0 && pos + wTemp > paintRc.left() && pos < paintRc.right())
			painter.drawText(pos, rc.y() + nSizeDefine_1, wTemp, nSizeDefine_10, Qt::AlignLeft, name);
	}

	DrawPropertyKeyframeMarkers(painter, paintRc, rc, m_objId, this);

	if (bSelect)
		nMediaTime += g_pContentViewData->TimeToPosition(g_pViewDataManager->GetTimeClipDurationOffset());

	QPen pen;
	pen.setColor(Qt::white);
	pen.setWidth(nSizeDefine_1);
	pen.setStyle(Qt::DashLine);
	painter.setPen(pen);
	if (nMediaTime > 0)
	{
		const int lineStart = std::max(rc.x() + nMediaTime, paintRc.left());
		const int lineEnd = std::min(rc.x() + rc.width(), paintRc.right() + 1);
		for (int linePos = rc.x() + nMediaTime; linePos < lineEnd; linePos += nMediaTime)
		{
			if (linePos >= lineStart && linePos > 0 && linePos < width())
				painter.drawLine(linePos, rc.y(), linePos, rc.y() + rc.height());
		}
	}

	if (bSelect)
	{
		painter.setBrush(Qt::NoBrush);
		painter.setPen(QPen(Qt::white, nSizeDefine_2));
		r.setLeft(r.left() + nSizeDefine_1);
		painter.drawRoundedRect(r, nSizeDefine_2, nSizeDefine_2);
	}

	QWidget::paintEvent(event);
}
