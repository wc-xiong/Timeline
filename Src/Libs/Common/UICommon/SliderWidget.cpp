#include "SliderWidget.h"
#include "SliderEx.h"
#include <QLabel>
#include "DoubleSpinBoxEx.h"
#include <QBoxLayout>
#include <QDebug>
#include <QMouseEvent>
#include <QStyle>
#include <QShowEvent>
#include <QSignalBlocker>
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

CSliderWidget::CSliderWidget(QWidget* pParent, bool bVerLayout):
	QWidget(pParent)
{
	setAttribute(Qt::WA_StyledBackground, true);
	m_bPrecent = false;

	m_fMinValue = 0.0;
	m_fMaxValue = 1.0;

	m_fExps = 0.1;

	m_bVerLayout = bVerLayout;

	InitWidget();

	CreateConnections();
}

CSliderWidget::~CSliderWidget()
{
}

void CSliderWidget::SetName(const QString &strName)
{
	m_strLabelText = strName;
	m_pNameLabel->setToolTip(strName);
	const int nWidth = m_pNameLabel->width();
	if (nWidth > 0)
		m_pNameLabel->setText(m_pNameLabel->fontMetrics().elidedText(strName, Qt::ElideRight, nWidth));
	else
		m_pNameLabel->setText(strName);
}

void CSliderWidget::SetSliderConfig(int nDecimals, double fMin, double fMax, double fDefaultValue, const QString &strSuffix /*= ""*/)
{
	blockSignals(true);

	m_bPrecent = false;

	m_nSliderType = nDecimals;
	m_pSpinBox->setDecimals(nDecimals);
	double v = 1;
	for (int i = 0; i < nDecimals; i++)
		v /= 10;
	m_pSpinBox->setSingleStep(v);
	m_pSpinBox->setStepType(QAbstractSpinBox::DefaultStepType);

	m_pSlider->setDecimals(nDecimals);

	m_fMinValue = fMin, m_fMaxValue = fMax;

	m_pSpinBox->setSuffix(strSuffix);
	if (strSuffix == "%")	//特判百分号
	{
		m_fMinValue *= 100.0;
		m_fMaxValue *= 100.0;
		fDefaultValue *= 100.0;
		m_bPrecent = true;
		m_pSpinBox->setSingleStep(1.0);
		m_pSpinBox->setStepType(QAbstractSpinBox::DefaultStepType);
	}

	m_pSpinBox->setRange(m_fMinValue, m_fMaxValue);
	m_pSlider->SetRange(m_fMinValue, m_fMaxValue, fDefaultValue);

	m_fExps = 0.1;
	for (int i = 0; i < m_nSliderType; i++)
		m_fExps *= 0.1;
	if (m_bPrecent)
		m_fExps *= 0.01;
	blockSignals(false);
}

void CSliderWidget::SetSingleStep(double fStep)
{
	m_pSpinBox->setSingleStep(fStep);
	m_pSpinBox->setStepType(QAbstractSpinBox::DefaultStepType);
}

void CSliderWidget::SetLabelConfig(int nWidth, Qt::Alignment alignment, int nInterval)
{
	m_pNameLabel->setFixedWidth(nWidth);
	m_pNameLabel->setAlignment(alignment);
	if (m_pHorLayout)
	{
		m_pHorLayout->insertSpacing(1, nInterval);
		m_pHorLayout->insertSpacing(3, nInterval);
	}
	if (!m_strLabelText.isEmpty())
		SetName(m_strLabelText);
}

void CSliderWidget::SetSpinBoxWidth(int width)
{
	m_pSpinBox->setFixedSize(width, UICommon_CtrlHeight);
}

void CSliderWidget::SetValue(double fValue)
{
	QSignalBlocker blockSelf(this);
	QSignalBlocker blockSpin(m_pSpinBox);
	if (m_bPrecent)
		fValue *= 100.0;
	m_pSpinBox->setValue(fValue);
	if (m_pSlider)
	{
		QSignalBlocker blockSlider(m_pSlider);
		m_pSlider->SetValue(fValue);
	}
}

double CSliderWidget::GetValue()
{
	return m_bPrecent ? (m_pSpinBox->value() / 100.0) : m_pSpinBox->value();
}

double CSliderWidget::GetMaximum()
{
	return m_fMaxValue;
}

double CSliderWidget::GetMinimum()
{
	return m_fMinValue;
}

void CSliderWidget::SetBtnsVisible(bool b)
{
	m_pSpinBox->setProperty("BtnVisible", b);
}

void CSliderWidget::SetReadOnly(bool bReadOnly)
{
	m_pSpinBox->setReadOnly(bReadOnly);
	if (bReadOnly)
		m_pSpinBox->setProperty("BtnVisible", false);
}

void CSliderWidget::SetSliderDefaultPos(double fDefaultValue)
{
	m_pSlider->SetDefaultValuePos(fDefaultValue);
}

void CSliderWidget::SetLabelProperty(const char* strProperty, bool bValue)
{
	m_pNameLabel->setProperty(strProperty, bValue);
}

void CSliderWidget::SetSliderProperty(const char* strProperty, bool bValue)
{
	m_pSlider->setProperty(strProperty, bValue);
}

void CSliderWidget::SetSpinBoxProperty(const char* strProperty, bool bValue)
{
	m_pSpinBox->setProperty(strProperty, bValue);
}

void CSliderWidget::SetWidgetProperty(const char* strProperty, QVariant flag)
{
	style()->unpolish(this);
	setProperty(strProperty, flag);
	style()->polish(this);
}

void CSliderWidget::SetBottomLine(bool bFlag)
{
	if (m_bVerLayout)
		return;

	setFixedHeight(nSizeDefine_32);
	layout()->setContentsMargins(nSizeDefine_16, nSizeDefine_4, nSizeDefine_16, nSizeDefine_4);

	SetWidgetProperty("PtyWithBottomLine", bFlag);
}

void CSliderWidget::SetContentsMargins(int left, int top, int right, int bottom)
{
	setFixedHeight(UICommon_CtrlHeight + top + bottom);
	layout()->setContentsMargins(left, top, right, bottom);
}

void CSliderWidget::slotSliderValueChange(double fValue)
{
	if (m_bRoundOff)
	{
		fValue = round(fValue);
		m_pSlider->SetValue(fValue);
	}

	if (IsEqual(m_pSpinBox->value(), fValue))
		return;

	m_pSpinBox->SetValue(fValue);

	double fRealValue = m_bPrecent ? fValue / 100.0 : fValue;
	emit signalValueChange(fRealValue);

	emit signalValueChangeBySliderOrEditor(fRealValue, true);
}

void CSliderWidget::slotSpinBoxValueChange(double fValue)
{
	if (IsEqual(m_pSlider->GetValue(), fValue))
		return;

	// 值改变之前先发信号
	emit signalValueChangeStart(m_bPrecent ? m_pSlider->GetValue()/100.0 : m_pSlider->GetValue());

	double fRealValue = m_bPrecent ? fValue / 100.0 : fValue;
	emit signalValueChange(fRealValue);

	// 值改变以后发信号
	emit signalValueChangedMouseRelease(fRealValue);

	emit signalValueChangeBySliderOrEditor(fRealValue, false);

	m_pSlider->SetValue(fValue);
}

void CSliderWidget::InitWidget()
{
	//setMinimumWidth(SysDpiZoom(300));

	m_pSlider = new CSliderEx(this);
	m_pSlider->setFixedHeight(UICommon_CtrlHeight);
	m_pSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	m_pNameLabel = new QLabel(this);
	m_pNameLabel->setFixedHeight(UICommon_CtrlHeight);
	m_pNameLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	m_pSpinBox = new CDoubleSpinBox(this);
	m_pSpinBox->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
	m_pSpinBox->setFixedSize(SysDpiZoom(76), UICommon_CtrlHeight);
	
	if (m_bVerLayout)
	{
		m_pNameLayout = new QHBoxLayout;
		m_pNameLayout->setMargin(0);
		m_pNameLayout->addWidget(m_pNameLabel);
		m_pNameLayout->addStretch();

		QHBoxLayout* pHorLayout2 = new QHBoxLayout;
		pHorLayout2->setMargin(0);
		pHorLayout2->addWidget(m_pSlider);
		pHorLayout2->addSpacing(nSizeDefine_12);
		pHorLayout2->addWidget(m_pSpinBox);

		QVBoxLayout* pVLayout = new QVBoxLayout;

		pVLayout->setMargin(0);
		pVLayout->setSpacing(0);
		pVLayout->addLayout(m_pNameLayout);
		pVLayout->addSpacing(SysDpiZoom(4));
		pVLayout->addItem(pHorLayout2);
		pVLayout->addStretch();

		setLayout(pVLayout);
	}
	else
	{
		setFixedHeight(UICommon_CtrlHeight);

		m_pHorLayout = new QHBoxLayout;

		m_pHorLayout->setMargin(0);
		m_pHorLayout->setSpacing(0);
		m_pHorLayout->addWidget(m_pNameLabel);
		m_pHorLayout->addWidget(m_pSlider);
		m_pHorLayout->addWidget(m_pSpinBox);

		setLayout(m_pHorLayout);
	}
}

void CSliderWidget::CreateConnections()
{
	connect(m_pSlider, SIGNAL(signalvalueChanged(double)), this, SLOT(slotSliderValueChange(double)));
	// % 模式下 CSliderEx 内部为 0~100，对外统一归一到 0~1（与 signalValueChange 一致）
	connect(m_pSlider, &CSliderEx::signalValueChangedMouseRelease, this, [this](double fValue) {
		const double fRealValue = m_bPrecent ? fValue / 100.0 : fValue;
		emit signalValueChangedMouseRelease(fRealValue);
	});
	connect(m_pSpinBox, SIGNAL(valueChanged(double)), this, SLOT(slotSpinBoxValueChange(double)));
	connect(m_pSlider, &CSliderEx::signalValueChangeStart, this, &CSliderWidget::signalValueChangeStart);

	connect(m_pSlider, &CSliderEx::signalDefaultValueChanged, this, [&](double fValue)
	{
		if (!IsEqual(fValue, m_pSlider->GetValue()))
		{
			m_pSpinBox->SetValue(fValue);
			m_pSlider->SetValue(fValue);
			double fRealValue = m_bPrecent ? fValue / 100.0 : fValue;
			emit signalValueChange(fRealValue);
			emit signalValueChangedMouseRelease(fRealValue);
		}
	});
}

inline bool CSliderWidget::IsEqual(double fValue1, double fValue2)
{
	return abs(fValue1 - fValue2) < m_fExps;
}

void CSliderWidget::SetSliderEnabled(bool b)
{
	m_pSlider->setEnabled(b);
	m_pSpinBox->setEnabled(b);
}

void CSliderWidget::showEvent(QShowEvent *event)
{
	__super::showEvent(event);
	if (!m_strLabelText.isEmpty())
		SetName(m_strLabelText);
}