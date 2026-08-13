#pragma once
#include "UICommon/SvgImageButton.h"
#include "UICommon/SliderEx.h"
#include "UICommon/TimeEditWidget.h"
#include "ContentViewData.h"
#include <QLabel>
#include <QLineEdit>

class CToolWidget :public QWidget
{
	Q_OBJECT
public:
	CToolWidget(QWidget* pParent = 0);
	~CToolWidget();

	void	UpdateTranslateText();
	void	UpdateTime();			//更新时间显示
	void	UpdateView();			//刷新界面
	void	UpdateKeyFrameNavButtons();	//刷新上一帧/下一帧按钮状态（媒体属性 KF / 路径相机 Fx KF）
	void	NavigateSelectedObjectKeyframe(bool bPre);	//跳转到当前选中片段上一/下一关键帧

protected:
	void    InitWidget();
	void    CreateConnects();
	void	BeginTimeEdit();
	void	FinishTimeEdit();
	bool	eventFilter(QObject* watched, QEvent* event) override;

signals:
	void	signalUpdateView(uint32_t);

public slots:
	void	slotDelete();			//删除图层槽函数 

protected:
	CSvgImageButton*	m_pBtnClipPos = nullptr;
	QLabel*				m_pLabelClipPos = nullptr;
	CSvgImageButton*	m_pBtnLocateNonius = nullptr;
	QLabel*				m_pLabelTime = nullptr;			//时间线播放时间Label
	QFrameTimeEdit*		m_pTimeEdit = nullptr;
	QLineEdit*			m_pFrameNumberEdit = nullptr; // 帧数模式双击编辑
	bool				m_bEditingAsTimeCode = true; // Begin 时锁定本次编辑模式
	QLabel*				m_pLabelFps = nullptr;

	CSvgImageButton*	m_pBtnTimeChange = nullptr;		//废弃控件
	CSvgImageButton*	m_pBtnDelete = nullptr;			//删除图层按键
	CSvgImageButton*	m_pBtnPreKeyFrame = nullptr;
	CSvgImageButton*	m_pBtnNextKeyFrame = nullptr;
	CSvgImageButton*	m_pBtnZoomBig = nullptr;		//时间刻度精度放大按键
	CSvgImageButton*	m_pBtnZoomSmall = nullptr;		//时间刻度精度缩小按键

	CSliderEx*			m_pSliderZoom = nullptr;		//时间刻度精度调整滑动条
};