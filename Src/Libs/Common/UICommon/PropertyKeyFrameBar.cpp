#include "PropertyKeyFrameBar.h"
#include "PropertyResetBtn.h"
#include "GroupWidgetBox.h"
#include "SvgImageButton.h"
#include "UICommonSizeDefine.h"
#include "UITranslate.h"

#include <QHBoxLayout>
#include <QVariant>

namespace
{
	CSvgImageButton* CreateKeyFrameIconBtn(const QString& strPath, QWidget* pParent)
	{
		CSvgImageButton* pBtn = new CSvgImageButton(strPath, pParent);
		pBtn->setFixedSize(nSizeDefine_16, nSizeDefine_16);
		pBtn->setProperty("transparentBkBtn", true);
		pBtn->setFocusPolicy(Qt::NoFocus);
		return pBtn;
	}
}

int CPropertyKeyFrameBar::GetBarWidth()
{
	return nSizeDefine_48;
}

CPropertyKeyFrameBar::CPropertyKeyFrameBar(QWidget* pParent /*= nullptr*/)
	: QWidget(pParent)
{
	setFixedWidth(GetBarWidth());
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	m_pBtnPre = CreateKeyFrameIconBtn(":/dark/images/PropertyWnd/KeyFrame/PreKey", this);
	m_pBtnToggle = CreateKeyFrameIconBtn(":/dark/images/PropertyWnd/KeyFrame/AddOrDelKey", this);
	m_pBtnToggle->setCheckable(true);
	m_pBtnToggle->setChecked(false);
	m_pBtnNext = CreateKeyFrameIconBtn(":/dark/images/PropertyWnd/KeyFrame/NextKey", this);

	m_pBtnPre->setToolTip(CTimeLineWndTranslate::PreKeyFrame());
	m_pBtnNext->setToolTip(CTimeLineWndTranslate::NextKeyFrame());

	QHBoxLayout* pLayout = new QHBoxLayout(this);
	pLayout->setMargin(0);
	pLayout->setSpacing(0);
	pLayout->addStretch();
	pLayout->addWidget(m_pBtnPre, 0, Qt::AlignRight | Qt::AlignVCenter);
	pLayout->addWidget(m_pBtnToggle, 0, Qt::AlignRight | Qt::AlignVCenter);
	pLayout->addWidget(m_pBtnNext, 0, Qt::AlignRight | Qt::AlignVCenter);

	connect(m_pBtnPre, &CSvgImageButton::clicked, this, &CPropertyKeyFrameBar::signalPreKeyFrameClicked);
	connect(m_pBtnToggle, &CSvgImageButton::clicked, this, &CPropertyKeyFrameBar::signalToggleKeyFrameClicked);
	connect(m_pBtnNext, &CSvgImageButton::clicked, this, &CPropertyKeyFrameBar::signalNextKeyFrameClicked);
}

CPropertyKeyFrameBar::~CPropertyKeyFrameBar()
{
}

void CPropertyKeyFrameBar::SetEnabled(bool bEnabled)
{
	if (m_pBtnPre)
		m_pBtnPre->setEnabled(bEnabled);
	if (m_pBtnToggle)
		m_pBtnToggle->setEnabled(bEnabled);
	if (m_pBtnNext)
		m_pBtnNext->setEnabled(bEnabled);
}

void CPropertyKeyFrameBar::SetNavEnabled(bool bPreEnabled, bool bNextEnabled)
{
	if (m_pBtnPre)
		m_pBtnPre->setEnabled(bPreEnabled);
	if (m_pBtnNext)
		m_pBtnNext->setEnabled(bNextEnabled);
}

void CPropertyKeyFrameBar::SetToggleChecked(bool bChecked)
{
	if (m_pBtnToggle)
	{
		m_pBtnToggle->setChecked(bChecked);
		m_pBtnToggle->update();
	}
}

bool CPropertyKeyFrameBar::IsToggleChecked() const
{
	return m_pBtnToggle && m_pBtnToggle->isChecked();
}

void AttachKeyFrameBarToLayout(QHBoxLayout* pLayout, CPropertyKeyFrameBar* pBar)
{
	if (!pLayout || !pBar)
		return;

	pLayout->addStretch();
	pLayout->addWidget(pBar, 0, Qt::AlignRight | Qt::AlignVCenter);
}

void AttachPropertyGroupHeaderWidgets(CGroupWidgetBox* pGroup,
	CPropertyResetBtn** ppResetBtn,
	CPropertyKeyFrameBar** ppKeyFrameBar)
{
	if (!pGroup || !ppResetBtn || !ppKeyFrameBar)
		return;

	*ppResetBtn = new CPropertyResetBtn(pGroup);
	pGroup->InsertHeadWidgetAfterName(*ppResetBtn);

	*ppKeyFrameBar = new CPropertyKeyFrameBar(pGroup);
	pGroup->InsertHeadWidgetAtRight(*ppKeyFrameBar);
}
