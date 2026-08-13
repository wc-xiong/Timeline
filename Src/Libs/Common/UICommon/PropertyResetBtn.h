#pragma once
#include "UICommonGlobal.h"
#include <QWidget>

class CSvgImageButton;
/**
 * @brief 属性分组标题行「重置」按钮
 *
 * 用于 CMediaPropertyFrame 各特效分组（音量/位置/裁剪等）标题右侧，
 * 点击后触发 signalResetClicked，由面板执行 ResetPropertyGroup。
 * 重置按钮的 enable 仅受图层锁定影响，不受游标是否在素材内限制。
 */
class UICOMMON_EXPORT CPropertyResetBtn : public QWidget
{
	Q_OBJECT
public:
	explicit CPropertyResetBtn(QWidget* pParent = nullptr);
	~CPropertyResetBtn() override;
	void SetEnabled(bool bEnabled);
	/**
	 * @brief 仅 hover 时显示图标；禁用时 hover 也不显示且不弹 tooltip（同属性区 Copy/Paste）
	 */
	void SetHoverOnlyVisible(bool bHoverOnly);
signals:
	/** 用户点击重置按钮时发出，由属性面板将对应分组所有属性恢复为默认值 */
	void signalResetClicked();
private:
	/** 重置图标按钮控件，显示循环箭头 SVG 图标，点击触发 signalResetClicked */
	CSvgImageButton* m_pBtn = nullptr;
};
