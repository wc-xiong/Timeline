#include "GroupWidgetBox.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QLineEdit>
#include <QPushButton>
#include <QStyle>
#include <QLayoutItem>
#include <QRadioButton>
#include "CheckBoxEx.h"
#include "UICommonSizeDefine.h"
#include "SvgImageButton.h"
#include "ToolTip.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CGroupWidgetBox::CGroupWidgetBox(QWidget* pParent, bool flag, bool showSplitLine):
	CBaseFrame(pParent),
	m_bUseCheckBox(flag),
	m_bShowSplitLine(showSplitLine)
{
	m_bGroupExpand = true;

	InitWidget();

	CreateConnections();
}

CGroupWidgetBox::~CGroupWidgetBox()
{
}

void CGroupWidgetBox::SetNameLabelProperty(const char* strProperty, QVariant flag)
{
  m_pNameLabel->setProperty(strProperty, flag);
}

void CGroupWidgetBox::SetName(const QString &strName)
{
	m_pNameLabel->setText(strName);
}

void CGroupWidgetBox::AddContainWidget(QWidget* pWidget)
{
	m_pContainVerLayout->addWidget(pWidget);
}

void CGroupWidgetBox::InsertContainWidget(QWidget* pWidget, int nIndex)
{
	m_pContainVerLayout->insertWidget(nIndex, pWidget);
}

void CGroupWidgetBox::AddContainLayout(QLayout* pItem)
{
	m_pContainVerLayout->addLayout(pItem);
}

void CGroupWidgetBox::AddContainStretch()
{
	m_pContainVerLayout->addStretch();
}

void CGroupWidgetBox::DelContainWidget(QWidget* pWidget)
{
	m_pContainVerLayout->removeWidget(pWidget);
}

void CGroupWidgetBox::DelContainLayout(QLayout* pItem)
{
	m_pContainVerLayout->removeItem(pItem);
}

void CGroupWidgetBox::SetSpacing(int nSpacing)
{
	m_pContainVerLayout->setSpacing(nSpacing);
}

void CGroupWidgetBox::SetHeadVisible(bool bVisible)
{
	m_pHeadFrame->setVisible(bVisible);
}

void CGroupWidgetBox::SetContentsMargins(int nLeft, int nTop, int nRight, int nBottom)
{
	m_pVerLayout->setContentsMargins(nLeft, nTop, nRight, nBottom);
}

void CGroupWidgetBox::SetContainContentsMargins(int nLeft, int nTop, int nRight, int nBottom)
{
	m_pContainVerLayout->setContentsMargins(nLeft, nTop, nRight, nBottom);
}

void CGroupWidgetBox::DelAllChildItems()
{
	QLayoutItem* pItem = nullptr;
	while (pItem = m_pContainVerLayout->takeAt(0))
	{
		if (pItem->widget())
			delete pItem->widget();
		if (pItem->layout())
			delete pItem->layout();
		delete pItem;
	}
}

void CGroupWidgetBox::SetEnabled(bool bEnabled)
{
	m_pContainFrame->setEnabled(bEnabled);
	m_pExpandBtn->setEnabled(true);

	if(m_pUndefineBtn)
		m_pUndefineBtn->setEnabled(bEnabled);

	emit sigEnableChange(bEnabled);
}

void CGroupWidgetBox::SetExpandBtnVisible(bool b)
{
	m_pExpandBtn->setVisible(b);
}

void CGroupWidgetBox::SetExpanded(bool bExpand)
{
	m_pExpandBtn->setChecked(bExpand);
	m_pContainFrame->setVisible(bExpand);
}

void CGroupWidgetBox::setSelected(bool flag)
{
	m_pChecCBtn->SetChecked(flag);
}

std::vector<QWidget*> CGroupWidgetBox::GetAllChildWidgets()
{
	std::vector<QWidget*> listWidgets;
	int nCount = m_pContainVerLayout->count();
	for (int i = 0; i < nCount; i++)
	{
		auto pLayoutItem = m_pContainVerLayout->itemAt(i);
		if (pLayoutItem->widget())
			listWidgets.push_back(pLayoutItem->widget());
	}
	return listWidgets;
}


void CGroupWidgetBox::slotGroupStateChange()
{
	m_pContainFrame->setVisible(m_pExpandBtn->isChecked());
}

void CGroupWidgetBox::slotGroupCheckBoxStateChange(bool checked)
{
	m_iCheckedState = checked;
	emit   signalCheckBoxClicked(checked);
}

void CGroupWidgetBox::InitWidget()
{
	m_pHeadFrame = new QFrame(this);
	m_pHeadFrame->setFixedHeight(nSizeDefine_30);

	m_pExpandBtn = new CSvgImageButton(":/dark/images/CommonCtrl/Folder" , m_pHeadFrame);
	m_pExpandBtn->setProperty("transparentBkBtn", true);
	m_pExpandBtn->setCheckable(true);
	m_pExpandBtn->setChecked(true);
	m_pExpandBtn->setFixedSize(SysDpiZoom(16), SysDpiZoom(16));

	m_pNameLabel = new QLabel(m_pHeadFrame);
	m_pNameLabel->setProperty("BoldName", true);
	m_pNameLabel->setFixedHeight(UICommon_CtrlHeight);
	m_pNameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	
	m_pHeadHorLayout = new QHBoxLayout(m_pHeadFrame);
	m_pHeadHorLayout->setMargin(0);
	m_pHeadHorLayout->setSpacing(0);
	m_pHeadHorLayout->setContentsMargins(0, 0, 0, 0);

	m_pHeadHorLayout->addWidget(m_pExpandBtn);
	m_pHeadHorLayout->addSpacing(SysDpiZoom(4));

	m_pHeadHorLayout->addWidget(m_pNameLabel);

	if (m_bUseCheckBox)
	{
		m_pChecCBtn = new CCheckBoxEx(this);
		m_pChecCBtn->setFixedSize(SysDpiZoom(18), SysDpiZoom(16));
		m_pHeadHorLayout->addSpacing(SysDpiZoom(4));
		m_pHeadHorLayout->addWidget(m_pChecCBtn);
	}

	m_pTipBtn = new CTipButton(this);
	m_pTipBtn->setFixedSize(nSizeDefine_16, nSizeDefine_16);
	m_pTipBtn->SetTrianglePosRatio(0.5);
	m_pTipBtn->setProperty("TipButton", true);
	m_pTipBtn->SetLeftTopMargin(SysDpiZoom(12), SysDpiZoom(6));
	m_pTipBtn->setVisible(false);

	m_pHeadHorLayout->addWidget(m_pTipBtn);

	m_pHeadHorLayout->addStretch();

	m_pContainFrame = new QFrame(this);
	m_pContainVerLayout = new QVBoxLayout(m_pContainFrame);
	m_pContainVerLayout->setMargin(0);
	m_pContainVerLayout->setSpacing(0);
	m_pContainVerLayout->setContentsMargins(0, 0, 0, 0);
	//m_pContainVerLayout->addSpacing(SysDpiZoom(4));

	m_pContainFrame->setLayout(m_pContainVerLayout);

	m_pVerLayout = new QVBoxLayout;
	m_pVerLayout->setMargin(0);
	m_pVerLayout->setSpacing(0);
	m_pVerLayout->setContentsMargins(0, 0, 0, 0);

	m_pVerLayout->addWidget(m_pHeadFrame);
	m_pVerLayout->addWidget(m_pContainFrame);
	
	if (m_bShowSplitLine)
	{
		auto pLine = new QFrame(this);
		pLine->setFixedHeight(2);
		pLine->setFrameShape(QFrame::HLine);
		pLine->setFrameShadow(QFrame::Sunken);
		pLine->setProperty("LineWnd", true);

		m_pVerLayout->addSpacing(SysDpiZoom(8));
		m_pVerLayout->addWidget(pLine);
	}

	setLayout(m_pVerLayout);
}

void CGroupWidgetBox::SetContainFrameStyle(const QString& strStyle, bool bTOrF)
{
	style()->unpolish(m_pContainFrame);
	m_pContainFrame->setProperty(strStyle.toStdString().c_str(), bTOrF);
	style()->polish(m_pContainFrame);
}

void CGroupWidgetBox::AddSpacing(int nSpacingValue)
{
	m_pContainVerLayout->addSpacing(nSpacingValue);
}

void CGroupWidgetBox::InsertSpacing(int nSpacingValue, int nIndex)
{
	m_pContainVerLayout->insertSpacing(nIndex, nSpacingValue);
}

void CGroupWidgetBox::SetTipVisible(bool bVisible)
{
	m_pTipBtn->setVisible(bVisible);
}

void CGroupWidgetBox::SetTipTextContent(const QString& text)
{
	m_pTipBtn->SetTextContent(text);
}

void CGroupWidgetBox::SetTipBubbleSize(int width, int height)
{
	m_pTipBtn->SetToolTipFixedSize(width, height);
}

void CGroupWidgetBox::InsertHeadWidgetAfterName(QWidget* pWidget)
{
	if (!pWidget || !m_pHeadHorLayout || !m_pNameLabel)
		return;

	// 期望插入到“名称Label”之后，并放在Tip按钮之前（Tip按钮在标题栏中靠右）
	int nameIdx = m_pHeadHorLayout->indexOf(m_pNameLabel);
	if (nameIdx < 0)
	{
		// 兜底：直接插入到Tip按钮之前
		int tipIdx = m_pHeadHorLayout->indexOf(m_pTipBtn);
		if (tipIdx < 0)
			tipIdx = m_pHeadHorLayout->count();
		m_pHeadHorLayout->insertWidget(tipIdx, pWidget);
		return;
	}

	// 给名称和按钮之间加一点间距，避免挤在一起
	const int insertIdx = nameIdx + 1;
	m_pHeadHorLayout->insertSpacing(insertIdx, SysDpiZoom(4));
	m_pHeadHorLayout->insertWidget(insertIdx + 1, pWidget);
}

void CGroupWidgetBox::InsertHeadWidgetAtRight(QWidget* pWidget)
{
	if (!pWidget || !m_pHeadHorLayout)
		return;

	const int nCount = m_pHeadHorLayout->count();
	if (nCount > 0)
	{
		QLayoutItem* pLastItem = m_pHeadHorLayout->itemAt(nCount - 1);
		if (pLastItem && pLastItem->spacerItem())
		{
			m_pHeadHorLayout->removeItem(pLastItem);
			delete pLastItem;
		}
	}

	pWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_pHeadHorLayout->addStretch();
	m_pHeadHorLayout->addWidget(pWidget, 0, Qt::AlignRight | Qt::AlignVCenter);
}

void CGroupWidgetBox::CreateConnections()
{
	connect(m_pExpandBtn, SIGNAL(clicked()), this, SLOT(slotGroupStateChange()));
	if(m_bUseCheckBox && m_pChecCBtn)
	{
		connect(m_pChecCBtn, &CCheckBoxEx::clicked, this, &CGroupWidgetBox::slotGroupCheckBoxStateChange);
	}

}

void CGroupWidgetBox::SetUndefineBtn(const QString& path)
{
	m_pUndefineBtn = new CSvgImageButton(path, this);
	m_pUndefineBtn->setProperty("transparentBkBtn", true);
	m_pUndefineBtn->setFixedSize(UICommon_ImageBtnWidth, UICommon_CtrlHeight);
	connect(m_pUndefineBtn, SIGNAL(clicked()), this, SIGNAL(signalUndefineBtnClicked()));

	m_pHeadHorLayout->addWidget(m_pUndefineBtn, 0, Qt::AlignRight);
}

void CGroupWidgetBox::SetUndefineBtnTip(const QString& tip)
{
	if (m_pUndefineBtn)
		m_pUndefineBtn->setToolTip(tip);
}

void CGroupWidgetBox::SetUndefineBtnSize(QSize btnSize)
{
	if (m_pUndefineBtn)
		m_pUndefineBtn->setFixedSize(btnSize);
}

/*---------------------------------------------------------------------------------*/
CGroupWidgetHBox::CGroupWidgetHBox(QWidget* pParent, bool flag, bool showSplitLine) :
	CBaseFrame(pParent),
	m_bUseCheckBox(flag),
	m_bShowSplitLine(showSplitLine)
{
	InitWidget();
	CreateConnections();
}

CGroupWidgetHBox::~CGroupWidgetHBox()
{
}

void CGroupWidgetHBox::SetName(const QString &strName)
{
	m_pNameLabel->setText(strName);
}

void CGroupWidgetHBox::AddContainWidget(QWidget* pWidget)
{
	m_pContainLayout->insertWidget(m_pContainLayout->count() - 1, pWidget);
}

void CGroupWidgetHBox::InsertContainWidget(QWidget* pWidget, int nIndex)
{
	m_pContainLayout->insertWidget(nIndex, pWidget);
}

void CGroupWidgetHBox::AddContainLayout(QLayout* pItem)
{
	m_pContainLayout->insertLayout(m_pContainLayout->count() - 1, pItem);
}

void CGroupWidgetHBox::AddContainSpacing(int nSpacing)
{
	m_pContainLayout->insertSpacing(m_pContainLayout->count() - 1, nSpacing);
}

void CGroupWidgetHBox::AddHeadSpacing(int nSpacing)
{
	m_pHeadLayout->insertSpacing(m_pHeadLayout->count(), nSpacing);
}

void CGroupWidgetHBox::AddContainStretch()
{
	m_pContainLayout->addStretch();
}

void CGroupWidgetHBox::DelContainWidget(QWidget* pWidget)
{
	m_pContainLayout->removeWidget(pWidget);
}

void CGroupWidgetHBox::DelContainLayout(QLayout* pItem)
{
	m_pContainLayout->removeItem(pItem);
}

void CGroupWidgetHBox::SetSpacing(int nSpacing)
{
	m_pContainLayout->setSpacing(nSpacing);
}

void CGroupWidgetHBox::SetContentsMargins(int nLeft, int nTop, int nRight, int nBottom)
{
	m_pLayout->setContentsMargins(nLeft, nTop, nRight, nBottom);
}

void CGroupWidgetHBox::SetContainContentsMargins(int nLeft, int nTop, int nRight, int nBottom)
{
	m_pContainLayout->setContentsMargins(nLeft, nTop, nRight, nBottom);
}

void CGroupWidgetHBox::DelAllChildItems()
{
	QLayoutItem* pItem = nullptr;
	while (pItem = m_pContainLayout->takeAt(0))
	{
		if (pItem->widget())
			delete pItem->widget();
		if (pItem->layout())
			delete pItem->layout();
		delete pItem;
	}
}

void CGroupWidgetHBox::SetEnabled(bool bEnabled)
{
	m_pContainFrame->setEnabled(bEnabled);
	emit sigEnableChange(bEnabled);
}

void CGroupWidgetHBox::setSelected(bool flag)
{
	m_pChecCBtn->SetChecked(flag);
}

void CGroupWidgetHBox::SetHeadNameWidth(int n)
{
	m_pNameLabel->setFixedWidth(n);
}

std::list<QWidget*> CGroupWidgetHBox::GetAllChildWidgetList()
{
	std::list<QWidget*> listWidgets;
	int nCount = m_pContainLayout->count();
	for (int i = 0; i < nCount; i++)
	{
		auto pLayoutItem = m_pContainLayout->itemAt(i);
		if (pLayoutItem->widget())
			listWidgets.push_back(pLayoutItem->widget());
	}
	return listWidgets;
}

void CGroupWidgetHBox::slotGroupCheckBoxStateChange(bool checked)
{
	m_iCheckedState = checked;
	emit   signalCheckBoxClicked(checked);
}

void CGroupWidgetHBox::InitWidget()
{
	m_pHeadFrame = new QFrame(this);

	m_pNameLabel = new QLabel(m_pHeadFrame);
	m_pNameLabel->setProperty("BoldName", true);
	m_pNameLabel->setFixedHeight(UICommon_CtrlHeight);
	m_pNameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

	m_pHeadLayout = new QHBoxLayout(m_pHeadFrame);
	m_pHeadLayout->setMargin(0);
	m_pHeadLayout->setSpacing(0);

	if (m_bUseCheckBox)
	{
		m_pChecCBtn = new CCheckBoxEx(this);
		m_pChecCBtn->setFixedSize(SysDpiZoom(18), SysDpiZoom(16));
		m_pHeadLayout->addWidget(m_pChecCBtn);
		m_pHeadLayout->addSpacing(nSizeDefine_8);
	}

	m_pHeadLayout->addWidget(m_pNameLabel);
	//m_pHeadLayout->addStretch();

	auto pHeadLayout = new QVBoxLayout;
	pHeadLayout->setMargin(0);
	pHeadLayout->setSpacing(0);
	pHeadLayout->addWidget(m_pHeadFrame);
	pHeadLayout->addStretch();

	m_pContainFrame = new QFrame(this);

	m_pContainLayout = new QVBoxLayout(m_pContainFrame);
	m_pContainLayout->setMargin(0);
	m_pContainLayout->setSpacing(0);
	m_pContainLayout->addStretch();

	m_pContainFrame->setLayout(m_pContainLayout);

	m_pLayout = new QHBoxLayout;
	m_pLayout->setMargin(0);
	m_pLayout->setSpacing(0);
	m_pLayout->addLayout(pHeadLayout);
	m_pLayout->addSpacing(SysDpiZoom(4));
	m_pLayout->addWidget(m_pContainFrame);

	if (m_bShowSplitLine)
	{
		QLabel *pSplitLine = new QLabel(this);
		pSplitLine->setFocusPolicy(Qt::NoFocus);
		pSplitLine->setStyleSheet("background-color: rgba(187,190,203,0.3);");
		pSplitLine->setFixedHeight(UICommon_DevideHeight);

		m_pLayout->addSpacing(SysDpiZoom(8));
		m_pLayout->addWidget(pSplitLine);
	}

	setLayout(m_pLayout);
}

void CGroupWidgetHBox::CreateConnections()
{
	if (m_bUseCheckBox && m_pChecCBtn)
	{
		connect(m_pChecCBtn, &CCheckBoxEx::clicked, this, &CGroupWidgetHBox::slotGroupCheckBoxStateChange);
	}
}
