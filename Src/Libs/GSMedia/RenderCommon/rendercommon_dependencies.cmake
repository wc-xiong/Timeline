#
# 项目名称
# 
SET(WIS_PROJECT_NAME rendercommon)

#
# C++版本 17
#
SET(CMAKE_CXX_STANDARD 17)

#
# 头文件路径
#
INCLUDE_DIRECTORIES(
	${THIRDPARTY_ROOT}/DirectXTex/inc/
	${THIRDPARTY_ROOT}/assimp/inc/
)
#
#lib搜索路径
#
if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
	LINK_DIRECTORIES(
	${THIRDPARTY_ROOT}/DirectXTex/lib/debug
	${THIRDPARTY_ROOT}/assimp/lib/debug
	)
else()
	LINK_DIRECTORIES(
	${THIRDPARTY_ROOT}/DirectXTex/lib/release
	${THIRDPARTY_ROOT}/assimp/lib/release
	)
endif()

# boost库（PickHelper 使用 boost::thread / boost::chrono）
IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
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

# 拷贝dll到编译bin目录下
if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
	SET(ASSIMP_EXEDIR ${THIRDPARTY_ROOT}/assimp/bin/debug)
else()
	SET(ASSIMP_EXEDIR ${THIRDPARTY_ROOT}/assimp/bin/release)
endif()

EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASSIMP_EXEDIR} ${EXECUTABLE_OUTPUT_PATH})

#
# 依赖非Qt库，非本项库
#
set(DEPENDENCIES
	common
    )
#
# 依赖的Qt库
#
SET(QT_MODULES
	Xml
    )
	
#
# 依赖的本项目内的库
#
SET(WIS_MODULE_DEPS
    )
