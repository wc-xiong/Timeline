#include "StyleHelper.h"

#include <QApplication>
#include <QFile>
#include <QGuiApplication>
#include <QRegularExpression>
#include <QScreen>

static float s_fMainScreenDpi = 1.0f;

void InitMainScreenDpi()
{
	QScreen* screen = QGuiApplication::primaryScreen();
	const int baseValue = 96;
	qreal dpiVal = screen->logicalDotsPerInch();
	qreal ratioVal = screen->devicePixelRatio();
	s_fMainScreenDpi = static_cast<float>(dpiVal * ratioVal / baseValue);
}

float GetSystemDpi()
{
	return s_fMainScreenDpi;
}

int SysDpiZoom(int nSize)
{
	return static_cast<int>(nSize * s_fMainScreenDpi + 0.5f);
}

QString ProcessDpiStyleSheet(const QString& rawQss)
{
	static const QRegularExpression dpiVarRe(QStringLiteral("(-?)@(\\d+)\\$"));
	QString result;
	int pos = 0;
	QRegularExpressionMatchIterator it = dpiVarRe.globalMatch(rawQss);
	while (it.hasNext())
	{
		const QRegularExpressionMatch match = it.next();
		result += rawQss.mid(pos, match.capturedStart(0) - pos);
		const QString sign = match.captured(1);
		const int nSize = match.captured(2).toInt();
		result += sign + QString::number(SysDpiZoom(nSize));
		pos = match.capturedEnd(0);
	}
	result += rawQss.mid(pos);
	return result;
}

void InitSystemDpi(const QString& qssPath)
{
	InitMainScreenDpi();

	QFile file(qssPath);
	if (!file.open(QFile::ReadOnly))
	{
		return;
	}

	const QString stylesheet = ProcessDpiStyleSheet(QString::fromUtf8(file.readAll()));
	qApp->setStyleSheet(stylesheet);
	file.close();
}
