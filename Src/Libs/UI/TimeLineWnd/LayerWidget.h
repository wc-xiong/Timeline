#pragma once

#include "ContentViewData.h"
#include <QWidget>
#include <utility>
#include <vector>

class QMimeData;
class CObjectWidget;
class CClipTransFxWidget;
class CLayersWidget;
class CLayerWidget : public QWidget
{
	Q_OBJECT
public:
	explicit CLayerWidget(CLayersWidget* pLayersWidget, QWidget* pParent = nullptr);
	~CLayerWidget() override;

	void UpdateView(const QUuid& layerId, int layerIndex);
	void LayoutObjects();
	int GetRequiredHeight() const;
	/** 检测重叠并创建/删除/更新转场模型，然后 LayoutTransFxWidgets */
	void RefreshTransFxForLayer();
	/** LayoutObjects + LayoutTransFxOnly */
	void LayoutTransFxWidgets();
	/** 仅重排转场控件，不移动素材几何 */
	void LayoutTransFxOnly();
	/** 转场控件置顶，避免被素材遮挡 */
	void RaiseTransFxWidgets();
	/** 拉伸预览：指定左右半场宽与交界点图层 X，不写模型 */
	void PreviewTransFxHalfWidths(const QUuid& transFxId, int leftHalfPx, int rightHalfPx,
		int previewJunctionLayerPx = -1);
	/** 读取当前已布局的左右半场像素宽度 */
	std::pair<int, int> GetTransFxHalfWidthsPx(const QUuid& transFxId) const;
	void CleanupOrphanTransFxWidgets();
	/** 图层坐标命中测试转场控件（用于点击选中，优先于素材） */
	CClipTransFxWidget* HitTestTransFxWidgetAt(const QPoint& posInLayer) const;
	const std::vector<CObjectWidget*>& GetObjects() const { return m_vecObjects; }
	QUuid GetLayerId() const { return m_layerId; }
	int GetLayerIndex() const { return m_layerIndex; }
	CLayersWidget* GetLayersWidget() const { return m_pLayersWidget; }
	void HandleDropEvent(class QDropEvent* event);
	bool TryCommitObjectDrop(const QMimeData* pMime, const QPoint& posInLayer);
	void HandleResourceDropAt(class QDropEvent* event, const QPoint& posInLayer);
	QUuid FindObjectIdAtLayerPos(const QPoint& posInLayer) const;

signals:
	void signalUpdateView(int nFlag);

protected:
	void paintEvent(QPaintEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dragMoveEvent(QDragMoveEvent* event) override;
	void dragLeaveEvent(QDragLeaveEvent* event) override;
	void dropEvent(QDropEvent* event) override;
	void contextMenuEvent(QContextMenuEvent* event) override;

private:
	void CheckObject();
	void InitLockPixmap();
	bool ShouldForwardMouseToLayers(const QPoint& pos) const;
	void ForwardMouseEventToLayers(QMouseEvent* event);
	void HandleLayerBackgroundMousePress(QMouseEvent* event);
	void HandleResourceDragMove(class QDragMoveEvent* event);
	void HandleResourceDrop(class QDropEvent* event, const QPoint& posInLayer);

	CLayersWidget* m_pLayersWidget = nullptr;
	QUuid m_layerId;
	int m_layerIndex = 0;
	std::vector<CObjectWidget*> m_vecObjects;
	QPixmap m_pixmapLock;
};