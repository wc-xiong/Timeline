#
# 项目名称
# 
SET(WIS_PROJECT_NAME signalcenter)
#
# 头文件路径
#
INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_LIST_DIR}/../Common/
)
#
#lib搜索路径
#
LINK_DIRECTORIES(
	${PROJECT_BINARY_DIR}
)


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
    )
	
#
# 依赖的本项目内的库
#
SET(WIS_MODULE_DEPS

    )
