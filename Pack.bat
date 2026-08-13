@echo off
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0"
set "ROOT=%CD%"
set "DSTDIR=%ROOT%\Installer\Timeline"
set "NSIS_SCRIPT=%ROOT%\Installer\installerScripts\setup.nsi"
set "NSIS_EXEC=%ROOT%\Installer\NSIS\Unicode\makensis.exe"
set "MAIN_VERSION=1.0"
set "VERSION_ADD="

echo [pack] Timeline NSIS 打包（无 SVN revision）

if not exist "%NSIS_EXEC%" (
    if exist "E:\project\Mapping3.0\3.0\Installer\NSIS\Unicode\makensis.exe" (
        set "NSIS_EXEC=E:\project\Mapping3.0\3.0\Installer\NSIS\Unicode\makensis.exe"
        echo [pack] 使用备用 NSIS: !NSIS_EXEC!
    ) else (
        echo [ERROR] 未找到 makensis.exe
        echo 请先运行 Installer\setup_nsis_link.bat 或安装 NSIS Unicode
        exit /b 1
    )
)

if not exist "%DSTDIR%\Timeline.exe" (
    echo [ERROR] 缺少 %DSTDIR%\Timeline.exe，请先 MoveFilesToInstaller.bat
    exit /b 1
)

set "BUILD_NUMBER="
for /f "usebackq delims=" %%i in (`git -C "%ROOT%" rev-list --count HEAD 2^>nul`) do set "BUILD_NUMBER=%%i"
if not defined BUILD_NUMBER (
    for /f "usebackq delims=" %%i in (`powershell -NoProfile -Command "Get-Date -Format yyyyMMdd"`) do set "BUILD_NUMBER=%%i"
)

for /f "usebackq delims=" %%i in (`powershell -NoProfile -Command "Get-Date -Format 'yyyy/MM/dd HH:mm:ss'"`) do set "DATETIME=%%i"
if not exist "%DSTDIR%\Resource" mkdir "%DSTDIR%\Resource"
(
    echo Timeline %MAIN_VERSION%
    echo Number:%BUILD_NUMBER%^(%DATETIME%^)
    echo.
) > "%DSTDIR%\Resource\Version.Info"

echo [pack] Version=%MAIN_VERSION%.%BUILD_NUMBER%
echo [pack] makensis "%NSIS_SCRIPT%"

pushd "%ROOT%\Installer\installerScripts"
"%NSIS_EXEC%" /V2 "setup.nsi"
set "ERR=!errorlevel!"
popd
if not "!ERR!"=="0" (
    echo [ERROR] makensis 失败 ERR=!ERR!
    exit /b !ERR!
)

set "SETUP_SRC=%ROOT%\Installer\installerScripts\Timeline_Setup.exe"
set "SETUP_NAME=Timeline_Setup_V%MAIN_VERSION%.%BUILD_NUMBER%%VERSION_ADD%.exe"
if not exist "%SETUP_SRC%" (
    echo [ERROR] 未生成 %SETUP_SRC%
    exit /b 1
)

move /Y "%SETUP_SRC%" "%ROOT%\%SETUP_NAME%" >nul
echo [pack] OK -^> %ROOT%\%SETUP_NAME%
exit /b 0
