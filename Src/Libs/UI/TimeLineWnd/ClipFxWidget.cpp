#include "ClipFxWidget.h"
#include "ClipFxPlotWidget.h"
#include "ClipFxPropertyRowWidget.h"
#include "ClipFxDefine.h"
#include "Common/MediaPropertyKeyframeIds.h"
#include "ClipFxKeyframeHelper.h"
#include "UICommon/PropertyKeyFrameBar.h"
#include "ContentViewData.h"
#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "Model/TimeLineModel/TimeLineObject.h"
#include "Common/SignalCenter/SignalCenterExport.h"
#include "Common/SignalCenter/GSNetSignalCenter.h"
#include "Controll/PlayStateCenter/TimelinePropertyKeyframeHelper.h"
#include "Controll/PlayerController/PlayerControllerExport.h"
#include "Controll/GlobalHistoryStack/GlobalHistoryStackExport.h"
#include "UICommon/UICommonSizeDefine.h"
#include "UICommon/UITranslate.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QFrame>
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>
#include <QEvent>
#include <QSet>
#include <QSize>
#include <QAbstractItemView>
#include <algorithm>
#include <set>
#include "TimelineWinMacros.h"

namespace
{
	enum ETreeItemType
	{
		kTreeItemGroup = 0,
		kTreeItemProperty = 1,
		kTreeItemPlot = 2,
	};

	const int kTreeItemTypeRole = Qt::UserRole + 1;

	CTimeLineObject* GetTimelineObject(const QUuid& id)
	{
		if (!g_pTimeLineModel || id.isNull())
			return nullptr;
		auto pObj = g_pTimeLineModel->GetTimeLineObject(id).lock();
		return dynamic_cast<CTimeLineObject*>(pObj.get());
	}

	QString GroupName(EMediaPropertyGroupId groupId)
	{
		switch (groupId)
		{
		case EMediaPropertyGroupId::Volume: return CPropertyTranslate::Volumn();
		case EMediaPropertyGroupId::Position: return CCommonTranslate::Position();
		case EMediaPropertyGroupId::Scale: return CSliceViewTranslate::Zoom();
		case EMediaPropertyGroupId::Rotation: return CSliceViewTranslate::Rotate();
		case EMediaPropertyGroupId::Crop: return CFxPropertyTranslate::Cut();
		case EMediaPropertyGroupId::Feather: return CFxPropertyTranslate::Feather();
		case EMediaPropertyGroupId::Keying: return CFxPropertyTranslate::Keying();
		case EMediaPropertyGroupId::Effect: return CFxPropertyTranslate::Effect();
		default: return QString();
		}
	}

	QString PropertyName(EMediaPropertyId propId)
	{
		switch (propId)
		{
		case EMediaPropertyId::VolumeIntensity: return CPropertyTranslate::Intensity();
		case EMediaPropertyId::PosX: return QStringLiteral("X");
		case EMediaPropertyId::PosY: return QStringLiteral("Y");
		case EMediaPropertyId::PosZ: return QStringLiteral("Z");
		case EMediaPropertyId::ScaleWidth: return CCommonTranslate::W();
		case EMediaPropertyId::ScaleHeight: return CCommonTranslate::H();
		case EMediaPropertyId::RotX: return QStringLiteral("X");
		case EMediaPropertyId::RotY: return QStringLiteral("Y");
		case EMediaPropertyId::RotZ: return QStringLiteral("Z");
		case EMediaPropertyId::CropFill: return CFxPropertyTranslate::Complement();
		case EMediaPropertyId::CropLeft: return CFxPropertyTranslate::Left();
		case EMediaPropertyId::CropRight: return CFxPropertyTranslate::Right();
		case EMediaPropertyId::CropTop: return CFxPropertyTranslate::Top();
		case EMediaPropertyId::CropBottom: return CFxPropertyTranslate::Bottom();
		case EMediaPropertyId::FeatherRadius: return CFxPropertyTranslate::FeatherRadius();
		case EMediaPropertyId::KeyingRed: return CCommonTranslate::Red();
		case EMediaPropertyId::KeyingGreen: return CCommonTranslate::Green();
		case EMediaPropertyId::KeyingBlue: return CCommonTranslate::Blue();
		case EMediaPropertyId::KeyingTolerance: return CFxPropertyTranslate::Threshold();
		case EMediaPropertyId::KeyingDegree: return CFxPropertyTranslate::Degree();
		case EMediaPropertyId::EffectRed: return CCommonTranslate::Red();
		case EMediaPropertyId::EffectGreen: return CCommonTranslate::Green();
		case EMediaPropertyId::EffectBlue: return CCommonTranslate::Blue();
		case EMediaPropertyId::EffectAlpha: return CCommonTranslate::Transparent();
		case EMediaPropertyId::EffectLuminance: return CCommonTranslate::Brightness();
		case EMediaPropertyId::EffectContrast: return CCommonTranslate::Contrast();
		case EMediaPropertyId::EffectHue: return CCommonTranslate::Hue();
		case EMediaPropertyId::EffectSaturation: return CCommonTranslate::Saturation();
		default: return QString();
		}
	}

	QString PropertyDisplayName(EMediaPropertyGroupId groupId, EMediaPropertyId propId)
	{
		switch (groupId)
		{
		case EMediaPropertyGroupId::Position:
			switch (propId)
			{
			case EMediaPropertyId::PosX: return QStringLiteral("X");
			case EMediaPropertyId::PosY: return QStringLiteral("Y");
			case EMediaPropertyId::PosZ: return QStringLiteral("Z");
			default: break;
			}
			break;
		case EMediaPropertyGroupId::Rotation:
			switch (propId)
			{
			case EMediaPropertyId::RotX: return QStringLiteral("X");
			case EMediaPropertyId::RotY: return QStringLiteral("Y");
			case EMediaPropertyId::RotZ: return QStringLiteral("Z");
			default: break;
			}
			break;
		case EMediaPropertyGroupId::Scale:
			switch (propId)
			{
			case EMediaPropertyId::ScaleWidth: return QStringLiteral("X");
			case EMediaPropertyId::ScaleHeight: return QStringLiteral("Y");
			default: break;
			}
			break;
		default:
			break;
		}
		return PropertyName(propId);
	}

	std::vector<EMediaPropertyId> GroupProps(EMediaPropertyGroupId groupId)
	{
		switch (groupId)
		{
		case EMediaPropertyGroupId::Volume: return { EMediaPropertyId::VolumeIntensity };
		case EMediaPropertyGroupId::Position:
			return { EMediaPropertyId::PosX, EMediaPropertyId::PosY, EMediaPropertyId::PosZ };
		case EMediaPropertyGroupId::Scale:
			return { EMediaPropertyId::ScaleWidth, EMediaPropertyId::ScaleHeight };
		case EMediaPropertyGroupId::Rotation:
			return { EMediaPropertyId::RotX, EMediaPropertyId::RotY, EMediaPropertyId::RotZ };
		case EMediaPropertyGroupId::Crop:
			return { EMediaPropertyId::CropFill, EMediaPropertyId::CropLeft, EMediaPropertyId::CropRight,
				EMediaPropertyId::CropTop, EMediaPropertyId::CropBottom };
		case EMediaPropertyGroupId::Feather: return { EMediaPropertyId::FeatherRadius };
		case EMediaPropertyGroupId::Keying:
			return { EMediaPropertyId::KeyingRed, EMediaPropertyId::KeyingGreen, EMediaPropertyId::KeyingBlue,
				EMediaPropertyId::KeyingTolerance, EMediaPropertyId::KeyingDegree };
		case EMediaPropertyGroupId::Effect:
			return { EMediaPropertyId::EffectRed, EMediaPropertyId::EffectGreen, EMediaPropertyId::EffectBlue,
				EMediaPropertyId::EffectAlpha, EMediaPropertyId::EffectLuminance, EMediaPropertyId::EffectContrast,
				EMediaPropertyId::EffectHue, EMediaPropertyId::EffectSaturation };
		default: return {};
		}
	}

	const EMediaPropertyGroupId kGroups[] = {
		EMediaPropertyGroupId::Volume,
		EMediaPropertyGroupId::Position,
		EMediaPropertyGroupId::Scale,
		EMediaPropertyGroupId::Rotation,
		EMediaPropertyGroupId::Crop,
		EMediaPropertyGroupId::Feather,
		EMediaPropertyGroupId::Keying,
		EMediaPropertyGroupId::Effect,
	};

	struct SClipFxTreeExpansion
	{
		QSet<int> expandedGroups;
		QSet<int> expandedProps;
	};

	SClipFxTreeExpansion CaptureTreeExpansion(QTreeWidget* pTree)
	{
		SClipFxTreeExpansion exp;
		if (!pTree)
			return exp;
		for (int i = 0; i < pTree->topLevelItemCount(); ++i)
		{
			auto* pGroup = pTree->topLevelItem(i);
			if (pGroup->isExpanded())
				exp.expandedGroups.insert(pGroup->data(0, Qt::UserRole).toInt());
			for (int j = 0; j < pGroup->childCount(); ++j)
			{
				auto* pPropItem = pGroup->child(j);
				if (pPropItem->data(0, kTreeItemTypeRole).toInt() != kTreeItemProperty)
					continue;
				if (pPropItem->isExpanded())
					exp.expandedProps.insert(pPropItem->data(0, Qt::UserRole).toInt());
			}
		}
		return exp;
	}

	void RestoreTreeExpansion(QTreeWidget* pTree, const SClipFxTreeExpansion& exp)
	{
		if (!pTree)
			return;
		pTree->blockSignals(true);
		for (int i = 0; i < pTree->topLevelItemCount(); ++i)
		{
			auto* pGroup = pTree->topLevelItem(i);
			const int groupId = pGroup->data(0, Qt::UserRole).toInt();
			const bool bGroupExpanded = exp.expandedGroups.contains(groupId);
			pGroup->setExpanded(bGroupExpanded);
			for (int j = 0; j < pGroup->childCount(); ++j)
			{
				auto* pPropItem = pGroup->child(j);
				if (pPropItem->data(0, kTreeItemTypeRole).toInt() != kTreeItemProperty)
					continue;
				const int propId = pPropItem->data(0, Qt::UserRole).toInt();
				pPropItem->setExpanded(bGroupExpanded && exp.expandedProps.contains(propId));
			}
		}
		pTree->blockSignals(false);
	}

	void UpdateTreeItemSizeHints(QTreeWidget* pTree)
	{
		if (!pTree)
			return;
		for (int i = 0; i < pTree->topLevelItemCount(); ++i)
		{
			auto* pGroup = pTree->topLevelItem(i);
			pGroup->setSizeHint(0, QSize(0, CLIPFX_GROUP_ROW_HEIGHT));
			for (int j = 0; j < pGroup->childCount(); ++j)
			{
				auto* pPropItem = pGroup->child(j);
				if (pPropItem->data(0, kTreeItemTypeRole).toInt() != kTreeItemProperty)
					continue;
				pPropItem->setSizeHint(0, QSize(0, CLIPFX_PROP_ROW_HEIGHT));
				for (int k = 0; k < pPropItem->childCount(); ++k)
				{
					auto* pPlotItem = pPropItem->child(k);
					if (pPlotItem->data(0, kTreeItemTypeRole).toInt() == kTreeItemPlot)
						pPlotItem->setSizeHint(0, QSize(0, CLIPFX_PLOT_ROW_HEIGHT));
				}
			}
		}
	}
}

CClipFxWidget::CClipFxWidget(QWidget* pParent)
	: QWidget(pParent)
{
	InitWidget();
	CreateConnects();
	InstallArrowCursorFilters();
	m_bInitialized = true;
}

CClipFxWidget::~CClipFxWidget()
{
	ClearPlotWidgets();
	ClearPropertyRowWidgets();
}

void CClipFxWidget::InitWidget()
{
	setCursor(Qt::ArrowCursor);

	auto* pLayout = new QVBoxLayout(this);
	pLayout->setContentsMargins(0, 0, 0, 0);
	pLayout->setSpacing(0);

	m_pTreeScroll = new QScrollArea(this);
	m_pTreeScroll->setWidgetResizable(true);
	m_pTreeScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_pTreeScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_pTreeScroll->setFrameShape(QFrame::NoFrame);
	m_pTreeScroll->setCursor(Qt::ArrowCursor);

	m_pTree = new QTreeWidget;
	m_pTree->setCursor(Qt::ArrowCursor);
	m_pTree->setHeaderHidden(true);
	m_pTree->setRootIsDecorated(true);
	m_pTree->setIndentation(CLIPFX_TREE_INDENTATION);
	m_pTree->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_pTree->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_pTree->setFrameShape(QFrame::NoFrame);
	m_pTree->setSelectionMode(QAbstractItemView::NoSelection);
	m_pTree->setFocusPolicy(Qt::NoFocus);
	m_pTree->setStyleSheet(
		QStringLiteral(
			"QTreeWidget { background: rgb(28,28,28); color: rgb(200,200,200); border: none; outline: none; }"
			"QTreeWidget::item { background: transparent; border: none; }"
			"QTreeWidget::item:hover { background: transparent; }"
			"QTreeWidget::item:selected { background: transparent; color: rgb(200,200,200); }"
			"QTreeWidget::item:selected:active { background: transparent; }"
			"QTreeWidget::branch:hover { background: transparent; }"));
	m_pTreeScroll->setWidget(m_pTree);
	pLayout->addWidget(m_pTreeScroll);
}

void CClipFxWidget::enterEvent(QEvent* event)
{
	setCursor(Qt::ArrowCursor);
	QWidget::enterEvent(event);
}

void CClipFxWidget::InstallArrowCursorFilters()
{
	setCursor(Qt::ArrowCursor);
	if (m_pTreeScroll)
		m_pTreeScroll->setCursor(Qt::ArrowCursor);
	if (m_pTree)
	{
		m_pTree->setCursor(Qt::ArrowCursor);
		if (m_pTree->viewport())
		{
			m_pTree->viewport()->setCursor(Qt::ArrowCursor);
			m_pTree->viewport()->installEventFilter(this);
		}
	}

	const auto widgets = findChildren<QWidget*>();
	for (auto* pWidget : widgets)
	{
		if (!pWidget || pWidget == this)
			continue;
		pWidget->setCursor(Qt::ArrowCursor);
		pWidget->installEventFilter(this);
	}
}

bool CClipFxWidget::eventFilter(QObject* watched, QEvent* event)
{
	switch (event->type())
	{
	case QEvent::Enter:
	case QEvent::MouseMove:
	case QEvent::HoverMove:
		if (auto* pWidget = qobject_cast<QWidget*>(watched))
			pWidget->setCursor(Qt::ArrowCursor);
		setCursor(Qt::ArrowCursor);
		break;
	default:
		break;
	}
	return QWidget::eventFilter(watched, event);
}

void CClipFxWidget::CreateConnects()
{
	connect(m_pTree, &QTreeWidget::itemExpanded, this, &CClipFxWidget::OnTreeItemExpanded);
	connect(m_pTree, &QTreeWidget::itemCollapsed, this, &CClipFxWidget::OnTreeItemCollapsed);
	if (m_pTree->verticalScrollBar())
	{
		connect(m_pTree->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int) {
			ScheduleUpdatePlotItemGeometries();
		});
	}
	connect(g_pSignalCenter, &CSignalCenter::signalNoniusTimeChanged, this, [this](qint64) {
		// 播放中只重绘游标线；暂停/Seek 再做选中与关键帧条全量刷新
		const bool bPlaying = g_pPlayerController && g_pPlayerController->IsPlaying();
		if (!bPlaying)
		{
			SyncSelectionWithPlayhead();
			RefreshKeyframeUi();
		}
		for (auto* pPlot : m_vecPlotWidgets)
		{
			if (pPlot && pPlot->isVisible())
				pPlot->update();
		}
	});
	connect(g_pSignalCenter, &CSignalCenter::signalRefreshWindow, this,
		[this](int wnd, int dataType) {
			if (!(wnd & SignalCenter::EWT_TimeLineWnd))
				return;
			if (dataType != SignalCenter::EDDT_MediaNodeDataChange
				&& dataType != SignalCenter::EDDT_All)
				return;
			OnKeyframeDataChanged();
		});
	connect(g_pSignalCenter, &CSignalCenter::signalClipFxPropertyKeyframeChanged, this,
		[this](const QUuid& mediaId, int nPropId, qint64 clipLocalMs) {
			if (mediaId != m_objectId)
				return;
			OnPropertyKeyframeChangedFromPanel(static_cast<EMediaPropertyId>(nPropId),
				std::chrono::milliseconds(clipLocalMs));
		});
	connect(g_pSignalCenter, &CSignalCenter::signalResetDataByHistoryStack, this, [this]() {
		OnKeyframeDataChanged();
	});
}

void CClipFxWidget::SetObjectId(const QUuid& id)
{
	m_objectId = id;
	if (m_bInitialized)
		RebuildTree();
}

void CClipFxWidget::NotifyHeightChanged()
{
	if (!m_bInitialized)
		return;
	if (m_bRebuildingTree)
		return;
	// 高度未变时不通知图层 Relayout，避免 LayerLabels 与 Layers 短暂不同步（图层缝隙闪烁）
	const int h = CalcHeight();
	if (h == m_nLastNotifiedHeight)
		return;
	m_nLastNotifiedHeight = h;
	emit signalHeightChanged();
}

void CClipFxWidget::NotifyClipKeyframeMarkersChanged()
{
	emit signalClipKeyframeMarkersChanged();
}

void CClipFxWidget::OnKeyframeDataChanged()
{
	auto pObject = GetTimelineObject(m_objectId);
	if (!pObject || !pObject->HasAnyPropertyKeyframes())
	{
		// 必须同步清树：若仅 hide 保留旧节点，再次添加同组关键帧时
		// NeedsTreeStructureRebuild 会误判为结构未变，跳过 Rebuild / UpdateViewportLayout，
		// 出现图层行高变化但 ClipFx 内容区仍为 0 高度的空白。
		m_selection = {};
		RebuildTree();
		return;
	}

	if (NeedsTreeStructureRebuild())
	{
		ScheduleRebuildTree();
		NotifyClipKeyframeMarkersChanged();
		return;
	}

	const bool bWasHidden = !isVisible();
	SyncSelectionWithPlayhead();
	for (auto* pPlot : m_vecPlotWidgets)
	{
		if (pPlot)
			pPlot->UpdatePlot();
	}
	RefreshKeyframeUi();
	// 树结构与展开未变时高度通常不变：禁止无谓 Relayout，避免滚动条/图层缝隙闪烁
	SyncClipFxVisibility();
	// 从隐藏恢复时必须刷新视口（取消全空曾把 scroll 固定为 0）
	if (bWasHidden && isVisible())
	{
		UpdateViewportLayout();
		NotifyHeightChanged();
	}
	NotifyClipKeyframeMarkersChanged();
}

bool CClipFxWidget::NeedsTreeStructureRebuild() const
{
	auto pObject = GetTimelineObject(m_objectId);
	if (!pObject || !m_pTree)
		return true;

	std::set<int> modelGroups;
	std::set<int> modelProps;
	for (auto groupId : kGroups)
	{
		bool bGroupHasData = false;
		for (auto propId : GroupProps(groupId))
		{
			if (pObject->IsPropertyKeyframeTrackEmpty(static_cast<int>(propId)))
				continue;
			bGroupHasData = true;
			modelProps.insert(static_cast<int>(propId));
		}
		if (bGroupHasData)
			modelGroups.insert(static_cast<int>(groupId));
	}

	if (m_pTree->topLevelItemCount() == 0)
		return !modelProps.empty();

	std::set<int> treeGroups;
	std::set<int> treeProps;
	for (int i = 0; i < m_pTree->topLevelItemCount(); ++i)
	{
		auto* pGroup = m_pTree->topLevelItem(i);
		treeGroups.insert(pGroup->data(0, Qt::UserRole).toInt());
		for (int j = 0; j < pGroup->childCount(); ++j)
		{
			auto* pPropItem = pGroup->child(j);
			if (pPropItem->data(0, kTreeItemTypeRole).toInt() != kTreeItemProperty)
				continue;
			treeProps.insert(pPropItem->data(0, Qt::UserRole).toInt());
		}
	}
	return modelGroups != treeGroups || modelProps != treeProps;
}

void CClipFxWidget::ScheduleRebuildTree()
{
	if (m_bRebuildScheduled)
		return;
	m_bRebuildScheduled = true;
	QTimer::singleShot(0, this, [this]() {
		m_bRebuildScheduled = false;
		RebuildTree();
	});
}

void CClipFxWidget::UpdateTranslateText()
{
	RebuildTree();
}

void CClipFxWidget::UpdateView()
{
	if (!m_pTree)
		return;

	// LayoutChildWidgets 会频繁调用 UpdateView：仅刷新可见 Plot，避免每次重设视口高度触发滚动条闪烁
	const int viewportW = std::max(m_pTree->viewport()->width(), CLIPFX_PLOT_MIN_WIDTH);
	for (auto* pPlot : m_vecPlotWidgets)
	{
		if (!pPlot || !pPlot->isVisible())
			continue;
		pPlot->SetPlotWidth(viewportW);
		pPlot->UpdatePlot();
	}
}

void CClipFxWidget::ClearPlotWidgets()
{
	qDeleteAll(m_vecPlotWidgets);
	m_vecPlotWidgets.clear();
}

void CClipFxWidget::ClearPropertyRowWidgets()
{
	// itemWidget 由 QTreeWidget 拥有，勿 qDeleteAll，仅清空缓存
	m_vecPropertyRowWidgets.clear();
}

void CClipFxWidget::ScheduleUpdatePlotItemGeometries()
{
	if (m_bItemWidgetsUpdateScheduled)
		return;
	m_bItemWidgetsUpdateScheduled = true;
	QTimer::singleShot(0, this, [this]() {
		m_bItemWidgetsUpdateScheduled = false;
		UpdatePlotItemGeometries();
	});
}

void CClipFxWidget::UpdatePlotItemGeometries()
{
	if (!m_pTree)
		return;

	// 先定位需显示的 Plot，再 hide 其余，避免“全 hide 一瞬”导致滚动条/图层高度闪烁
	QSet<CClipFxPlotWidget*> plotsToShow;
	const int viewportW = std::max(m_pTree->viewport()->width(), CLIPFX_PLOT_MIN_WIDTH);
	for (int i = 0; i < m_pTree->topLevelItemCount(); ++i)
	{
		auto* pGroup = m_pTree->topLevelItem(i);
		if (!pGroup->isExpanded())
			continue;

		for (int j = 0; j < pGroup->childCount(); ++j)
		{
			auto* pPropItem = pGroup->child(j);
			if (pPropItem->data(0, kTreeItemTypeRole).toInt() != kTreeItemProperty)
				continue;

			auto* pPropRow = qobject_cast<CClipFxPropertyRowWidget*>(m_pTree->itemWidget(pPropItem, 0));
			const bool bPropExpanded = pPropItem->isExpanded();
			if (pPropRow)
			{
				pPropRow->show();
				pPropRow->setFixedHeight(CLIPFX_PROP_ROW_HEIGHT);
				pPropRow->setMaximumWidth(QWIDGETSIZE_MAX);
				pPropRow->SetDetailVisible(bPropExpanded);
			}

			if (!bPropExpanded || pPropItem->childCount() <= 0)
				continue;

			auto* pPlotItem = pPropItem->child(0);
			if (pPlotItem->data(0, kTreeItemTypeRole).toInt() != kTreeItemPlot)
				continue;

			const int propId = pPropItem->data(0, Qt::UserRole).toInt();
			CClipFxPlotWidget* pPlot = nullptr;
			for (auto* pCandidate : m_vecPlotWidgets)
			{
				if (pCandidate && static_cast<int>(pCandidate->GetPropertyId()) == propId)
				{
					pPlot = pCandidate;
					break;
				}
			}
			if (!pPlot)
				continue;

			const QRect itemRect = m_pTree->visualItemRect(pPlotItem);
			if (itemRect.height() <= 0)
				continue;

			pPlot->setGeometry(0, itemRect.top(), viewportW, itemRect.height());
			pPlot->SetPlotWidth(viewportW);
			pPlot->show();
			pPlot->raise();
			plotsToShow.insert(pPlot);
		}
	}

	for (auto* pPlot : m_vecPlotWidgets)
	{
		if (pPlot && !plotsToShow.contains(pPlot))
			pPlot->hide();
	}
}

void CClipFxWidget::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);
	UpdatePlotItemGeometries();
}

int CClipFxWidget::CalcTreeContentHeight() const
{
	// 仅按树结构算内容高度，不依赖 IsClipFxTrackVisible。
	// 否则折叠时 UpdateViewportLayout 会把 scroll 固定为 0，手动打开后外层有高度但内容区仍空白。
	if (!m_pTree || m_objectId.isNull())
		return 0;
	auto pObject = GetTimelineObject(m_objectId);
	if (!pObject || !pObject->HasAnyPropertyKeyframes())
		return 0;

	int h = 0;
	for (int i = 0; i < m_pTree->topLevelItemCount(); ++i)
	{
		auto* pGroup = m_pTree->topLevelItem(i);
		h += CLIPFX_GROUP_ROW_HEIGHT;
		if (!pGroup->isExpanded())
			continue;
		for (int j = 0; j < pGroup->childCount(); ++j)
		{
			auto* pPropItem = pGroup->child(j);
			if (pPropItem->data(0, kTreeItemTypeRole).toInt() != kTreeItemProperty)
				continue;
			h += CLIPFX_PROP_ROW_HEIGHT;
			if (pPropItem->isExpanded())
				h += CLIPFX_PLOT_ROW_HEIGHT;
		}
	}
	return h + CLIPFX_TREE_VIEWPORT_PADDING;
}

void CClipFxWidget::UpdateViewportLayout()
{
	if (!m_pTree || !m_pTreeScroll || m_bRebuildingTree)
		return;

	const int contentH = CalcTreeContentHeight();
	const int viewportH = contentH <= 0 ? 0 : std::min(contentH, CLIPFX_TREE_VIEWPORT_MAX_HEIGHT);

	if (contentH != m_nLastTreeContentHeight)
	{
		m_nLastTreeContentHeight = contentH;
		m_pTree->setMinimumHeight(contentH);
	}

	if (viewportH != m_nLastScrollViewportHeight)
	{
		m_nLastScrollViewportHeight = viewportH;
		m_pTreeScroll->setFixedHeight(viewportH);
	}

	// 内容未超上限时强制关闭滚动条，避免 AsNeeded 在等高 setFixedHeight 时闪现
	const bool bNeedScroll = contentH > CLIPFX_TREE_VIEWPORT_MAX_HEIGHT;
	m_pTreeScroll->setVerticalScrollBarPolicy(
		bNeedScroll ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);
}

int CClipFxWidget::CalcViewportHeight() const
{
	const int contentH = CalcTreeContentHeight();
	if (contentH <= 0)
		return 0;
	return std::min(contentH, CLIPFX_TREE_VIEWPORT_MAX_HEIGHT);
}

int CClipFxWidget::CalcHeight() const
{
	if (!m_pTree || m_objectId.isNull())
		return 0;
	// 重建树中间态（clear 后未补齐）不要向外报告塌缩高度，否则 Relayout 会先缩后扩造成图层缝隙
	if (m_bRebuildingTree)
		return m_nLastNotifiedHeight > 0 ? m_nLastNotifiedHeight : 0;
	auto pObject = GetTimelineObject(m_objectId);
	if (!pObject || !pObject->HasAnyPropertyKeyframes() || !pObject->IsClipFxTrackVisible())
		return 0;
	return CalcViewportHeight();
}

void CClipFxWidget::RefreshAfterTrackVisibilityChanged()
{
	// 强制重算视口缓存，避免折叠期间曾写入的 0 高度残留
	m_nLastTreeContentHeight = -1;
	m_nLastScrollViewportHeight = -1;
	UpdateViewportLayout();
	SyncClipFxVisibility();
	NotifyHeightChanged();
}

void CClipFxWidget::SyncClipFxVisibility()
{
	auto pObject = GetTimelineObject(m_objectId);
	const bool bShouldShow = pObject
		&& pObject->HasAnyPropertyKeyframes()
		&& pObject->IsClipFxTrackVisible();
	if (bShouldShow == isVisible())
		return;
	if (bShouldShow)
		show();
	else
		hide();
}

void CClipFxWidget::OnPropertyKeyframeChangedFromPanel(EMediaPropertyId propId, std::chrono::milliseconds clipLocalMs)
{
	auto pObject = GetTimelineObject(m_objectId);
	if (!pObject || !pObject->HasAnyPropertyKeyframes())
	{
		m_selection = {};
		RebuildTree();
		return;
	}

	if (NeedsTreeStructureRebuild())
	{
		ScheduleRebuildTree();
		NotifyClipKeyframeMarkersChanged();
		return;
	}

	const bool bWasHidden = !isVisible();
	if (clipLocalMs.count() >= 0
		&& pObject->HasPropertyKeyframeAt(static_cast<int>(propId), clipLocalMs))
	{
		m_selection.propId = propId;
		m_selection.keyTime = clipLocalMs;
		if (auto* pPlot = FindPlotWidget(propId))
			pPlot->SetSelectedKeyframe(clipLocalMs);
	}
	else
	{
		SyncSelectionWithPlayhead();
	}

	for (auto* pPlot : m_vecPlotWidgets)
	{
		if (pPlot)
			pPlot->UpdatePlot();
	}
	RefreshKeyframeUi();
	SyncClipFxVisibility();
	if (bWasHidden && isVisible())
	{
		UpdateViewportLayout();
		NotifyHeightChanged();
	}
	NotifyClipKeyframeMarkersChanged();
}

void CClipFxWidget::RebuildTree()
{
	if (!m_pTree)
		return;

	// 重建树会销毁子控件，尽量保留仍有效的关键帧选中态与展开状态
	const auto savedSelection = m_selection;
	const auto expansion = CaptureTreeExpansion(m_pTree);

	m_bRebuildingTree = true;
	m_pTree->setUpdatesEnabled(false);
	m_pTree->blockSignals(true);
	ClearKeyframeSelection();
	ClearPlotWidgets();
	// itemWidget 由 QTreeWidget 拥有，clear 时销毁；此处只清空缓存指针
	m_vecPropertyRowWidgets.clear();
	m_pTree->clear();
	m_pTree->blockSignals(false);

	auto pObject = GetTimelineObject(m_objectId);
	if (!pObject || !pObject->HasAnyPropertyKeyframes())
	{
		if (pObject)
			pObject->SetClipFxTrackVisible(false);
		m_pTree->setUpdatesEnabled(true);
		m_bRebuildingTree = false;
		m_nLastTreeContentHeight = -1;
		m_nLastScrollViewportHeight = -1;
		UpdateViewportLayout();
		SyncClipFxVisibility();
		NotifyHeightChanged();
		NotifyClipKeyframeMarkersChanged();
		return;
	}

	for (auto groupId : kGroups)
	{
		bool bGroupHasData = false;
		for (auto propId : GroupProps(groupId))
		{
			if (!pObject->IsPropertyKeyframeTrackEmpty(static_cast<int>(propId)))
			{
				bGroupHasData = true;
				break;
			}
		}
		if (!bGroupHasData)
			continue;

		auto* pGroupItem = new QTreeWidgetItem(m_pTree, { GroupName(groupId) });
		pGroupItem->setData(0, Qt::UserRole, static_cast<int>(groupId));
		pGroupItem->setData(0, kTreeItemTypeRole, kTreeItemGroup);
		pGroupItem->setSizeHint(0, QSize(0, CLIPFX_GROUP_ROW_HEIGHT));
		pGroupItem->setExpanded(false);

		for (auto propId : GroupProps(groupId))
		{
			if (pObject->IsPropertyKeyframeTrackEmpty(static_cast<int>(propId)))
				continue;

			auto* pPropItem = new QTreeWidgetItem(pGroupItem);
			pPropItem->setData(0, Qt::UserRole, static_cast<int>(propId));
			pPropItem->setData(0, kTreeItemTypeRole, kTreeItemProperty);
			pPropItem->setSizeHint(0, QSize(0, CLIPFX_PROP_ROW_HEIGHT));
			pPropItem->setExpanded(false);
			pPropItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

			auto* pPropRow = new CClipFxPropertyRowWidget(PropertyDisplayName(groupId, propId), m_pTree);
			m_vecPropertyRowWidgets.push_back(pPropRow);
			m_pTree->setItemWidget(pPropItem, 0, pPropRow);

			auto* pPlotItem = new QTreeWidgetItem(pPropItem);
			pPlotItem->setData(0, kTreeItemTypeRole, kTreeItemPlot);
			pPlotItem->setSizeHint(0, QSize(0, CLIPFX_PLOT_ROW_HEIGHT));
			pPlotItem->setFlags(pPlotItem->flags() & ~Qt::ItemIsSelectable);

			auto* pPlot = new CClipFxPlotWidget(m_objectId, propId, m_pTree->viewport());
			m_vecPlotWidgets.push_back(pPlot);
			connect(pPlot, &CClipFxPlotWidget::signalKeyframeSelected, this, &CClipFxWidget::OnPlotKeyframeSelected);
			connect(pPlot, &CClipFxPlotWidget::signalSelectionCleared, this, &CClipFxWidget::OnPlotSelectionCleared);
			connect(pPlot, &CClipFxPlotWidget::signalDataChanged, this, [this, propId]() {
				RefreshKeyframeUi();
				NotifyClipKeyframeMarkersChanged();
				if (!m_objectId.isNull())
				{
					auto* pTl = GetTimelineObject(m_objectId);
					std::chrono::milliseconds timelineMs =
						g_pPlayerController ? g_pPlayerController->GetCurTime() : std::chrono::milliseconds(0);
					if (pTl)
						timelineMs = pTl->GetStartTime() + CClipFxKeyframeHelper::GetAlignedClipLocalMs(m_objectId);
					CTimelinePropertyKeyframeHelper::ApplyPropertyKeyframesToMedia(m_objectId, timelineMs);
					CTimelinePropertyKeyframeHelper::ClearFxWhenNoKeyframeTracks(m_objectId);
					switch (propId)
					{
					case EMediaPropertyId::EffectRed:
					case EMediaPropertyId::EffectGreen:
					case EMediaPropertyId::EffectBlue:
					case EMediaPropertyId::EffectAlpha:
					case EMediaPropertyId::EffectLuminance:
					case EMediaPropertyId::EffectContrast:
					case EMediaPropertyId::EffectHue:
					case EMediaPropertyId::EffectSaturation:
						CTimelinePropertyKeyframeHelper::ApplyEffectChannelAtTime(
							m_objectId, static_cast<int>(propId), timelineMs);
						CTimelinePropertyKeyframeHelper::ResetEffectWhenNoKeyframeTracks(m_objectId);
						break;
					default:
						break;
					}
				}
				if (g_pSignalCenter)
				{
					emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_SliceWnd, SignalCenter::EDDT_MediaNodeDataChange);
					emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
				}
				if (g_pGSNetSignal && !m_objectId.isNull())
					emit g_pGSNetSignal->signalModifyMediaPropertyKeyframes(m_objectId);
			});
			pPlot->hide();
		}
	}

	RestoreTreeExpansion(m_pTree, expansion);
	UpdateTreeItemSizeHints(m_pTree);
	m_pTree->setUpdatesEnabled(true);
	m_bRebuildingTree = false;

	m_nLastTreeContentHeight = -1;
	m_nLastScrollViewportHeight = -1;
	UpdatePlotItemGeometries();
	ConnectPropertyRows();

	if (savedSelection.IsValid())
	{
		auto pObj2 = GetTimelineObject(m_objectId);
		if (pObj2 && pObj2->HasPropertyKeyframeAt(static_cast<int>(*savedSelection.propId), savedSelection.keyTime))
		{
			m_selection = savedSelection;
			if (auto* pPlot = FindPlotWidget(*savedSelection.propId))
				pPlot->SetSelectedKeyframe(savedSelection.keyTime);
		}
	}

	SyncSelectionWithPlayhead();
	RefreshKeyframeUi();
	UpdateViewportLayout();
	SyncClipFxVisibility();
	NotifyHeightChanged();
	NotifyClipKeyframeMarkersChanged();
	InstallArrowCursorFilters();
}

void CClipFxWidget::OnTreeItemExpanded(QTreeWidgetItem* item)
{
	if (!item || !m_pTree || m_bRebuildingTree)
		return;

	const int type = item->data(0, kTreeItemTypeRole).toInt();
	if (type == kTreeItemGroup)
		CollapseOtherGroups(item);
	else if (type == kTreeItemProperty)
		CollapseSiblingPropertyItems(item);

	ScheduleUpdatePlotItemGeometries();
	UpdateViewportLayout();
	NotifyHeightChanged();
}

void CClipFxWidget::OnTreeItemCollapsed(QTreeWidgetItem* item)
{
	if (!item || !m_pTree || m_bRebuildingTree)
		return;

	if (item->data(0, kTreeItemTypeRole).toInt() == kTreeItemGroup)
		CollapseGroupPropertyItems(item);

	ScheduleUpdatePlotItemGeometries();
	UpdateViewportLayout();
	NotifyHeightChanged();
}

void CClipFxWidget::CollapseOtherGroups(QTreeWidgetItem* pExpandedGroup)
{
	if (!m_pTree || !pExpandedGroup)
		return;

	m_pTree->blockSignals(true);
	for (int i = 0; i < m_pTree->topLevelItemCount(); ++i)
	{
		auto* pGroup = m_pTree->topLevelItem(i);
		if (pGroup == pExpandedGroup)
			continue;
		if (pGroup->isExpanded())
		{
			CollapseGroupPropertyItems(pGroup);
			pGroup->setExpanded(false);
		}
	}
	m_pTree->blockSignals(false);
}

void CClipFxWidget::CollapseGroupPropertyItems(QTreeWidgetItem* pGroup)
{
	if (!pGroup)
		return;
	for (int j = 0; j < pGroup->childCount(); ++j)
	{
		auto* pPropItem = pGroup->child(j);
		if (pPropItem->data(0, kTreeItemTypeRole).toInt() != kTreeItemProperty)
			continue;
		pPropItem->setExpanded(false);
	}
}

void CClipFxWidget::CollapseSiblingPropertyItems(QTreeWidgetItem* pExpandedProp)
{
	if (!m_pTree || !pExpandedProp)
		return;
	auto* pGroup = pExpandedProp->parent();
	if (!pGroup)
		return;

	m_pTree->blockSignals(true);
	for (int j = 0; j < pGroup->childCount(); ++j)
	{
		auto* pPropItem = pGroup->child(j);
		if (pPropItem == pExpandedProp)
			continue;
		if (pPropItem->data(0, kTreeItemTypeRole).toInt() == kTreeItemProperty)
			pPropItem->setExpanded(false);
	}
	m_pTree->blockSignals(false);
}

void CClipFxWidget::ConnectPropertyRows()
{
	if (!m_pTree)
		return;

	for (int i = 0; i < m_pTree->topLevelItemCount(); ++i)
	{
		auto* pGroup = m_pTree->topLevelItem(i);
		for (int j = 0; j < pGroup->childCount(); ++j)
		{
			auto* pPropItem = pGroup->child(j);
			if (pPropItem->data(0, kTreeItemTypeRole).toInt() != kTreeItemProperty)
				continue;

			const auto propId = static_cast<EMediaPropertyId>(pPropItem->data(0, Qt::UserRole).toInt());
			auto* pPropRow = qobject_cast<CClipFxPropertyRowWidget*>(m_pTree->itemWidget(pPropItem, 0));
			if (!pPropRow)
				continue;

			connect(pPropRow, &CClipFxPropertyRowWidget::signalInterpModeClicked, this,
				[this, propId](EPropertyKeyframeCurveMode mode) {
					OnInterpModeClicked(propId, mode);
				});
			connect(pPropRow, &CClipFxPropertyRowWidget::signalExpandToggleRequested, this,
				[pPropItem]() {
					if (pPropItem)
						pPropItem->setExpanded(!pPropItem->isExpanded());
				});

			if (auto* pBar = pPropRow->GetKeyFrameBar())
			{
				connect(pBar, &CPropertyKeyFrameBar::signalPreKeyFrameClicked, this,
					[this, propId]() { OnPropertyKeyFrameOp(propId, static_cast<int>(EKeyFrameOp::Pre)); });
				connect(pBar, &CPropertyKeyFrameBar::signalToggleKeyFrameClicked, this,
					[this, propId]() { OnPropertyKeyFrameOp(propId, static_cast<int>(EKeyFrameOp::Toggle)); });
				connect(pBar, &CPropertyKeyFrameBar::signalNextKeyFrameClicked, this,
					[this, propId]() { OnPropertyKeyFrameOp(propId, static_cast<int>(EKeyFrameOp::Next)); });
			}
		}
	}
}

void CClipFxWidget::ClearKeyframeSelection()
{
	m_selection = {};
	for (auto* pPlot : m_vecPlotWidgets)
	{
		if (pPlot)
			pPlot->ClearSelection();
	}
}

void CClipFxWidget::SyncSelectionWithPlayhead()
{
	if (!m_selection.IsValid())
		return;

	const auto playheadMs = CClipFxKeyframeHelper::GetAlignedClipLocalMs(m_objectId);
	if (playheadMs == m_selection.keyTime)
		return;

	auto pObject = GetTimelineObject(m_objectId);
	const int nPropId = static_cast<int>(*m_selection.propId);

	// Seek / Pre/Next 后若游标落在同属性另一关键帧上，平移选中时间，勿清空
	if (pObject && pObject->HasPropertyKeyframeAt(nPropId, playheadMs))
	{
		m_selection.keyTime = playheadMs;
		if (auto* pPlot = FindPlotWidget(*m_selection.propId))
			pPlot->SetSelectedKeyframe(playheadMs);
		return;
	}

	// 游标因帧对齐略晚于选中 KF、但仍未跨越下一关键帧时：保留选中，
	// 否则 Pre 会按 playhead 导航回到同一帧，表现为上一帧无效
	if (pObject && playheadMs > m_selection.keyTime)
	{
		const auto times = pObject->GetPropertyKeyframeTimes(nPropId);
		auto it = std::upper_bound(times.begin(), times.end(), m_selection.keyTime);
		if (it == times.end() || playheadMs < *it)
			return;
	}

	// Seek 后游标已不在选中关键帧上时清除 Plot 选中，避免插值图标与当前时刻错位
	ClearKeyframeSelection();
}

void CClipFxWidget::RefreshKeyframeUi()
{
	if (!m_pTree)
		return;

	auto pObject = GetTimelineObject(m_objectId);
	for (int i = 0; i < m_pTree->topLevelItemCount(); ++i)
	{
		auto* pGroup = m_pTree->topLevelItem(i);
		for (int j = 0; j < pGroup->childCount(); ++j)
		{
			auto* pPropItem = pGroup->child(j);
			if (pPropItem->data(0, kTreeItemTypeRole).toInt() != kTreeItemProperty)
				continue;

			const auto propId = static_cast<EMediaPropertyId>(pPropItem->data(0, Qt::UserRole).toInt());
			auto* pPropRow = qobject_cast<CClipFxPropertyRowWidget*>(m_pTree->itemWidget(pPropItem, 0));
			if (!pPropRow)
				continue;

			CClipFxKeyframeHelper::RefreshClipFxKeyframeBar(m_objectId, propId, pPropRow->GetKeyFrameBar());

			const bool bSelected = m_selection.IsValid() && *m_selection.propId == propId;
			pPropRow->SetInterpIconsEnabled(bSelected);

			if (bSelected && pObject)
			{
				if (IsPropertyKeyframeCurveModeLockedToLine(propId))
				{
					// 仅刷新 UI，不在 Refresh 中写模型（避免 Pre/Next 路径级联 OnKeyframeDataChanged）
					pPropRow->SetAutoInterpEnabled(false);
					pPropRow->SetFreeInterpEnabled(false);
					pPropRow->SetInterpMode(EPropertyKeyframeCurveMode::Line);
				}
				else
				{
					const bool bAutoOk = CClipFxKeyframeHelper::CanUseAutoBezierCurve(
						pObject, propId, m_selection.keyTime);
					const bool bFreeOk = CClipFxKeyframeHelper::CanUseFreeBezierCurve(
						pObject, propId, m_selection.keyTime);
					pPropRow->SetAutoInterpEnabled(bAutoOk);
					pPropRow->SetFreeInterpEnabled(bFreeOk);

					EPropertyKeyframeCurveMode mode = EPropertyKeyframeCurveMode::Line;
					SPropertyKeyframeNode node;
					const bool bHasNode = pObject->GetPropertyKeyframeNode(
						static_cast<int>(propId), m_selection.keyTime, node);
					if (bHasNode)
						mode = node.outCurve;

					// 邻居不足时回退直线：非法模式不可保持选中
					if (mode == EPropertyKeyframeCurveMode::FreeBezier && !bFreeOk)
						mode = EPropertyKeyframeCurveMode::Line;
					else if (mode == EPropertyKeyframeCurveMode::AutoBezier && !bAutoOk)
						mode = EPropertyKeyframeCurveMode::Line;

					if (bHasNode && node.outCurve != mode)
						pObject->SetPropertyKeyframeCurveMode(static_cast<int>(propId), m_selection.keyTime, mode);

					pPropRow->SetInterpMode(mode);
				}
			}
			else
			{
				pPropRow->SetAutoInterpEnabled(false);
				pPropRow->SetFreeInterpEnabled(false);
			}

			if (auto* pPlot = FindPlotWidget(propId))
			{
				if (bSelected)
					pPlot->SetSelectedKeyframe(m_selection.keyTime);
				else
					pPlot->ClearSelection();
			}
		}
	}
}

void CClipFxWidget::OnPlotKeyframeSelected(EMediaPropertyId propId, qint64 keyTimeMs)
{
	m_selection.propId = propId;
	m_selection.keyTime = std::chrono::milliseconds(keyTimeMs);

	for (auto* pPlot : m_vecPlotWidgets)
	{
		if (!pPlot)
			continue;
		if (pPlot->GetPropertyId() == propId)
			pPlot->SetSelectedKeyframe(m_selection.keyTime);
		else
			pPlot->ClearSelection();
	}

	RefreshKeyframeUi();
	CClipFxKeyframeHelper::SyncActivePropertyFrame(m_objectId, propId, m_selection.keyTime);
}

void CClipFxWidget::OnPlotSelectionCleared(EMediaPropertyId propId)
{
	if (m_selection.IsValid() && *m_selection.propId == propId)
		m_selection = {};
	RefreshKeyframeUi();
}

void CClipFxWidget::OnPropertyKeyFrameOp(EMediaPropertyId propId, int op)
{
	const auto keyOp = static_cast<EKeyFrameOp>(op);
	if (keyOp == EKeyFrameOp::Toggle)
	{
		if (CClipFxKeyframeHelper::TogglePropertyKeyframe(m_objectId, propId))
		{
			const auto playheadMs = CClipFxKeyframeHelper::GetAlignedClipLocalMs(m_objectId);
			auto pObject = GetTimelineObject(m_objectId);
			if (pObject && pObject->HasPropertyKeyframeAt(static_cast<int>(propId), playheadMs))
			{
				m_selection.propId = propId;
				m_selection.keyTime = playheadMs;
			}
			else if (m_selection.IsValid() && *m_selection.propId == propId)
			{
				m_selection = {};
			}
			RefreshKeyframeUi();
			NotifyClipKeyframeMarkersChanged();
			{
				std::chrono::milliseconds timelineMs = g_pPlayerController
					? g_pPlayerController->GetCurTime() : playheadMs;
				if (pObject)
					timelineMs = pObject->GetStartTime() + playheadMs;
				CTimelinePropertyKeyframeHelper::ApplyPropertyKeyframesToMedia(m_objectId, timelineMs);
				CTimelinePropertyKeyframeHelper::ClearFxWhenNoKeyframeTracks(m_objectId);
				switch (propId)
				{
				case EMediaPropertyId::EffectRed:
				case EMediaPropertyId::EffectGreen:
				case EMediaPropertyId::EffectBlue:
				case EMediaPropertyId::EffectAlpha:
				case EMediaPropertyId::EffectLuminance:
				case EMediaPropertyId::EffectContrast:
				case EMediaPropertyId::EffectHue:
				case EMediaPropertyId::EffectSaturation:
					CTimelinePropertyKeyframeHelper::ApplyEffectChannelAtTime(
						m_objectId, static_cast<int>(propId), timelineMs);
					CTimelinePropertyKeyframeHelper::ResetEffectWhenNoKeyframeTracks(m_objectId);
					break;
				default:
					break;
				}
			}
			CClipFxKeyframeHelper::SyncActivePropertyFrame(m_objectId, propId, playheadMs);
			if (g_pSignalCenter)
			{
				emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_SliceWnd, SignalCenter::EDDT_MediaNodeDataChange);
				emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
				emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_TimeLineWnd, SignalCenter::EDDT_MediaNodeDataChange);
			}
			if (g_pGSNetSignal && !m_objectId.isNull())
				emit g_pGSNetSignal->signalModifyMediaPropertyKeyframes(m_objectId);
			HistoryStackSave();
		}
		return;
	}

	if (keyOp == EKeyFrameOp::Pre || keyOp == EKeyFrameOp::Next)
	{
		auto pObject = GetTimelineObject(m_objectId);
		if (!pObject)
			return;
		const auto times = pObject->GetPropertyKeyframeTimes(static_cast<int>(propId));
		if (times.empty())
			return;

		// 有选中关键帧时以其为导航原点：游标因帧对齐略晚于当前 KF 时，
		// 按 playhead 做 Pre 会 lower_bound 回到同一帧，表现为上一帧“无法跳转”
		std::chrono::milliseconds curLocal = CClipFxKeyframeHelper::GetAlignedClipLocalMs(m_objectId);
		if (m_selection.IsValid() && *m_selection.propId == propId)
			curLocal = m_selection.keyTime;

		std::chrono::milliseconds target = curLocal;
		if (keyOp == EKeyFrameOp::Pre)
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

		m_selection.propId = propId;
		m_selection.keyTime = target;
		CClipFxKeyframeHelper::SeekToClipLocalTime(m_objectId, target);
		RefreshKeyframeUi();
		CClipFxKeyframeHelper::SyncActivePropertyFrame(m_objectId, propId, target);
		return;
	}
}

void CClipFxWidget::OnInterpModeClicked(EMediaPropertyId propId, EPropertyKeyframeCurveMode mode)
{
	if (!m_selection.IsValid() || *m_selection.propId != propId)
		return;

	if (IsPropertyKeyframeCurveModeLockedToLine(propId))
		mode = EPropertyKeyframeCurveMode::Line;

	if (CClipFxKeyframeHelper::ApplyPropertyKeyframeCurveMode(m_objectId, propId, m_selection.keyTime, mode))
	{
		if (auto* pPlot = FindPlotWidget(propId))
			pPlot->UpdatePlot();
		RefreshKeyframeUi();
		HistoryStackSave();
	}
}

CClipFxPropertyRowWidget* CClipFxWidget::FindPropertyRowWidget(EMediaPropertyId propId) const
{
	if (!m_pTree)
		return nullptr;
	for (int i = 0; i < m_pTree->topLevelItemCount(); ++i)
	{
		auto* pGroup = m_pTree->topLevelItem(i);
		for (int j = 0; j < pGroup->childCount(); ++j)
		{
			auto* pPropItem = pGroup->child(j);
			if (pPropItem->data(0, kTreeItemTypeRole).toInt() != kTreeItemProperty)
				continue;
			if (static_cast<EMediaPropertyId>(pPropItem->data(0, Qt::UserRole).toInt()) != propId)
				continue;
			return qobject_cast<CClipFxPropertyRowWidget*>(m_pTree->itemWidget(pPropItem, 0));
		}
	}
	return nullptr;
}

CClipFxPlotWidget* CClipFxWidget::FindPlotWidget(EMediaPropertyId propId) const
{
	for (auto* pPlot : m_vecPlotWidgets)
	{
		if (pPlot && pPlot->GetPropertyId() == propId)
			return pPlot;
	}
	return nullptr;
}
