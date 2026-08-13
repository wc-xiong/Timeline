
## 加载项目名称和项目依赖项的文件
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/uicommon_dependencies.cmake)
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
    ${CMAKE_CURRENT_LIST_DIR}/uicommon_dependencies.cmake
	${CMAKE_CURRENT_LIST_DIR}/uicommon.cmake
)

source_group("DockWidget" FILES "${CMAKE_CURRENT_LIST_DIR}/DockWidget/ads_globals.cpp")
source_group("DockWidget" FILES "${CMAKE_CURRENT_LIST_DIR}/DockWidget/ads_globals.h")


source_group("DockWidget" FILES "${CMAKE_CURRENT_LIST_DIR}/DockWidget/DockAreaTabBar.cpp")
source_group("DockWidget" FILES "${CMAKE_CURRENT_LIST_DIR}/DockWidget/DockAreaTabBar.h")
source_group("DockWidget" FILES "${CMAKE_CURRENT_LIST_DIR}/DockWidget/DockTitleBar.cpp")
source_group("DockWidget" FILES "${CMAKE_CURRENT_LIST_DIR}/DockWidget/DockTitleBar.h")

source_group("DockWidget" FILES "${CMAKE_CURRENT_LIST_DIR}/DockWidget/DockAreaWidget.cpp")
source_group("DockWidget" FILES "${CMAKE_CURRENT_LIST_DIR}/DockWidget/DockAreaWidget.h")

source_group("DockWidget" FILES "${CMAKE_CURRENT_LIST_DIR}/DockWidget/DockContainerWidget.cpp")
source_group("DockWidget" FILES "${CMAKE_CURRENT_LIST_DIR}/DockWidget/DockContainerWidget.h")

source_group("DockWidget" FILES "${CMAKE_CURRENT_LIST_DIR}/DockWidget/DockManager.cpp")
source_group("DockWidget" FILES "${CMAKE_CURRENT_LIST_DIR}/DockWidget/DockManager.h")

source_group("DockWidget" FILES "${CMAKE_CURRENT_LIST_DIR}/DockWidget/DockSplitter.cpp")
source_group("DockWidget" FILES "${CMAKE_CURRENT_LIST_DIR}/DockWidget/DockSplitter.h")

source_group("DockWidget" FILES "${CMAKE_CURRENT_LIST_DIR}/DockWidget/DockWidget.cpp")
source_group("DockWidget" FILES "${CMAKE_CURRENT_LIST_DIR}/DockWidget/DockWidget.h")
source_group("DockWidget" FILES "${CMAKE_CURRENT_LIST_DIR}/DockWidget/DockWidgetTab.cpp")
source_group("DockWidget" FILES "${CMAKE_CURRENT_LIST_DIR}/DockWidget/DockWidgetTab.h")

source_group("DockWidget" FILES "${CMAKE_CURRENT_LIST_DIR}/DockWidget/FloatDockTitleBar.cpp")
source_group("DockWidget" FILES "${CMAKE_CURRENT_LIST_DIR}/DockWidget/FloatDockTitleBar.h")

source_group("DockWidget" FILES "${CMAKE_CURRENT_LIST_DIR}/DockWidget/DockDragDropWidget.cpp")
source_group("DockWidget" FILES "${CMAKE_CURRENT_LIST_DIR}/DockWidget/DockDragDropWidget.h")