################################################################################
# Config.cmake - CMake build configuration of wiSCADA
################################################################################
##
INCLUDE(CheckCXXSymbolExists)
## 禁用MSVC下的警告
IF(MSVC)
     ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
     ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
     ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_WARNING)
     ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
	 ###########################################################
#解决VS下std中的Min/Max无法正常使用的编译错误
#解决VS下socket函数重定义的编译错误
    ADD_DEFINITIONS(-DNOMINMAX)
    ADD_DEFINITIONS(-DWIN32_LEAN_AND_MEAN)
ENDIF(MSVC)
## 启用C++11支持
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# prevent in-source builds
IF(NOT INSOURCEBUILD AND (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR}))
    MESSAGE(FATAL_ERROR "
        CMake generation for this project is not allowed within the source directory!
        Remove the CMake cache files and try again from another folder, e.g.:
          rm -r CMakeCache.txt CMakeFiles/
          mkdir build
          cd build
          cmake ..
        If you really want an in-source build, pass -DINSOURCEBUILD=1"
	)
ENDIF()

# If the user specifies -DCMAKE_BUILD_TYPE on the command line, take their definition
# and dump it in the cache along with proper documentation, otherwise set CMAKE_BUILD_TYPE
# to Release prior to calling PROJECT()
IF(DEFINED CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
ELSE(DEFINED CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
ENDIF(DEFINED CMAKE_BUILD_TYPE)
# On Release builds cmake automatically defines NDEBUG, so we
# explicitly undefine it:
# This matches stripped (Release) and nonstripped (RelWithDebInfo) builds
if(CMAKE_BUILD_TYPE MATCHES "Rel.*" AND NOT MSVC)
    ADD_DEFINITIONS("-UNDEBUG")
endif()

## 输出CMake构建过程中的详细信息
IF(CMAKE_BUILD_TYPE MATCHES Debug)
	ADD_DEFINITIONS(-DCMAKE_VERBOSE_MAKEFILE=ON)
	ADD_DEFINITIONS(-D_DEBUG -DDEBUG)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

##
IF(WIN32)
	SET(_WIN32_WINNT 0x0A00 CACHE STRING "Define Windows API version to use.")
	ADD_DEFINITIONS(-D_WIN32_WINNT=${_WIN32_WINNT})
ENDIF()
# 工程采用Unicode
ADD_DEFINITIONS(-D_UNICODE -DUNICODE)

# 动态链接库输出规则
IF(WIN32)
    #Postfix of xxxd.x
    SET(WIS_DEBUG_POSTFIX d)
ELSEIF(APPLE)
    #Postfix of xxx_debug.x
    SET(WIS_DEBUG_POSTFIX _debug)
ELSE()
    #Postfix of xxx.x
    SET(WIS_DEBUG_POSTFIX "")
ENDIF()

if(DEFINED CMAKE_DEBUG_POSTFIX)
    SET(WIS_DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
endif()

IF (CMAKE_BUILD_TYPE MATCHES Debug)
	ADD_DEFINITIONS (-DWIS_DEBUG_POSTFIX="${WIS_DEBUG_POSTFIX}")
ENDIF (CMAKE_BUILD_TYPE MATCHES Debug)

# 苹果下的特殊设置`
IF(APPLE)
    INCLUDE(BundleUtilities)
    #favor mac frameworks over unix libraries
    set(CMAKE_FIND_FRAMEWORK FIRST)
ENDIF()

IF(APPLE)
    # Make sure we can find the 'ibtool' program. we need it to compile xibs
    find_program(IBTOOL ibtool HINTS "/usr/bin" "${OSX_DEVELOPER_ROOT}/usr/bin")
    IF (${IBTOOL} STREQUAL "IBTOOL-NOTFOUND")
        MESSAGE (FATAL_ERROR "ibtool can not be found and is needed to compile the .xib files. It should have been installed with
                    the Apple developer tools. The default system paths were searched in addition to ${OSX_DEVELOPER_ROOT}/usr/bin")
    ENDIF ()

    ADD_DEFINITIONS("-x objective-c++")
    INCLUDE_DIRECTORIES(apple)
    # for objective-c++
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTARGET_OS_MAC")
    # for pure objective-c
    SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -x objective-c -DTARGET_OS_MAC")
ENDIF(APPLE)

### 公司简称
##SET(COMPANY_SHORT_NAME "${COMPANY_SHORT_NAME}" CACHE STRING  "The Company name of short ")
##STRING(LENGTH "${COMPANY_SHORT_NAME}" COMPANY_SHORT_NAME_LENGTH)
##IF(${COMPANY_SHORT_NAME_LENGTH} MATCHES "0")
##    MESSAGE(WARNING "Please set the Company name of short first. -DCOMPANY_SHORT_NAME=XXXX ")
##ENDIF()
##ADD_DEFINITIONS(-DCOMPANY_SHORT_NAME=\"${COMPANY_SHORT_NAME}\")

### 多目录支持
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

### 操作系统 编译器 版本检测
## 操作系统
SET(OSName
	${CMAKE_SYSTEM_NAME}
)	
## 编译器名称
IF(UNIX) # Apple 或 Linux
	GET_FILENAME_COMPONENT(CompilerName ${CMAKE_CXX_COMPILER} NAME_WE)
ELSE()
	SET(CompilerName
		${CMAKE_CXX_COMPILER_ID}
	)
ENDIF()
## 编译器版本
IF(MSVC)
	if(MSVC_VERSION EQUAL 1400)
		SET(CompilerVersion 8.0)
	elseif(MSVC_VERSION EQUAL 1500)
		SET(CompilerVersion 9.0)
	elseif(MSVC_VERSION EQUAL 1600)
		SET(CompilerVersion 10.10)
	elseif(MSVC_VERSION EQUAL 1700)
		SET(CompilerVersion 11.0)
	elseif(MSVC_VERSION EQUAL 1800)
		SET(CompilerVersion 12.0)
	elseif(MSVC_VERSION EQUAL 1900)
		SET(CompilerVersion 14.0)
	elseif(MSVC_VERSION MATCHES "^191[0-9]$")
        SET(CompilerVersion 15.0)
    elseif(MSVC_VERSION MATCHES "^192[0-9]$")
        set(CompilerVersion 16.0)
	else()
		MESSAGE(WARNING "Does not recognize MSVC_VERSION \"${MSVC_VERSION}\".")

	endif()
ELSEIF(APPLE)
	EXECUTE_PROCESS(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion
		OUTPUT_VARIABLE CompilerVersion
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
ELSE()
	SET(CompilerVersion
		${CMAKE_CXX_COMPILER_VERSION}
		)
ENDIF()

## 编译器x86或者x64
IF(APPLE)
	EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE )
	STRING(TOLOWER  "${ARCHITECTURE}" CompilerArch)
ELSEIF(WIN32)
	STRING(TOLOWER  "${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" CompilerArch)
ELSEIF(UNIX)
    EXECUTE_PROCESS(COMMAND ${CMAKE_CXX_COMPILER} -dumpmachine
                    OUTPUT_VARIABLE ARCHITECTURE
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

    IF(ARCHITECTURE MATCHES "arm-")
            SET(CompilerArch "arm")
    ELSEIF(ARCHITECTURE MATCHES "aarch64")
            SET(CompilerArch "aarch64")
    ELSEIF(ARCHITECTURE MATCHES "x86_64-")
            SET(CompilerArch "x64")
    ENDIF()
ENDIF()

##Windows openMP支持
##IF(WIN32)
##    FIND_PACKAGE( OpenMP REQUIRED)
##    if(OPENMP_FOUND)
##        OPTION(USE_OPENMP "OPENMP " OFF)
##        IF(USE_OPENMP)
##            message("OPENMP FOUND")
##            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
##            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
##            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
##        ENDIF()
##    ELSE()
##        message("OPENMP NOT FOUND")
##    endif()
##ENDIF()

IF(CompilerArch MATCHES "x86_x64" OR CompilerArch MATCHES "x86_64")
	SET(CompilerArch
		"x64")
ENDIF()

######################使用多处理器编译########################
IF (WIN32)
		if(MSVC)
				OPTION(USE_MP "use multiple" ON)
				OPTION(ProjectConfig_Global_COMPILE_FLAGS_WITH_MP 
					"Set The Global Option COMPILE_FLAGS /MP to target." ON)
				if(ProjectConfig_Global_COMPILE_FLAGS_WITH_MP OR USE_MP)
					set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
					set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
				endif()
				set(VS_STARTUP_PROJECT ${PROJECT_NAME})
		ENDIF(MSVC)
ENDIF()

## 系统检测
message("=================")
message("OSName:${OSName}")
message("CompilerName:${CompilerName}")
message("CompilerVersion:${CompilerVersion}")
message("CompilerArch:${CompilerArch}")
