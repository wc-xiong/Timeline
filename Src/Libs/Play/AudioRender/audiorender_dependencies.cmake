#
# 项目名称
# 
SET(WIS_PROJECT_NAME audiorender)
#
# 头文件路径
#
INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_LIST_DIR}/../../Common
	${SDL_INCLUDEDIR}
	${FFMPEG_INCLUDEDIR}
	${CMAKE_CURRENT_LIST_DIR}/../../Model
	${BOOST_INCLUDEDIR}
)

#
#lib搜索路径
#
LINK_DIRECTORIES(
	${PROJECT_BINARY_DIR}
	${SDL_LIBRARYDIR}
	${FFMPEG_LIBRARYDIR}
	${BOOST_LIBRARYDIR}
)

#
# 依赖非Qt库，非本项库
#
set(DEPENDENCIES
	common
	playcore
	timeserver
	globalmodel
	timelinemodel
    )
#
# 依赖的Qt库
#
SET(QT_MODULES
	Core
    )
	
#
# 依赖的本项目内的库
#
SET(WIS_MODULE_DEPS

    )