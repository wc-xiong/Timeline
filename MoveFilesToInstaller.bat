@echo off
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0"
set "ROOT=%CD%"
set "SRC=%ROOT%\Pack\bin"
set "DST=%ROOT%\Installer\Timeline"

echo [MoveFilesToInstaller] %SRC% -^> %DST%

if not exist "%SRC%\Timeline.exe" (
    echo [ERROR] 请先运行 PrepareDeploy.bat，缺少 %SRC%\Timeline.exe
    exit /b 1
)

if not exist "%DST%\scripts" (
    mkdir "%DST%\scripts" 2>nul
)

rem 用 robocopy 镜像发布内容，保留 scripts 目录
robocopy "%SRC%" "%DST%" /MIR /XD scripts /NFL /NDL /NJH /NJS /nc /ns /np
set "RC=!errorlevel!"
if !RC! GEQ 8 (
    echo [ERROR] robocopy 失败 code=!RC!
    exit /b 1
)

del /q "%DST%\*.pdb" 2>nul
del /q "%DST%\*.ilk" 2>nul

if exist "%ROOT%\Src\App\Timeline\resources\Timeline.ico" (
    copy /Y "%ROOT%\Src\App\Timeline\resources\Timeline.ico" "%DST%\Timeline.ico" >nul
)

set "BUILD_NUMBER="
for /f "usebackq delims=" %%i in (`git -C "%ROOT%" rev-list --count HEAD 2^>nul`) do set "BUILD_NUMBER=%%i"
if not defined BUILD_NUMBER (
    for /f "usebackq delims=" %%i in (`powershell -NoProfile -Command "Get-Date -Format yyyyMMdd"`) do set "BUILD_NUMBER=%%i"
)
if not exist "%DST%\Resource" mkdir "%DST%\Resource"
(
    echo Timeline 1.0
    echo Number:!BUILD_NUMBER!
    echo.
) > "%DST%\Resource\Version.Info"

echo [MoveFilesToInstaller] OK BUILD_NUMBER=!BUILD_NUMBER!
exit /b 0
