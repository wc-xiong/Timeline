###
SET(Boost_DEBUG ${Boost_DEBUG} CACHE PATH "Boost_DEBUG " FORCE)
###
SET(Boost_NO_BOOST_CMAKE ON)
###
SET(Boost_USE_STATIC_LIBS ON)
###
SET(Boost_USE_MULTITHREADED ON)
### 移除Boost的自动链接的规则
ADD_DEFINITIONS(-DBOOST_ALL_NO_LIB)
ADD_DEFINITIONS(-DBOOST_BEAST_NO_THREAD_LOCAL)

if (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
    set(Boost_ARCHITECTURE "-x64")
else()
    set(Boost_ARCHITECTURE "-x32")
endif()

# Timeline：优先使用工程 THIRDPARTY_ROOT/boost（include + lib/debug|release）
# 忽略环境变量 / 旧 CMakeCache 中指向外部 Mapping 的 BOOST_HOME
IF(DEFINED THIRDPARTY_ROOT AND EXISTS "${THIRDPARTY_ROOT}/boost/include")
	SET(BOOST_HOME "${THIRDPARTY_ROOT}/boost" CACHE PATH "BOOST_HOME dependency path" FORCE)
	SET(BOOST_INCLUDEDIR "${BOOST_HOME}/include" CACHE PATH "BOOST_INCLUDEDIR dependency path" FORCE)
	# 仅链入当前配置对应目录：debug/release 下同名 libboost_*-mt-x64-*.lib，
	# 若同时 LINK_DIRECTORIES 且 debug 在前，Release 会误链 Debug CRT 的 Boost。
	IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
		SET(BOOST_LIBRARYDIR "${BOOST_HOME}/lib/debug" CACHE PATH "BOOST_LIBRARYDIR dependency path" FORCE)
	ELSE()
		SET(BOOST_LIBRARYDIR "${BOOST_HOME}/lib/release" CACHE PATH "BOOST_LIBRARYDIR dependency path" FORCE)
	ENDIF()
	LINK_DIRECTORIES("${BOOST_LIBRARYDIR}")
	# 清掉可能残留的外部路径结果，避免 FindBoost 继续用 Mapping
	UNSET(Boost_INCLUDE_DIR CACHE)
	UNSET(Boost_LIBRARY_DIRS)
	FOREACH(_boost_comp IN ITEMS system filesystem program_options random chrono date_time thread)
		STRING(TOUPPER "${_boost_comp}" _boost_comp_u)
		UNSET(Boost_${_boost_comp_u}_LIBRARY_DEBUG CACHE)
		UNSET(Boost_${_boost_comp_u}_LIBRARY_RELEASE CACHE)
		UNSET(Boost_${_boost_comp_u}_LIBRARY CACHE)
	ENDFOREACH()
	UNSET(FIND_PACKAGE_MESSAGE_DETAILS_Boost CACHE)

	SET(Boost_INCLUDE_DIRS ${BOOST_INCLUDEDIR})
	SET(Boost_LIBRARY_DIRS ${BOOST_LIBRARYDIR})
ELSEIF((NOT "${BOOST_INCLUDEDIR}" STREQUAL "") AND (NOT "${BOOST_LIBRARYDIR}" STREQUAL ""))
	SET(BOOST_INCLUDEDIR ${BOOST_INCLUDEDIR} CACHE PATH "BOOST_INCLUDEDIR dependency path" FORCE)
	SET(BOOST_LIBRARYDIR ${BOOST_LIBRARYDIR} CACHE PATH "BOOST_LIBRARYDIR dependency path" FORCE)

	SET(Boost_INCLUDE_DIRS ${BOOST_INCLUDEDIR})
	SET(Boost_LIBRARY_DIRS ${BOOST_LIBRARYDIR})
ELSE()
	# 初次检查BOOST_ROOT变量
	IF("${BOOST_ROOT}" STREQUAL "")
		FILE (TO_CMAKE_PATH "$ENV{BOOST_ROOT}" BOOST_ROOT)
	ENDIF()
	IF(NOT "${BOOST_ROOT}" STREQUAL "")
		SET(BOOST_ROOT ${BOOST_ROOT} CACHE PATH "BOOST_ROOT dependency path" FORCE)
	ELSE()
		IF("${BOOST_HOME}" STREQUAL "")
			FILE (TO_CMAKE_PATH "$ENV{BOOST_HOME}" BOOST_HOME)
		ENDIF()
		IF("${BOOST_HOME}" STREQUAL "")
			MESSAGE(FATAL_ERROR "Boost not found. Expected ${THIRDPARTY_ROOT}/boost or -DBOOST_HOME=...")
		ELSE()
			SET(BOOST_HOME ${BOOST_HOME} CACHE PATH "BOOST_HOME dependency path" FORCE)
		ENDIF()

		SET(BOOST_INCLUDEDIR ${BOOST_HOME}/include)
		IF(EXISTS "${BOOST_HOME}/lib/debug" OR EXISTS "${BOOST_HOME}/lib/release")
			IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
				SET(BOOST_LIBRARYDIR ${BOOST_HOME}/lib/debug)
			ELSE()
				SET(BOOST_LIBRARYDIR ${BOOST_HOME}/lib/release)
			ENDIF()
		ELSE()
			SET(BOOST_LIBRARYDIR ${BOOST_HOME}/${OSName}/${CompilerName}/${CompilerArch}/${CompilerVersion})
		ENDIF()

		SET(Boost_INCLUDE_DIRS ${BOOST_INCLUDEDIR})
		SET(Boost_LIBRARY_DIRS ${BOOST_LIBRARYDIR})
	ENDIF()
ENDIF()

SET(Boost_ADDITIONAL_VERSIONS "1.65.0" "1.69.0" "1.74.0" "1.75.0")

FIND_PACKAGE (Boost 1.65.0 COMPONENTS "${BOOST_COMPONENTS}")

message("${BOOST_HOME}")
message("${Boost_INCLUDE_DIRS}")
message("${Boost_LIBRARY_DIRS}")

IF(Boost_FOUND)
	INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
	LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
	MESSAGE (STATUS "-- Boost Version:${Boost_VERSION}")
	message (STATUS "-- Boost Include Directories")
	foreach (include_dir ${Boost_INCLUDE_DIRS})
		message (STATUS "       " ${include_dir})
	endforeach ()
	message (STATUS "-- Boost Library Directories")
	foreach (library_dir ${Boost_LIBRARY_DIRS})
		message (STATUS "       " ${library_dir})
	endforeach ()
ELSE()
	MESSAGE(FATAL_ERROR "Failed to find required dependency: boost.\
		This Allow use -DBOOST_ROOT=xxx Or -DBOOST_HOME=xxx Or -DBOOST_INCLUDEDIR=xxx -DBOOST_LIBRARYDIR")
ENDIF()
