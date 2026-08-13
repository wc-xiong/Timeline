#pragma once
#include "ContentViewData.h"
#include <QWidget>

class CLayersToolWidget;
class CScaleWidget;
class CBarWidget : public QWidget
{
	Q_OBJECT
public:
	CBarWidget(QWidget* pParent = 0);
	~CBarWidget();
	void	UpdateTranslateText();
	void	UpdateView(uint32_t type);
	void	Clear();

protected:
	void    InitWidget();
	void    CreateConnects();

signals:
	void	signalUpdateView(int);

protected:
	CLayersToolWidget*	m_pLayersTool = nullptr;
	CScaleWidget*		m_pScale = nullptr;
};
