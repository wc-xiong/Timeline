#pragma once

#include "GlobalModelGlobal.h"

#include <QString>

namespace AppPath {

GLOBALMODEL_EXPORT void Initialize();
GLOBALMODEL_EXPORT bool IsInitialized();

GLOBALMODEL_EXPORT QString ApplicationDir();
GLOBALMODEL_EXPORT QString ConfigRoot();
GLOBALMODEL_EXPORT QString AppDataRoot();
GLOBALMODEL_EXPORT QString LogRoot();

GLOBALMODEL_EXPORT QString ConfigDir();
GLOBALMODEL_EXPORT QString ConfigFile(const QString& fileName);
GLOBALMODEL_EXPORT QString LanguageJson();
GLOBALMODEL_EXPORT QString StartupSettingJson();
GLOBALMODEL_EXPORT QString ProjectSettingIni();
GLOBALMODEL_EXPORT QString RecentProjectJson();
GLOBALMODEL_EXPORT QString DisplayerConfigIni();
GLOBALMODEL_EXPORT QString MachineConfigIni();
GLOBALMODEL_EXPORT QString ControllerConfigIni();

GLOBALMODEL_EXPORT QString ResourceFile(const QString& fileName);
GLOBALMODEL_EXPORT QString TranslationsDir();
GLOBALMODEL_EXPORT QString TempDataDir();
GLOBALMODEL_EXPORT QString TempDataFolder();

GLOBALMODEL_EXPORT QString DefaultLayoutPath(int eStage);
GLOBALMODEL_EXPORT QString UserDefinedLayoutPath(int eStage);

GLOBALMODEL_EXPORT void EnsureConfigDir();
GLOBALMODEL_EXPORT void EnsureDirExists(const QString& dirPath);

} // namespace AppPath

#define APP_PATH_INIT() AppPath::Initialize()

#define APP_PATH_APPLICATION_DIR AppPath::ApplicationDir()
#define APP_PATH_CONFIG_ROOT AppPath::ConfigRoot()
#define APP_PATH_APPDATA_ROOT AppPath::AppDataRoot()
#define APP_PATH_LOG_ROOT AppPath::LogRoot()

#define APP_PATH_CONFIG_DIR AppPath::ConfigDir()
#define APP_PATH_CONFIG_FILE(name) AppPath::ConfigFile(name)
#define APP_PATH_LANGUAGE_JSON AppPath::LanguageJson()
#define APP_PATH_STARTUP_SETTING_JSON AppPath::StartupSettingJson()
#define APP_PATH_PROJECT_SETTING_INI AppPath::ProjectSettingIni()
#define APP_PATH_RECENT_PROJECT_JSON AppPath::RecentProjectJson()
#define APP_PATH_DISPLAYER_CONFIG_INI AppPath::DisplayerConfigIni()
#define APP_PATH_MACHINE_CONFIG_INI AppPath::MachineConfigIni()
#define APP_PATH_CONTROLLER_CONFIG_INI AppPath::ControllerConfigIni()

#define APP_PATH_RESOURCE_FILE(name) AppPath::ResourceFile(name)
#define APP_PATH_TRANSLATIONS_DIR AppPath::TranslationsDir()
#define APP_PATH_TEMP_DATA_DIR AppPath::TempDataDir()
#define APP_PATH_TEMP_DATA_FOLDER AppPath::TempDataFolder()

#define APP_PATH_DEFAULT_LAYOUT(stage) AppPath::DefaultLayoutPath(stage)
#define APP_PATH_USER_DEFINED_LAYOUT(stage) AppPath::UserDefinedLayoutPath(stage)

#define APP_PATH_ENSURE_CONFIG_DIR() AppPath::EnsureConfigDir()
#define APP_PATH_ENSURE_DIR(dir) AppPath::EnsureDirExists(dir)
