@echo off
cd /d "%~dp0"
call ..\setup_nsis_link.bat
set "NSIS=..\NSIS\Unicode\makensis.exe"
if not exist "%NSIS%" set "NSIS=E:\project\Mapping3.0\3.0\Installer\NSIS\Unicode\makensis.exe"
"%NSIS%" /V2 "setup.nsi"
pause
