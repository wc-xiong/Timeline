#include "LineEditWidget.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QDebug>
#include <QLineEdit>
#include <QStyle>
#include "UICommonSizeDefine.h"
#include "LineEditEx.h"
#include "CheckBoxEx.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CLineEditWidget::CLineEditWidget(QWidget* pParent, bool flag):
	QWidget(pParent),
	m_bUseCheckBox(flag)
{
	setAttribute(Qt::WA_StyledBackground, true);
	InitWidget();

	CreateConnections();
}

CLineEditWidget::~CLineEditWidget()
{
}

bool CLineEditWidget::IsChecked()
{
	return m_pCheckBox->isChecked();
}

void CLineEditWidget::SetChecked(bool bVlaue, bool signalEmitted)
{
	m_pCheckBox->SetChecked(bVlaue);
	if (!m_pCheckBox->isChecked())
	{
		m_pNameLabel->setEnabled(false);
		m_pEdit->setEnabled(false);
	}
	else
	{
		m_pNameLabel->setEnabled(true);
		m_pEdit->setEnabled(true);
	}
	if(signalEmitted)
		emit signalClicked();
}

void CLineEditWidget::SetName(const QString &strName,bool b, bool bElideMarginChangedByDpi)
{
	QFontMetrics fontWidth(m_pNameLabel->font());//得到每个字符的宽度

	float dpi = GetSystemDpi();
	int margin = SysDpiZoom(10);
	
	if (bElideMarginChangedByDpi)
	{
		if (dpi > 1.25)
			margin += nSizeDefine_10;
		if (dpi > 1.5)
			margin += nSizeDefine_10;
		if (dpi > 1.75)
			margin += nSizeDefine_10;
	}

	QString elideNote = fontWidth.elidedText(strName, Qt::ElideRight, m_pNameLabel->width() - margin);
	m_pNameLabel->setText(elideNote);
	if(b)
		m_pNameLabel->setText(strName);
	m_pNameLabel->setToolTip(strName);
}

void CLineEditWidget::SetLabelConfig(int nWidth, Qt::Alignment alignment, int nInterval)
{
	m_pNameLabel->setFixedWidth(nWidth);
	m_pNameLabel->setAlignment(alignment);
	m_pHorLayout->insertSpacing(1, nInterval);
}

void CLineEditWidget::SetEditConfig(Qt::Alignment alignment)
{
	m_pEdit->setAlignment(alignment);
}

void CLineEditWidget::SetLabelEditInterval(int nInterval /*= 0*/)
{
	m_pHorLayout->insertSpacing(1, nInterval);
}

void CLineEditWidget::SetLineEditWidth(int nWidth)
{
	m_pEdit->setFixedWidth(nWidth);
	m_pHorLayout->addStretch();
}

void CLineEditWidget::SetBottomLine(bool bFlag)
{
	setFixedHeight(nSizeDefine_32);
	m_pHorLayout->setContentsMargins(nSizeDefine_16, nSizeDefine_4, nSizeDefine_16, nSizeDefine_4);

	SetWidgetProperty("PtyWithBottomLine", bFlag);
}

void CLineEditWidget::SetReadOnly(bool bReadOnly)
{
	m_pEdit->setReadOnly(bReadOnly);

	if (bReadOnly)
		m_pEdit->setFocusPolicy(Qt::NoFocus);
	else
		m_pEdit->setFocusPolicy(Qt::ClickFocus);
}

void CLineEditWidget::SetText(const QString &text)
{
	blockSignals(true);
	m_pEdit->setText(text);
	m_tCurtext = text;
	blockSignals(false);
}

QString CLineEditWidget::GetText()
{
	return m_tCurtext;
}

QString CLineEditWidget::GetName()
{
	return m_pNameLabel->text();
}

void CLineEditWidget::SetValidator(QValidator * validator)
{
	m_pEdit->setValidator(validator);
}

void CLineEditWidget::slotTextChange(const QString &text)
{
	if (m_tCurtext == text)
		return;

	m_tCurtext = text;
	//qDebug() << "-----------------LineEdit Text is:" << text;

	emit signalTextChange(text);
}
void CLineEditWidget::slotCheckBoxClicked() 
{
	if (!m_pCheckBox->isChecked())
	{
		m_pNameLabel->setEnabled(false);
		m_pEdit->setEnabled(false);
	}
	else
	{
		m_pNameLabel->setEnabled(true);
		m_pEdit->setEnabled(true);
	}
	emit signalClicked();
}

void CLineEditWidget::SetEchoMode(QLineEdit::EchoMode mode)
{
	m_pEdit->setEchoMode(mode);
}

void CLineEditWidget::SetPlaceholderText(const QString &holderText)
{
	// 设置占位字符颜色
	QPalette palette;
	palette.setColor(QPalette::PlaceholderText, Qt::gray);
	m_pEdit->setPalette(palette);

	m_pEdit->setPlaceholderText(holderText);
}

void CLineEditWidget::SetMaxInputLen(int length)
{
	m_pEdit->setMaxLength(length);
}

void CLineEditWidget::SetCompleter(QCompleter* completer)
{
	m_pEdit->blockSignals(true);
	m_pEdit->setCompleter(completer);
	m_pEdit->blockSignals(false);
}

QCompleter* CLineEditWidget::GetCompleter()
{
	return m_pEdit->completer();
}

void CLineEditWidget::SetFocus()
{
	m_pEdit->setFocus();
}

int CLineEditWidget::GetEditCursorPos()
{
	return m_pEdit->cursorPosition();
}

void CLineEditWidget::SetEditCursorPos(int nPos)
{
	m_pEdit->setCursorPosition(nPos);
}

void CLineEditWidget::SetLabelProperty(const char* strProperty, bool bValue)
{
	m_pNameLabel->setProperty(strProperty, bValue);
}

void CLineEditWidget::SetLineEditProperty(const char* strProperty, bool bValue)
{
	m_pEdit->setProperty(strProperty, bValue);
}

void CLineEditWidget::SetLineEditTextColor(const char* strProperty)
{
	m_pEdit->setStyleSheet(strProperty);
}

QWidget* CLineEditWidget::GetLineEdit()
{
	return m_pEdit;
}

void CLineEditWidget::SetWidgetProperty(const char * strProperty, QVariant flag)
{
	style()->unpolish(this);
	setProperty(strProperty, flag);
	style()->polish(this);
}

void CLineEditWidget::InitWidget()
{
	setFixedHeight(UICommon_CtrlHeight);

	m_pNameLabel = new QLabel(this);
	m_pNameLabel->setFixedHeight(UICommon_CtrlHeight);
	m_pNameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

	m_pEdit = new CLineEditEx(this);
	m_pEdit->setFixedHeight(UICommon_CtrlHeight);
	m_pEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setProperty("CLineEditEx", true);
	
	m_pHorLayout = new QHBoxLayout;
	m_pHorLayout->setMargin(0);
	m_pHorLayout->setSpacing(0);

	if (m_bUseCheckBox)
	{
		m_pCheckBox = new CCheckBoxEx(this);
		m_pCheckBox->setFixedSize(SysDpiZoom(18), SysDpiZoom(16));
		m_pHorLayout->addWidget(m_pCheckBox);
		m_pHorLayout->addSpacing(SysDpiZoom(4));
	}

	m_pHorLayout->addWidget(m_pNameLabel);
	m_pHorLayout->addWidget(m_pEdit);
	
	setLayout(m_pHorLayout);
}

void CLineEditWidget::CreateConnections()
{
	connect(m_pEdit, SIGNAL(textChanged(const QString &)), this, SLOT(slotTextChange(const QString &)));
	connect(m_pEdit, SIGNAL(editingFinished()), this, SLOT(slotEditFinish()));
	if (m_pCheckBox != nullptr)
		connect(m_pCheckBox, SIGNAL(clicked()), this, SLOT(slotCheckBoxClicked()));
}

void CLineEditWidget::slotEditFinish()
{
	emit signalEditingFinished();

	emit signalEditFinished(m_pEdit->GetRealText());
}