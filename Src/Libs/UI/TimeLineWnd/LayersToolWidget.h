#pragma once
#include "ContentViewData.h"
#include "UICommon/BaseFrame.h"
#include "UICommon/SvgImageButton.h"
#include "LayerLabelsWidget.h"

class CLayersToolWidget :public QWidget
{
	Q_OBJECT
public:
	CLayersToolWidget(QWidget* pParent = 0);
	~CLayersToolWidget();
	void	UpdateTranslateText();
	void	UpdateView();

protected:
	void    InitWidget();
	void    CreateConnects();
	void	paintEvent(QPaintEvent* event)override;

signals:
	void	signalUpdateView(int);

protected slots :
	void	slotStopPlay(bool bBtn);

protected:
	CSvgImageButton* m_pBtnLock = nullptr;
	CSvgImageButton* m_pBtnProcess = nullptr;
	CSvgImageButton* m_pBtnHide = nullptr;
	CSvgImageButton* m_pBtnMute = nullptr;
	CTimeLineLayerLabelColor*	m_pColorButton = nullptr;
};