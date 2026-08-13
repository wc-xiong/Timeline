###############################################################################
# Timeline - Qt timeline editor
###############################################################################
CMAKE_MINIMUM_REQUIRED(VERSION 3.16.0 FATAL_ERROR)
SET(CMAKE_CXX_STANDARD 17)

# 优先使用工程内 3rdparty（相对源码树），避免继承环境变量中的外部路径
SET(_TL_BUNDLED_3RDPARTY "${CMAKE_SOURCE_DIR}/3rdparty")
IF(EXISTS "${_TL_BUNDLED_3RDPARTY}/ffmpeg" OR EXISTS "${_TL_BUNDLED_3RDPARTY}")
    SET(THIRDPARTY_ROOT "${_TL_BUNDLED_3RDPARTY}" CACHE PATH "3rdparty root directory" FORCE)
ELSEIF("${THIRDPARTY_ROOT}" STREQUAL "")
    FILE(TO_CMAKE_PATH "$ENV{THIRDPARTY_ROOT}" THIRDPARTY_ROOT)
ENDIF()
IF("${THIRDPARTY_ROOT}" STREQUAL "")
    MESSAGE(FATAL_ERROR "THIRDPARTY_ROOT not found. Place libs under ${_TL_BUNDLED_3RDPARTY} or pass -DTHIRDPARTY_ROOT=...")
ENDIF()
UNSET(_TL_BUNDLED_3RDPARTY)

PROJECT("Timeline")
SET(IDE_MAIN_APP_TARGET "Timeline")
ADD_DEFINITIONS(-DIDE_APP_TARGET=\"${IDE_MAIN_APP_TARGET}\")

IF(MSVC)
    ADD_COMPILE_OPTIONS(/utf-8)
    # 修复 QStringLiteral 中文乱码（MSVC 拼接 u"" + 窄串缺陷）
    # 注意：/FI 路径勿再包引号，否则 cl 会把引号算进文件名
    ADD_COMPILE_OPTIONS(/FI${CMAKE_SOURCE_DIR}/Src/Libs/Common/Common/MsvcQStringLiteralFix.h)
ENDIF()

SET(CMAKE_MODULE_PATH
    ${CMAKE_SOURCE_DIR}/cmake
    ${CMAKE_MODULE_PATH}
)

INCLUDE(wisConfig)
INCLUDE(wisUtilities)
ADD_DEFINITIONS(-D_UNICODE -DUNICODE)
INCLUDE(wisCPPProjUtilities)
INCLUDE(wisFindQt)

SET(BOOST_COMPONENTS system filesystem program_options random)
INCLUDE(FindWISBoost)

IF(NOT INSOURCEBUILD AND (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR}))
    MESSAGE(FATAL_ERROR "In-source builds are not allowed. Use build/Debug or build/Release.")
ENDIF()

# Out-of-source: -B build/Debug|Release → BIN/LIB = build/bin|lib；bat 再同步到仓库根 bin/
SET(BIN_DIR ${CMAKE_BINARY_DIR}/../bin)
SET(LIB_DIR ${CMAKE_BINARY_DIR}/../lib)
SET(LIBRARY_OUTPUT_PATH ${LIB_DIR})
SET(EXECUTABLE_OUTPUT_PATH ${BIN_DIR})
MESSAGE(STATUS "BIN_DIR=${BIN_DIR}")
MESSAGE(STATUS "LIB_DIR=${LIB_DIR}")
MESSAGE(STATUS "CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}")

# Runtime deps used by play/render later
INCLUDE(FindFFmpeg)
INCLUDE(FindSDL)
INCLUDE(FindRapidjson)
INCLUDE(FindOpenCV)

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Src
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/Libs
    ${CMAKE_BINARY_DIR}
    ${THIRDPARTY_ROOT}
    ${BOOST_INCLUDEDIR}
)

LINK_DIRECTORIES(
    ${LIB_DIR}
)

IF(MSVC)
    ADD_DEFINITIONS(/W3)
ENDIF()

ADD_SUBDIRECTORY(Src)

MESSAGE(STATUS "THIRDPARTY_ROOT=${THIRDPARTY_ROOT}")
MESSAGE(STATUS "BOOST_INCLUDEDIR=${BOOST_INCLUDEDIR}")
