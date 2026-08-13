#pragma once
#include "UICommonGlobal.h"
#include <QSlider>

class UICOMMON_EXPORT CSliderEx : public QSlider
{
	Q_OBJECT

public:
	CSliderEx(QWidget* pParent = 0, bool bHasDefault = true);
	~CSliderEx();

public:
	void setDecimals(int nDecimals);

	void SetRange(double fMin, double fMax, double fDefaultValue);

	void SetValue(double fValue);

	double GetValue();

	void SetDefaultValuePos(double fDefaultValue);

private slots:
	void slotSliderValueChange(int nValue);

protected:
	virtual void mousePressEvent(QMouseEvent* event) override;

	virtual void paintEvent(QPaintEvent *ev) override;

	//滑动条暂不支持通过滚轮改变数值
	virtual void wheelEvent(QWheelEvent *event) override;

	virtual void resizeEvent(QResizeEvent *event) override;

	virtual void mouseMoveEvent(QMouseEvent *event) override;

	virtual void mouseReleaseEvent(QMouseEvent *event) override;

	bool eventFilter(QObject *watched, QEvent *event) override;

signals:
	//滑动条数值发生变化
	void signalvalueChanged(double fValue);

	//滑动条最值发生变化
	void signalRangeChanged(double fMin, double fMax);

	// 鼠标左键松开时数值变化
	void signalValueChangedMouseRelease(double fValue);
	void signalValueChangeStart(double fvalue);

	void signalDefaultValueChanged(double fValue);
protected:
	void InitWidget();

	void CreateConnections();

private:
	int m_nDecimals;	//滑动条数据类型

	double m_fDefaultValue;

	double m_fConverValue = 1.0;

	// 默认值的位置
	int m_nDefaultValuePos = 0;

	// 默认值上下色块位置
	QRect m_rcDefaultValueTop;
	QRect m_rcDefaultValueBottom;

	bool m_bHasDefault;		// 绘制默认值标志
};
