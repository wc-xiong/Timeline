#
# 项目名称
# 
SET(WIS_PROJECT_NAME timelinewnd)
#
# C++版本 17
#
SET(CMAKE_CXX_STANDARD 17)
#
# 依赖非Qt库，非本项库
#
set(DEPENDENCIES
	uicommon
	timelinemodel
    resourcemodel
    modelmodel
    signalcenter
    threadmanager
    selectcontroller
    playercontroller
	playstatecenter
    signalcenter
    viewdatamanager
    globalhistorystack
    screenmodel
	timeserver
    )

#
# 依赖的Qt库
#
SET(QT_MODULES
    Gui   
    Widgets
    )
	
#
# 依赖的本项目内的库
#
SET(WIS_MODULE_DEPS
	playcommandcenter
	playstatecenter
    )
