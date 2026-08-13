#include "PropertyResetBtn.h"
#include "SvgImageButton.h"
#include "UICommonSizeDefine.h"
#include "UITranslate.h"

#include <QHBoxLayout>
#include <QVariant>

CPropertyResetBtn::CPropertyResetBtn(QWidget* pParent /*= nullptr*/)
	: QWidget(pParent)
{
	m_pBtn = new CSvgImageButton(":/dark/images/PropertyWnd/ResetConnTestChart", this);
	m_pBtn->setFixedSize(nSizeDefine_16, nSizeDefine_16);
	m_pBtn->setProperty("transparentBkBtn", true);
	m_pBtn->setToolTip(CFxPropertyTranslate::Reset());
	m_pBtn->setFocusPolicy(Qt::NoFocus);

	QHBoxLayout* pLayout = new QHBoxLayout(this);
	pLayout->setMargin(0);
	pLayout->setSpacing(0);
	pLayout->addWidget(m_pBtn);

	connect(m_pBtn, &CSvgImageButton::clicked, this, &CPropertyResetBtn::signalResetClicked);
}

CPropertyResetBtn::~CPropertyResetBtn()
{
}

void CPropertyResetBtn::SetEnabled(bool bEnabled)
{
	if (m_pBtn)
		m_pBtn->setEnabled(bEnabled);
}

void CPropertyResetBtn::SetHoverOnlyVisible(bool bHoverOnly)
{
	if (!m_pBtn)
		return;
	if (bHoverOnly)
	{
		m_pBtn->SetHideToolTipWhenDisabled(true);
		m_pBtn->SetImagePath(QStringLiteral(":/dark/images/PropertyWnd/ResetHoverOnly"));
	}
	else
	{
		m_pBtn->SetHideToolTipWhenDisabled(false);
		m_pBtn->SetImagePath(QStringLiteral(":/dark/images/PropertyWnd/ResetConnTestChart"));
	}
	m_pBtn->update();
}
