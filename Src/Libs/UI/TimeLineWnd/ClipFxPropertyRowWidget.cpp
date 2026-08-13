#include "ClipFxPropertyRowWidget.h"
#include "ClipFxDefine.h"
#include "UICommon/PropertyKeyFrameBar.h"
#include "UICommon/UITranslate.h"
#include "UICommon/SvgImageButton.h"
#include "UICommon/UICommonSizeDefine.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QVariant>
#include "TimelineWinMacros.h"

namespace
{
	CSvgImageButton* CreateInterpIconBtn(const QString& strImagePath, QWidget* pParent)
	{
		auto* pBtn = new CSvgImageButton(strImagePath, pParent);
		pBtn->setFixedSize(nSizeDefine_16, nSizeDefine_16);
		pBtn->setProperty("transparentBkBtn", QVariant(true));
		pBtn->setCheckable(true);
		pBtn->setFocusPolicy(Qt::NoFocus);
		return pBtn;
	}
}

CClipFxPropertyRowWidget::CClipFxPropertyRowWidget(const QString& strPropertyName, QWidget* pParent)
	: QWidget(pParent)
{
	setCursor(Qt::ArrowCursor);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	auto* pLayout = new QHBoxLayout(this);
	pLayout->setContentsMargins(0, 0, nSizeDefine_8, 0);
	pLayout->setSpacing(nSizeDefine_4);

	m_pLabel = new QLabel(strPropertyName, this);
	m_pLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	m_pLabel->setMinimumWidth(0);
	pLayout->addWidget(m_pLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);

	// 名称 | stretch | 三插值按钮 | stretch | 关键帧栏 —— 两侧 stretch 等权，按钮在行内水平居中
	pLayout->addStretch(1);

	m_pBtnInterpLinear = CreateInterpIconBtn(QStringLiteral(CLIPFX_RES_INTERP_LINEAR), this);
	m_pBtnInterpBezier = CreateInterpIconBtn(QStringLiteral(CLIPFX_RES_INTERP_BEZIER), this);
	m_pBtnInterpStep = CreateInterpIconBtn(QStringLiteral(CLIPFX_RES_INTERP_STEP), this);
	pLayout->addWidget(m_pBtnInterpLinear, 0, Qt::AlignCenter);
	pLayout->addWidget(m_pBtnInterpStep, 0, Qt::AlignCenter);
	pLayout->addWidget(m_pBtnInterpBezier, 0, Qt::AlignCenter);

	pLayout->addStretch(1);

	m_pKeyFrameBar = new CPropertyKeyFrameBar(this);
	pLayout->addWidget(m_pKeyFrameBar, 0, Qt::AlignRight | Qt::AlignVCenter);

	SetDetailVisible(false);
	SetInterpIconsEnabled(false);

	m_pBtnInterpLinear->setToolTip(CTimeLineWndTranslate::LinearKeyFrameCurve());
	m_pBtnInterpBezier->setToolTip(CTimeLineWndTranslate::AutoBezierKeyFrameCurve());
	m_pBtnInterpStep->setToolTip(CTimeLineWndTranslate::FreeBezierKeyFrameCurve());

	connect(m_pBtnInterpLinear, &CSvgImageButton::clicked, this, [this]() {
		emit signalInterpModeClicked(EPropertyKeyframeCurveMode::Line);
	});
	connect(m_pBtnInterpBezier, &CSvgImageButton::clicked, this, [this]() {
		emit signalInterpModeClicked(EPropertyKeyframeCurveMode::AutoBezier);
	});
	connect(m_pBtnInterpStep, &CSvgImageButton::clicked, this, [this]() {
		emit signalInterpModeClicked(EPropertyKeyframeCurveMode::FreeBezier);
	});
}

void CClipFxPropertyRowWidget::SetDetailVisible(bool bVisible)
{
	if (m_pBtnInterpLinear)
		m_pBtnInterpLinear->setVisible(bVisible);
	if (m_pBtnInterpStep)
		m_pBtnInterpStep->setVisible(bVisible);
	if (m_pBtnInterpBezier)
		m_pBtnInterpBezier->setVisible(bVisible);
	if (m_pKeyFrameBar)
		m_pKeyFrameBar->setVisible(bVisible);
}

void CClipFxPropertyRowWidget::SetInterpIconsEnabled(bool bEnabled)
{
	if (m_pBtnInterpLinear)
		m_pBtnInterpLinear->setEnabled(bEnabled);
	if (m_pBtnInterpBezier)
		m_pBtnInterpBezier->setEnabled(bEnabled);
	if (m_pBtnInterpStep)
		m_pBtnInterpStep->setEnabled(bEnabled);
	if (!bEnabled)
	{
		if (m_pBtnInterpLinear) m_pBtnInterpLinear->setChecked(false);
		if (m_pBtnInterpBezier) m_pBtnInterpBezier->setChecked(false);
		if (m_pBtnInterpStep) m_pBtnInterpStep->setChecked(false);
	}
}

void CClipFxPropertyRowWidget::SetBezierInterpModesEnabled(bool bEnabled)
{
	if (m_pBtnInterpBezier)
		m_pBtnInterpBezier->setEnabled(bEnabled);
	if (m_pBtnInterpStep)
		m_pBtnInterpStep->setEnabled(bEnabled);
	if (!bEnabled)
	{
		if (m_pBtnInterpBezier)
			m_pBtnInterpBezier->setChecked(false);
		if (m_pBtnInterpStep)
			m_pBtnInterpStep->setChecked(false);
	}
}

void CClipFxPropertyRowWidget::SetAutoInterpEnabled(bool bEnabled)
{
	if (m_pBtnInterpBezier)
		m_pBtnInterpBezier->setEnabled(bEnabled);
	if (!bEnabled && m_pBtnInterpBezier)
		m_pBtnInterpBezier->setChecked(false);
}

void CClipFxPropertyRowWidget::SetFreeInterpEnabled(bool bEnabled)
{
	if (m_pBtnInterpStep)
		m_pBtnInterpStep->setEnabled(bEnabled);
	if (!bEnabled && m_pBtnInterpStep)
		m_pBtnInterpStep->setChecked(false);
}

void CClipFxPropertyRowWidget::SetInterpMode(EPropertyKeyframeCurveMode mode)
{
	m_interpMode = mode;
	UpdateInterpButtonChecked(mode);
}

void CClipFxPropertyRowWidget::UpdateInterpButtonChecked(EPropertyKeyframeCurveMode mode)
{
	if (m_pBtnInterpLinear)
		m_pBtnInterpLinear->setChecked(mode == EPropertyKeyframeCurveMode::Line);
	if (m_pBtnInterpBezier)
		m_pBtnInterpBezier->setChecked(mode == EPropertyKeyframeCurveMode::AutoBezier);
	if (m_pBtnInterpStep)
		m_pBtnInterpStep->setChecked(mode == EPropertyKeyframeCurveMode::FreeBezier);
}

void CClipFxPropertyRowWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
		emit signalExpandToggleRequested();
	QWidget::mouseDoubleClickEvent(event);
}
