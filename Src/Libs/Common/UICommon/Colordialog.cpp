#include "colordialog.h"
#include "colorpicker.h"
#include "SpinBoxEx.h"
#include "UITranslate.h"
#include "UICommonSizeDefine.h"

#include <QLabel>
#include <QLayout>
#include <QApplication>

using namespace UICommon;
//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

//#ifdef Q_CC_MSVC
//#pragma execution_character_set("utf-8")
//#endif


CColorDialog::CColorDialog(QWidget *parent): CFramelessDialog(parent)
{
    this->setProperty("isColorEditDialog",true);
    setWindowTitle(CCommonTranslate::ColorPicker());

    setDialogMovable(true); 
    setDialogResizable(false);
    setMinButtonVisible(false);
    setMaxButtonVisible(false);
	setBottomBarVisible(false);

    setFixedSize(SysDpiZoom(230), SysDpiZoom(306));

    initUI();
    initConnect();
}

CColorDialog::~CColorDialog()
{

}

void CColorDialog::InitColor(const QColor &color)
{
	m_RSpinBox->SetValue(color.red());
	m_GSpinBox->SetValue(color.green());
	m_BSpinBox->SetValue(color.blue());
	m_pColorPicker->setCurColor(m_RSpinBox->value(), m_GSpinBox->value(), m_BSpinBox->value());
}

void CColorDialog::initUI()
{
	m_pTitleBarLayout->setMargin(nSizeDefine_1);
	m_pTitleBar->setProperty("ColorDialogTitle", true);
	m_pTitleBar->setFixedSize(SysDpiZoom(228), nSizeDefine_40); 
	m_pTitleBar->SetTitleFixedWidth(SysDpiZoom(100));
	m_pTitleBar->SetTitleNameProperty("ColorDialogTitleBarName",true);

    m_pColorPicker = new CColorPicker(this);
	m_pColorPicker->setFixedSize(SysDpiZoom(210), SysDpiZoom(198));

    m_pColorHueSlider =new CColorHueSlider(this);
	m_pColorHueSlider->setFixedSize(SysDpiZoom(202), SysDpiZoom(12));

	m_pRGBLabel = new QLabel(this);
	m_pRGBLabel->setFixedSize(nSizeDefine_24, nSizeDefine_16);
	m_pRGBLabel->setText("RGB");

	m_RSpinBox = new CSpinBox(this);
	m_RSpinBox->setProperty("ColorDialogSpinBox", true);
	m_RSpinBox->setFixedSize(SysDpiZoom(52), nSizeDefine_24);
	m_RSpinBox->setMinimum(0);
	m_RSpinBox->setMaximum(255);

	m_GSpinBox = new CSpinBox(this);
	m_GSpinBox->setProperty("ColorDialogSpinBox", true);
	m_GSpinBox->setFixedSize(SysDpiZoom(52), nSizeDefine_24);
	m_GSpinBox->setMinimum(0);
	m_GSpinBox->setMaximum(255);

	m_BSpinBox = new CSpinBox(this);
	m_BSpinBox->setProperty("ColorDialogSpinBox", true);
	m_BSpinBox->setFixedSize(SysDpiZoom(52), nSizeDefine_24);
	m_BSpinBox->setMinimum(0);
	m_BSpinBox->setMaximum(255);

	QHBoxLayout* pEditLayout =new QHBoxLayout();
    pEditLayout->setSpacing(SysDpiZoom(8));
	pEditLayout->setMargin(0);
	pEditLayout->addStretch();
	pEditLayout->addWidget(m_pRGBLabel);
	pEditLayout->addWidget(m_RSpinBox);
	pEditLayout->addWidget(m_GSpinBox);
	pEditLayout->addWidget(m_BSpinBox);
	pEditLayout->addStretch();

	QVBoxLayout *pMainLayout = new QVBoxLayout;
	pMainLayout->setContentsMargins(0, 0, 0, nSizeDefine_12);
	pMainLayout->setSpacing(0);
	pMainLayout->addWidget(m_pColorPicker, 0, Qt::AlignHCenter);
	pMainLayout->addSpacing(nSizeDefine_10);
	pMainLayout->addWidget(m_pColorHueSlider, 0, Qt::AlignHCenter);
	pMainLayout->addSpacing(nSizeDefine_12);
	pMainLayout->addLayout(pEditLayout);
	pMainLayout->addStretch();
    
    setLayout(pMainLayout);
}

void CColorDialog::initConnect()
{
    connect(m_pColorHueSlider, &CColorHueSlider::newColor, m_pColorPicker, &CColorPicker::setColor);
	connect(m_pColorPicker, &CColorPicker::newHueColor, m_pColorHueSlider, &CColorHueSlider::setColor);
    connect(m_pColorPicker, &CColorPicker::newColor,this, [&](int r, int g, int b)
    {		
		m_RSpinBox->SetValue(r);
		m_GSpinBox->SetValue(g);
		m_BSpinBox->SetValue(b);

		emit signalNewColor(QColor(r, g, b));
    });

	connect(m_RSpinBox, SIGNAL(valueChanged(int)), this, SLOT(slotColorSpinBoxchange(int)));
	connect(m_GSpinBox, SIGNAL(valueChanged(int)), this, SLOT(slotColorSpinBoxchange(int)));
	connect(m_BSpinBox, SIGNAL(valueChanged(int)), this, SLOT(slotColorSpinBoxchange(int)));
}


void CColorDialog::slotColorSpinBoxchange(int i)
{
	m_pColorPicker->setCurColor(m_RSpinBox->value(), m_GSpinBox->value(), m_BSpinBox->value());
	emit signalNewColor(QColor(m_RSpinBox->value(), m_GSpinBox->value(), m_BSpinBox->value()));
}