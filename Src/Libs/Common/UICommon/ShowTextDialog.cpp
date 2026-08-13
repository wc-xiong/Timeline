#include "ShowTextDialog.h"

using namespace UICommon;

#include "UITranslate.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include "LineEditEx.h"
#include "UICommonSizeDefine.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CShowTextDialog::CShowTextDialog(QWidget *parent)
	: CFramelessDialog(parent)
{
	InitWidget();
}

CShowTextDialog::CShowTextDialog(QWidget * parent, const QString &strTitle, QStringList strListTexts)
	: CFramelessDialog(parent)
{
	InitWidget();

	SetText(strListTexts);
	SetTitle(strTitle);
}

CShowTextDialog::~CShowTextDialog()
{
}

void CShowTextDialog::InitWidget()
{
	setCancelButtonVisible(false);

	setMinimumSize(SysDpiZoom(468), SysDpiZoom(140));
	setDialogResizable(false);
	setWindowModality(Qt::WindowModal);

	m_pVEditLayout = new QVBoxLayout;
	m_pVEditLayout->setMargin(0);
	m_pVEditLayout->setSpacing(0);

	QScrollArea* pScrollArea = new QScrollArea(this);
	pScrollArea->setProperty("ShowTextDialog", true);
	pScrollArea->setWidgetResizable(true);
	pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	QWidget* pWidget = new QWidget(this);
	pWidget->setLayout(m_pVEditLayout);
	pScrollArea->setWidget(pWidget);
	pWidget->setAutoFillBackground(false);

	QVBoxLayout* layout = new QVBoxLayout;
	layout->setSpacing(0);
	layout->setContentsMargins(SysDpiZoom(12), SysDpiZoom(24), SysDpiZoom(12), SysDpiZoom(8));
	layout->addWidget(pScrollArea);
	
	setLayout(layout);
}

void UICommon::CShowTextDialog::SetText(QStringList strListTexts)
{
	ClearAllLayoutItem();

	for (auto it : strListTexts)
	{
		CLineEditEx* pEditName = new CLineEditEx(this);
		pEditName->setProperty("LineEditExReadOnly", true);
		pEditName->setFixedHeight(nSizeDefine_24);
		pEditName->setEditWidthTolerance(nSizeDefine_5);
		pEditName->setText(it);
		pEditName->setReadOnly(true);

		m_pVEditLayout->addWidget(pEditName);
	}
}

void UICommon::CShowTextDialog::SetTitle(const QString &strText)
{
	m_pTitleBar->SetTitleName(strText);
}

void UICommon::CShowTextDialog::ClearAllLayoutItem()
{
	QLayoutItem* pItem = nullptr;
	while (pItem = m_pVEditLayout->takeAt(0))
	{
		if (pItem->widget())
			delete pItem->widget();
		if (pItem->layout())
			delete pItem->layout();
		delete pItem;
	}
}
