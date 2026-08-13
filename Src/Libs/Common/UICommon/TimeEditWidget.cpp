#include "TimeEditWidget.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QDebug>
#include <QTimeEdit>
#include <QLineEdit>
#include <QDateTime>
#include <QFocusEvent>
#include <QKeyEvent>
#include <cmath>
#include "../Common/CommonFunctionApi.h"
#include "UICommonSizeDefine.h"
#include "Common/SignalCenter/SignalCenterExport.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

QFrameTimeEdit::QFrameTimeEdit(QWidget* parent, int fps)
    : QTimeEdit(parent), m_fps(fps), m_bFrame(false)
{
    setTime(QTime(0, 0, 0, 0));
    setTimeRange(QTime(0, 0, 0, 0), QTime(23, 59, 59, 999));

    m_defaultDate = dateTime().date();
}

QFrameTimeEdit::QFrameTimeEdit(const QTime& time, QWidget* parent, int fps)
    : QTimeEdit(time, parent), m_fps(fps), m_bFrame(false)
{
    setTimeRange(QTime(0, 0, 0, 0), QTime(23, 59, 59, 999));
}

int QFrameTimeEdit::fps() const
{
    return m_fps;
}

void QFrameTimeEdit::setFps(int fps)
{
    m_fps = fps;
    update(); // 刷新显示
}

void QFrameTimeEdit::setFrame(bool b)
{
    m_bFrame = b;
    if (m_bFrame)
        setDisplayFormat(QStringLiteral("hh:mm:ss"));
    else
        setDisplayFormat(QStringLiteral("hh:mm:ss:zzz"));
    update(); // 刷新显示
}

void QFrameTimeEdit::setEditorText(const QString& text)
{
	blockSignals(true);
	if (m_bFrame)
	{
		// 解析后钳制到合法范围，显示与 QTime 必须一致（避免超 max 仍显示大值）
		const QDateTime dt = dateTimeFromText(text);
		if (dt.isValid())
		{
			const QTime t = qBound(minimumTime(), dt.time(), maximumTime());
			setTime(t);
			lineEdit()->setText(textFromDateTime(QDateTime(m_defaultDate, t)));
		}
		else
		{
			lineEdit()->setText(text);
		}
	}
	else
	{
		lineEdit()->setText(text);
	}
	blockSignals(false);
}

QString QFrameTimeEdit::editorText() const
{
	return lineEdit()->text();
}

QString QFrameTimeEdit::textFromDateTime(const QDateTime& datetime) const
{
    if (m_bFrame)
    {
        QTime time = datetime.time();

        const int total_ms = (time.hour() * 3600 + time.minute() * 60 + time.second()) * 1000 + time.msec();
        const int hours = total_ms / (3600 * 1000);
        const int minutes = (total_ms / (60 * 1000)) % 60;
        const int seconds = (total_ms / 1000) % 60;
        const int mss = total_ms % 1000;
        const int frames = static_cast<int>((static_cast<double>(mss) / 1000.0) * m_fps);

        return QString("%1:%2:%3:%4")
            .arg(hours, 2, 10, QLatin1Char('0'))
            .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(seconds, 2, 10, QLatin1Char('0'))
            .arg(frames, 2, 10, QLatin1Char('0'));
    }
    else
    {
        return __super::textFromDateTime(datetime);
    }
}

QDateTime QFrameTimeEdit::dateTimeFromText(const QString& text) const
{
    if (m_bFrame)
    {
        QStringList parts = text.split(':');
        bool ok = false;
        int hours = 0;
        int minutes = 0;
        int seconds = 0;
        int frames = 0;

        if (parts.size() == 4)
        {
            hours = parts[0].toInt(&ok);
            if (!ok) return QDateTime(m_defaultDate, QTime(0, 0, 0, 0));
            minutes = parts[1].toInt(&ok);
            if (!ok) return QDateTime(m_defaultDate, QTime(0, 0, 0, 0));
            seconds = parts[2].toInt(&ok);
            if (!ok) return QDateTime(m_defaultDate, QTime(0, 0, 0, 0));
            frames = parts[3].toInt(&ok);
            if (!ok) return QDateTime(m_defaultDate, QTime(0, 0, 0, 0));
        }
        else if (parts.size() == 3)
        {
            // 支持 HH:MM:SS.FF，或仅 HH:MM:SS（帧按 0）
            QStringList secondsParts = parts[2].split('.');
            hours = parts[0].toInt(&ok);
            if (!ok) return QDateTime(m_defaultDate, QTime(0, 0, 0, 0));
            minutes = parts[1].toInt(&ok);
            if (!ok) return QDateTime(m_defaultDate, QTime(0, 0, 0, 0));
            if (secondsParts.size() == 2)
            {
                seconds = secondsParts[0].toInt(&ok);
                if (!ok) return QDateTime(m_defaultDate, QTime(0, 0, 0, 0));
                frames = secondsParts[1].toInt(&ok);
                if (!ok) return QDateTime(m_defaultDate, QTime(0, 0, 0, 0));
            }
            else
            {
                seconds = parts[2].toInt(&ok);
                if (!ok) return QDateTime(m_defaultDate, QTime(0, 0, 0, 0));
                frames = 0;
            }
        }
        else
        {
            return QDateTime(m_defaultDate, QTime(0, 0, 0, 0));
        }

        const int total_frames = (hours * 3600 + minutes * 60 + seconds) * m_fps + frames;
        const int total_ms = static_cast<int>(std::ceil(static_cast<double>(total_frames) * 1000.0 / m_fps));

        const int outHours = total_ms / (3600 * 1000);
        const int outMinutes = (total_ms / (60 * 1000)) % 60;
        const int outSeconds = (total_ms / 1000) % 60;
        const int ms = total_ms % 1000;

        return QDateTime(m_defaultDate, QTime(outHours, outMinutes, outSeconds, ms));
    }
    else
    {
        return __super::dateTimeFromText(text);
    }
}

QValidator::State QFrameTimeEdit::validate(QString& input, int& pos) const
{
    if (m_bFrame)
    {
        Q_UNUSED(pos)

        QStringList parts = input.split(':');
        bool ok = false;
        int hours = 0;
        int minutes = 0;
        int seconds = 0;
        int frames = 0;

        if (parts.size() == 4)
        {
            hours = parts[0].toInt(&ok);
            if (!ok || hours < 0)
                return QValidator::Intermediate;
            minutes = parts[1].toInt(&ok);
            if (!ok || minutes < 0 || minutes > 59)
                return QValidator::Intermediate;
            seconds = parts[2].toInt(&ok);
            if (!ok || seconds < 0 || seconds > 59)
                return QValidator::Intermediate;
            frames = parts[3].toInt(&ok);
            if (!ok || frames < 0 || frames >= m_fps)
                return QValidator::Intermediate;
        }
        else if (parts.size() == 3)
        {
            // 支持 HH:MM:SS.FF，或编辑过程中的 HH:MM:SS
            QStringList secondsParts = parts[2].split('.');
            hours = parts[0].toInt(&ok);
            if (!ok || hours < 0)
                return QValidator::Intermediate;
            minutes = parts[1].toInt(&ok);
            if (!ok || minutes < 0 || minutes > 59)
                return QValidator::Intermediate;
            if (secondsParts.size() == 2)
            {
                seconds = secondsParts[0].toInt(&ok);
                if (!ok || seconds < 0 || seconds > 59)
                    return QValidator::Intermediate;
                frames = secondsParts[1].toInt(&ok);
                if (!ok || frames < 0 || frames >= m_fps)
                    return QValidator::Intermediate;
            }
            else
            {
                seconds = parts[2].toInt(&ok);
                if (!ok || seconds < 0 || seconds > 59)
                    return QValidator::Intermediate;
            }
        }
        else
        {
            return QValidator::Intermediate;
        }

        return QValidator::Acceptable;
    }
    else
    {
        return __super::validate(input, pos);
    }
}

void QFrameTimeEdit::fixup(QString& input) const
{
	// 非帧模式走基类修复
	if (!m_bFrame)
	{
		QTimeEdit::fixup(input);
		return;
	}

	// 帧模式：规范为 HH:MM:SS:FF，并钳制到 min/max
	const QDateTime dt = dateTimeFromText(input);
	if (!dt.isValid())
		return;
	const QTime t = qBound(minimumTime(), dt.time(), maximumTime());
	input = textFromDateTime(QDateTime(m_defaultDate, t));
}

void QFrameTimeEdit::commitFrameText()
{
	if (!m_bFrame || !lineEdit())
		return;

	// 从当前行文本解析并写回 QTime / 显示
	const QDateTime dt = dateTimeFromText(lineEdit()->text());
	if (!dt.isValid())
		return;

	const QTime t = qBound(minimumTime(), dt.time(), maximumTime());
	if (t != time())
		setTime(t);
	lineEdit()->setText(textFromDateTime(QDateTime(m_defaultDate, t)));
}

void QFrameTimeEdit::focusOutEvent(QFocusEvent* event)
{
	// 失焦前先提交帧，再交给基类 interpret
	commitFrameText();
	QTimeEdit::focusOutEvent(event);
}

void QFrameTimeEdit::keyPressEvent(QKeyEvent* event)
{
	// 回车确认前先提交帧，避免 interpret 丢帧
	if (m_bFrame && (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return))
		commitFrameText();
	QTimeEdit::keyPressEvent(event);
}

void QFrameTimeEdit::wheelEvent(QWheelEvent *ev)
{
    if (!m_bFrame)
    {
        __super::wheelEvent(ev);
        return;
    }

    const int steps = ev->delta() / 120;
    if (steps == 0)
        return;

    const QDateTime dt = dateTimeFromText(lineEdit()->text());
    if (!dt.isValid())
        return;

    const QTime time = dt.time();
    const int total_ms = (time.hour() * 3600 + time.minute() * 60 + time.second()) * 1000 + time.msec();
    const int frames = static_cast<int>((static_cast<double>(total_ms % 1000) / 1000.0) * m_fps);
    const int total_frames = (time.hour() * 3600 + time.minute() * 60 + time.second()) * m_fps + frames;
    const int new_total_frames = qMax(0, total_frames + steps);
    const int new_total_ms = static_cast<int>(std::ceil(static_cast<double>(new_total_frames) * 1000.0 / m_fps));

    const QTime newTime(
        new_total_ms / (3600 * 1000),
        (new_total_ms / (60 * 1000)) % 60,
        (new_total_ms / 1000) % 60,
        new_total_ms % 1000);

    // 不屏蔽信号，保证外层 m_tCurTime 随滚轮改帧同步
    const QTime clampedTime = qBound(minimumTime(), newTime, maximumTime());
    setTime(clampedTime);
    lineEdit()->setText(textFromDateTime(QDateTime(m_defaultDate, clampedTime)));
    ev->accept();
}


CTimeEditWidget::CTimeEditWidget(QWidget* pParent):
	QWidget(pParent)
{
	InitWidget();

	CreateConnections();

	setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
}

CTimeEditWidget::~CTimeEditWidget()
{
}

void CTimeEditWidget::SetName(const QString &strName)
{
	int nWidth = m_pNameLabel->width();
	m_pNameLabel->setToolTip(strName);
	m_pNameLabel->setText(m_pNameLabel->fontMetrics().elidedText(strName, Qt::ElideRight, nWidth));
	m_strLabelText = strName;
}

void CTimeEditWidget::SetLabelConfig(int nWidth, Qt::Alignment alignment, int nInterval)
{
	m_pNameLabel->setFixedWidth(nWidth);
	m_pNameLabel->setAlignment(alignment);
	m_pHorLayout->insertSpacing(1, nInterval);
}

void CTimeEditWidget::SetEditConfig(Qt::Alignment alignment)
{
	m_pTimeEdit->setAlignment(alignment);
}

void CTimeEditWidget::SetLabelProperty(const char* strProperty, bool bValue)
{
	m_pNameLabel->setProperty(strProperty, bValue);
}

void CTimeEditWidget::SetTextConfig(int nInterval, Qt::Alignment alignment /*= Qt::AlignLeft | Qt::AlignVCenter*/)
{
	m_pNameLabel->setAlignment(alignment);
	m_pHorLayout->insertSpacing(1, nInterval);
}

void CTimeEditWidget::SetTimeEditWidth(int nWidth)
{
	m_pTimeEdit->setFixedWidth(nWidth);
}

void CTimeEditWidget::SetShowMs(bool bShow)
{
	if(bShow)
		m_pTimeEdit->setDisplayFormat("hh:mm:ss:zzz");
	else
		m_pTimeEdit->setDisplayFormat("hh:mm:ss");
}

void CTimeEditWidget::SetReadOnly(bool bReadOnly)
{
	m_pTimeEdit->setReadOnly(bReadOnly);

	m_pTimeEdit->setFocusPolicy(bReadOnly ? Qt::NoFocus : Qt::ClickFocus);
}

void CTimeEditWidget::SetMinMaxTime(const QTime &tMin, const QTime &tMax)
{
	blockSignals(true);
	m_pTimeEdit->setMinimumTime(tMin);
	m_pTimeEdit->setMaximumTime(tMax);
	// 范围变化后同步缓存，避免 GetMillisecond 仍返回超范围旧值
	m_tCurTime = m_pTimeEdit->time();
	m_tCommittedTime = m_tCurTime;
	blockSignals(false);
}

void CTimeEditWidget::SetMinMaxTime(int nMin, int nMax)
{
	blockSignals(true);
	m_pTimeEdit->setMinimumTime(IntConvertToTime(nMin));
	m_pTimeEdit->setMaximumTime(IntConvertToTime(nMax));
	// 范围变化后同步缓存，避免 GetMillisecond 仍返回超范围旧值
	m_tCurTime = m_pTimeEdit->time();
	m_tCommittedTime = m_tCurTime;
	blockSignals(false);
}

void CTimeEditWidget::SetMinTime(int nMin)
{
	blockSignals(true);
	m_pTimeEdit->setMinimumTime(IntConvertToTime(nMin));
	m_tCurTime = m_pTimeEdit->time();
	m_tCommittedTime = m_tCurTime;
	blockSignals(false);
}

void CTimeEditWidget::SetTime(const QTime &curTime)
{
	blockSignals(true);
	m_tCurTime = curTime;
	m_tCommittedTime = curTime;
	m_pTimeEdit->setTime(curTime);
	blockSignals(false);
}

void CTimeEditWidget::SetTime(int nMS)
{
	blockSignals(true);
	m_tCurTime = IntConvertToTime(nMS);
	m_tCommittedTime = m_tCurTime;
	m_pTimeEdit->setTime(m_tCurTime);
	blockSignals(false);
}

int CTimeEditWidget::GetMillisecond()
{
	return TimeConvertToIntMs(m_tCurTime);
}

QString CTimeEditWidget::editorText() const
{
	return m_pTimeEdit ? m_pTimeEdit->editorText() : QString();
}

void CTimeEditWidget::setEditorText(const QString& text)
{
	if (!m_pTimeEdit)
		return;
	m_pTimeEdit->setEditorText(text);
	m_tCurTime = m_pTimeEdit->time();
	m_tCommittedTime = m_tCurTime;
}

bool CTimeEditWidget::IsReadOnly()
{
	return m_pTimeEdit->isReadOnly();
}

void CTimeEditWidget::SetDisplayFormat(const QString &strFormat)
{
	m_pTimeEdit->setDisplayFormat(strFormat);
}

void CTimeEditWidget::SetFrameEdit(bool b)
{
    m_pTimeEdit->setFrame(b);
}

void CTimeEditWidget::SetFps(float fFps)
{
	m_pTimeEdit->setFps(static_cast<int>(std::ceil(fFps)));
}

QTime CTimeEditWidget::GetTime()
{
	return m_pTimeEdit->time();
}

void CTimeEditWidget::slotTimeChange(const QTime &time)
{
	if (m_tCurTime == time)
		return;

	m_tCurTime = time;
	emit signalTimeChange(time);
}

void CTimeEditWidget::slotTimeEditFinish()
{
	// 编辑结束：再提交一次行文本，同步缓存后再通知外部
	if (m_pTimeEdit)
	{
		m_pTimeEdit->setEditorText(m_pTimeEdit->editorText());
		m_tCurTime = m_pTimeEdit->time();
		m_tCommittedTime = m_tCurTime;
	}
	emit signalTimeEditFinish();
}

void CTimeEditWidget::slotTimeLineFpsChange(float fFps)
{
	SetFps(fFps);
	m_pTimeEdit->setTime(m_tCurTime);
}

void CTimeEditWidget::InitWidget()
{
	setFixedHeight(UICommon_CtrlHeight);

	m_pNameLabel = new QLabel(this);
	m_pNameLabel->setFixedHeight(UICommon_CtrlHeight);
	m_pNameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

	m_pTimeEdit = new QFrameTimeEdit(this);
	m_pTimeEdit->setFixedHeight(UICommon_CtrlHeight);
	m_pTimeEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_pTimeEdit->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
	m_pTimeEdit->setFrame(true);
	SetShowMs(true);
	SetMinMaxTime(QTime(0, 0, 0, 0), QTime(23, 59, 59, 999));
	
	m_pHorLayout = new QHBoxLayout;
	m_pHorLayout->setMargin(0);
	m_pHorLayout->setSpacing(0);

	m_pHorLayout->addWidget(m_pNameLabel);
	m_pHorLayout->addWidget(m_pTimeEdit);

	setLayout(m_pHorLayout);

	m_tCurTime = m_pTimeEdit->time();
	m_tCommittedTime = m_tCurTime;
}

void CTimeEditWidget::CreateConnections()
{
	connect(m_pTimeEdit, SIGNAL(timeChanged(const QTime &)), this, SLOT(slotTimeChange(const QTime &)));
	connect(m_pTimeEdit, &QTimeEdit::editingFinished, this, &CTimeEditWidget::slotTimeEditFinish);
	connect(g_pSignalCenter, &CSignalCenter::signalTimeLineFpsChange, this, &CTimeEditWidget::slotTimeLineFpsChange);
}
