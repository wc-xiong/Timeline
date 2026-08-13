#pragma once

#include <QJsonObject> // { }
#include <QJsonArray> // [ ]
#include <QJsonDocument> // 解析Json
#include <QJsonValue> // int float double bool null { } [ ]
#include <QJsonParseError>

#include <QDir>
#include <QFile>
#include <QTextCodec>
#include <QTranslator>
#include <QTextStream>
#include <QStorageInfo>
#include <QStandardPaths>

#include <fstream> 
#include <QCoreApplication>

#include "CommonAboutCustomization.h"

//获取AppData文件路径
QString GetAppDataPath();

//获取解复用缩略图路径
QString GetImageThumbnailPath();

// 获取该路径下JSON文件内容
QJsonObject LoadJsonFromFile(QString strPath);

// 将JSON内容保存至文件
void SaveJsonToFile(QJsonObject obj, QString strPath);

// 获取文件后缀
QString GetFileExt(const QString& strPath);

// 将字符串写入文件
void WriteStringToFile(const QString &str, const QString &path);
/**
 * @brief 获取默认的Dump输出目录
 *
 * @param const QString & 应用程序名称
 * @param const QString & 公司名称（简称）
 * @return 返回默认的Dump输出目录
 */
QString GetDumpDirDefault(const QString &appName, const QString &strCompanyShortName);

// 获取文件编码格式（UTF8/GBK）
QString GetFileCodeType(const QString &fileName);

// 判断保存路径所在磁盘是否空间不足或不可用,阈值（10MB）
bool IsSavePathDiskInsufficient(const QString& path, const qint64 kMinFreeDiskBytes = 10 * 1024 * 1024);

//////////////////////////////////////////////////////////////////////////
inline QString GetAppDataPath()
{
	// %AppData文件路径%
	QStringList listAppDataPath = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
	assert(listAppDataPath.size() > 0);

	QString strAppDataPath = listAppDataPath.at(0) + DATAPATH;

	QDir dir(strAppDataPath);
	if (!dir.exists())
		QDir().mkpath(strAppDataPath);

	return strAppDataPath;
}

inline QString GetImageThumbnailPath()
{
	//获取解复用缩略图路径
	QStringList listAppDataPath = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
	assert(listAppDataPath.size() > 0);

	QString strAppDataPath = listAppDataPath.at(0) + DATAPATH + "/ImageThumbnail/";

	QDir dir(strAppDataPath);
	if (!dir.exists())
		QDir().mkpath(strAppDataPath);

	return strAppDataPath;
}

inline QJsonObject LoadJsonFromFile(QString strPath)
{
	QFile file(strPath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Truncate))
		return QJsonObject();

	// 读取文件的全部内容
	QTextStream stream(&file);
	stream.setCodec("UTF-8");		// 设置读取编码是UTF8
	QString str = stream.readAll();
	file.close();

	// QJsonParseError类用于在JSON解析期间报告错误。
	QJsonParseError jsonError;
	// 将json解析为UTF-8编码的json文档，并从中创建一个QJsonDocument。
	// 如果解析成功，返回QJsonDocument对象，否则返回null
	QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8(), &jsonError);
	// 判断是否解析失败
	if (jsonError.error != QJsonParseError::NoError && !doc.isNull())
		assert(0);

	return doc.object();
}

inline void SaveJsonToFile(QJsonObject obj, QString strPath)
{
	QFile file(strPath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
		assert(0);

	QJsonDocument doc;// 将json对象里的数据转换为字符串
	doc.setObject(obj);// 将object设置为本文档的主要对象

	QTextStream stream(&file);
	stream.setCodec("UTF-8");		// 设置写入编码是UTF8

	stream << doc.toJson();// 写入文件
	file.close();
}

inline QString GetFileExt(const QString& strPath)
{
	QString strExt;
	int nIndex = strPath.lastIndexOf(".");
	if (nIndex > 0)
	{
		strExt = strPath.right(strPath.length() - nIndex - 1);
	}

	return strExt;
}

inline void WriteStringToFile(const QString &str, const QString &path)
{
	std::ofstream OsWrite(path.toStdString(), std::ofstream::app);
	OsWrite << str.toStdString() << std::endl;
	OsWrite.flush();
	OsWrite.close();
}

inline QString GetDumpDirDefault(const QString &appName, const QString &strCompanyShortName)
{
	QString fullDumpPath = QDir::cleanPath(QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)));
	QStringList dirNames;
	dirNames = fullDumpPath.split('/');
	if (dirNames.count() <= 0)
		dirNames = fullDumpPath.split('\\');
	Q_ASSERT(dirNames.count() > 0);
	if (!(QCoreApplication::applicationName().isNull() || QCoreApplication::applicationName().isEmpty()))
	{
		dirNames.removeLast();
	}
	if (!(QCoreApplication::organizationName().isNull() || QCoreApplication::organizationName().isEmpty()))
	{
		dirNames.removeLast();
	}
	fullDumpPath = QDir::cleanPath(dirNames.join(QDir::separator()));
	if (!fullDumpPath.endsWith(QDir::separator()))
		fullDumpPath.append(QDir::separator());
	fullDumpPath.append(strCompanyShortName);
	fullDumpPath.append(QDir::separator());
	fullDumpPath.append(appName);
	fullDumpPath.append(QDir::separator());
	fullDumpPath.append("dump//");
	QDir dir(fullDumpPath);
	if (!dir.exists(fullDumpPath))
		dir.mkpath(fullDumpPath);
	return fullDumpPath;
}

inline QString GetFileCodeType(const QString &fileName)
{
	QFile file(fileName);
	if (file.open(QIODevice::ReadOnly))
	{
		QByteArray buff = file.readLine();

		QTextCodec::ConverterState state;
		QTextCodec *codec = QTextCodec::codecForName("UTF-8");
		QString text = codec->toUnicode(buff.constData(), buff.size(), &state);
		if (state.invalidChars > 0)
			return QString("GBK");
		else
			return QString("UTF8");

		file.close();
	}
	return "GBK";
}

inline bool IsSavePathDiskInsufficient(const QString& path, const qint64 kMinFreeDiskBytes)
{
	if (path.isEmpty())
		return true;

	const QFileInfo fi(path);
	const QString checkPath = fi.absolutePath().isEmpty() ? path : fi.absolutePath();
	QStorageInfo storage(checkPath);
	if (!storage.isValid() || !storage.isReady())
		return true;

	const qint64 nAvailable = storage.bytesAvailable();
	return nAvailable >= 0 && nAvailable < kMinFreeDiskBytes;
}