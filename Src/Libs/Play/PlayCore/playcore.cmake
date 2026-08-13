
## 加载项目名称和项目依赖项的文件
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/playcore_dependencies.cmake)
## 项目名称
PROJECT(${WIS_PROJECT_NAME})

# Source files.
FILE(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
#####################################################
# Header files.
FILE(GLOB_RECURSE HEADERS "${CMAKE_CURRENT_LIST_DIR}/*.h")
#####################################################
# Resource files.
SET (RESOURCES
)
########################################################
# Other files
SET(OTHER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/playcore_dependencies.cmake
	${CMAKE_CURRENT_LIST_DIR}/playcore.cmake
)

source_group("VideoDecode" FILES "${CMAKE_CURRENT_LIST_DIR}/VideoDecode/AVDecoder.h")
source_group("VideoDecode" FILES "${CMAKE_CURRENT_LIST_DIR}/VideoDecode/AVDecoder.cpp")
source_group("VideoDecode" FILES "${CMAKE_CURRENT_LIST_DIR}/VideoDecode/PlayCoreInclude.h")
source_group("VideoDecode" FILES "${CMAKE_CURRENT_LIST_DIR}/VideoDecode/VideoDecoder.h")
source_group("VideoDecode" FILES "${CMAKE_CURRENT_LIST_DIR}/VideoDecode/VideoDecoder.cpp")
source_group("VideoDecode" FILES "${CMAKE_CURRENT_LIST_DIR}/VideoDecode/VideoDecoderD3D.h")
source_group("VideoDecode" FILES "${CMAKE_CURRENT_LIST_DIR}/VideoDecode/VideoDecoderD3D.cpp")
source_group("VideoDecode" FILES "${CMAKE_CURRENT_LIST_DIR}/VideoDecode/VideoDecoderD3D11.h")
source_group("VideoDecode" FILES "${CMAKE_CURRENT_LIST_DIR}/VideoDecode/VideoDecoderD3D11.cpp")
source_group("VideoDecode" FILES "${CMAKE_CURRENT_LIST_DIR}/VideoDecode/VideoDecoderHW.h")
source_group("VideoDecode" FILES "${CMAKE_CURRENT_LIST_DIR}/VideoDecode/VideoDecoderHW.cpp")
source_group("VideoDecode" FILES "${CMAKE_CURRENT_LIST_DIR}/VideoDecode/VideoDecoderSW.h")
source_group("VideoDecode" FILES "${CMAKE_CURRENT_LIST_DIR}/VideoDecode/VideoDecoderSW.cpp")
source_group("VideoDecode" FILES "${CMAKE_CURRENT_LIST_DIR}/VideoDecode/VideoFormat.h")
source_group("VideoDecode" FILES "${CMAKE_CURRENT_LIST_DIR}/VideoDecode/VideoFormat.cpp")