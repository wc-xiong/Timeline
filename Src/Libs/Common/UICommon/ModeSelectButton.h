#pragma once
#include "UICommonGlobal.h"
#include <QPushButton>
#include <QPixmap>

// 模式选择按钮
class UICOMMON_EXPORT CModeSelectBtn : public QPushButton
{
	Q_OBJECT
public:
	CModeSelectBtn(const QString & strPath, const int borderWidth, QWidget *parent = nullptr);

	void SetPixmap(QPixmap pix);
	
	// 桌面拷贝
	void SetUsed(bool bUsed);

	// 最近工程
	void SetHasBackgroundColor(bool bHasBackGroundColor);

protected:
	void mouseDoubleClickEvent(QMouseEvent *event) override;
	void paintEvent(QPaintEvent * event) override;

signals:
	void signalDoubleClicked();

private:
	QString m_strImagePath;
	int m_nBorderWidth;

	QPixmap m_pix;

	///////////////////////////////////////
	bool m_bUsed = false;
	bool m_bHasBackGroundColor = true; // 是否需要绘制浅色底色
	///////////////////////////////////////
};