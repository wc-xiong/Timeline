colormsg(_HIBLUE_ "---Start---Configuring UICommon library:")
########################################################
## 包含源文件
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/uicommon.cmake)
## 项目分组
SET (PROJECT_FOLDER 
    Common
    )

## 生成动态库
WIS_QT_INIT_LIBRARY_MODULE(TRUE FALSE)
SET(${PROJECTNAMEU}_TARGET_LINK ${${PROJECTNAMEU}_TARGET_LINK} PARENT_SCOPE)
colormsg(_HIBLUE_ "---End---")