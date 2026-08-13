#define _USE_MATH_DEFINES
#include "TopWidget.h"
#include <QHBoxLayout>
#include "TimelineWinMacros.h"

#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif

CTopWidget::CTopWidget(QWidget* pParent /*= 0*/)
    :QWidget(pParent)
{
    InitWidget();
    CreateConnects();
}

CTopWidget::~CTopWidget()
{
}

void CTopWidget::UpdateView(uint32_t type)
{
	if(m_pTool && type & WndType::WndType_TimeLineTool)
		m_pTool->UpdateView();
}

void CTopWidget::UpdateTime()
{
	m_pTool->UpdateTime();
}

void CTopWidget::UpdateTranslateText()
{
	m_pTool->UpdateTranslateText();
}

void CTopWidget::InitWidget()
{
    m_pTool = new CToolWidget(this);

    QHBoxLayout* pLayout = new QHBoxLayout(this);
    pLayout->setMargin(0);
    pLayout->setSpacing(0);

    pLayout->addWidget(m_pTool, 1);
}

void CTopWidget::CreateConnects()
{
    connect(m_pTool, &CToolWidget::signalUpdateView, this, [&](int type) {
        emit signalUpdateView(type);
    });
}
