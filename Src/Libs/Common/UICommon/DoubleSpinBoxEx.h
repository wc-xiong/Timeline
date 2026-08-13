#pragma once
#include "UICommonGlobal.h"
#include <QDoubleSpinBox>
#include <QPoint>

class QMouseEvent;

//通常的上下键
class UICOMMON_EXPORT CDoubleSpinBox :public QDoubleSpinBox
{
public:
	CDoubleSpinBox(QWidget *parent = nullptr);
	~CDoubleSpinBox();

	QPoint MoveCalculator(QSize CalculatorSize);

	void SetValue(double fValue);
protected:
	bool event(QEvent *event) override;
	bool eventFilter(QObject *watched, QEvent *event) override;
	void timerEvent(QTimerEvent * event);
	virtual void keyReleaseEvent(QKeyEvent *event) override;

	QString textFromValue(double val) const override;

	// 连续点击步进键可能被识别为双击；仅在数值区域双击时返回 true。
	bool ShouldCopyOnDoubleClick(const QMouseEvent* me) const;

protected:
	bool m_bPress = false;
	int	m_nTimerId = -1;
};

//左右键，且normal状态下不显示加减图标
class UICOMMON_EXPORT CHDoubleSpinBox :public CDoubleSpinBox
{
public:
	CHDoubleSpinBox(QWidget *parent = nullptr);
	~CHDoubleSpinBox();

protected:
	bool eventFilter(QObject *watched, QEvent *event) override;
	bool event(QEvent *event) override;
};

// 双击复制数值到系统剪贴板 + 右键点击信号
class UICOMMON_EXPORT CCopyableDoubleSpinBox : public CDoubleSpinBox
{
	Q_OBJECT
public:
	explicit CCopyableDoubleSpinBox(QWidget* parent = nullptr);
	~CCopyableDoubleSpinBox();

signals:
	void signalDoubleClickCopied();
	void signalRightClicked(const QPoint& globalPos);

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	void copyCurrentTextToClipboard();
};

// 横向版本（沿用 CHDoubleSpinBox 的左右键/样式行为）
class UICOMMON_EXPORT CHCopyableDoubleSpinBox : public CHDoubleSpinBox
{
	Q_OBJECT
public:
	explicit CHCopyableDoubleSpinBox(QWidget* parent = nullptr);
	~CHCopyableDoubleSpinBox();

signals:
	void signalDoubleClickCopied();
	void signalRightClicked(const QPoint& globalPos);

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	void copyCurrentTextToClipboard();
};