#pragma once
#include "UICommonGlobal.h"
#include <QPushButton>
#include <QPixmap>

class UICOMMON_EXPORT CSvgImageButton : public QPushButton
{
	Q_OBJECT

public:
	// Svg照片按钮
	// 传入图片路径要为图片所在文件夹路径。
	// 若是有Checked两种状态则路径为Checked文件夹上一级文件夹路径
	// 文件夹照片命名格式Normal.svg、Hover.svg、Down.svg、Disable.svg。
	// 文件夹下至少存在Normal.png
	CSvgImageButton(const QString &strImagePath, QWidget* pParent = 0, bool bSendMouseEventToParent = true);
	~CSvgImageButton();

	void SetImagePath(const QString &strImagePath);
	void SetHideToolTipWhenDisabled(bool bHide); // 禁用时是否隐藏 tooltip
	bool HideToolTipWhenDisabled() const { return m_bHideToolTipWhenDisabled; }

protected:
	bool event(QEvent* event) override;

	void mousePressEvent(QMouseEvent *e) override;

	void mouseReleaseEvent(QMouseEvent *e) override;

	void paintEvent(QPaintEvent *event) override;

private:
	QString m_strImagePath;
	bool m_bSendMouseEventToParent = true;
	bool m_bHideToolTipWhenDisabled = false; // 禁用时是否隐藏 tooltip
};
