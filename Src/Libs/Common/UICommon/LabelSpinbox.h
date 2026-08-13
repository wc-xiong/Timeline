#pragma once

#include "UICommonGlobal.h"
#include <QWidget>

QT_BEGIN_NAMESPACE
class CSpinBox;
class QLabel;
class QHBoxLayout;
QT_END_NAMESPACE

class UICOMMON_EXPORT CLabelSpinbox  : public QWidget
{
	Q_OBJECT

public:
	CLabelSpinbox(QWidget *parent = nullptr, const QString& labelText = QString(), int spinboxVuale = 0);
	virtual ~CLabelSpinbox();

	//设置QLabel设置，宽度，文字对齐方式、QLabel与TimeEdit中间间距
	void SetLabelConfig(int nWidth, Qt::Alignment alignment = Qt::AlignLeft | Qt::AlignVCenter, int nInterval = 0);

	// 仅更新标签宽度并刷新省略显示
	void SetLabelWidth(int nWidth);

	void SetSpinBoxWidth(int nWidth);
	void EnableSpinBoxStretch(int nMinWidth = 0);
	void SetSpinBoxAlignment(Qt::Alignment aalignment);
	void SetConfig(int nLabelWidth, int nSpinBoxWidth, int nInterval = 0);
	void SetBottomLine(bool bFlag = true);
	void SetWidgetContentMargin(int nLeft, int nTop, int nRight, int nBottom);

	void	SetText(const QString& text, bool bAdaptive = false);
	QString	GetText();
	
	int		GetValue();
	QString GetSuffix();

	void	SetValue(int value);
	void	SetMinimum(int min);
	void	SetMaximum(int max);
	void	SetMinMaximum(int nMin, int nMax);
	void	SetPrefix(const QString& prefix);
	void	SetSuffix(const QString& suffix);
	void	SetSingleStep(int val);

	void	SetReadOnly(bool bReadOnly);

	//设置spinbox里面上下箭头不可见，这种box可当lineEdit使用
	void	SetBtnsVisible(bool b);

	void	SetLabelProperty(const char* strProperty, bool bValue);
	void	SetSpinBoxProperty(const char* strProperty, bool bValue);
	void	SetWidgetProperty(const char* strProperty, bool bValue);

	void	SetSpinBoxShowSymbol(bool bShow);
	void	SetSpinBoxDisplaySymbol(const QString& symbol);
	bool	IsSpinBoxShowSymbol() const;
	QString	GetSpinBoxDisplaySymbol() const;

	void SetSignalFouceOut(bool bUse);
	bool eventFilter(QObject *watched, QEvent *event);

	QWidget* GetSpinBox();
protected:
	void showEvent(QShowEvent *event);

signals:
	void	valueChanged(int i);

	void	signalFouceOut();
	void	signalEditingFinished();

private:
	void    InitWidget();           // 初始化，创建控件
	void    CreateConnects();       // 连接信号槽

private:
	QLabel* m_pLabel = nullptr;
	CSpinBox* m_pSpinBox = nullptr;
	QHBoxLayout* m_pMainLayout = nullptr;
	QString m_strLabelText;
};
