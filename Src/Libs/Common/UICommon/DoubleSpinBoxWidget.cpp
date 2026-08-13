#include "DoubleSpinBoxWidget.h"
#include "SliderEx.h"
#include <QLabel>
#include "DoubleSpinBoxEx.h"
#include <QBoxLayout>
#include <QDebug>
#include <QMouseEvent>
#include <QShowEvent>
#include "UICommonSizeDefine.h"
#include <QLocale>

//#include "DoubleSpinBoxEx.h"

namespace
{
QString FormatSpinBoxToolTip(double value, int decimals)
{
	return QLocale::c().toString(value, 'f', qBound(0, decimals, 15));
}
}

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CDoubleSpinBoxWidget::CDoubleSpinBoxWidget(QWidget* pParent, bool bHorizontal):
	QWidget(pParent)
{

	InitWidget(bHorizontal);

	CreateConnections();
}

CDoubleSpinBoxWidget::~CDoubleSpinBoxWidget()
{
}

void CDoubleSpinBoxWidget::ReplaceSpinBox(QDoubleSpinBox* pNewSpinBox)
{
	if (!pNewSpinBox || !m_pHorLayout)
		return;
	if (pNewSpinBox == m_pSpinBox)
		return;

	// 记住旧值/状态，尽量无感替换
	const double oldValue = m_pSpinBox ? m_pSpinBox->value() : 0.0;
	const int oldDecimals = m_pSpinBox ? m_pSpinBox->decimals() : 0;
	const QString oldSuffix = m_pSpinBox ? m_pSpinBox->suffix() : QString();
	const double oldMin = m_pSpinBox ? m_pSpinBox->minimum() : 0.0;
	const double oldMax = m_pSpinBox ? m_pSpinBox->maximum() : DBL_MAX;
	const bool oldReadOnly = m_pSpinBox ? m_pSpinBox->isReadOnly() : false;
	const Qt::Alignment oldAlign = m_pSpinBox ? m_pSpinBox->alignment() : Qt::AlignHCenter;
	const QAbstractSpinBox::StepType oldStepType = m_pSpinBox ? m_pSpinBox->stepType() : QAbstractSpinBox::AdaptiveDecimalStepType;
	const QSizePolicy oldSizePolicy = m_pSpinBox ? m_pSpinBox->sizePolicy() : QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	// 移除并释放旧 spinbox
	if (m_pSpinBox)
	{
		m_pHorLayout->removeWidget(m_pSpinBox);
		m_pSpinBox->hide();
		m_pSpinBox->disconnect();
		m_pSpinBox->setParent(nullptr);
		m_pSpinBox->deleteLater();
	}

	// 接管新 spinbox
	m_pSpinBox = pNewSpinBox;
	m_pSpinBox->setParent(this);

	// 统一基础属性（尽量保持原行为）
	m_pSpinBox->setStepType(oldStepType);
	m_pSpinBox->setFixedHeight(UICommon_CtrlHeight);
	m_pSpinBox->setSizePolicy(oldSizePolicy);
	m_pSpinBox->setAlignment(oldAlign);
	m_pSpinBox->setMinimum(oldMin);
	m_pSpinBox->setMaximum(oldMax);
	m_pSpinBox->setSuffix(oldSuffix);
	m_pSpinBox->setDecimals(oldDecimals);
	m_pSpinBox->setReadOnly(oldReadOnly);
	m_pSpinBox->setFocusPolicy(oldReadOnly ? Qt::NoFocus : Qt::ClickFocus);

	// 插回布局（NameLabel 在 0，SpinBox 在 1）
	const int spinIdx = m_pHorLayout->count() > 1 ? 1 : m_pHorLayout->count();
	m_pHorLayout->insertWidget(spinIdx, m_pSpinBox);

	// 还原数值（放到最后，避免范围/小数位未设置导致截断）
	blockSignals(true);
	m_pSpinBox->setValue(oldValue);
	blockSignals(false);

	CreateConnections();
}

void CDoubleSpinBoxWidget::SetName(const QString &strName)
{
	m_strLabelText = strName;
	const int nWidth = m_pNameLabel->width();
	if (nWidth <= 0)
	{
		m_pNameLabel->setText(strName);
		m_pNameLabel->setToolTip(QString());
		return;
	}

	const QString strElided = m_pNameLabel->fontMetrics().elidedText(strName, Qt::ElideRight, nWidth);
	m_pNameLabel->setText(strElided);
	m_pNameLabel->setToolTip(strElided != strName ? strName : QString());
}

QString CDoubleSpinBoxWidget::GetName()
{ 
	return m_pNameLabel->text(); 
}

void CDoubleSpinBoxWidget::SetLabelConfig(int nWidth, Qt::Alignment alignment /*= Qt::AlignLeft | Qt::AlignVCenter*/, int nInterval /*= 0*/)
{
	m_pNameLabel->setFixedWidth(nWidth);
	m_pNameLabel->setAlignment(alignment);
	m_pHorLayout->insertSpacing(1, nInterval);
	if (!m_strLabelText.isEmpty())
		SetName(m_strLabelText);
}

void CDoubleSpinBoxWidget::SetTextConfig(int nInterval, Qt::Alignment alignment /*= Qt::AlignLeft | Qt::AlignVCenter*/)
{
	m_pNameLabel->setAlignment(alignment);
	m_pHorLayout->insertSpacing(1, nInterval);
}

void CDoubleSpinBoxWidget::SetSuffix(QString strSuffix)
{
	m_pSpinBox->setSuffix(strSuffix);
}

void CDoubleSpinBoxWidget::SetDecimals(int nCount)
{
	double v = 1;
	for (int i = 0; i < nCount; i++)
		v /= 10;
	m_pSpinBox->setSingleStep(v);

	m_pSpinBox->setDecimals(nCount);

	m_pSpinBox->setStepType(QAbstractSpinBox::DefaultStepType);
}

void CDoubleSpinBoxWidget::SetSingleStep(int nCount)
{
	double v = 1;
	for (int i = 0; i < nCount; i++)
		v /= 10;
	m_pSpinBox->setSingleStep(v);
}

void CDoubleSpinBoxWidget::SetSpinBoxWidth(int nWidth)
{
	m_pSpinBox->setFixedWidth(nWidth);
}

void CDoubleSpinBoxWidget::SetValue(double fValue)
{
	blockSignals(true);
	m_pSpinBox->setValue(fValue);
	blockSignals(false);

	if (m_bHoverShowValue)
	{
		m_pSpinBox->setToolTip(FormatSpinBoxToolTip(fValue, qMax(m_pSpinBox->decimals(), 3)));
	}
}

double CDoubleSpinBoxWidget::GetValue()
{
	return m_pSpinBox->value();
}

void CDoubleSpinBoxWidget::SetReadOnly(bool bReadOnly)
{
	m_pSpinBox->setReadOnly(bReadOnly);
	m_pSpinBox->setFocusPolicy(bReadOnly ? Qt::NoFocus : Qt::ClickFocus);
	if(bReadOnly)
		m_pSpinBox->setProperty("BtnVisible", false);
}

void CDoubleSpinBoxWidget::SetMaxandMinValue(double max, double min)
{
	blockSignals(true);
	m_pSpinBox->setMaximum(max);
	m_pSpinBox->setMinimum(min);
	blockSignals(false);
}

void CDoubleSpinBoxWidget::SetBtnsVisible(bool b)
{
	m_pSpinBox->setProperty("BtnVisible", b);
}

void CDoubleSpinBoxWidget::SetLabelProperty(const char* strProperty, bool bValue)
{
	m_pNameLabel->setProperty(strProperty, bValue);
}

void  CDoubleSpinBoxWidget::SetSpinBoxProperty(const char* strProperty, bool bValue)
{
	m_pSpinBox->setProperty(strProperty, bValue);
}

void CDoubleSpinBoxWidget::SetSpinBoxAlignment(Qt::Alignment aalignment)
{
	m_pSpinBox->setAlignment(aalignment);
}

void CDoubleSpinBoxWidget::SetSpecialValueText(const QString & str)
{
	blockSignals(true);
	m_pSpinBox->setSpecialValueText(str);
	blockSignals(false);
}

void CDoubleSpinBoxWidget::InitWidget(bool bHorizontal)
{
	setFixedHeight(UICommon_CtrlHeight);

	m_pNameLabel = new QLabel(this);
	m_pNameLabel->setFixedHeight(UICommon_CtrlHeight);

	if(bHorizontal)
		m_pSpinBox = new CHDoubleSpinBox(this);
	else
		m_pSpinBox = new CDoubleSpinBox(this);

	m_pSpinBox->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
	
	m_pSpinBox->setFixedHeight(UICommon_CtrlHeight);
	m_pSpinBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_pSpinBox->setMaximum(DBL_MAX);
	m_pSpinBox->setAlignment(Qt::AlignHCenter);

	m_pHorLayout = new QHBoxLayout;

	m_pHorLayout->setMargin(0);
	m_pHorLayout->setSpacing(0);

	m_pHorLayout->addWidget(m_pNameLabel);
	m_pHorLayout->addWidget(m_pSpinBox);

	setLayout(m_pHorLayout);
}

void CDoubleSpinBoxWidget::CreateConnections()
{
	if (!m_pSpinBox)
		return;

	connect(m_pSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [&](double value) {
		if (m_bHoverShowValue)
		{
			m_pSpinBox->setToolTip(FormatSpinBoxToolTip(value, qMax(m_pSpinBox->decimals(), 3)));
		}
		emit signalValueChange(value);
	});

	connect(m_pSpinBox, SIGNAL(editingFinished()), this, SIGNAL(signalEditingFinished()));

	// 仅转发实现了扩展信号的 SpinBox（普通 QDoubleSpinBox / CDoubleSpinBox 无此信号）
	if (qobject_cast<CCopyableDoubleSpinBox*>(m_pSpinBox) || qobject_cast<CHCopyableDoubleSpinBox*>(m_pSpinBox))
	{
		connect(m_pSpinBox, SIGNAL(signalDoubleClickCopied()), this, SIGNAL(signalDoubleClickCopied()));
		connect(m_pSpinBox, SIGNAL(signalRightClicked(QPoint)), this, SIGNAL(signalRightClicked(QPoint)));
	}
}

void CDoubleSpinBoxWidget::showEvent(QShowEvent *event)
{
	__super::showEvent(event);
	if (!m_strLabelText.isEmpty())
		SetName(m_strLabelText);
}