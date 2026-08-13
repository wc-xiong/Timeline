@echo off
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0"
set "ROOT=%CD%"

rem 固定使用工程内 3rdparty，不继承外部环境 THIRDPARTY_ROOT（如 E:\project\...\3rdparty）
set "THIRDPARTY_ROOT=%ROOT%\3rdparty"

:trimTHIRDPARTY_ROOT
if not defined THIRDPARTY_ROOT goto trimTHIRDPARTY_ROOT_done
if "!THIRDPARTY_ROOT:~-1!"==";" set "THIRDPARTY_ROOT=!THIRDPARTY_ROOT:~0,-1!" & goto trimTHIRDPARTY_ROOT
if "!THIRDPARTY_ROOT:~-1!"==" " set "THIRDPARTY_ROOT=!THIRDPARTY_ROOT:~0,-1!" & goto trimTHIRDPARTY_ROOT
:trimTHIRDPARTY_ROOT_done
:trimQT_DIR
if not defined QT_DIR goto trimQT_DIR_done
if "!QT_DIR:~-1!"==";" set "QT_DIR=!QT_DIR:~0,-1!" & goto trimQT_DIR
if "!QT_DIR:~-1!"==" " set "QT_DIR=!QT_DIR:~0,-1!" & goto trimQT_DIR
:trimQT_DIR_done

if exist "%QT_DIR%\bin\qmake.exe" goto qt_prefix_ok
if exist "%QT_DIR%\qmake.exe" (
    for %%I in ("%QT_DIR%\..") do set "QT_DIR=%%~fI"
)
:qt_prefix_ok

if "%QT_DIR%"=="" (
    echo [ERROR] Environment variable QT_DIR is not set.
    exit /b 1
)
if not exist "%THIRDPARTY_ROOT%\" (
    echo [ERROR] THIRDPARTY_ROOT is not a valid directory: "%THIRDPARTY_ROOT%"
    echo Expected bundled libs under: "%ROOT%\3rdparty"
    exit /b 1
)
if not exist "%QT_DIR%\bin\" (
    echo [ERROR] QT_DIR must point to Qt prefix with a bin subfolder: "%QT_DIR%"
    exit /b 1
)

set "PATH=%QT_DIR%\bin;%PATH%"

where cl >nul 2>&1
if errorlevel 1 (
    set "VSW=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
    if exist "!VSW!" (
        set "VSTMP=%TEMP%\Timeline_vs2017_path.txt"
        "!VSW!" -version "[15.0,16.0)" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath > "!VSTMP!" 2>nul
        set "VS2017="
        for /f "usebackq delims=" %%i in ("!VSTMP!") do set "VS2017=%%i"
        del "!VSTMP!" 2>nul
        if not defined VS2017 (
            echo [ERROR] Visual Studio 2017 with VC++ x64 tools not found via vswhere.
            exit /b 1
        )
        if not exist "!VS2017!\VC\Auxiliary\Build\vcvars64.bat" (
            echo [ERROR] vcvars64.bat not found under: "!VS2017!"
            exit /b 1
        )
        call "!VS2017!\VC\Auxiliary\Build\vcvars64.bat" || exit /b 1
    ) else (
        echo [ERROR] MSVC cl.exe not in PATH and vswhere.exe not found.
        exit /b 1
    )
) else (
    echo [INFO] Using existing MSVC environment.
)

set "CMAKE_GEN=Visual Studio 15 2017"
set "CMAKE_ARCH=x64"
if not "%CMAKE_GENERATOR%"=="" set "CMAKE_GEN=%CMAKE_GENERATOR%"
if not "%CMAKE_GENERATOR_PLATFORM%"=="" set "CMAKE_ARCH=%CMAKE_GENERATOR_PLATFORM%"

set "BUILD_DIR=%ROOT%\build\Debug"
set "BUILD_BIN=%ROOT%\build\bin"
set "ROOT_BIN=%ROOT%\bin"

echo [INFO] THIRDPARTY_ROOT="%THIRDPARTY_ROOT%"
echo [INFO] Configuring CMake: generator="%CMAKE_GEN%" -A %CMAKE_ARCH% (CMAKE_BUILD_TYPE=Debug)
echo [INFO] Binary dir: "%BUILD_DIR%"
cmake -S "%ROOT%" -B "%BUILD_DIR%" -G "%CMAKE_GEN%" -A "%CMAKE_ARCH%" -DCMAKE_BUILD_TYPE=Debug -DTHIRDPARTY_ROOT="%THIRDPARTY_ROOT%" -DQT_DIR="%QT_DIR%" -DRUN_TS=ON -DRUN_QM=ON || exit /b 1

echo [INFO] Building Debug...
cmake --build "%BUILD_DIR%" --config Debug --parallel || exit /b 1

if not exist "%BUILD_BIN%\Timeline.exe" (
    echo [ERROR] Expected output missing: "%BUILD_BIN%\Timeline.exe"
    exit /b 1
)
if not exist "%ROOT_BIN%\" mkdir "%ROOT_BIN%"
copy /Y "%BUILD_BIN%\Timeline.exe" "%ROOT_BIN%\Timeline.exe" >nul || exit /b 1
rem 同步 exe 旁 HLSL（FXC 产出 .cso + 源码 .hlsl/.hlsli）；优先用 build\bin\HLSL
if exist "%BUILD_BIN%\HLSL\" (
    if not exist "%ROOT_BIN%\HLSL\" mkdir "%ROOT_BIN%\HLSL"
    xcopy /E /Y /Q /I "%BUILD_BIN%\HLSL\*" "%ROOT_BIN%\HLSL\" >nul
) else if exist "%ROOT%\Src\Libs\GSMedia\RenderBase2D\HLSL\" (
    if not exist "%ROOT_BIN%\HLSL\" mkdir "%ROOT_BIN%\HLSL"
    xcopy /E /Y /Q /I "%ROOT%\Src\Libs\GSMedia\RenderBase2D\HLSL\*" "%ROOT_BIN%\HLSL\" >nul
    echo [WARN] build\bin\HLSL missing; synced HLSL sources only ^(no .cso^)
)

echo.
echo [OK] Debug build finished.
echo      CMake output : "%BUILD_BIN%\Timeline.exe"
echo      Run / Pack   : "%ROOT_BIN%\Timeline.exe"
echo [TIP] Legacy "%ROOT%\Debug" configure dir is unused; delete manually if present.
exit /b 0
