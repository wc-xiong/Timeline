#include "ComboBoxWidget.h"
#include "ComboBoxEx.h"
#include "LineEditEx.h"
#include "CompleterEx.h"
#include "ToolTip.h"

#include <QLabel>
#include <QVariant>
#include <QListView>
#include <QLineEdit>
#include <QHBoxLayout>

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

CComboBoxWidget::CComboBoxWidget(QWidget* pParent, bool bAddTip):
	QWidget(pParent)
{
	m_bAddTip = bAddTip;
	setAttribute(Qt::WA_StyledBackground, true);
	InitWidget();

	CreateConnections();
}

CComboBoxWidget::~CComboBoxWidget()
{
}

void CComboBoxWidget::SetName(const QString &strName)
{
	m_strLabelText = strName;
	m_pNameLabel->setToolTip(strName);
	if(m_bLimitLabelWidth)
		m_pNameLabel->setText(m_pNameLabel->fontMetrics().elidedText(strName, Qt::ElideRight, m_pNameLabel->width()));
	else
		m_pNameLabel->setText(strName);
}

void CComboBoxWidget::SetLabelConfig(int nWidth, Qt::Alignment alignment, int nInterval, int nInterval1)
{
	m_pNameLabel->setFixedWidth(nWidth);

	m_pNameLabel->setAlignment(alignment);
	m_pHorLayout->insertSpacing(1, nInterval);

	if (m_bAddTip)
	{
		m_pHorLayout->insertSpacing(3, nInterval1);
	}

	m_pNameLabel->setText(m_pNameLabel->fontMetrics().elidedText(m_strLabelText, Qt::ElideRight, m_pNameLabel->width()));
}

void CComboBoxWidget::SetTextConfig(int nInterval, Qt::Alignment alignment /*= Qt::AlignLeft | Qt::AlignVCenter*/)
{
	m_pNameLabel->setAlignment(alignment);
	m_pHorLayout->insertSpacing(1, nInterval);

	m_pNameLabel->setText(m_strLabelText);
}

void CComboBoxWidget::SetInterval(int nInterval /*= 0*/, int pos)
{
	m_pHorLayout->insertSpacing(pos, nInterval);
	m_pNameLabel->setText(m_strLabelText);
}

void CComboBoxWidget::SetEditable()
{
	m_pCombo->setProperty("EditComboBox", true);
	m_pCombo->SetEditable(true);
	CCompleterEx *pCompleter = new CCompleterEx(m_pCombo->model(), this);
	pCompleter->setFilterMode(Qt::MatchContains);
	m_pCombo->setCompleter(pCompleter);
}

void CComboBoxWidget::AddItems(const QStringList & listItems, bool bSelectPreIndex, bool bDelPreOptions)
{
	m_pCombo->blockSignals(true);

	int nIndex = m_pCombo->currentIndex();

	if (bDelPreOptions)
	{
		m_listOptions.clear();
		m_pCombo->clear();
		m_listOptions = listItems;
	}
	else
		m_listOptions << listItems;

	int i = 0;
	for (auto item : listItems)
	{
		m_pCombo->addItem(item);
		m_pCombo->setItemData(i++, item, Qt::ToolTipRole);
	}

	if (bSelectPreIndex && nIndex < listItems.size() && nIndex >= 0)
	{
		m_pCombo->setCurrentIndex(nIndex);
		m_pCombo->setCurrentText(listItems[nIndex]);
	}
		

	m_pCombo->blockSignals(false);
}

void CComboBoxWidget::AddItemsTip(const QStringList & listItems)
{
	m_pCombo->blockSignals(true);

	for (int i = 0; i < listItems.size(); ++i) 
	{
		m_pCombo->setItemData(i, listItems[i], Qt::ToolTipRole);
	}

	m_pCombo->blockSignals(false);
}

void CComboBoxWidget::AddItemTip(int index, const QString & strTip)
{
	if (index < 0 || index >= m_pCombo->count())
		return; 

	m_pCombo->blockSignals(true);

	m_pCombo->setItemData(index, strTip, Qt::ToolTipRole);

	m_pCombo->blockSignals(false);
}

void CComboBoxWidget::AddItem(const QString &strItem, const QIcon &icon /*= QIcon()*/)
{
	m_pCombo->blockSignals(true);

	m_pCombo->addItem(icon, strItem);
	m_listOptions << strItem;

	m_pCombo->blockSignals(false);
}

void CComboBoxWidget::AddItem(const QString & atext, const QVariant & auserData)
{
	m_pCombo->blockSignals(true);

	m_pCombo->addItem(atext, auserData);
	m_listOptions << atext;

	m_pCombo->blockSignals(false);
}

void CComboBoxWidget::SetItemEnable(int index, bool b)
{
	m_pCombo->setItemData(m_bPlaceholder ? index : index + 1, b ? QVariant(1 | 32) : QVariant(0), Qt::UserRole - 1);
	m_pCombo->setItemData(m_bPlaceholder ? index : index + 1, b ? QColor(220, 220, 220) : QColor(120, 124, 126), Qt::ForegroundRole);
}

void CComboBoxWidget::SetReadOnly(bool bReadOnly)
{
	m_pCombo->setEditable(!bReadOnly);
}

void CComboBoxWidget::SetMaxVisibleItems(int nCount)
{
	m_pCombo->setMaxVisibleItems(nCount);
}

void CComboBoxWidget::SetSelectIndex(int nIndex)
{	
	style()->unpolish(m_pCombo);
	m_pCombo->setProperty("QComboBoxPlaceholder", m_bPlaceholder && nIndex == 0);
	style()->polish(m_pCombo);

	if ((m_bPlaceholder && nIndex <= m_listOptions.size()) || (!m_bPlaceholder && nIndex < m_listOptions.size()))
	{
		m_pCombo->SetCurrentIndex(nIndex);
		auto p = m_pCombo->currentText();
		slotUpdateComboDisplayText(nIndex);
	}

}

int CComboBoxWidget::GetSelectIndex()
{
	return m_pCombo->currentIndex();
}

QString CComboBoxWidget::GetSelectText()
{
	return m_pCombo->currentText();
}

void CComboBoxWidget::SetComboxWidth(int nWidth)
{
	m_pCombo->setFixedWidth(nWidth);
}

void CComboBoxWidget::SetComboBoxEnable(bool bEnable)
{
	m_pCombo->setEnabled(bEnable);
}

int CComboBoxWidget::GetIndexFromItem(QString strItem)
{
	return m_listOptions.indexOf(strItem);
}

void CComboBoxWidget::Clear()
{
	m_pCombo->blockSignals(true);
	m_listOptions.clear();
	m_pCombo->SetCurrentIndex(-1);
	m_pCombo->clear();
	m_pCombo->blockSignals(false);
}

int CComboBoxWidget::Count()
{
	return m_pCombo->count();
}

void CComboBoxWidget::SetComboBoxShowText(const QString &strText)
{
	m_pCombo->blockSignals(true);
	m_pCombo->setCurrentText(strText);
	m_listOptions[m_pCombo->currentIndex()] = strText;
	m_pCombo->blockSignals(false);
}

void CComboBoxWidget::SetCompleter(QCompleter* completer)
{
	m_pCombo->blockSignals(true);
	m_pCombo->setCompleter(completer);
	m_pCombo->blockSignals(false);
}

QCompleter* CComboBoxWidget::GetCompleter()
{
	return m_pCombo->completer();
}

void CComboBoxWidget::SetComboBoxEditable(bool bEnable)
{
	m_pCombo->setEditable(bEnable);
}

bool CComboBoxWidget::SetLabelProperty(const char* strProperty, QVariant flag)
{
	return m_pNameLabel->setProperty(strProperty, flag);
}

void CComboBoxWidget::SetComboProperty(const char* strProperty, QVariant flag)
{
	m_pCombo->setProperty(strProperty, flag);
}

void CComboBoxWidget::SetPlaneHolder(const QString & text)
{
	m_strPlaceholder = text;
	m_bPlaceholder = true;
	m_pCombo->SetPlaceholder(m_bPlaceholder);

	m_pCombo->blockSignals(true);

	// 添加提示信息作为第一项，并设置为默认显示
	m_pCombo->insertItem(0, m_strPlaceholder, -1);
	m_pCombo->setCurrentIndex(0);

	qobject_cast<QListView *>(m_pCombo->view())->setRowHidden(0, true);

	m_pCombo->blockSignals(false);
}

void CComboBoxWidget::SetBottomLine(bool bFlag)
{
	setFixedHeight(nSizeDefine_32);
	m_pHorLayout->setContentsMargins(nSizeDefine_16, nSizeDefine_4, nSizeDefine_16, nSizeDefine_4);

	SetWidgetProperty("PtyWithBottomLine", bFlag);
}

void CComboBoxWidget::SetWidgetContentMargin(int nLeft, int nTop, int nRight, int nBottom)
{
	setFixedHeight(UICommon_CtrlHeight + nTop + nBottom);
	m_pHorLayout->setContentsMargins(nLeft, nTop, nRight, nBottom);
}

void CComboBoxWidget::SetTipTextContent(const QString& text)
{
	m_pTipBtn->SetTextContent(text);
}

void CComboBoxWidget::SetTipBubbleSize(int width, int height)
{
	m_pTipBtn->SetToolTipFixedSize(width, height);
}

void CComboBoxWidget::SetElideComboDisplayText(bool bElide)
{
	m_bElideComboDisplayText = bElide;
	if (!bElide)
		return;

	m_pCombo->setEditable(true);
	Qt::Alignment nAlignment = Qt::AlignLeft | Qt::AlignVCenter;
	if (QLineEdit* pOldEdit = m_pCombo->lineEdit())
		nAlignment = pOldEdit->alignment();

	CLineEditEx* pEdit = new CLineEditEx(m_pCombo);
	pEdit->SetSendMsgToParent(true);
	pEdit->setAlignment(nAlignment);
	pEdit->setReadOnly(true);
	pEdit->setFocusPolicy(Qt::NoFocus);
	m_pCombo->setLineEdit(pEdit);

	connect(m_pCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CComboBoxWidget::slotUpdateComboDisplayText, Qt::UniqueConnection);
	slotUpdateComboDisplayText(m_pCombo->currentIndex());
}

void CComboBoxWidget::slotUpdateComboDisplayText(int nIndex)
{
	// 对齐模式与省略模式都会挂 lineEdit；只要存在就同步当前项文字
	CLineEditEx* pEdit = qobject_cast<CLineEditEx*>(m_pCombo->lineEdit());
	if (!pEdit)
		return;
	if (nIndex < 0)
	{
		pEdit->clear();
		return;
	}
	pEdit->setText(m_pCombo->itemText(nIndex));
}

void CComboBoxWidget::slotActivated(int nIndex)
{
	if (m_bPlaceholder)
	{
		style()->unpolish(m_pCombo);
		m_pCombo->setProperty("QComboBoxPlaceholder", false);
		style()->polish(m_pCombo);
	}
	emit signalActivated(m_bPlaceholder ? (nIndex - 1) : nIndex);

}

void CComboBoxWidget::slotSelectChange(int nIndex)
{
	emit signalSelectChange(m_bPlaceholder ? (nIndex - 1) : nIndex);
}

void CComboBoxWidget::slotSelectChange(const QString &strText)
{
	//qDebug() << "----------------Current Text Changed:" << strText;

	emit signalSelectTextChange(strText);
}

void CComboBoxWidget::slotEditTextChanged(const QString &text)
{
	emit signalEditTextChange(text);

	//qDebug() << "----------------Edit Text Changed:" << text;
}

void CComboBoxWidget::InitWidget()
{
	setFixedHeight(UICommon_CtrlHeight);

	m_pNameLabel = new QLabel(this);
	m_pNameLabel->setFixedHeight(UICommon_CtrlHeight);
	m_pNameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

	m_pTipBtn = new CTipButton(this);
	m_pTipBtn->setFixedSize(nSizeDefine_16, nSizeDefine_16);
	m_pTipBtn->SetTrianglePosRatio(0.5);
	m_pTipBtn->setProperty("TipButton", true);
	m_pTipBtn->SetLeftTopMargin(SysDpiZoom(12), SysDpiZoom(6));
	m_pTipBtn->setVisible(m_bAddTip);

	m_pCombo = new CComboBoxEx(this);
	m_pCombo->setFixedHeight(UICommon_CtrlHeight);
	m_pCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	
	QRegExp reg("^[^\\\\/:*?\"<>|]{1,256}$");
	m_pCombo->setValidator(new QRegExpValidator(reg, this));

	m_pHorLayout = new QHBoxLayout;
	m_pHorLayout->setMargin(0);
	m_pHorLayout->setSpacing(0);
	m_pHorLayout->addWidget(m_pNameLabel);
	m_pHorLayout->addWidget(m_pTipBtn);
	m_pHorLayout->addWidget(m_pCombo);

	setLayout(m_pHorLayout);
}

void CComboBoxWidget::CreateConnections()
{
	connect(m_pCombo, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(slotSelectChange(const QString &)));

	connect(m_pCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSelectChange(int)));
	
	connect(m_pCombo, SIGNAL(editTextChanged(const QString &)), this, SLOT(slotEditTextChanged(const QString &)));

	connect(m_pCombo, SIGNAL(activated(int)), this, SLOT(slotActivated(int)));
}

QVariant CComboBoxWidget::CurrentData()
{
	return m_pCombo->currentData();
}

void CComboBoxWidget::showEvent(QShowEvent * event)
{
	__super::showEvent(event);
	SetName(m_strLabelText);
}

void CComboBoxWidget::SetComboTextAlignment(Qt::Alignment nAlignment)
{
	m_pCombo->SetTextAlignment(nAlignment);
	// 与 SetElideComboDisplayText 一致：索引变化时刷新收起态 lineEdit 文字
	connect(m_pCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CComboBoxWidget::slotUpdateComboDisplayText, Qt::UniqueConnection);
	slotUpdateComboDisplayText(m_pCombo->currentIndex());
}

void CComboBoxWidget::SetWidgetProperty(const char* strProperty, QVariant flag)
{
	style()->unpolish(this);
	setProperty(strProperty, flag);
	style()->polish(this);
}
