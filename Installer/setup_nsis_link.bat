@echo off
setlocal EnableExtensions
cd /d "%~dp0"
set "LINKDIR=%CD%\NSIS"
set "SRC=E:\project\Mapping3.0\3.0\Installer\NSIS"

if exist "%LINKDIR%\Unicode\makensis.exe" (
    echo [OK] NSIS 已就绪: %LINKDIR%
    exit /b 0
)

if not exist "%SRC%\Unicode\makensis.exe" (
    echo [ERROR] 未找到 NSIS: %SRC%
    echo 请将 NSIS Unicode 拷贝到 Installer\NSIS\ 或修改本脚本路径
    exit /b 1
)

if exist "%LINKDIR%" rmdir /s /q "%LINKDIR%" 2>nul
mklink /J "%LINKDIR%" "%SRC%"
if errorlevel 1 (
    echo [WARN] 目录联接失败，尝试复制（较慢）...
    mkdir "%LINKDIR%" 2>nul
    xcopy /E /I /Y "%SRC%\*" "%LINKDIR%\"
)
echo [OK] NSIS -^> %LINKDIR%
exit /b 0
