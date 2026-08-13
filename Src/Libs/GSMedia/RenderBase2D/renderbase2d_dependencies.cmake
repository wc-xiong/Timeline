#
# 项目名称
# 
SET(WIS_PROJECT_NAME renderbase2d)

#
# C++版本 17
#
SET(CMAKE_CXX_STANDARD 17)

if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
	LINK_DIRECTORIES(
	${THIRDPARTY_ROOT}/NVAPI/Lib
	)
else()
	LINK_DIRECTORIES(
	${THIRDPARTY_ROOT}/NVAPI/Lib
	)
endif()

#
# 依赖非Qt库，非本项库
#
set(DEPENDENCIES
	common
	timeserver
	rendercommon
	renderdevice
	#nvapiwrapper
	slicemodel
    )
#
# 依赖的Qt库
#
SET(QT_MODULES
    )
	
#
# 依赖的本项目内的库
#
SET(WIS_MODULE_DEPS
    )
