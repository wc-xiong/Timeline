
## 加载项目名称和项目依赖项的文件
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/rendercommon_dependencies.cmake)
## 项目名称
PROJECT(${WIS_PROJECT_NAME})

INCLUDE_DIRECTORIES(
)

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
    ${CMAKE_CURRENT_LIST_DIR}/rendercommon_dependencies.cmake
	${CMAKE_CURRENT_LIST_DIR}/rendercommon.cmake
)