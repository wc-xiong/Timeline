#pragma once
#include <QWidget>
#include <QString>
#include "Model/TimeLineModel/TimeLineObject.h"

class QLabel;
class QWidget;
class QMouseEvent;
class CSvgImageButton;
class CPropertyKeyFrameBar;
/**
 * @brief 时间线 Clip 特效属性行控件
 *
 * 布局：左侧属性名称 | 中间水平居中的三个插值图标（线性 / 自动贝塞尔 / 自由曲线）| 右侧关键帧导航栏。
 * 作为 QTreeWidget 的 itemWidget 嵌入属性节点；双击整行可切换对应 Plot 曲线的展开/折叠。
 */
class CClipFxPropertyRowWidget : public QWidget
{
	Q_OBJECT
public:
	explicit CClipFxPropertyRowWidget(const QString& strPropertyName, QWidget* pParent = nullptr);
	void SetDetailVisible(bool bVisible);
	void SetInterpIconsEnabled(bool bEnabled);
	void SetBezierInterpModesEnabled(bool bEnabled);
	void SetAutoInterpEnabled(bool bEnabled);
	void SetFreeInterpEnabled(bool bEnabled);
	void SetInterpMode(EPropertyKeyframeCurveMode mode);
	CPropertyKeyFrameBar* GetKeyFrameBar() const { return m_pKeyFrameBar; }
	CSvgImageButton* GetInterpLinearBtn() const { return m_pBtnInterpLinear; }
	CSvgImageButton* GetInterpBezierBtn() const { return m_pBtnInterpBezier; }
	CSvgImageButton* GetInterpStepBtn() const { return m_pBtnInterpStep; }

signals:
	/**
	 * @brief 用户点击线性 / 自动贝塞尔 / 自由曲线插值图标
	 * @param mode 用户选择的目标插值曲线模式
	 */
	void signalInterpModeClicked(EPropertyKeyframeCurveMode mode);
	/**
	 * @brief 用户双击属性行，请求切换 Plot 展开/折叠
	 *
	 * itemWidget 会遮挡树节点默认点击区域，故通过此信号由 CClipFxWidget 处理。
	 */
	void signalExpandToggleRequested();

protected:
	/**
	 * @brief 双击整行时发射 signalExpandToggleRequested
	 * @param event Qt 鼠标双击事件
	 */
	void mouseDoubleClickEvent(QMouseEvent* event) override;
	/**
	 * @brief 根据插值模式更新三个 SVG 按钮的 checked 状态
	 * @param mode 当前应高亮的插值模式
	 */
	void UpdateInterpButtonChecked(EPropertyKeyframeCurveMode mode);

private:
	QLabel* m_pLabel = nullptr;                       // 左侧属性名称文本标签
	CPropertyKeyFrameBar* m_pKeyFrameBar = nullptr;  // 关键帧导航栏：上一帧 / 添加或删除 / 下一帧
	CSvgImageButton* m_pBtnInterpLinear = nullptr;  // 线性插值模式切换按钮
	CSvgImageButton* m_pBtnInterpBezier = nullptr;  // 自动贝塞尔插值模式切换按钮
	CSvgImageButton* m_pBtnInterpStep = nullptr;    // 自由曲线（阶跃或自由贝塞尔）插值模式切换按钮
	EPropertyKeyframeCurveMode m_interpMode = EPropertyKeyframeCurveMode::Line; // 当前显示的插值模式（与选中关键帧同步）
};