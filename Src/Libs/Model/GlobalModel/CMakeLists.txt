colormsg(_HIBLUE_ "---Start---Configuring globalmodel library:")
########################################################
## 包含源文件
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/globalmodel.cmake)
## 项目分组
SET (PROJECT_FOLDER 
    Model
    )
## 生成动态库
WIS_QT_INIT_LIBRARY_MODULE(FALSE FALSE)
SET(${PROJECTNAMEU}_TARGET_LINK ${${PROJECTNAMEU}_TARGET_LINK} PARENT_SCOPE)

IF(MSVC)
	TARGET_LINK_OPTIONS(globalmodel PRIVATE "/DELAYLOAD:${OPENCV_DLL}")
ENDIF()

colormsg(_HIBLUE_ "---End---")