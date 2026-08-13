#
# 项目名称
# 
SET(WIS_PROJECT_NAME playcore)
#
# 头文件路径
#
INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_LIST_DIR}/../../Common
	${FFMPEG_INCLUDEDIR}
	${SDL_INCLUDEDIR}
	${THIRDPARTY_ROOT}/
	${BOOST_INCLUDEDIR}
)

#
#lib搜索路径
#
LINK_DIRECTORIES(
	${PROJECT_BINARY_DIR}
	${FFMPEG_LIBRARYDIR}
	${SDL_LIBRARYDIR}
	${BOOST_LIBRARYDIR}
)

LINK_LIBRARIES(
	${Boost_THREAD_LIBRARY}
	${Boost_CHRONO_LIBRARY}
)

# boost库
IF( CMAKE_BUILD_TYPE STREQUAL "Debug" )
	LINK_DIRECTORIES(
		${THIRDPARTY_ROOT}/boost/lib/debug
		${THIRDPARTY_ROOT}/boost/include
	)
ELSE()
	LINK_DIRECTORIES(
		${THIRDPARTY_ROOT}/boost/lib/release
		${THIRDPARTY_ROOT}/boost/include
	)
ENDIF()

#
# 依赖非Qt库，非本项库
#
set(DEPENDENCIES
	renderdevice
	common
    )
#
# 依赖的Qt库
#
SET(QT_MODULES
	Core
	Gui
	Widgets
    )
	
#
# 依赖的本项目内的库
#
SET(WIS_MODULE_DEPS

    )