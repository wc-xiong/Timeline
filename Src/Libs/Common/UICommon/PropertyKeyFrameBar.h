#pragma once
#include "UICommonGlobal.h"
#include <QWidget>

class CSvgImageButton;
class CPropertyResetBtn;
/**
 * @brief 属性行/分组标题最右侧的关键帧操作按钮组
 *
 * 布局：[ stretch ] [ < ] [ ◊ ] [ > ]
 * - Pre：跳转上一关键帧
 * - Toggle：在当前游标添加/删除关键帧（◊ 高亮表示该时刻有关键帧）
 * - Next：跳转下一关键帧
 */
class UICOMMON_EXPORT CPropertyKeyFrameBar : public QWidget
{
	Q_OBJECT
public:
	/**
	 * @brief 构造函数，创建 Pre / Toggle / Next 三个 SVG 图标按钮并布局
	 * @param pParent 父级 QWidget 指针，通常为属性行容器
	 */
	explicit CPropertyKeyFrameBar(QWidget* pParent = nullptr);
	~CPropertyKeyFrameBar() override;
	void SetEnabled(bool bEnabled);
	void SetNavEnabled(bool bPreEnabled, bool bNextEnabled);
	void SetToggleChecked(bool bChecked);
	bool IsToggleChecked() const;
	static int GetBarWidth();

signals:
	/** 用户点击「上一关键帧」按钮时发出，由属性面板执行关键帧导航 */
	void signalPreKeyFrameClicked();
	/** 用户点击「添加/删除关键帧」Toggle 按钮时发出，由属性面板切换当前时刻关键帧 */
	void signalToggleKeyFrameClicked();
	/** 用户点击「下一关键帧」按钮时发出，由属性面板执行关键帧导航 */
	void signalNextKeyFrameClicked();
private:
	/** 上一关键帧导航按钮（<），跳转到当前属性轨道上时间更早的关键帧 */
	CSvgImageButton* m_pBtnPre = nullptr;
	/** 关键帧 Toggle 按钮（◊），在当前游标时刻添加或删除关键帧；高亮表示该时刻已有关键帧 */
	CSvgImageButton* m_pBtnToggle = nullptr;
	/** 下一关键帧导航按钮（>），跳转到当前属性轨道上时间更晚的关键帧 */
	CSvgImageButton* m_pBtnNext = nullptr;
};
/**
 * @brief 在水平布局末尾追加弹性空白（stretch）与关键帧栏，供单行属性控件包装使用
 * @param pLayout 目标 QHBoxLayout 水平布局指针
 * @param pBar 待附加的关键帧操作栏实例
 */
UICOMMON_EXPORT void AttachKeyFrameBarToLayout(class QHBoxLayout* pLayout, CPropertyKeyFrameBar* pBar);
class CGroupWidgetBox;
/**
 * @brief 为属性分组标题行安装「重置」按钮与分组级关键帧栏
 * @details Reset 按钮紧跟分组名称左侧区域，KeyFrame 栏靠右与各行属性关键帧栏列对齐
 * @param pGroup 目标分组容器 CGroupWidgetBox 指针
 * @param ppResetBtn 输出：创建的重置按钮指针的存放地址
 * @param ppKeyFrameBar 输出：创建的分组级关键帧栏指针的存放地址
 */
UICOMMON_EXPORT void AttachPropertyGroupHeaderWidgets(CGroupWidgetBox* pGroup,
	CPropertyResetBtn** ppResetBtn, CPropertyKeyFrameBar** ppKeyFrameBar);