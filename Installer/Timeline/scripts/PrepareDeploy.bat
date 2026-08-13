@echo off
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0..\..\.."
set "ROOT=%CD%"
set "PACKBIN=%ROOT%\Pack\bin"
set "SRCBIN=%ROOT%\bin"

echo [PrepareDeploy] 收集发布文件到 Pack\bin

if "%QT_DIR%"=="" (
    echo [ERROR] QT_DIR 未设置
    exit /b 1
)
set "THIRDPARTY_ROOT=%ROOT%\3rdparty"

if exist "%QT_DIR%\bin\qmake.exe" (
    set "QTBIN=%QT_DIR%\bin"
) else if exist "%QT_DIR%\qmake.exe" (
    for %%I in ("%QT_DIR%\..") do set "QTBIN=%%~fI\bin"
) else (
    set "QTBIN=%QT_DIR%\bin"
)

rem 运行/打包入口：仓库根 bin\（Debug.bat / Release.bat 会从 build\bin 同步）
if not exist "%SRCBIN%\Timeline.exe" if exist "%ROOT%\build\bin\Timeline.exe" (
    if not exist "%SRCBIN%" mkdir "%SRCBIN%"
    copy /Y "%ROOT%\build\bin\Timeline.exe" "%SRCBIN%\Timeline.exe" >nul
)
if not exist "%SRCBIN%\Timeline.exe" (
    echo [ERROR] 缺少 Timeline.exe，请先 Release.bat
    exit /b 1
)

set "IS_DEBUG=0"
findstr /M /C:"Qt5Cored.dll" "%SRCBIN%\Timeline.exe" >nul 2>nul && set "IS_DEBUG=1"
findstr /M /C:"opencv_world450d.dll" "%SRCBIN%\Timeline.exe" >nul 2>nul && set "IS_DEBUG=1"

if "!IS_DEBUG!"=="1" (
    echo [ERROR] 当前 Timeline.exe 为 Debug（依赖 Qt5*d / opencv_world450d^）
    echo         安装包请使用 Release：先运行 Release.bat，确认成功后再打包。
    exit /b 1
)

if exist "%PACKBIN%" rmdir /s /q "%PACKBIN%"
mkdir "%PACKBIN%" 2>nul

echo [PrepareDeploy] 复制 bin -^> Pack\bin
xcopy /E /I /Y /Q "%SRCBIN%\*" "%PACKBIN%\" >nul
if errorlevel 1 (
    echo [ERROR] xcopy 失败
    exit /b 1
)

del /q "%PACKBIN%\*.pdb" 2>nul
del /q "%PACKBIN%\*.ilk" 2>nul
del /q "%PACKBIN%\*.exp" 2>nul
rem 仅删除明确的 Debug 运行库，禁止 *d.dll 通配
del /q "%PACKBIN%\opencv_world450d.dll" 2>nul
for %%F in (Qt5Cored Qt5Guid Qt5Widgetsd Qt5Svgd Qt5Networkd Qt5Xmld Qt5Concurrentd) do (
    del /q "%PACKBIN%\%%F.dll" 2>nul
)

echo [PrepareDeploy] windeployqt --release
"%QTBIN%\windeployqt.exe" --release --compiler-runtime --no-translations "%PACKBIN%\Timeline.exe"
if errorlevel 1 echo [WARN] windeployqt 返回非零

if exist "%THIRDPARTY_ROOT%\ffmpeg\bin" xcopy /Y /Q "%THIRDPARTY_ROOT%\ffmpeg\bin\*.dll" "%PACKBIN%\" >nul 2>nul
if exist "%THIRDPARTY_ROOT%\SDL3\bin\SDL3.dll" copy /Y "%THIRDPARTY_ROOT%\SDL3\bin\SDL3.dll" "%PACKBIN%\" >nul

set "OPENCV_DLL=%THIRDPARTY_ROOT%\opencv2\bin\Release\opencv_world450.dll"
if not exist "!OPENCV_DLL!" set "OPENCV_DLL=%THIRDPARTY_ROOT%\opencv2\bin\opencv_world450.dll"
if exist "!OPENCV_DLL!" (
    copy /Y "!OPENCV_DLL!" "%PACKBIN%\opencv_world450.dll" >nul
    echo [PrepareDeploy] opencv_world450.dll OK
) else (
    echo [ERROR] 未找到 !OPENCV_DLL!
    exit /b 1
)

if not exist "%PACKBIN%\HLSL\" if exist "%ROOT%\Src\Libs\GSMedia\RenderBase2D\HLSL" (
    mkdir "%PACKBIN%\HLSL" 2>nul
    xcopy /E /I /Y /Q "%ROOT%\Src\Libs\GSMedia\RenderBase2D\HLSL\*" "%PACKBIN%\HLSL\" >nul
)

if not exist "%PACKBIN%\platforms\qwindows.dll" (
    echo [ERROR] 缺少 platforms\qwindows.dll
    exit /b 1
)
if not exist "%PACKBIN%\opencv_world450.dll" (
    echo [ERROR] 缺少 opencv_world450.dll
    exit /b 1
)
if exist "%PACKBIN%\opencv_world450d.dll" del /q "%PACKBIN%\opencv_world450d.dll"

echo [PrepareDeploy] OK -^> %PACKBIN%
exit /b 0
