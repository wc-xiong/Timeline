#pragma once
#include "UICommonGlobal.h"
#include <QWidget>

QT_BEGIN_NAMESPACE
class QLabel;
class QHBoxLayout;
class QDoubleSpinBox;
class CHDoubleSpinBox;
QT_END_NAMESPACE

class UICOMMON_EXPORT CDoubleSpinBoxWidget : public QWidget
{
	Q_OBJECT

public:
	CDoubleSpinBoxWidget(QWidget* pParent = 0, bool bHorizontal = false);
	~CDoubleSpinBoxWidget();

public:
	//设置滑动条名称
	void SetName(const QString &strName);
	QString GetName();

	//设置QLabel设置，宽度，文字对齐方式、QLabel与TimeEdit中间间距
	void SetLabelConfig(int nWidth, Qt::Alignment alignment = Qt::AlignLeft | Qt::AlignVCenter, int nInterval = 0);

	void SetTextConfig(int nInterval, Qt::Alignment alignment = Qt::AlignLeft | Qt::AlignVCenter);

	void SetSuffix(QString strSuffix);

	void SetDecimals(int nCount);

	void SetSingleStep(int nCount);

	void SetSpinBoxWidth(int nWidth);

	void SetValue(double fValue);

	double GetValue();

	void SetReadOnly(bool bReadOnly);

	void SetMaxandMinValue(double max = DBL_MAX, double min = 0);

	void SetBtnsVisible(bool b);

	void SetLabelProperty(const char* strProperty, bool bValue);

	void SetSpinBoxProperty(const char* strProperty, bool bValue); 

	void SetSpinBoxAlignment(Qt::Alignment aalignment);

	void SetSpecialValueText(const QString& str);

	void ShowHoverValue(bool bShow) { m_bHoverShowValue = bShow; }

	// 替换内部 SpinBox（接管所有权，旧 SpinBox 会被移除并 deleteLater）
	void ReplaceSpinBox(QDoubleSpinBox* pNewSpinBox);
	QDoubleSpinBox* GetSpinBox() const { return m_pSpinBox; }

protected:
	void showEvent(QShowEvent *event) override;

signals:
	//SpinBox数值改变
	void signalValueChange(double fValue);

	void signalEditingFinished();

	// 新增 SpinBox 扩展信号（如新 SpinBox 不支持则不会触发）
	void signalDoubleClickCopied();
	void signalRightClicked(const QPoint& globalPos);
protected:
	void InitWidget(bool bHorizontal);

	void CreateConnections();

private:
	QLabel* m_pNameLabel = nullptr;
	QDoubleSpinBox* m_pSpinBox = nullptr;

	QHBoxLayout* m_pHorLayout = nullptr;

	bool m_bHoverShowValue = false;
	QString m_strLabelText; // 标签完整文本
};
