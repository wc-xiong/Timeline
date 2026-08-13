#include "DisplayerDataMgr.h"
#include "AppPath.h"
#include "Common/CommonDefine.h"
#include "Common/CommonFunctionFileOp.h"
#include "Common/CommonRapidJson.h"
#include "Common/LanguageDefine.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QHostInfo>
#include <QStandardPaths>
#include <QTranslator>
#include <algorithm>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

namespace
{
	QString normalizeCachePath(const QString& path)
	{
		QString normalized = QDir::fromNativeSeparators(path.trimmed());
		while (normalized.endsWith(QLatin1Char('/')))
			normalized.chop(1);
		return normalized;
	}

	bool isNestedCachePath(const QString& first, const QString& second)
	{
		if (first.isEmpty() || second.isEmpty())
			return false;

		const QString a = normalizeCachePath(first).toLower() + QLatin1Char('/');
		const QString b = normalizeCachePath(second).toLower() + QLatin1Char('/');
		return b.startsWith(a) || a.startsWith(b);
	}

	bool copyFileEntry(const QFileInfo& info, const QString& dstDirPath, QStringList& copiedPaths)
	{
		const QString dstPath = QDir(dstDirPath).filePath(info.fileName());
		if (QFile::exists(dstPath) && !QFile::remove(dstPath))
			return false;

		if (!QFile::copy(info.absoluteFilePath(), dstPath))
			return false;

		copiedPaths.append(dstPath);
		return true;
	}

	bool copyDirRecursively(const QString& src, const QString& dst, QStringList& copiedPaths)
	{
		QDir srcDir(src);
		if (!srcDir.exists())
			return true;

		QDir dstDir(dst);
		if (!dstDir.exists())
		{
			if (!QDir().mkpath(dst))
				return false;
			copiedPaths.append(dst);
		}

		const QFileInfoList entries = srcDir.entryInfoList(
			QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
		for (const QFileInfo& info : entries)
		{
			if (info.isDir())
			{
				if (!copyDirRecursively(info.absoluteFilePath(), dstDir.filePath(info.fileName()), copiedPaths))
					return false;
			}
			else if (!copyFileEntry(info, dst, copiedPaths))
			{
				return false;
			}
		}
		return true;
	}

	void rollbackCopiedPaths(QStringList& copiedPaths)
	{
		std::sort(copiedPaths.begin(), copiedPaths.end(),
			[](const QString& a, const QString& b) { return a.length() > b.length(); });

		for (const QString& path : copiedPaths)
		{
			const QFileInfo info(path);
			if (info.isDir())
				QDir(path).removeRecursively();
			else if (info.exists())
				QFile::remove(path);
		}
		copiedPaths.clear();
	}
}

CDisplayerDataMgr* CDisplayerDataMgr::GetInstance()
{
	static CDisplayerDataMgr instance;
	return &instance;
}

CDisplayerDataMgr::CDisplayerDataMgr()
{
	LoadDefault();
	EnsureConfigFile();
	LoadSettings();
	LoadTranslator();
}

CDisplayerDataMgr::~CDisplayerDataMgr()
{
}

void CDisplayerDataMgr::LoadDefault()
{
	m_nLanguageType = EML_Chinese;
	m_bAutoSelNetworkAdapter = true;
	m_strNetworkAdapter.clear();
	m_bAutoReciveController = true;
	m_strLocalIp = QStringLiteral("127.0.0.1");
	m_strControllerIp = "";
	m_strControllerDeviceName.clear();
	m_strFileCahePath = QStringLiteral("C:\\GrandMappingFileCache\\");
}

void CDisplayerDataMgr::EnsureConfigFile()
{
	if (!QFile::exists(APP_PATH_DISPLAYER_CONFIG_INI))
		SaveToFile();
}

bool CDisplayerDataMgr::SaveToFile()
{
	rapidjson::Document doc;
	doc.SetObject();
	auto& allocator = doc.GetAllocator();

	doc.AddMember("AutoSelectNetworkAdapter", m_bAutoSelNetworkAdapter, allocator);
	RapidJsonAddStringMember(doc, "NetworkAdapter", m_strNetworkAdapter, allocator);
	doc.AddMember("AcceptRemoteController", m_bAutoReciveController, allocator);
	RapidJsonAddStringMember(doc, "LocalIp", m_strLocalIp, allocator);
	RapidJsonAddStringMember(doc, "FileCachePath", m_strFileCahePath, allocator);
	doc.AddMember("LanguageType", m_nLanguageType, allocator);

	return SaveRapidJsonToFile(doc, APP_PATH_DISPLAYER_CONFIG_INI);
}

void CDisplayerDataMgr::LoadSettings()
{
	rapidjson::Document obj;
	if (!LoadRapidJsonFromFile(APP_PATH_DISPLAYER_CONFIG_INI, obj))
		return;

	m_bAutoSelNetworkAdapter = obj.HasMember("AutoSelectNetworkAdapter") && obj["AutoSelectNetworkAdapter"].IsBool()
		? obj["AutoSelectNetworkAdapter"].GetBool() : m_bAutoSelNetworkAdapter;
	m_strNetworkAdapter = obj.HasMember("NetworkAdapter") && obj["NetworkAdapter"].IsString()
		? QString::fromUtf8(obj["NetworkAdapter"].GetString()) : m_strNetworkAdapter;
	m_bAutoReciveController = obj.HasMember("AcceptRemoteController") && obj["AcceptRemoteController"].IsBool()
		? obj["AcceptRemoteController"].GetBool() : m_bAutoReciveController;
	m_strLocalIp = obj.HasMember("LocalIp") && obj["LocalIp"].IsString()
		? QString::fromUtf8(obj["LocalIp"].GetString()) : m_strLocalIp;
	m_strFileCahePath = obj.HasMember("FileCachePath") && obj["FileCachePath"].IsString()
		? QString::fromUtf8(obj["FileCachePath"].GetString()) : m_strFileCahePath;
	m_nLanguageType = obj.HasMember("LanguageType") && obj["LanguageType"].IsInt()
		? obj["LanguageType"].GetInt() : m_nLanguageType;
}

void CDisplayerDataMgr::SetAutoSelectNetworkAdapter(bool bValue)
{
	if (m_bAutoSelNetworkAdapter == bValue)
		return;
	m_bAutoSelNetworkAdapter = bValue;
	SaveToFile();
}

void CDisplayerDataMgr::SetNetworkAdapter(const QString& strValue)
{
	if (m_strNetworkAdapter == strValue)
		return;
	m_strNetworkAdapter = strValue;
	SaveToFile();
}

void CDisplayerDataMgr::SetAcceptRemoteController(bool bValue)
{
	if (m_bAutoReciveController == bValue)
		return;
	m_bAutoReciveController = bValue;
	SaveToFile();
}

void CDisplayerDataMgr::SetLocalIp(const QString& strValue)
{
	if (m_strLocalIp == strValue)
		return;
	m_strLocalIp = strValue;
	SaveToFile();
}

void CDisplayerDataMgr::SetControllerIp(const QString& strValue)
{
	m_strControllerIp = strValue;
}

void CDisplayerDataMgr::SetControllerDeviceName(const QString& strValue)
{
	if (m_strControllerDeviceName == strValue)
		return;
	m_strControllerDeviceName = strValue;
	SaveToFile();
}

void CDisplayerDataMgr::SetFileCachePath(const QString& strValue)
{
	if (m_strFileCahePath == strValue)
		return;
	m_strFileCahePath = strValue;
	SaveToFile();
}

ECachePathMigrateResult CDisplayerDataMgr::MigrateFileCachePath(const QString& newPath)
{
	const QString normalizedNew = normalizeCachePath(newPath);
	if (normalizedNew.isEmpty())
		return ECachePathMigrateResult::InvalidTarget;

	const QString normalizedOld = normalizeCachePath(m_strFileCahePath);
	if (normalizedNew.compare(normalizedOld, Qt::CaseInsensitive) == 0)
		return ECachePathMigrateResult::Success;

	if (isNestedCachePath(normalizedOld, normalizedNew))
		return ECachePathMigrateResult::InvalidTarget;

	const bool oldExists = QDir(normalizedOld).exists();
	const bool newExistedBefore = QDir(normalizedNew).exists();

	if (oldExists)
	{
		QStringList copiedPaths;
		if (!copyDirRecursively(normalizedOld, normalizedNew, copiedPaths))
		{
			rollbackCopiedPaths(copiedPaths);
			return ECachePathMigrateResult::CopyFailed;
		}

		if (!QDir(normalizedOld).removeRecursively())
		{
			rollbackCopiedPaths(copiedPaths);
			return ECachePathMigrateResult::DeleteSourceFailed;
		}
	}
	else if (!newExistedBefore && !QDir().mkpath(normalizedNew))
	{
		return ECachePathMigrateResult::CreateDirectoryFailed;
	}

	m_strFileCahePath = QDir::toNativeSeparators(normalizedNew);
	SaveToFile();
	return ECachePathMigrateResult::Success;
}

void CDisplayerDataMgr::SetLanguageType(int nType)
{
	Q_UNUSED(nType);
	// Timeline：固定中文
	m_nLanguageType = 0; // EML_Chinese
}

void CDisplayerDataMgr::LoadTranslator()
{
	// Timeline：不使用国际化
	if (!qApp)
		return;

	while (!m_listTranslator.empty())
	{
		auto item = m_listTranslator.back();
		m_listTranslator.pop_back();
		qApp->removeTranslator(item);
		delete item;
	}
	m_listTranslator.clear();
}
