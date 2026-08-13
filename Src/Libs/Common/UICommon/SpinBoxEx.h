#pragma once
#include "UICommonGlobal.h"
#include <QSpinBox>
#include <QTimer>

class UICOMMON_EXPORT CSpinBox :public QSpinBox
{
	Q_OBJECT
public:
	CSpinBox(QWidget *parent = nullptr);
	~CSpinBox();

	QPoint MoveCalculator(QSize CalculatorSize);

	void timerEvent(QTimerEvent * event);

	void SetValue(int nValue);
	// 控制是否显示符号的开关
	void setShowSymbol(bool show)
	{
		if (m_showSymbol != show)
		{
			m_showSymbol = show;
			update(); // 刷新显示
		}
	}

	// 设置显示的符号（支持任意单个字符或字符串）
	void setDisplaySymbol(const QString &symbol)
	{
		if (symbol != m_displaySymbol)
		{
			m_displaySymbol = symbol;
			if (m_showSymbol) // 只有在符号模式下才需要刷新
				update();
		}
	}

	// 获取当前状态和符号
	bool isShowSymbol() const { return m_showSymbol; }
	QString displaySymbol() const { return m_displaySymbol; }

	// 数值转显示文本：根据模式决定显示符号还是数值
	QString textFromValue(int value) const override
	{
		if (m_showSymbol && !m_displaySymbol.isEmpty())
		{
			return m_displaySymbol; // 符号模式：显示自定义符号
		}
		else
		{
			return QSpinBox::textFromValue(value); // 正常模式：显示数值
		}
	}

protected:
	bool event(QEvent *event) override; 
	bool eventFilter(QObject *watched, QEvent *event) override;

	virtual void keyReleaseEvent(QKeyEvent *event) override;

	bool m_showSymbol;       // 符号显示模式开关
	QString m_displaySymbol; // 自定义显示符号
	int	m_nTimerId = -1;
};
