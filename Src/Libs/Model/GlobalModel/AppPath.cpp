#include "AppPath.h"

#include "Common/CommonAboutCustomization.h"
#include "Common/CommonDefine.h"
#include "Common/CommonFunctionFileOp.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QScreen>
#include <QStandardPaths>

namespace {

QString g_applicationDir;
QString g_configRoot;
QString g_appDataRoot;
QString g_logRoot;
bool g_initialized = false;

QString JoinPath(const QString& base, const QString& subPath)
{
	return QDir::cleanPath(base + QLatin1Char('/') + subPath);
}

QString LayoutStageFileName(int eStage)
{
	switch (eStage)
	{
	case ESL_Preview:
		return QStringLiteral("Preview");
	case ESL_Debug:
		return QStringLiteral("Debug");
	case ESL_PlayControll:
		return QStringLiteral("PlayControl");
	default:
		break;
	}
	return QString();
}

bool UseHighDpiLayoutSuffix()
{
	QScreen* screen = QGuiApplication::primaryScreen();
	if (!screen)
		return false;

	const int baseValue = 96;
	const qreal dpiVal = screen->logicalDotsPerInch();
	const qreal ratioVal = screen->devicePixelRatio();
	return dpiVal * ratioVal / baseValue > 1.0;
}

void EnsureInitializedInternal()
{
	if (g_initialized)
		return;

	const QStringList listDoc = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
	Q_ASSERT(!listDoc.isEmpty());
	g_configRoot = QDir::cleanPath(listDoc.at(0) + QStringLiteral(DATAPATH));

	g_applicationDir = QApplication::applicationDirPath();
	g_appDataRoot = GetAppDataPath();

	const QString appDataLocation = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).first();
	g_logRoot = JoinPath(QFileInfo(appDataLocation).absolutePath(),
		QStringLiteral("%1/GrandMappingLog").arg(QStringLiteral(COMPANY_NAME)));

	QDir().mkpath(JoinPath(g_configRoot, QStringLiteral("Config")));
	QDir().mkpath(JoinPath(g_configRoot, QStringLiteral("Config/TempDataDir")));
	g_initialized = true;
}

} // namespace

namespace AppPath {

void Initialize()
{
	EnsureInitializedInternal();
}

bool IsInitialized()
{
	return g_initialized;
}

QString ApplicationDir()
{
	EnsureInitializedInternal();
	return g_applicationDir;
}

QString ConfigRoot()
{
	EnsureInitializedInternal();
	return g_configRoot;
}

QString AppDataRoot()
{
	EnsureInitializedInternal();
	return g_appDataRoot;
}

QString LogRoot()
{
	EnsureInitializedInternal();
	return g_logRoot;
}

QString ConfigDir()
{
	return JoinPath(ConfigRoot(), QStringLiteral("Config"));
}

QString ConfigFile(const QString& fileName)
{
	return JoinPath(ConfigDir(), fileName);
}

QString LanguageJson()
{
	return ConfigFile(QStringLiteral("Language.json"));
}

QString StartupSettingJson()
{
	return ConfigFile(QStringLiteral("StartupSetting.json"));
}

QString ProjectSettingIni()
{
	return ConfigFile(QStringLiteral("ProjectSetting.ini"));
}

QString RecentProjectJson()
{
	return ConfigFile(QStringLiteral("RecentPrjFile.json"));
}

QString DisplayerConfigIni()
{
	return ConfigFile(QStringLiteral("DisplayerConfig.ini"));
}

QString MachineConfigIni()
{
	return ConfigFile(QStringLiteral("MachineConfig.ini"));
}

QString ControllerConfigIni()
{
	return ConfigFile(QStringLiteral("ControllerConfig.ini"));
}

QString ResourceFile(const QString& fileName)
{
	return JoinPath(JoinPath(ApplicationDir(), QStringLiteral("Resource")), fileName);
}

QString TranslationsDir()
{
	return JoinPath(ApplicationDir(), QStringLiteral("translations"));
}

QString TempDataDir()
{
	return JoinPath(ConfigDir(), QStringLiteral("TempDataDir"));
}

QString TempDataFolder()
{
	return JoinPath(AppDataRoot(), QStringLiteral("tempFolder"));
}

QString DefaultLayoutPath(int eStage)
{
	const QString fileName = LayoutStageFileName(eStage);
	if (fileName.isEmpty())
		return QString();

	const QString suffix = UseHighDpiLayoutSuffix() ? QStringLiteral("125.layout") : QStringLiteral(".layout");
	return JoinPath(JoinPath(ApplicationDir(), QStringLiteral("Resource")), fileName + suffix);
}

QString UserDefinedLayoutPath(int eStage)
{
	const QString fileName = LayoutStageFileName(eStage);
	if (fileName.isEmpty())
		return QString();

	const QString suffix = UseHighDpiLayoutSuffix() ? QStringLiteral("125.layout") : QStringLiteral(".layout");
	return JoinPath(ConfigDir(), fileName + suffix);
}

void EnsureDirExists(const QString& dirPath)
{
	QDir dir(dirPath);
	if (!dir.exists())
		QDir().mkpath(dirPath);
}

void EnsureConfigDir()
{
	EnsureDirExists(ConfigDir());
	EnsureDirExists(TempDataDir());
}

} // namespace AppPath
