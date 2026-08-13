; 安装文件清单（参考 Mapping app.nsh；无 .svn）
SetOutPath "$INSTDIR"
File "${APP_FILE_DIR}\*.exe"
File /nonfatal "${APP_FILE_DIR}\*.dll"
File /nonfatal "${APP_FILE_DIR}\*.ico"

SetOutPath "$INSTDIR\HLSL"
File /r "${APP_FILE_DIR}\HLSL\*.*"

SetOutPath "$INSTDIR\platforms"
File /r "${APP_FILE_DIR}\platforms\*.*"

SetOutPath "$INSTDIR\styles"
File /r "${APP_FILE_DIR}\styles\*.*"

SetOutPath "$INSTDIR\imageformats"
File /r "${APP_FILE_DIR}\imageformats\*.*"

SetOutPath "$INSTDIR\iconengines"
File /r "${APP_FILE_DIR}\iconengines\*.*"

IfFileExists "${APP_FILE_DIR}\Resource\Version.Info" 0 timeline_skip_resource
  SetOutPath "$INSTDIR\Resource"
  File /r "${APP_FILE_DIR}\Resource\*.*"
timeline_skip_resource:

SetOutPath "$INSTDIR"
