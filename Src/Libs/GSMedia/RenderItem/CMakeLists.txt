colormsg(_HIBLUE_ "---Start---Configuring RenderItem library:")
########################################################
## 包含源文件
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/renderitem.cmake)
## 项目分组
SET (PROJECT_FOLDER
		GSMedia
    )
## 生成动态库
WIS_QT_INIT_LIBRARY_MODULE(FALSE FALSE)
SET(${PROJECTNAMEU}_TARGET_LINK ${${PROJECTNAMEU}_TARGET_LINK} PARENT_SCOPE)
colormsg(_HIBLUE_ "---End---")