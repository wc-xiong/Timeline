#
# 项目名称
# 
SET(WIS_PROJECT_NAME threadmanager)

# boost库（ThreadPool 使用 boost::thread / boost::chrono）
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
    )
	
#
# 依赖的本项目内的库
#
SET(WIS_MODULE_DEPS
    )
