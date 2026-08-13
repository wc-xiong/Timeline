#
# 项目名称
# 
SET(WIS_PROJECT_NAME uicommon)
#
# 依赖非Qt库，非本项库
#
set(DEPENDENCIES
	gsresource
	threadmanager
    )
#
# 依赖的Qt库
#
SET(QT_MODULES
	Core
    Gui
    Widgets
	Svg
    )
	
#
# 依赖的本项目内的库
#
SET(WIS_MODULE_DEPS
	signalcenter
    )
