#include "LabelSpinbox.h"
#include <QLabel>
#include "SpinBoxEx.h"
#include <QHBoxLayout>
#include <QFontMetrics>
#include "UICommonSizeDefine.h"
#include <QEvent>
#include <QStyle>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CLabelSpinbox::CLabelSpinbox(QWidget *parent, const QString& labelText, int spinboxVuale)
	: QWidget(parent)
{
	setAttribute(Qt::WA_StyledBackground, true);
	InitWidget();
	CreateConnects();
	
	// 限制提前，保证默认value设置成功
	SetMinimum(INT_MIN);
	SetMaximum(INT_MAX);

	SetText(labelText);
	SetValue(spinboxVuale);
}

CLabelSpinbox::~CLabelSpinbox()
{}

void CLabelSpinbox::SetLabelConfig(int nWidth, Qt::Alignment alignment /*= Qt::AlignLeft | Qt::AlignVCenter*/, int nInterval)
{
	m_pLabel->setFixedWidth(nWidth);
	m_pLabel->setAlignment(alignment);
	m_pMainLayout->insertSpacing(1, nInterval);
}

void CLabelSpinbox::SetLabelWidth(int nWidth)
{
	m_pLabel->setFixedWidth(nWidth);
	if (!m_strLabelText.isEmpty())
		SetText(m_strLabelText);
}

void CLabelSpinbox::SetSpinBoxWidth(int nWidth)
{
	m_pSpinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_pSpinBox->setFixedWidth(nWidth);
}

void CLabelSpinbox::EnableSpinBoxStretch(int nMinWidth)
{
	if (nMinWidth > 0)
		m_pSpinBox->setMinimumWidth(nMinWidth);
	else
		m_pSpinBox->setMinimumWidth(0);

	m_pSpinBox->setMaximumWidth(QWIDGETSIZE_MAX);
	m_pSpinBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	m_pMainLayout->removeWidget(m_pSpinBox);
	m_pMainLayout->addWidget(m_pSpinBox, 1);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setFixedHeight(UICommon_CtrlHeight);
}

void CLabelSpinbox::SetSpinBoxAlignment(Qt::Alignment aalignment)
{
	m_pSpinBox->setAlignment(aalignment);
}

void CLabelSpinbox::SetConfig(int nLabelWidth, int nSpinBoxWidth, int nInterval)
{
	m_pLabel->setFixedWidth(nLabelWidth);
	m_pSpinBox->setFixedWidth(nSpinBoxWidth);

	if(nInterval)
		m_pMainLayout->insertSpacing(1, nInterval);

	this->setFixedWidth(nLabelWidth + nSpinBoxWidth + nInterval);
}

void CLabelSpinbox::SetWidgetContentMargin(int nLeft, int nTop, int nRight, int nBottom)
{
	setFixedHeight(UICommon_CtrlHeight + nTop + nBottom);
	m_pMainLayout->setContentsMargins(nLeft, nTop, nRight, nBottom);
}

void CLabelSpinbox::InitWidget()
{
	m_pLabel = new QLabel(this);
	m_pLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	m_pLabel->setMinimumWidth(SysDpiZoom(24));

	m_pSpinBox = new CSpinBox(this);

	m_pSpinBox->setFixedHeight(UICommon_CtrlHeight);

	m_pMainLayout = new QHBoxLayout;
	m_pMainLayout->setSpacing(0);
	m_pMainLayout->addWidget(m_pLabel);
	m_pMainLayout->addWidget(m_pSpinBox, Qt::AlignLeft);

	m_pMainLayout->setContentsMargins(0, 0, 0, 0);
	setLayout(m_pMainLayout);
}

void CLabelSpinbox::CreateConnects()
{
	connect(m_pSpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged(int)));
	connect(m_pSpinBox, SIGNAL(editingFinished()), this, SIGNAL(signalEditingFinished()));
}

void CLabelSpinbox::SetBottomLine(bool bFlag)
{
	setFixedHeight(nSizeDefine_32);
	m_pMainLayout->setContentsMargins(nSizeDefine_16, nSizeDefine_4, nSizeDefine_16, nSizeDefine_4);

	SetWidgetProperty("PtyWithBottomLine", bFlag);
}

void CLabelSpinbox::SetText(const QString& text, bool bAdaptive)
{
	m_strLabelText = text;
	if (bAdaptive)
	{
		int len = SysDpiZoom(fontMetrics().width(text) + 4);
		int temp = len - m_pLabel->width();

		m_pLabel->setFixedWidth(len);
		setFixedWidth(width() + temp);
	}

	int nWidth = m_pLabel->width();
	m_pLabel->setToolTip(text);
	auto str = m_pLabel->fontMetrics().elidedText(text, Qt::ElideRight, nWidth);
	m_pLabel->setText(str);
}

QString	CLabelSpinbox::GetText()
{
	return m_pLabel->text();
}

void CLabelSpinbox::SetValue(int value)
{
	blockSignals(true);
	m_pSpinBox->blockSignals(true);
	m_pSpinBox->setValue(value);
	m_pSpinBox->blockSignals(false);
	blockSignals(false);
}

int CLabelSpinbox::GetValue()
{
	return m_pSpinBox->value();
}

void CLabelSpinbox::SetMinimum(int min)
{
	m_pSpinBox->setMinimum(min);
}

void CLabelSpinbox::SetMaximum(int max)
{
	m_pSpinBox->setMaximum(max);
}

void CLabelSpinbox::SetMinMaximum(int nMin, int nMax)
{
	blockSignals(true);
	m_pSpinBox->setMinimum(nMin);
	m_pSpinBox->setMaximum(nMax);
	blockSignals(false);
}

void CLabelSpinbox::SetPrefix(const QString& prefix)
{
	m_pSpinBox->setPrefix(prefix);
}

void CLabelSpinbox::SetSuffix(const QString& suffix)
{
	m_pSpinBox->setSuffix(suffix);
}

QString CLabelSpinbox::GetSuffix()
{
	return m_pSpinBox->suffix();
}

void CLabelSpinbox::SetSingleStep(int val)
{
	m_pSpinBox->setSingleStep(val);
}

void CLabelSpinbox::SetReadOnly(bool bReadOnly)
{
	m_pSpinBox->setReadOnly(bReadOnly);
	m_pSpinBox->setFocusPolicy(bReadOnly ? Qt::NoFocus : Qt::ClickFocus);
}

void CLabelSpinbox::SetBtnsVisible(bool b)
{
	m_pSpinBox->setProperty("BtnVisible", b);
}

void CLabelSpinbox::SetLabelProperty(const char* strProperty, bool bValue)
{
	m_pLabel->setProperty(strProperty, bValue);
}

void CLabelSpinbox::SetSignalFouceOut(bool bUse)
{
	if (bUse)
		m_pSpinBox->installEventFilter(this);
	else
		m_pSpinBox->removeEventFilter(this);
}

void CLabelSpinbox::SetSpinBoxProperty(const char* strProperty, bool bValue)
{
	m_pSpinBox->setProperty(strProperty, bValue);
}

void CLabelSpinbox::SetWidgetProperty(const char* strProperty, bool bValue)
{
	style()->unpolish(this);
	this->setProperty(strProperty, bValue);
	style()->polish(this);
}


// 设置SpinBox符号显示模式
void CLabelSpinbox::SetSpinBoxShowSymbol(bool bShow)
{
	if (m_pSpinBox)
	{
		m_pSpinBox->setShowSymbol(bShow);
		update();
	}
}

// 设置SpinBox显示符号
void CLabelSpinbox::SetSpinBoxDisplaySymbol(const QString& symbol)
{
	if (m_pSpinBox)
	{
		m_pSpinBox->setDisplaySymbol(symbol);
		update();
	}
}

// 获取符号显示模式状态
bool CLabelSpinbox::IsSpinBoxShowSymbol() const
{
	return m_pSpinBox ? m_pSpinBox->isShowSymbol() : false;
}

// 获取当前显示符号
QString CLabelSpinbox::GetSpinBoxDisplaySymbol() const
{
	return m_pSpinBox ? m_pSpinBox->displaySymbol() : QString();
}

QWidget* CLabelSpinbox::GetSpinBox()
{
	return m_pSpinBox;
}

bool CLabelSpinbox::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == m_pSpinBox)
	{
		if (event->type() == QEvent::FocusOut)
		{
			emit signalFouceOut();
		}
	}

	return __super::eventFilter(watched, event);
}

void CLabelSpinbox::showEvent(QShowEvent * event)
{
	__super::showEvent(event);
	
	int nWidth = m_pLabel->width();
	m_pLabel->setToolTip(m_strLabelText);
	auto str = m_pLabel->fontMetrics().elidedText(m_strLabelText, Qt::ElideRight, nWidth);
	m_pLabel->setText(str);
}
