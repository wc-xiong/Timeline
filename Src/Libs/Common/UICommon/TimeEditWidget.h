#pragma once
#include "UICommonGlobal.h"
#include <QWidget>
#include <QIcon>
#include <QTime>
#include <QString>

#include <QTimeEdit>
#include <QValidator>
#include <QWheelEvent>

//时间编辑框窗口，包括QLabel，TimeEdit

QT_BEGIN_NAMESPACE
class QLabel;
class QHBoxLayout;
class QFocusEvent;
class QKeyEvent;
QT_END_NAMESPACE

class UICOMMON_EXPORT QFrameTimeEdit : public QTimeEdit
{
	Q_OBJECT
		Q_PROPERTY(int fps READ fps WRITE setFps)

public:
	explicit QFrameTimeEdit(QWidget* parent = 0, int fps = 60);
	explicit QFrameTimeEdit(const QTime& time, QWidget* parent = 0, int fps = 60);

	int fps() const;
	void setFps(int fps);

	void setFrame(bool b);
	void setEditorText(const QString& text);
	QString editorText() const;

protected:
	// 重写关键方法
	QString textFromDateTime(const QDateTime& datetime) const override;
	QDateTime dateTimeFromText(const QString& text) const override;
	QValidator::State validate(QString& input, int& pos) const override;
	void fixup(QString& input) const override;
	void focusOutEvent(QFocusEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	virtual void wheelEvent(QWheelEvent *ev) override;

private:
	// 将行编辑中的 HH:MM:SS:FF 提交到 QTime
	void commitFrameText();

private:
	bool m_bFrame;

	int m_fps;
	QDate m_defaultDate;
};

class UICOMMON_EXPORT CTimeEditWidget : public QWidget
{
	Q_OBJECT

public:
	CTimeEditWidget(QWidget* pParent = 0);
	~CTimeEditWidget();

public:

	//设置ComboBox名称
	void SetName(const QString &strName);
	
	void SetLabelProperty(const char* strProperty, bool bValue);

	//设置QLabel设置，宽度，文字对齐方式、QLabel与TimeEdit中间间距
	void SetLabelConfig(int nWidth, Qt::Alignment alignment = Qt::AlignLeft | Qt::AlignVCenter, int nInterval = 0);

	void SetEditConfig(Qt::Alignment alignment);

	void SetTextConfig(int nInterval, Qt::Alignment alignment = Qt::AlignLeft | Qt::AlignVCenter);

	void SetTimeEditWidth(int nWidth);

	void SetShowMs(bool bShow);

	void SetReadOnly(bool bReadOnly);

	void SetMinMaxTime(const QTime &tMin, const QTime &tMax);
	
	void SetMinMaxTime(int nMin, int nMax);

	void SetMinTime(int nMin);

	void SetTime(const QTime &curTime);

	QTime GetTime();

	//毫秒为单位
	void SetTime(int nMs);

	int GetMillisecond();

	QString editorText() const;
	void setEditorText(const QString& text);

	bool IsReadOnly();

	void SetDisplayFormat(const QString &strFormat);

	void SetFrameEdit(bool b);

	void SetFps(float fFps);

signals:
	void signalTimeChange(const QTime &time);
	void signalTimeEditFinish();

protected slots:
	void slotTimeChange(const QTime &time);
	void slotTimeEditFinish(); // 编辑结束：先提交帧文本再对外通知
	void slotTimeLineFpsChange(float fFps);

protected:
	void InitWidget();

	void CreateConnections();

private:
	QTime m_tCurTime;
	QTime m_tCommittedTime;

private:
	QLabel* m_pNameLabel = nullptr;
	QFrameTimeEdit* m_pTimeEdit = nullptr;
	QHBoxLayout* m_pHorLayout = nullptr;
	QString m_strLabelText;
};
