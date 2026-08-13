
## 加载项目名称和项目依赖项的文件
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/baseclasses_dependencies.cmake)
## 项目名称
PROJECT(${WIS_PROJECT_NAME})

INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_LIST_DIR}/../../Common
)

# Source files.
FILE(GLOB_RECURSE SOURCES "${THIRDPARTY_ROOT}/BaseClasses/*.cpp")
#####################################################
# Header files.
FILE(GLOB_RECURSE HEADERS "${THIRDPARTY_ROOT}/BaseClasses/*.h")
#####################################################
# Resource files.
SET (RESOURCES
)
########################################################
# Other files
SET(OTHER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/baseclasses_dependencies.cmake
	${CMAKE_CURRENT_LIST_DIR}/baseclasses.cmake
)