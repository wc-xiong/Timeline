#pragma once
#include "UICommonGlobal.h"
#include <QWidget>

//滑动条窗口，包括QLabel，QSpinBox，QSlider
//滑动条支持小数、整数、及带百分号（带百分号时不需要将数值上下限乘100）
//可选择水平布局，垂直布局（默认垂直，在创建时设置）

QT_BEGIN_NAMESPACE
class CSliderEx;
class QLabel;
class QHBoxLayout;
class CDoubleSpinBox;
QT_END_NAMESPACE

class UICOMMON_EXPORT CSliderWidget : public QWidget
{
	Q_OBJECT

public:
	CSliderWidget(QWidget* pParent = 0, bool bVerLayout = true);
	~CSliderWidget();

public:
	//设置滑动条名称
	void SetName(const QString &strName);

	//设置滑动条属性
	void SetSliderConfig(int nDecimals, double fMin, double fMax, double fDefaultValue, const QString &strSuffix = "");

	void SetSingleStep(double fStep);

	void SetLabelConfig(int nWidth, Qt::Alignment alignment = Qt::AlignLeft | Qt::AlignVCenter, int nInterval = 0);

	void SetSpinBoxWidth(int width);

	void SetRoundOff(bool bValue) { m_bRoundOff = bValue; }

	//设置滑动条值
	void SetValue(double fValue);

	//获取滑动条当前值（编辑框显示值）
	double GetValue();

	//获取滑动条的最大值
	double GetMaximum();
	
	//获取滑动条的最小值
	double GetMinimum();

	// 设置spinBox 按钮显示与否
	void SetBtnsVisible(bool b);

	// 设置Readonly
	void SetReadOnly(bool bReadOnly);

	// 设置滑动条默认竖线位置
	void SetSliderDefaultPos(double fDefaultValue);

	void SetLabelProperty(const char* strProperty, bool bValue);

	void SetSliderProperty(const char* strProperty, bool bValue);

	void SetSpinBoxProperty(const char* strProperty, bool bValue);

	void SetWidgetProperty(const char* strProperty, QVariant flag);

	void SetBottomLine(bool bFlag = true);

	void SetContentsMargins(int left, int top, int right, int bottom);

	QHBoxLayout* GetNameLayout() const { return m_pNameLayout; }

	void SetSliderEnabled(bool b);

protected:
	void showEvent(QShowEvent *event) override;

private slots:
	//处理通过滑动条改变数值
	void slotSliderValueChange(double fValue);

	//处理通过编辑框改变数值
	void slotSpinBoxValueChange(double fValue);

signals:
	//滑动条数值改变
	void signalValueChange(double fValue);

	void signalValueChangeBySliderOrEditor(double fValue, bool bSlider);

	// 鼠标左键松开时数值变化
	void signalValueChangedMouseRelease(double fValue);
	// 鼠标左键按下开始之前
	void signalValueChangeStart(double fvalue);


protected:
	void InitWidget();

	void CreateConnections();

	bool IsEqual(double fValue1, double fValue2);

private:
	QHBoxLayout* m_pNameLayout = nullptr;

	CSliderEx* m_pSlider = nullptr;
	QLabel* m_pNameLabel = nullptr;
	CDoubleSpinBox* m_pSpinBox = nullptr;

	QHBoxLayout* m_pHorLayout = nullptr;

private:
	bool m_bPrecent;			//是否后缀显示百分号
	bool m_bVerLayout;			//滑动条和窗口是否垂直布局
	bool m_bRoundOff = false;	//滑动条修改数值时，自动四舍五入
	int m_nSliderType;			//滑动条数值类型（小数点位数）
	double m_fMinValue;			//滑动条最小值（编辑框显示值，可能非真实值）
	double m_fMaxValue;			//滑动条最大值（编辑框显示值，可能非真实值）
	double m_fExps;				//根据滑动条小数点位数判断最小偏差值
	QString m_strLabelText;		// 标签完整文本
};
