@echo off
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0..\..\.."
set "ROOT=%CD%"

echo [build-release] ROOT=%ROOT%

if "%QT_DIR%"=="" (
    echo [ERROR] QT_DIR 未设置
    exit /b 1
)
set "THIRDPARTY_ROOT=%ROOT%\3rdparty"

call "%ROOT%\Release.bat"
if errorlevel 1 (
    echo [ERROR] Release.bat 失败
    exit /b 1
)

if not exist "%ROOT%\bin\Timeline.exe" (
    echo [ERROR] 未找到 %ROOT%\bin\Timeline.exe
    exit /b 1
)

echo [build-release] OK
exit /b 0
