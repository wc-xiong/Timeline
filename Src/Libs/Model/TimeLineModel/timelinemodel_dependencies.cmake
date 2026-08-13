#
# 项目名称
# 
SET(WIS_PROJECT_NAME timelinemodel)

#
# C++版本 17
#
SET(CMAKE_CXX_STANDARD 17)

#
# 头文件路径
#
INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_LIST_DIR}/../../Common
	${SDL_INCLUDEDIR}
)
#
#lib搜索路径
#
LINK_DIRECTORIES(
	${PROJECT_BINARY_DIR}
	${SDL_LIBRARYDIR}
)

#
# 依赖非Qt库，非本项库
#
set(DEPENDENCIES
	rendercommon
	signalcenter
	globalmodel
    )
#
# 依赖的Qt库
#
SET(QT_MODULES
	Core
	Xml
	Gui
    )
	
#
# 依赖的本项目内的库
#
SET(WIS_MODULE_DEPS
    )