#pragma once
#include "UICommonGlobal.h"
#include <QWidget>

// 继承QWidget的界面都继承此基类

class UICOMMON_EXPORT CBaseWidget : public QWidget
{
	Q_OBJECT

public:
	CBaseWidget(QWidget* pParent = 0);
	virtual ~CBaseWidget();

protected:
	void changeEvent(QEvent *event)	override;

	// 翻译文本
	virtual void UpdateTranslateText(){}
};
