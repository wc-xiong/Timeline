#pragma once
#include "ContentViewData.h"
#include "UICommon/BaseFrame.h"
#include "UICommon/SvgImageButton.h"
#include <QLabel>

class CLayersControlWidget :public QWidget
{
	Q_OBJECT
public:
	CLayersControlWidget(QWidget* pParent = 0);
	~CLayersControlWidget();
	void	UpdateTranslateText();
	void	UpdateView();

protected:
	void    InitWidget();
	void    CreateConnects();

signals:
	void	signalUpdateView(int);

protected:
	QLabel* m_pLabelCtrlAlignSection = nullptr;	// 拖拽提示：Ctrl 吸附；单选时附加 Alt 新建图层
	CSvgImageButton* m_pBtnAddLayer = nullptr;	//添加图层按钮
};