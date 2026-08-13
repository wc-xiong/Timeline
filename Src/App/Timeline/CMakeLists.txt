colormsg(_HIBLUE_ "---Start---Configuring Timeline App")

project(${IDE_MAIN_APP_TARGET})

ADD_DEFINITIONS(-DIDE_LIBRARY_BASENAME="lib")
ADD_DEFINITIONS(-DIDE_BINARY_BASENAME="bin")
ADD_DEFINITIONS(
    -DTIMELINEMODEL_STATIC_LIB
    -DTIMELINEWND_STATIC_LIB
    -DUICOMMON_STATIC_LIB
    -DSIGNALCENTER_STATIC_LIB
    -DGLOBALMODEL_STATIC_LIB
    -DCOMMON_STATIC_LIB
    -DPLAYERCONTROLLER_STATIC_LIB
    -DSELECTCONTROLLER_STATIC_LIB
    -DPLAYCOMMANDCENTER_STATIC_LIB
    -DGLOBALHISTORYSTACK_STATIC_LIB
    -DVIEWDATAMANAGER_STATIC_LIB
    -DPLAYSTATECENTER_STATIC_LIB
    -DMEDIAMODEL_STATIC_LIB
    -DRESOURCEMODEL_STATIC_LIB
    -DRESOURCEWND_STATIC_LIB
    -DDEMUXER_STATIC_LIB
    -DSCREENMODEL_STATIC_LIB
    -DMODELMODEL_STATIC_LIB
    -DTIMESERVER_STATIC_LIB
    -DTHREADMANAGER_STATIC_LIB
    -DRENDERCOMMON_STATIC_LIB
    -DRENDERDEVICE_STATIC_LIB
    -DRENDERBASE2D_STATIC_LIB
    -DRENDERITEM_STATIC_LIB
    -DPLAYCORE_STATIC_LIB
    -DSTREAMMEDIA_STATIC_LIB
    -DNETCONTROLLER_STATIC_LIB
    -DNETMATERIALTRANSFER_STATIC_LIB
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    ADD_DEFINITIONS(-DLCT_DEBUG)
endif()

set(LIBS_ROOT ${CMAKE_SOURCE_DIR}/Src/Libs)

SET(OPENCV_INCLUDEDIR ${THIRDPARTY_ROOT}/opencv2/Inc)
IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(OPENCV_LIBRARYDIR ${THIRDPARTY_ROOT}/opencv2/lib/Debug)
ELSE()
    SET(OPENCV_LIBRARYDIR ${THIRDPARTY_ROOT}/opencv2/lib/Release)
ENDIF()

SET(NVAPI_INCLUDEDIR ${THIRDPARTY_ROOT}/NVAPI)
SET(NVAPI_LIBRARYDIR ${THIRDPARTY_ROOT}/NVAPI/Lib)

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/Src
    ${CMAKE_SOURCE_DIR}/Src/Libs
    ${CMAKE_SOURCE_DIR}/Src/Libs/Common
    ${CMAKE_SOURCE_DIR}/Src/Libs/Common/UICommon
    ${CMAKE_SOURCE_DIR}/Src/Libs/UI/TimeLineWnd
    ${CMAKE_SOURCE_DIR}/Src/Libs/UI/ResourceWnd
    ${CMAKE_SOURCE_DIR}/Src/Libs/UI/PropertyWnd
    ${CMAKE_SOURCE_DIR}/Src/Libs/Model/ResourceModel
    ${CMAKE_SOURCE_DIR}/Src/Libs/Model/MediaModel
    ${CMAKE_SOURCE_DIR}/Src/Libs/GSMedia/Demuxer
    ${CMAKE_SOURCE_DIR}/Src/Libs/GSMedia/RenderDevice
    ${CMAKE_SOURCE_DIR}/Src/Libs/GSMedia/RenderBase2D
    ${CMAKE_SOURCE_DIR}/Src/Libs/GSMedia/RenderItem
    ${CMAKE_SOURCE_DIR}/Src/Libs/GSMedia/RenderCommon
    ${CMAKE_SOURCE_DIR}/Src/Libs/Play/PlayCore
    ${CMAKE_SOURCE_DIR}/Src/Libs/Play/PlayCore/VideoDecode
    ${CMAKE_SOURCE_DIR}/Src/Libs/Controll
    ${RAPIDJSON_INCLUDEDIR}
    ${FFMPEG_INCLUDEDIR}
    ${SDL_INCLUDEDIR}
    ${OPENCV_INCLUDEDIR}
    ${NVAPI_INCLUDEDIR}
    ${THIRDPARTY_ROOT}/DirectXTex/Inc
)

FILE(GLOB APP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
FILE(GLOB APP_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
FILE(GLOB_RECURSE TLW_SOURCES "${LIBS_ROOT}/UI/TimeLineWnd/*.cpp")
FILE(GLOB_RECURSE TLM_SOURCES "${LIBS_ROOT}/Model/TimeLineModel/*.cpp")
FILE(GLOB_RECURSE UIC_SOURCES "${LIBS_ROOT}/Common/UICommon/*.cpp")
FILE(GLOB SC_SOURCES "${LIBS_ROOT}/Common/SignalCenter/*.cpp")
FILE(GLOB COMMON_SOURCES "${LIBS_ROOT}/Common/Common/*.cpp")
FILE(GLOB STUB_SOURCES "${LIBS_ROOT}/Stubs/*.cpp")
FILE(GLOB_RECURSE RW_SOURCES "${LIBS_ROOT}/UI/ResourceWnd/*.cpp")
FILE(GLOB_RECURSE RM_SOURCES "${LIBS_ROOT}/Model/ResourceModel/*.cpp")
FILE(GLOB_RECURSE DEMUX_SOURCES "${LIBS_ROOT}/GSMedia/Demuxer/*.cpp")
FILE(GLOB_RECURSE MM_SOURCES "${LIBS_ROOT}/Model/MediaModel/*.cpp")
FILE(GLOB PW_SOURCES "${LIBS_ROOT}/UI/PropertyWnd/*.cpp")

# GPU 管线阶段0/1：RenderDevice + PlayCore（硬解）+ RenderCommon 基础
SET(RENDERDEVICE_SOURCES
    ${LIBS_ROOT}/GSMedia/RenderDevice/DeviceResource.cpp
    ${LIBS_ROOT}/GSMedia/RenderDevice/D3DWrapper.cpp
    ${LIBS_ROOT}/GSMedia/RenderDevice/GlobalData.cpp
    ${LIBS_ROOT}/GSMedia/RenderDevice/pch.cpp
)
SET(RENDERCOMMON_GPU_SOURCES
    ${LIBS_ROOT}/GSMedia/RenderCommon/SimpleMath.cpp
    ${LIBS_ROOT}/GSMedia/RenderCommon/DXTrace.cpp
    ${LIBS_ROOT}/GSMedia/RenderCommon/Vertex.cpp
    ${LIBS_ROOT}/GSMedia/RenderCommon/RenderStates.cpp
    ${LIBS_ROOT}/GSMedia/RenderCommon/TextureRender.cpp
    ${LIBS_ROOT}/GSMedia/RenderCommon/Model.cpp
    ${LIBS_ROOT}/GSMedia/RenderCommon/EffectHelper.cpp
    ${LIBS_ROOT}/GSMedia/RenderCommon/CommonFunction.cpp
    ${LIBS_ROOT}/GSMedia/RenderCommon/WICTextureLoader.cpp
)
# 阶段3：BasicEffect + ClipFx 渲染（不含 Custom2D/Grid）
SET(RENDERBASE2D_GPU_SOURCES
    ${LIBS_ROOT}/GSMedia/RenderBase2D/BasicEffect.cpp
    ${LIBS_ROOT}/GSMedia/RenderBase2D/pch.cpp
)
SET(RENDERITEM_GPU_SOURCES
    ${LIBS_ROOT}/GSMedia/RenderItem/D3DEffectRender.cpp
    ${LIBS_ROOT}/GSMedia/RenderItem/VertexCalculator.cpp
)
FILE(GLOB_RECURSE PLAYCORE_SOURCES "${LIBS_ROOT}/Play/PlayCore/*.cpp")

SET(GM_SOURCES
    ${LIBS_ROOT}/Model/GlobalModel/PrjSettingData.cpp
    ${LIBS_ROOT}/Model/GlobalModel/SoftwareSettingData.cpp
    ${LIBS_ROOT}/Model/GlobalModel/AppPath.cpp
    ${LIBS_ROOT}/Model/GlobalModel/DisplayerDataMgr.cpp
)

SET(PLAYSTATE_SOURCES
    ${LIBS_ROOT}/Controll/PlayStateCenter/TimelinePropertyKeyframeHelper.cpp
    ${LIBS_ROOT}/Controll/PlayStateCenter/TimelineFilterHelper.cpp
)

SET(MOC_EXTRA_HEADERS
    ${LIBS_ROOT}/Controll/PlayerController/PlayerController.h
    ${LIBS_ROOT}/Controll/SelectController/SelectController.h
    ${LIBS_ROOT}/Common/UICommon/DockWidgetEx.h
    ${LIBS_ROOT}/Common/UICommon/CalculatorDialog.h
    ${LIBS_ROOT}/UI/PropertyWnd/PropertyFrame.h
    ${LIBS_ROOT}/UI/PropertyWnd/MediaPropertyFrame.h
    ${LIBS_ROOT}/UI/PropertyWnd/TimeLinePropertyFrame.h
    ${LIBS_ROOT}/UI/PropertyWnd/TimeLineTransFxPropertyFrame.h
    ${LIBS_ROOT}/UI/PropertyWnd/TimelinePropertyHost.h
)

SET(SOURCES
    ${APP_SOURCES}
    ${TLW_SOURCES}
    ${TLM_SOURCES}
    ${UIC_SOURCES}
    ${SC_SOURCES}
    ${COMMON_SOURCES}
    ${STUB_SOURCES}
    ${GM_SOURCES}
    ${PLAYSTATE_SOURCES}
    ${RW_SOURCES}
    ${RM_SOURCES}
    ${DEMUX_SOURCES}
    ${MM_SOURCES}
    ${PW_SOURCES}
    ${MOC_EXTRA_HEADERS}
    ${RENDERDEVICE_SOURCES}
    ${RENDERCOMMON_GPU_SOURCES}
    ${RENDERBASE2D_GPU_SOURCES}
    ${RENDERITEM_GPU_SOURCES}
    ${PLAYCORE_SOURCES}
)

SET(HEADERS ${APP_HEADERS})

SET(RESOURCES
    ${CMAKE_SOURCE_DIR}/Src/Libs/GSResource/timeline_resource.qrc
)
set(CMAKE_AUTORCC ON)

SET(OTHER_FILES CMakeLists.txt)

# Mapping 同构：HLSL 编入目标，FXC 输出到 BIN_DIR/HLSL/*.cso（与 exe 同级）
SET(_TL_HLSL_DIR "${LIBS_ROOT}/GSMedia/RenderBase2D/HLSL")
SET(_TL_HLSL_SHADER_SOURCES
	${_TL_HLSL_DIR}/VS_Basic.hlsl
	${_TL_HLSL_DIR}/VS_3D.hlsl
	${_TL_HLSL_DIR}/VS_2D.hlsl
	${_TL_HLSL_DIR}/VS_PNT.hlsl
	${_TL_HLSL_DIR}/PS_Basic.hlsl
	${_TL_HLSL_DIR}/PS_PhwNT.hlsl
	${_TL_HLSL_DIR}/PS_2D.hlsl
	${_TL_HLSL_DIR}/PS_3D.hlsl
	${_TL_HLSL_DIR}/XEffect.hlsl
	${_TL_HLSL_DIR}/VS_Tessellation.hlsl
	${_TL_HLSL_DIR}/HS_Transfer.hlsl
	${_TL_HLSL_DIR}/DS_Liner.hlsl
)
FILE(GLOB _TL_HLSL_HEADERS "${_TL_HLSL_DIR}/*.hlsli")
LIST(APPEND OTHER_FILES ${_TL_HLSL_SHADER_SOURCES} ${_TL_HLSL_HEADERS})
FILE(MAKE_DIRECTORY "${BIN_DIR}/HLSL")
set_source_files_properties(${_TL_HLSL_DIR}/VS_Basic.hlsl PROPERTIES VS_SHADER_OBJECT_FILE_NAME "${BIN_DIR}/HLSL/%(Filename).cso" VS_SHADER_TYPE Vertex VS_SHADER_MODEL 5.0 VS_SHADER_ENTRYPOINT VS)
set_source_files_properties(${_TL_HLSL_DIR}/VS_3D.hlsl PROPERTIES VS_SHADER_OBJECT_FILE_NAME "${BIN_DIR}/HLSL/%(Filename).cso" VS_SHADER_TYPE Vertex VS_SHADER_MODEL 5.0 VS_SHADER_ENTRYPOINT VS)
set_source_files_properties(${_TL_HLSL_DIR}/VS_2D.hlsl PROPERTIES VS_SHADER_OBJECT_FILE_NAME "${BIN_DIR}/HLSL/%(Filename).cso" VS_SHADER_TYPE Vertex VS_SHADER_MODEL 5.0 VS_SHADER_ENTRYPOINT VS)
set_source_files_properties(${_TL_HLSL_DIR}/VS_PNT.hlsl PROPERTIES VS_SHADER_OBJECT_FILE_NAME "${BIN_DIR}/HLSL/%(Filename).cso" VS_SHADER_TYPE Vertex VS_SHADER_MODEL 5.0 VS_SHADER_ENTRYPOINT VS)
set_source_files_properties(${_TL_HLSL_DIR}/PS_Basic.hlsl PROPERTIES VS_SHADER_OBJECT_FILE_NAME "${BIN_DIR}/HLSL/%(Filename).cso" VS_SHADER_TYPE Pixel VS_SHADER_MODEL 5.0 VS_SHADER_ENTRYPOINT PS)
set_source_files_properties(${_TL_HLSL_DIR}/PS_PhwNT.hlsl PROPERTIES VS_SHADER_OBJECT_FILE_NAME "${BIN_DIR}/HLSL/%(Filename).cso" VS_SHADER_TYPE Pixel VS_SHADER_MODEL 5.0 VS_SHADER_ENTRYPOINT PS)
set_source_files_properties(${_TL_HLSL_DIR}/PS_2D.hlsl PROPERTIES VS_SHADER_OBJECT_FILE_NAME "${BIN_DIR}/HLSL/%(Filename).cso" VS_SHADER_TYPE Pixel VS_SHADER_MODEL 5.0 VS_SHADER_ENTRYPOINT PS)
set_source_files_properties(${_TL_HLSL_DIR}/PS_3D.hlsl PROPERTIES VS_SHADER_OBJECT_FILE_NAME "${BIN_DIR}/HLSL/%(Filename).cso" VS_SHADER_TYPE Pixel VS_SHADER_MODEL 5.0 VS_SHADER_ENTRYPOINT PS)
set_source_files_properties(${_TL_HLSL_DIR}/XEffect.hlsl PROPERTIES VS_SHADER_OBJECT_FILE_NAME "${BIN_DIR}/HLSL/%(Filename).cso" VS_SHADER_TYPE Pixel VS_SHADER_MODEL 5.0 VS_SHADER_ENTRYPOINT PS)
set_source_files_properties(${_TL_HLSL_DIR}/VS_Tessellation.hlsl PROPERTIES VS_SHADER_OBJECT_FILE_NAME "${BIN_DIR}/HLSL/%(Filename).cso" VS_SHADER_TYPE Vertex VS_SHADER_MODEL 5.0 VS_SHADER_ENTRYPOINT VS)
set_source_files_properties(${_TL_HLSL_DIR}/HS_Transfer.hlsl PROPERTIES VS_SHADER_OBJECT_FILE_NAME "${BIN_DIR}/HLSL/%(Filename).cso" VS_SHADER_TYPE Hull VS_SHADER_MODEL 5.0 VS_SHADER_ENTRYPOINT HS)
set_source_files_properties(${_TL_HLSL_DIR}/DS_Liner.hlsl PROPERTIES VS_SHADER_OBJECT_FILE_NAME "${BIN_DIR}/HLSL/%(Filename).cso" VS_SHADER_TYPE Domain VS_SHADER_MODEL 5.0 VS_SHADER_ENTRYPOINT DS)

IF(WIN32)
    LIST(APPEND SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/Timeline.rc
    )
    LIST(APPEND OTHER_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/Timeline.ico
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/Timeline.svg
    )
ENDIF()

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    LINK_DIRECTORIES(
        ${THIRDPARTY_ROOT}/boost/lib/debug
        ${THIRDPARTY_ROOT}/boost/include
        ${THIRDPARTY_ROOT}/DirectXTex/lib/Debug
        ${OPENCV_LIBRARYDIR}
        ${NVAPI_LIBRARYDIR}
    )
ELSE()
    LINK_DIRECTORIES(
        ${THIRDPARTY_ROOT}/boost/lib/release
        ${THIRDPARTY_ROOT}/boost/include
        ${THIRDPARTY_ROOT}/DirectXTex/lib/Release
        ${OPENCV_LIBRARYDIR}
        ${NVAPI_LIBRARYDIR}
    )
ENDIF()

SET(CMAKE_CXX_STANDARD 17)

SET(QT_MODULES
    Gui
    Widgets
    Svg
    Xml
    Network
    Concurrent
)

set(DEPENDENCIES)
SET(WIS_MODULE_DEPS)

LINK_DIRECTORIES(
    ${PROJECT_BINARY_DIR}
    ${LIBRARY_OUTPUT_PATH}
    ${FFMPEG_LIBRARYDIR}
    ${SDL_LIBRARYDIR}
    ${THIRDPARTY_ROOT}/DirectXTex/lib/Debug
    ${THIRDPARTY_ROOT}/DirectXTex/lib/Release
    ${OPENCV_LIBRARYDIR}
    ${NVAPI_LIBRARYDIR}
)

SET(PROJECT_FOLDER APP)

WIS_QT_INIT_EXE_MODULE(FALSE FALSE)

SET(${PROJECTNAMEU}_TARGET_LINK ${${PROJECTNAMEU}_TARGET_LINK} PARENT_SCOPE)
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/level='asInvoker' /uiAccess='false'")

# Visual Studio 筛选器：按 Src 下真实目录树分类（SOURCE_GROUP TREE）
# 补齐各模块头文件，便于在 VS 中按目录浏览（不改变链接集合）
FILE(GLOB_RECURSE _TL_VS_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/*.h"
	"${LIBS_ROOT}/UI/TimeLineWnd/*.h"
	"${LIBS_ROOT}/UI/ResourceWnd/*.h"
	"${LIBS_ROOT}/UI/PropertyWnd/*.h"
	"${LIBS_ROOT}/Model/TimeLineModel/*.h"
	"${LIBS_ROOT}/Model/ResourceModel/*.h"
	"${LIBS_ROOT}/Model/MediaModel/*.h"
	"${LIBS_ROOT}/Model/GlobalModel/*.h"
	"${LIBS_ROOT}/Common/UICommon/*.h"
	"${LIBS_ROOT}/Common/SignalCenter/*.h"
	"${LIBS_ROOT}/Common/Common/*.h"
	"${LIBS_ROOT}/Stubs/*.h"
	"${LIBS_ROOT}/Controll/PlayStateCenter/*.h"
	"${LIBS_ROOT}/Controll/PlayerController/*.h"
	"${LIBS_ROOT}/Controll/SelectController/*.h"
	"${LIBS_ROOT}/GSMedia/Demuxer/*.h"
	"${LIBS_ROOT}/GSMedia/RenderDevice/*.h"
	"${LIBS_ROOT}/GSMedia/RenderCommon/*.h"
	"${LIBS_ROOT}/GSMedia/RenderBase2D/*.h"
	"${LIBS_ROOT}/GSMedia/RenderItem/*.h"
	"${LIBS_ROOT}/Play/PlayCore/*.h"
)
SET(_TL_VS_FILTER_FILES ${SOURCES} ${HEADERS} ${_TL_VS_HEADERS} ${OTHER_FILES} ${RESOURCES})
# 仅保留位于 Src 树下的文件，满足 SOURCE_GROUP(TREE) 约束
SET(_TL_VS_TREE_FILES "")
FOREACH(_f IN LISTS _TL_VS_FILTER_FILES)
	FILE(TO_CMAKE_PATH "${_f}" _f_norm)
	STRING(FIND "${_f_norm}" "${CMAKE_SOURCE_DIR}/Src" _idx)
	IF(_idx EQUAL 0)
		LIST(APPEND _TL_VS_TREE_FILES "${_f_norm}")
	ENDIF()
ENDFOREACH()
LIST(REMOVE_DUPLICATES _TL_VS_TREE_FILES)
IF(_TL_VS_TREE_FILES)
	SOURCE_GROUP(TREE "${CMAKE_SOURCE_DIR}/Src" PREFIX "Src" FILES ${_TL_VS_TREE_FILES})
	# 头文件加入目标仅用于 VS 显示/浏览，不参与额外编译规则
	TARGET_SOURCES(${PROJECT_NAME} PRIVATE ${_TL_VS_HEADERS})
ENDIF()

# PlayCore / RenderDevice 额外系统库（与 wisQt 一致用 plain 签名）
TARGET_LINK_LIBRARIES(${PROJECT_NAME}
    d3d11 dxgi dxguid d3dcompiler d2d1 dwrite winmm dwmapi
)

# 部署 HLSL：FXC 已输出 .cso 到 BIN_DIR/HLSL；此处再拷源码树（.hlsli / convert / .hlsl 回退）
# 与 Mapping 一致：运行时目录为 exe 旁 HLSL\（优先 .cso）
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/HLSL"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${BIN_DIR}/HLSL"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${_TL_HLSL_DIR}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/HLSL"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${_TL_HLSL_DIR}"
        "${BIN_DIR}/HLSL"
    COMMENT "Deploy HLSL sources (+keep FXC .cso) next to Timeline.exe"
)

colormsg(_HIBLUE_ "---End---")
