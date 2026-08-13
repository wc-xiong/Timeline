#
# 项目名称
# 
SET(WIS_PROJECT_NAME common)

#
# C++版本 17
#
SET(CMAKE_CXX_STANDARD 17)


INCLUDE_DIRECTORIES(
	${THIRDPARTY_ROOT}/openssl/inc
)

#
# 包含的库
#
LINK_DIRECTORIES(
	${THIRDPARTY_ROOT}/openssl/lib
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
    )
#
# 依赖的Qt库
#
SET(QT_MODULES
	Core
	Gui
	Network
	Xml
    )
	
#
# 依赖的本项目内的库
#
SET(WIS_MODULE_DEPS

    )
