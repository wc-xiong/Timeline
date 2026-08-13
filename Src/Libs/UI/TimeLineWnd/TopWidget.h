#pragma once
#include "ToolWidget.h"
#include <QWidget>

class CTopWidget : public QWidget
{
    Q_OBJECT
public:
    CTopWidget(QWidget* pParent = 0);
    ~CTopWidget();

    void    UpdateView(uint32_t type);
	void    UpdateTime();
	void    UpdateTranslateText();

protected:
    void    InitWidget();
    void    CreateConnects();

signals:
    void    signalUpdateView(int);

protected:
    CToolWidget*        m_pTool = nullptr;
};
