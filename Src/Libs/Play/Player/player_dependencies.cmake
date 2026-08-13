#
# 项目名称
# 
SET(WIS_PROJECT_NAME player)

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
	${BOOST_INCLUDEDIR}
)
#
#lib搜索路径
#
LINK_DIRECTORIES(
	${PROJECT_BINARY_DIR}
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
	playcore
	common
	renderdevice
	rendercommon
	timeserver
	resourcemodel
	mediamodel
	streammedia
	globalmodel
	uicommon
    )
#
# 依赖的Qt库
#
SET(QT_MODULES
	Core
	Gui
	Network
    )
	
#
# 依赖的本项目内的库
#
SET(WIS_MODULE_DEPS
	playstatecenter
    )