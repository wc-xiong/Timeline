#
# 项目名称
#
SET(WIS_PROJECT_NAME decodercontrol)

#
# 头文件路径
#
INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_LIST_DIR}/../../Common
)

#
# 依赖非Qt库，非本项库
#
set(DEPENDENCIES
	signalcenter
    renderdevice
    texturecenter
    player
    streammedia
    audiorender
    common
    mediamodel
    resourcemodel
    )
#
# 依赖的Qt库
#
SET(QT_MODULES
    Core
    Xml
    Gui
    )

#
# 依赖的本项目内的库
#
SET(WIS_MODULE_DEPS
    timelinemodel
    playstatecenter
    screenmodel
    )
