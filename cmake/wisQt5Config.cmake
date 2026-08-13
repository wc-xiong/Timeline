# Qt5
FIND_PACKAGE(Qt5Core)
FIND_PACKAGE(Qt5Concurrent)
FIND_PACKAGE(Qt5LinguistTools)

#翻译工具
SET(LUPDATE ${_qt5_linguisttools_install_prefix}/bin/lupdate)
SET(LUPDATE_OPTIONS -locations absolute -no-ui-lines -no-sort)
SET(LRELEASE ${_qt5_linguisttools_install_prefix}/bin/lrelease)
SET(LCONVERT ${_qt5_linguisttools_install_prefix}/bin/lconvert)
SET(XMLPATTERNS ${_qt5XmlPatterns_install_prefix}/bin/xmlpatterns)
SET(RCC ${_qt5_linguisttools_install_prefix}/bin/rcc)

ADD_DEFINITIONS(-DQT_NO_CAST_TO_ASCII)
ADD_DEFINITIONS("-DQT_DISABLE_DEPRECATED_BEFORE=0x040900")
IF (MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
ENDIF ()
IF (NOT APPLE)
    ADD_DEFINITIONS(-DQT_USE_FAST_OPERATOR_PLUS -DQT_USE_FAST_CONCATENATION)
ENDIF ()
## 多核编译
IF (WIN32)
    IF (MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    ENDIF ()
ENDIF ()
## 全局变量标识是否是辅助应用程序
SET(IS_TOOL_APP 
    FALSE)
### Using Qt 5 with CMake older than 2.8.9########################
# Add compiler flags for building executables (-fPIE)
###set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

function (glob_recurse_first _globbingExpression _resultVar)
    file (GLOB_RECURSE _pathList "${_globbingExpression}")
    # canonical order
    list (SORT _pathList)
    # filter first match from each directory
    set (_firstMatched "")
    set (_latestDir "//")
    foreach (_path IN LISTS _pathList)
        get_filename_component(_dir "${_path}" DIRECTORY)
        if (NOT "${_dir}" STREQUAL "${_latestDir}")
            list (APPEND _firstMatched "${_path}")
            set (_latestDir "${_dir}")
        endif()
    endforeach()
    set (${_resultVar} ${_firstMatched} PARENT_SCOPE)
endfunction()

FUNCTION(WIS_QT_INIT_COMMON_LIB
        INIT_TYPE
        HAS_QM
        MERGE_QM)
	MESSAGE("INIT_TYPE = ${INIT_TYPE}, HAS_QM = ${HAS_QM}, MERGE_QM = ${MERGE_QM}")
    IF (EXISTS _UNICODE)
        ## Qt工程采用Unicode
        ADD_DEFINITIONS(-D_UNICODE -DUNICODE)
    ENDIF ()
    #包含头文件
    INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})
    #项目名称全打写 全小写
    IF (${INIT_TYPE} STREQUAL "EXE" OR ${INIT_TYPE} STREQUAL "CONSOLE") ##生成可执行程序
        STRING(TOUPPER "${PROJECT_NAME}" PROJECTNAMEU)
        SET(PROJECTNAMEL
                ${PROJECT_NAME})
    ELSE()
		# 如果指定采用默认项目名称，则不强制将项目名称修改为小写
		IF(RUN_NORMAL_LIBRARY_NAME)
			STRING(TOUPPER "${PROJECT_NAME}" PROJECTNAMEU)
			SET(PROJECTNAMEL
					${PROJECT_NAME})
		ELSE()
			STRING(TOLOWER "${PROJECT_NAME}" PROJECTNAMEL)
			STRING(TOUPPER "${PROJECT_NAME}" PROJECTNAMEU)
		ENDIF()
    ENDIF()
    ##--------------------------------------------------------------------------
    ## 链接的Qt库
    SET(QT_LINK_DEPS
    )
    FOREACH (deps ${QT_MODULES})
        IF (NOT Qt5${deps}_FOUND)
            FIND_PACKAGE(Qt5${deps})
	    SET(CMAKE_MODULE_PATH 
		${CMAKE_MODULE_PATH}
		${QT_DIR}/lib/cmake/Qt5${deps} 
		)
        ENDIF ()
        LIST(APPEND QT_LINK_DEPS "Qt5::${deps}")
    ENDFOREACH ()
    ## 判断Qt::Core是否存在
    LIST(LENGTH QT_LINK_DEPS qt_link_deps_len)
    IF (${qt_link_deps_len} STREQUAL "0")
        LIST(APPEND QT_LINK_DEPS "Qt5::Core")
    ELSE ()
        LIST(FIND QT_LINK_DEPS "Qt5::Core" QtModuleIndex)
        IF (${QtModuleIndex} STREQUAL "-1")
            LIST(INSERT QT_LINK_DEPS 0 "Qt5::Core")
        ENDIF ()
    ENDIF ()

    ## 判断Qt::Concurrent是否存在
    LIST(FIND QT_LINK_DEPS "Qt5::Concurrent" QtModuleIndex)
    IF (${QtModuleIndex} STREQUAL "-1")
        LIST(LENGTH QT_LINK_DEPS qt_link_deps_len)
        IF (${qt_link_deps_len} STREQUAL "1")
            LIST(APPEND QT_LINK_DEPS "Qt5::Concurrent")
        ELSE ()
            LIST(INSERT QT_LINK_DEPS 1 "Qt5::Concurrent")
        ENDIF ()
    ENDIF ()

    # Header files needs to be processed by Qt's pre-processor to generate Moc files.
    #QT5_WRAP_CPP(MOC_HDS ${HEADERS})
    # UI files need to be processed by Qt wrapper to generate UI headers.
    IF (Qt5Widgets_FOUND)
        QT5_WRAP_UI(UI_HDS ${FORMS})
    ENDIF ()
    # RC files need to be processed by Qt wrapper to generate RC headers.
    QT5_ADD_RESOURCES(RCC_HDS ${RESOURCES})
    LIST(APPEND OTHER_FILES ${RESOURCES})
    
    ## TS文件生成的目录       
    string(LENGTH "${TRANSLATE_SOURCE_DIR}" TRANSLATE_SOURCE_DIRlen)
    IF(${TRANSLATE_SOURCE_DIRlen} EQUAL 0)
        SET(TRANSLATE_SOURCE_DIR
                ${PROJECT_SOURCE_DIR}
            )
    ENDIF()
        
    ## 如果有需要翻译文件
    IF (HAS_QM AND RUN_TS)
        set(LANGUAGE_LIST ${TEST})
		
		#MESSAGE(WARNING "${LANGUAGE_LIST}")

        separate_arguments(LANGUAGE_LIST)
		
        list(LENGTH LANGUAGE_LIST LanguageCnt)
        IF(${LanguageCnt} EQUAL 0) ## 没有设置要翻译的目标语言
            MESSAGE(SEND_ERROR "Error, the variable LANGUAGES is not defined!")
        ENDIF()
        #构造ts
        FOREACH(lang ${LANGUAGE_LIST})
            string(LENGTH "${lang}" langlen)           
            IF(NOT ${langlen} EQUAL 0)
                LIST(APPEND _TS ${TRANSLATE_SOURCE_DIR}/ts/${PROJECTNAMEL}_${lang}.ts )
            ENDIF()
        ENDFOREACH()
        FILE(GLOB_RECURSE translate_HEADERS ${TRANSLATE_SOURCE_DIR}/*.h)
        FILE(GLOB_RECURSE translate_SOURCES ${TRANSLATE_SOURCE_DIR}/*.cpp)
        FILE(GLOB_RECURSE translate_FORMS ${TRANSLATE_SOURCE_DIR}/*.ui)
        SET(lang_sources ${translate_HEADERS} ${translate_SOURCES} ${translate_FORMS} ${translate_OTHERS} ${OTHER_TRANSLATE})
        EXECUTE_PROCESS(
            COMMAND ${CMAKE_COMMAND} -E make_directory ${TRANSLATE_SOURCE_DIR}/ts
            COMMAND ${CMAKE_COMMAND} -E make_directory ${BIN_DIR}/translations
            COMMAND ${LUPDATE} ${LUPDATE_OPTIONS} ${lang_sources} -ts ${_TS}
        )
    ENDIF ()
    ## 如果是插件，依赖的插件
    IF (${INIT_TYPE} STREQUAL "PLUGIN")
        SET(dependencyList
                )
        LIST(LENGTH WIS_PLUGIN_DEPS depPluginCnt)
        SET(depPluginIndex
                0)
        FOREACH (dep ${WIS_PLUGIN_DEPS})
            STRING(TOUPPER "${dep}" dep_u)
            LIST(APPEND LINK_DEPS "${${dep_u}_TARGET_LINK}")
            SET(${dep_u}_VERSION ${${dep_u}_VERSION_MAJOR}.${${dep_u}_VERSION_MINOR}.${${dep_u}_VERSION_PATCH})            
            IF (${depPluginCnt} STREQUAL "1")
                SET(dependencyList
                        "{\"Name\" : \"${dep}\",\"Version\" :\"${${dep_u}_VERSION}\"}"
                    )
            ELSE ()
                math(EXPR depPluginIndex "${depPluginIndex} + 1")
                IF (${depPluginIndex} STREQUAL ${depPluginCnt})
                    SET(dependencyList
                            "${dependencyList} {\"Name\" : \"${dep}\",\"Version\" : \"${${dep_u}_VERSION}\"}"
                        )
                ELSE ()
                    SET(dependencyList
                        "${dependencyList} {\"Name\" : \"${dep}\",\"Version\" : \"${${dep_u}_VERSION}\"},"
                        )
                ENDIF ()
            ENDIF ()
        ENDFOREACH ()

        SET(dependencyList
                "\"Dependencies\":[${dependencyList}]"
            )
        ## 处理插件的描述信息
    FILE(REMOVE ${PROJECT_BINARY_DIR}/${PROJECTNAMEL}.json)
        CONFIGURE_FILE(${PROJECTNAMEL}.json.cm ${PROJECT_BINARY_DIR}/${PROJECTNAMEL}.json)
        ### 下面这行必须有 Q_PLUGIN_METADATA宏要求，json文件必须添加至源码编译环境中
        SET(OTHER_FILES
                ${OTHER_FILES}
                ${PROJECT_BINARY_DIR}/${PROJECTNAMEL}.json
            )
    ENDIF ()
    ## translation ts file
    FILE(GLOB_RECURSE TS_FILES "${CMAKE_CURRENT_LIST_DIR}/ts/*.ts")
    # Group all ui files in the same virtual directory.
    SOURCE_GROUP("Form Files" FILES ${FORMS})
    # Group generated files in the same isolated virtual directory.
    SOURCE_GROUP("Generated Files\\qrc" FILES ${RCC_HDS})
    #SOURCE_GROUP("Generated Files\\ui"         FILES     ${UI_HDS})
    #SOURCE_GROUP("Generated Files\\moc"     FILES     ${MOC_HDS})
    # Group all Other files in the same virtual directory.
    SOURCE_GROUP("Other Files" FILES ${OTHER_FILES})
    SOURCE_GROUP("Other Files\\ts" FILES ${TS_FILES})
    ##--------------------------------------------------------------------------
    IF (${INIT_TYPE} STREQUAL "EXE" OR ${INIT_TYPE} STREQUAL "CONSOLE") ##生成可执行程序
        IF(APPLE) #application bundle generation
            IF(IS_TOOL_APP)
                ADD_EXECUTABLE(${PROJECTNAMEL} WIN32
                    ${HEADERS}
                    ${SOURCES}
                    ${FORMS}
                    ${RESOURCES}
                    ${OTHER_FILES}
                    ${TS_FILES}
                    #${MOC_HDS}
                    #${UI_HDS}
                    ${RCC_HDS}
					${SOURCE_GROUP_FILES}
                )
            ELSE()
                SET(APP_ICNS ${PROJECT_SOURCE_DIR}/${PROJECTNAMEL}.icns)
                SET(APP_PLIST ${PROJECT_SOURCE_DIR}/Info.plist)
                LIST(APPEND OTHER_FILES ${APP_ICNS} ${APP_PLIST})
                SET(BUNDLE_LOADER "${BIN_DIR}/${IDE_APPBUNDLE_NAME}") # BUNDLE_LOADER is used by plugins needing to extend the cocoa wrapper (SDLVideo).
                SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -bundle_loader \"${BUNDLE_LOADER}\"")

                # icon
                SET_SOURCE_FILES_PROPERTIES(
                        ${APP_ICNS}
                        PROPERTIES MACOSX_PACKAGE_LOCATION Resources
                )            # default values
                SET_SOURCE_FILES_PROPERTIES(
                        ${APP_PLIST}
                        PROPERTIES MACOSX_PACKAGE_LOCATION .
                )

                SET(MACOSX_BUNDLE_ICON_FILE ${APP_ICNS})
                SET(PRODUCT_NAME ${PROJECT_NAME}) # for info.plist
                ADD_EXECUTABLE(${PROJECTNAMEL} WIN32 MACOSX_BUNDLE
                    ${HEADERS}
                    ${SOURCES}
                    ${FORMS}
                    ${RESOURCES}
                    ${OTHER_FILES}
                    ${TS_FILES}
                    #${MOC_HDS}
                    #${UI_HDS}
                    ${RCC_HDS}
					${SOURCE_GROUP_FILES}
                )

                # custom info.plist
                SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
                        MACOSX_BUNDLE_INFO_PLIST ${APP_PLIST}
                        OUTPUT_NAME ${IDE_APPBUNDLE_NAME}
                       )
                IF(NOT ${IDE_APPBUNDLE_NAME} MATCHES ${PROJECTNAMEL})
                    # Create a symlink in the build tree to provide a "${PROJECTNAMEL}" next
                    # to the "${IDE_APPBUNDLE_NAME}" executable that refers to the application bundle.
                    add_custom_command(TARGET ${PROJECTNAMEL} POST_BUILD
                            COMMAND ln -sf ./${IDE_APPBUNDLE_NAME}
                            $<TARGET_FILE_DIR:${PROJECTNAMEL}>/${PROJECTNAMEL}
                            )
                ENDIF()

            ENDIF()

            SET(BUNDLE_FRAMEWORK_PATH "${LIB_DIR}")

            SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
                INSTALL_RPATH ${LIB_DIR}
                )

        ELSE(APPLE)
            set(WINDOWS
            )

            #其中WINDOWS变量的作用是:调试版附带控制台,发布版去除控制台
            IF (CMAKE_BUILD_TYPE MATCHES Debug)

            ELSE()
                set(WINDOWS WIN32)
            ENDIF()
            # Qt4::WinMain
            SET(QT_USE_QTMAIN TRUE)#qtmain
            ### 应用程序的名称不强制改变大小写。           
            # 生成目标程序
            ADD_EXECUTABLE(${PROJECTNAMEL} ${WINDOWS}
                ${HEADERS}
                ${SOURCES}
                ${FORMS}
                ${RESOURCES}
                ${OTHER_FILES}
                ${TS_FILES}
                #${MOC_HDS}
                #${UI_HDS}
                ${RCC_HDS}
				${SOURCE_GROUP_FILES}
            )

            TARGET_LINK_LIBRARIES(${PROJECTNAMEL}
                ${QT_QTMAIN_LIBRARY}
            )
            IF (MSVC)
                STRING(REPLACE "/" "\\" _qt5_runtime_directory "${_qt5Core_install_prefix}/bin")
                SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES VS_GLOBAL_LocalDebuggerEnvironment "PATH=${_qt5_runtime_directory};$(PATH)")
                ##需要管理员权限,放在add_executable的后面
                SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
                        LINK_FLAGS "/level='requireAdministrator' /uiAccess='false'")
                ##"/MANIFESTUAC:\"level='asInvoker' uiAccess='false'\"")#"/MANIFESTUAC:\" level='requireAdministrator' uiAccess='false' "/SUBSYSTEM:WINDOWS")

                IF (CMAKE_BUILD_TYPE MATCHES Debug)
                    SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
                            LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
                    SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
                            COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
                ELSE ()
                    IF (${INIT_TYPE} STREQUAL "CONSOLE")
                        SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
                                LINK_FLAGS_RELEASE "/SUBSYSTEM:CONSOLE")
                        SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
                                COMPILE_DEFINITIONS_RELEASE "_CONSOLE")
                        SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
                                LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
                        SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
                                COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")
                    ELSE ()
                        SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
                                LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDOWS")
                        SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
                                LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
                    ENDIF ()
                ENDIF ()
            ENDIF (MSVC)
        ENDIF ()
    ELSEIF (${INIT_TYPE} STREQUAL "LIB")## 生成静态链接库
        ADD_LIBRARY(${PROJECTNAMEL} STATIC
                ${HEADERS}
                ${SOURCES}
                ${FORMS}
                ${RESOURCES}
                ${OTHER_FILES}
                ${TS_FILES}
                #${MOC_HDS}
                #${UI_HDS}
                ${RCC_HDS}
				${SOURCE_GROUP_FILES}
                )
    ELSE ()## 生成动态链接库或者插件
        ADD_LIBRARY(${PROJECTNAMEL} SHARED
                ${HEADERS}
                ${SOURCES}
                ${FORMS}
                ${RESOURCES}
                ${OTHER_FILES}
                ${TS_FILES}
                #${MOC_HDS}
                #${UI_HDS}
                ${RCC_HDS}
				${SOURCE_GROUP_FILES}
                )
    ENDIF ()
    ## 自定义pdb文件的输出路径
    IF (${INIT_TYPE} STREQUAL "LIB")## 生成静态链接库
        # Static libraries have no linker PDBs, thus the compiler PDBs are relevant
        SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
            COMPILE_PDB_OUTPUT_DIRECTORY_DEBUG "${PDB_DIR}")
        SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
            COMPILE_PDB_OUTPUT_DIRECTORY_RELEASE "${PDB_DIR}")
        SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
            COMPILE_PDB_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PDB_DIR}")
        SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
            COMPILE_PDB_OUTPUT_DIRECTORY_MINSIZEREL "${PDB_DIR}")
    ELSE()
        # DLLs export debug symbols in the linker PDB (the compiler PDB is an intermediate file)
        SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
            PDB_OUTPUT_DIRECTORY_DEBUG "${PDB_DIR}")
        SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
            PDB_OUTPUT_DIRECTORY_RELEASE "${PDB_DIR}")
        SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
            PDB_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PDB_DIR}")
        SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
            PDB_OUTPUT_DIRECTORY_MINSIZEREL "${PDB_DIR}")
    ENDIF()  
    IF(WIN32)
        IF(MSVC)
            ## 取消增量链接
            IF(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
                SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES 
                    LINK_FLAGS "/INCREMENTAL:NO")
            ENDIF()
        ENDIF()
    ENDIF()
    ##项目分组
    IF(PROJECT_FOLDER)
        SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES 
            FOLDER ${PROJECT_FOLDER})
    ENDIF()
    ##--------------------------------------------------------------------------
    SET(LINK_DEPS
    )
    ## 当前工程依赖的本工程的项目lib
    FOREACH (deps ${WIS_MODULE_DEPS})
        STRING(TOUPPER "${deps}" deps)        
        LIST(APPEND LINK_DEPS "${${deps}_TARGET_LINK}")
    ENDFOREACH ()
    ## 当前工程依赖的本工程的项目Plugin
    FOREACH (deps ${WIS_PLUGIN_DEPS})
        STRING(TOUPPER "${deps}" deps)
        LIST(APPEND LINK_DEPS "${${deps}_TARGET_LINK}")
    ENDFOREACH ()
    ## 依赖的第三方lib
    FOREACH (deps ${DEPENDENCIES})
        LIST(APPEND LINK_DEPS "${deps}")
    ENDFOREACH ()
    ### 当前工程依赖的boost库
    FOREACH (deps ${NOVA_BOOST_DEPENDS})
        IF(NOT Boost_VERSION VERSION_LESS 106600)###版本号大于1.66
            #  -x86     Architecture and address model tag
            #           First character is the architecture, then word-size, either 32 or 64
            #           Only used in 'versioned' layout, added in Boost 1.66.0
            set(_boost_ARCHITECTURE_TAG "")
            string(APPEND _boost_ARCHITECTURE_TAG "-")
            # This needs to be kept in-sync with the section of CMakePlatformId.h.in
            # inside 'defined(_WIN32) && defined(_MSC_VER)'
            STRING(LENGTH "${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" tempLen)
            IF(${tempLen} EQUAL 0)
                SET(CMAKE_CXX_COMPILER_ARCHITECTURE_ID "X86")
            ENDIF()
            if(${CMAKE_CXX_COMPILER_ARCHITECTURE_ID} STREQUAL "IA64")
                string(APPEND _boost_ARCHITECTURE_TAG "i")
            elseif(${CMAKE_CXX_COMPILER_ARCHITECTURE_ID} STREQUAL "X86"
                        OR ${CMAKE_CXX_COMPILER_ARCHITECTURE_ID} STREQUAL "x64")
                string(APPEND _boost_ARCHITECTURE_TAG "x")
            elseif(${CMAKE_CXX_COMPILER_ARCHITECTURE_ID} MATCHES "^ARM")
                string(APPEND _boost_ARCHITECTURE_TAG "a")
            elseif(${CMAKE_CXX_COMPILER_ARCHITECTURE_ID} STREQUAL "MIPS")
                string(APPEND _boost_ARCHITECTURE_TAG "m")
            endif()

            if(CMAKE_SIZEOF_VOID_P EQUAL 8)
                string(APPEND _boost_ARCHITECTURE_TAG "64")
            else()
                string(APPEND _boost_ARCHITECTURE_TAG "32")
            endif()
            IF(MSVC)
                IF(CMAKE_BUILD_TYPE MATCHES Debug)
                    LIST(APPEND LINK_DEPS "libboost_${deps}-mt-gd${_boost_ARCHITECTURE_TAG}.lib")
                ELSE()
                    LIST(APPEND LINK_DEPS "libboost_${deps}-mt${_boost_ARCHITECTURE_TAG}.lib")
                ENDIF()
            ELSE()
                IF(CMAKE_BUILD_TYPE MATCHES Debug)
                    LIST(APPEND LINK_DEPS "boost_${deps}-mt-d${_boost_ARCHITECTURE_TAG}.a")
                ELSE()
                    LIST(APPEND LINK_DEPS "boost_${deps}-mt${_boost_ARCHITECTURE_TAG}.a")
                ENDIF()
            ENDIF()
        ELSE()
            IF(MSVC)
                IF(CMAKE_BUILD_TYPE MATCHES Debug)
                    LIST(APPEND LINK_DEPS "libboost_${deps}-mt-gd.lib")
                ELSE()
                    LIST(APPEND LINK_DEPS "libboost_${deps}-mt.lib")
                ENDIF()
            ELSE()
                IF(CMAKE_BUILD_TYPE MATCHES Debug)
                    LIST(APPEND LINK_DEPS "boost_${deps}-mt-d.a")
                ELSE()
                    LIST(APPEND LINK_DEPS "boost_${deps}-mt.a")
                ENDIF()
            ENDIF()
        ENDIF()
    ENDFOREACH ()
    
    ## 苹果专用的framework库
    IF(APPLE)
        FOREACH (framework ${FRAMEWORKS})
            FIND_LIBRARY(FRAMEWORK_LIBRARY ${framework})
            LIST(APPEND LINK_DEPS "${FRAMEWORK_LIBRARY}")
        ENDFOREACH ()
    ENDIF()
    TARGET_LINK_LIBRARIES(${PROJECTNAMEL}
            ${LINK_DEPS}
    )
    ## 项目依赖关系
    STRING(LENGTH "${WIS_MODULE_DEPS}" varlen)
    IF (NOT ${varlen} MATCHES "0")
        ADD_DEPENDENCIES(${PROJECTNAMEL}
            ${WIS_MODULE_DEPS}
        )
    ENDIF ()
    STRING(LENGTH "${WIS_PLUGIN_DEPS}" varlen)
    IF (NOT ${varlen} MATCHES "0")
        ADD_DEPENDENCIES(${PROJECTNAMEL}
                ${WIS_PLUGIN_DEPS}
                )
    ENDIF ()
    ##--------------------------------------------------------------------------
    ## 链接的Qt库
    FOREACH (deps ${QT_LINK_DEPS})
        TARGET_LINK_LIBRARIES(${PROJECTNAMEL}
                ${deps})
    ENDFOREACH ()
    ##--------------------------------------------------------------------------
    ## 设置生成的文件名后缀和生成文件的目标路径
    IF (WIN32 OR APPLE)
        IF (NOT ${INIT_TYPE} MATCHES "EXE" AND NOT ${INIT_TYPE} MATCHES "CONSOLE" )##非可执行程序
            SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
                    DEBUG_POSTFIX ${WIS_DEBUG_POSTFIX}
                    )
        ENDIF ()
    ENDIF ()
    IF (${INIT_TYPE} STREQUAL "PLUGIN")##插件
         SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
                 RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PLUGIN_DIR}
                 ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${LIB_DIR}
                 LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PLUGIN_DIR}
                 )
         SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
                 RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PLUGIN_DIR}
                 ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${LIB_DIR}
                 LIBRARY_OUTPUT_DIRECTORY_RELEASE ${PLUGIN_DIR}
                 )
         SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
                 RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${PLUGIN_DIR}
                 ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${LIB_DIR}
                 LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${PLUGIN_DIR}
                 )
    ELSE ()##其它
        IF(APPLE)
            SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
                INSTALL_NAME_DIR ${LIB_DIR}
                LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR}
                )
            ## 非主应用程序
            IF(IS_TOOL_APP)
                SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
                    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
                )
            ENDIF()
        ELSE()
            SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
                    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BIN_DIR}
                    ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${LIB_DIR}
                    LIBRARY_OUTPUT_DIRECTORY_DEBUG ${BIN_DIR}
                    )
            SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
                    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BIN_DIR}
                    ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${LIB_DIR}
                    LIBRARY_OUTPUT_DIRECTORY_RELEASE ${BIN_DIR}
                    )
            SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
                    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${BIN_DIR}
                    ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${LIB_DIR}
                    LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${BIN_DIR}
                    )
         ENDIF()
    ENDIF ()
    ##--------------------------------------------------------------------------
    IF (WIN32)
        IF (MSVC)
            SET_TARGET_PROPERTIES(${PROJECTNAMEL} PROPERTIES
                    VS_KEYWORD Qt4VSv1.0 #可以用来改变visual studio的关键字，例如如果该选项被设置为Qt4VSv1.0的话，QT集成将会运行得更好。
                    VS_GLOBAL_KEYWORD Qt4VSv1.0
                    )
        ENDIF (MSVC)
        SET_TARGET_PROPERTIES(${PROJECTNAMEL}
                PROPERTIES
                OUTPUT_NAME "${PROJECTNAMEL}"
                #VERSION                 ${${PROJECTNAMEU}_VERSION}
                CLEAN_DIRECT_OUTPUT 1)
    ELSE ()
        SET_TARGET_PROPERTIES(${PROJECTNAMEL}
                PROPERTIES
                #VERSION             ${${PROJECTNAMEU}_VERSION}
                #SOVERSION            ${${PROJECTNAMEU}_SOVERSION}
                CLEAN_DIRECT_OUTPUT 1)
    ENDIF ()
    IF (WIN32)
        IF (MSVC)
            ## 设置多核编译
            SET_TARGET_PROPERTIES(${PROJECTNAMEL}
                    PROPERTIES
                    COMPILE_FLAGS "/MP")  #Yes 多核编译        
            SET_TARGET_PROPERTIES(${PROJECTNAMEL}
                    PROPERTIES
                    COMPILE_FLAGS "/EHsc") #yes 启用C++异常
        ENDIF ()
    ENDIF ()
    
    ## 添加自定义的生成qm文件的处理过程
    IF (HAS_QM AND RUN_QM)
        IF(QMFILEDIR)
        ELSE()
            IF(MERGE_QM)
                SET(QMFILEDIR ${CMAKE_BINARY_DIR}/translations)
            ELSE()
                SET(QMFILEDIR ${BIN_DIR}/translations)
            ENDIF()
        ENDIF()

        EXECUTE_PROCESS(
            COMMAND ${CMAKE_COMMAND} -E make_directory ${QMFILEDIR}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${BIN_DIR}/translations
        )
        ## 转换语言数组
        set(LANGUAGE_LIST ${TEST} )
        separate_arguments(LANGUAGE_LIST)
        list(LENGTH LANGUAGE_LIST LanguageCnt)
        IF(${LanguageCnt} EQUAL 0) ## 没有设置要翻译的目标语言
            MESSAGE(SEND_ERROR "Error, the variable LANGUAGES is not defined!")
        ENDIF()
        ## 转换动态库所在目录数组
        SET(LIBRARY_DIRS_LIST ${WIS_LIBRARY_DIRS} )
        SEPARATE_ARGUMENTS(LIBRARY_DIRS_LIST)
        LIST(LENGTH LIBRARY_DIRS_LIST LibraryDirCnt)
        IF(${LibraryDirCnt} EQUAL 0) ## 项目中动态库的项目所在的目录
            MESSAGE(SEND_ERROR "WARNING, the variable WIS_LIBRARY_DIRS is not defined!")
        ENDIF()
        ## 转换插件所在目录数组
        SET(PLUGIN_DIRS_LIST ${WIS_PLUGIN_DIRS} )
		
        SEPARATE_ARGUMENTS(PLUGIN_DIRS_LIST)
        ## 获取当前项目以来的其他库
        SET(ALL_DEP_MODULE
            )
        ## 递归获取当前项目依赖的所有子项目
        SET(done_libs
            )
        LIST(APPEND done_libs
            ${WIS_MODULE_DEPS})
       
        WHILE(ON)
            LIST(LENGTH done_libs done_libs_Cnt)
            IF(${done_libs_Cnt} EQUAL 0)
                BREAK()
            ENDIF()
            FOREACH(dep ${done_libs})
                SET(FIND_DEPENDENCIE OFF)
                SET(DEPENDENCIE_FILE
                    )
                FOREACH(LibraryDir ${LIBRARY_DIRS_LIST})
					glob_recurse_first("${LibraryDir}/*${dep}_dependencies.cmake" _fileList)
					foreach (_file ${_fileList})
						message (STATUS "${_file}")
						IF(EXISTS ${_file})
							SET(FIND_DEPENDENCIE ON)
							SET(DEPENDENCIE_FILE ${_file})
						ENDIF()
					endforeach()
                ENDFOREACH()
                IF(FIND_DEPENDENCIE)
                    LIST(REMOVE_ITEM done_libs ${dep})
                    LIST(APPEND ALL_DEP_MODULE ${dep})
                    INCLUDE(${DEPENDENCIE_FILE})
                    LIST(APPEND done_libs ${WIS_MODULE_DEPS})
                ELSE()
                    LIST(REMOVE_ITEM done_libs ${dep})
                    MESSAGE(WARNING "${PROJECTNAMEL} Library dependency ${dep}_dependencies.cmake not found")
                    BREAK()
                ENDIF()
            ENDFOREACH()
        ENDWHILE()
        IF (${INIT_TYPE} STREQUAL "PLUGIN")##插件
            ## 递归获取当前插件项目依赖的所有插件项目
            SET(done_plugins
                )
            LIST(APPEND done_plugins
                ${WIS_PLUGIN_DEPS})
       
            WHILE(ON)
                LIST(LENGTH done_plugins done_plugins_Cnt)
                IF(${done_plugins_Cnt} EQUAL 0)
                    BREAK()
                ENDIF()
                FOREACH(dep ${done_plugins})
                    SET(FIND_DEPENDENCIE OFF)
                    SET(DEPENDENCIE_FILE
                        )
                    FOREACH(PluginDir ${PLUGIN_DIRS_LIST})
						glob_recurse_first("${LibraryDir}/*${dep}_dependencies.cmake" _fileList)
						foreach (_file ${_fileList})
							message (STATUS "${_file}")
							IF(EXISTS ${_file})
								SET(FIND_DEPENDENCIE ON)
								SET(DEPENDENCIE_FILE ${_file})
							ENDIF()
						endforeach()
#                        IF(EXISTS ${PluginDir}/${dep}/${dep}_dependencies.cmake)
#                            SET(FIND_DEPENDENCIE ON)
#                            SET(DEPENDENCIE_FILE ${PluginDir}/${dep}/${dep}_dependencies.cmake)
#                        ELSEIF(EXISTS ${PluginDir}/${dep}plugin/${dep}plugin_dependencies.cmake) ### 目录名称为(项目名称+plugin)
#                            SET(FIND_DEPENDENCIE ON)
#                            SET(DEPENDENCIE_FILE ${PluginDir}/${dep}plugin/${dep}plugin_dependencies.cmake)
#                        ENDIF()
                    ENDFOREACH()
                    IF(FIND_DEPENDENCIE)
                        LIST(REMOVE_ITEM done_plugins ${dep})
                        LIST(APPEND ALL_DEP_MODULE ${dep})
                        INCLUDE(${DEPENDENCIE_FILE})
                        LIST(APPEND done_plugins ${WIS_PLUGIN_DEPS})
                    ELSE()
                        LIST(REMOVE_ITEM done_plugins ${dep})
                        MESSAGE(WARNING "${PROJECTNAMEL} Library dependency ${dep}_dependencies.cmake not found")
                        BREAK()
                    ENDIF()
                ENDFOREACH()
            ENDWHILE()
        ENDIF()
        LIST(LENGTH ALL_DEP_MODULE AllDepModuleCnt)
        IF(NOT AllDepModuleCnt  EQUAL 0)
            LIST(REMOVE_DUPLICATES ALL_DEP_MODULE)
        ENDIF()
        #构造qm
        FOREACH(lang ${LANGUAGE_LIST})
            string(LENGTH "${lang}" langlen)
            IF(NOT ${langlen} EQUAL 0)  
                ## 生成单项目的qm文件
                IF(EXISTS ${TRANSLATE_SOURCE_DIR}/ts/${PROJECTNAMEL}_${lang}.ts)
			ADD_CUSTOM_COMMAND(TARGET ${PROJECTNAMEL} POST_BUILD 
				COMMAND echo "General ${PROJECTNAMEL}  ${lang} qm files"
				COMMAND ${LRELEASE} ${TRANSLATE_SOURCE_DIR}/ts/${PROJECTNAMEL}_${lang}.ts -qm  ${QMFILEDIR}/${PROJECTNAMEL}_${lang}.qm
			)
                ENDIF()
                ## 需要合并QM文件
                IF(MERGE_QM)
                    ## 生成Qt库的qm文件
                    IF(EXISTS ${THIRDPARTY_ROOT}/qttranslations/${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}/qt_${lang}.ts)
                        ADD_CUSTOM_COMMAND(TARGET ${PROJECTNAMEL} POST_BUILD 
                            COMMAND echo "General Qt  ${lang} qm files"
                            COMMAND ${LRELEASE} ${THIRDPARTY_ROOT}/qttranslations/${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}/qt_${lang}.ts -qm  ${QMFILEDIR}/qt_${lang}.qm
                            )
                    ELSE()
                        IF (NOT ${lang} MATCHES "en")
                            MESSAGE(WARNING "${THIRDPARTY_ROOT}/qttranslations/${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}/qt_${lang}.ts not exists")
                        ENDIF()
                    ENDIF()
                    SET(in_qm_files
                        ${QMFILEDIR}/${PROJECTNAMEL}_${lang}.qm
                        )
                    ## 追加Qt的语言资源
                    IF(EXISTS ${QMFILEDIR}/qt_${lang}.qm)
                        LIST(APPEND in_qm_files
                            ${QMFILEDIR}/qt_${lang}.qm
                            )
                    ENDIF()
                    ## 添加依赖项
                    FOREACH(dep ${ALL_DEP_MODULE})
                        STRING(TOLOWER "${dep}" DEP_MODULE_L)
                        IF(EXISTS ${QMFILEDIR}/${DEP_MODULE_L}_${lang}.qm)
                            LIST(APPEND in_qm_files
                                    ${QMFILEDIR}/${DEP_MODULE_L}_${lang}.qm
                                )
                        ENDIF()
                        ###MESSAGE(STATUS >>>>> ${QMFILEDIR}/${DEP_MODULE_L}_${lang}.qm)
                    ENDFOREACH()
                    ## 合并目标项目的qm文件
                    MESSAGE(STATUS "Merge ${PROJECTNAMEL} ${lang} qm files")
                    IF(EXISTS ${QMFILEDIR}/${PROJECTNAMEL}_${lang}.qm)
                        ADD_CUSTOM_COMMAND(TARGET ${PROJECTNAMEL} POST_BUILD 
                            COMMAND echo "Merge ${PROJECTNAMEL}  ${lang} qm files"
                            COMMAND ${LCONVERT} -o ${BIN_DIR}/translations/${PROJECTNAMEL}_${lang}.qm ${in_qm_files}
                            )
                    ENDIF()                    
                ENDIF()
            ENDIF()
        ENDFOREACH()
    ENDIF()
    
    # configuration summary
    boost_report_value(LINK_DEPS)
    wis_report_directory_property(COMPILE_DEFINITIONS)
    IF (NOT ${INIT_TYPE} MATCHES "EXE" AND NOT ${INIT_TYPE} MATCHES "CONSOLE")
        wis_target_output_name(${PROJECTNAMEL} ${PROJECTNAMEU}_TARGET_LINK)
        # # Export core target name to make it visible by backends
        SET(${PROJECTNAMEU}_TARGET_LINK ${${PROJECTNAMEU}_TARGET_LINK} PARENT_SCOPE)
    ENDIF ()
ENDFUNCTION()

#_______________________________________________________________________________
##初始化预编译头
FUNCTION(WIS_INIT_PCH
        PrecompiledHeader PrecompiledSource)
    ## 预编译头判断
    IF (NOT ${PrecompiledHeader} STREQUAL "" AND NOT ${PrecompiledSource} STREQUAL "")
        message("-------------------------${PrecompiledHeader}----${PrecompiledSource}")
        ## 预编译头
        # 头文件的文件名可以任意，并且gcc下面其实并不需要stdafx.cpp，但是考虑到对Visual Studio的兼容性，
        # 这里仍然需要一个cpp文件
        ADD_PRECOMPILED_HEADER(${PROJECT_NAME} ${PrecompiledHeader} ${PrecompiledSource})
        ## 处理要使用预编译头的源文件
        SET(USE_PRECOMPILED_SRC
                )
        FOREACH (source ${SOURCES})
            IF (NOT (source MATCHES ".*${PrecompiledSource}*"))
                LIST(APPEND USE_PRECOMPILED_SRC ${source})
            ENDIF ()
        ENDFOREACH ()
        # 然后简单的一个调用就搞定了
        USE_PRECOMPILED_HEADER(USE_PRECOMPILED_SRC)
    ENDIF ()
ENDFUNCTION()
#_______________________________________________________________________________
# 构造动态链接库
#
# HEADERS         头文件
# SOURCES         源文件
# FORMS         窗体文件
# RESOURCES        资源文件
# OTHER_FILES    其它文件
#
# DEPENDENCIES     依赖的第三方的lib
#
# WIS_MODULE_DEPS 依赖的本解决方案内的项目名称
MACRO(WIS_QT_INIT_LIBRARY_MODULE
        HAS_QM MERGE_QM)
    ##构造目标项目
    WIS_QT_INIT_LIBRARY_MODULE_PCH(${HAS_QM} ${MERGE_QM} "" "")
ENDMACRO(WIS_QT_INIT_LIBRARY_MODULE)
#
# 带预编译头的动态链接库
#
MACRO(WIS_QT_INIT_LIBRARY_MODULE_PCH
        HAS_QM MERGE_QM
        PrecompiledHeader PrecompiledSource
        )
    #项目名称大写
    STRING(TOUPPER "${PROJECT_NAME}" PROJECTNAMEU)
    ## 动态库导出
    ADD_DEFINITIONS(-D${PROJECTNAMEU}_LIBRARY)
    ##构造目标项目
    WIS_QT_INIT_COMMON_LIB("DLL" ${HAS_QM} ${MERGE_QM})
    ## 预编译头设置
    WIS_INIT_PCH("${PrecompiledHeader}" "${PrecompiledSource}")
ENDMACRO(WIS_QT_INIT_LIBRARY_MODULE_PCH)
#_______________________________________________________________________________
# 静态链接库
#
MACRO(WIS_QT_INIT_STATIC_MODULE
        HAS_QM MERGE_QM)
    ##构造目标项目
    WIS_QT_INIT_STATIC_MODULE_PCH(${HAS_QM} ${MERGE_QM} "" "")
ENDMACRO(WIS_QT_INIT_STATIC_MODULE)
#
# 带预编译头的静态链接库
#
MACRO(WIS_QT_INIT_STATIC_MODULE_PCH
        HAS_QM MERGE_QM
        PrecompiledHeader PrecompiledSource)
    #项目名称大写
    STRING(TOUPPER "${PROJECT_NAME}" PROJECTNAMEU)
    ## 静态库导出
    ADD_DEFINITIONS(-D${PROJECTNAMEU}_STATIC_LIB)
    ##构造目标项目
    WIS_QT_INIT_COMMON_LIB("LIB" ${HAS_QM} ${MERGE_QM})
    ## 预编译头设置
    WIS_INIT_PCH("${PrecompiledHeader}" "${PrecompiledSource}")
ENDMACRO(WIS_QT_INIT_STATIC_MODULE_PCH)
#_______________________________________________________________________________
# 插件
#
#
MACRO(WIS_QT_INIT_PLUGIN_MODULE
        HAS_QM MERGE_QM)
    ##构造目标项目
    WIS_QT_INIT_PLUGIN_MODULE_PCH(${HAS_QM} ${MERGE_QM} "" "")
ENDMACRO(WIS_QT_INIT_PLUGIN_MODULE)
#
# 插件带预编译头
#
MACRO(WIS_QT_INIT_PLUGIN_MODULE_PCH
        HAS_QM MERGE_QM
        PrecompiledHeader PrecompiledSource)
    #项目名称大写
    STRING(TOUPPER "${PROJECT_NAME}" PROJECTNAMEU)
    ## 动态库导出
    ADD_DEFINITIONS(-D${PROJECTNAMEU}_LIBRARY)
    ## 插件特殊标记
    ADD_DEFINITIONS(-DQT_PLUGIN -DQT_DLL)
    ##构造目标项目
    WIS_QT_INIT_COMMON_LIB("PLUGIN" ${HAS_QM} ${MERGE_QM})
    ## 预编译头设置
    WIS_INIT_PCH("${PrecompiledHeader}" "${PrecompiledSource}")
ENDMACRO(WIS_QT_INIT_PLUGIN_MODULE_PCH)
##############主应用程序#########################################################
# 控制台应用程序
#
MACRO(WIS_QT_INIT_CONSOLE_EXE_MODULE
        HAS_QM MERGE_QM)
    ##构造目标项目
    WIS_QT_INIT_CONSOLE_EXE_MODULE_PCH(${HAS_QM} ${MERGE_QM} "" "")
ENDMACRO(WIS_QT_INIT_CONSOLE_EXE_MODULE)
#_______________________________________________________________________________
# 依赖于Qt的控制台应用程序 带预编译头
#
#
MACRO(WIS_QT_INIT_CONSOLE_EXE_MODULE_PCH
        HAS_QM MERGE_QM
        PrecompiledHeader PrecompiledSource)
    ##构造目标项目
    WIS_QT_INIT_COMMON_LIB("CONSOLE" ${HAS_QM} ${MERGE_QM})
    ## 预编译头设置
    WIS_INIT_PCH("${PrecompiledHeader}" "${PrecompiledSource}")
ENDMACRO(WIS_QT_INIT_CONSOLE_EXE_MODULE_PCH)
#_______________________________________________________________________________
# 依赖于Qt的可执行程序
#
MACRO(WIS_QT_INIT_EXE_MODULE
        HAS_QM MERGE_QM)
    ##构造目标项目
    WIS_QT_INIT_EXE_MODULE_PCH(${HAS_QM} ${MERGE_QM} "" "")
ENDMACRO(WIS_QT_INIT_EXE_MODULE)
#_______________________________________________________________________________
# 依赖于Qt的可执行程序 带预编译头
#
MACRO(WIS_QT_INIT_EXE_MODULE_PCH
        HAS_QM MERGE_QM
        PrecompiledHeader PrecompiledSource)
    ##构造目标项目
    WIS_QT_INIT_COMMON_LIB("EXE" ${HAS_QM} ${MERGE_QM})
    ## 预编译头设置
    WIS_INIT_PCH("${PrecompiledHeader}" "${PrecompiledSource}")
ENDMACRO(WIS_QT_INIT_EXE_MODULE_PCH)
##############主程序的辅助程序#####################################################
# 控制台应用程序
#
MACRO(WIS_QT_INIT_TOOL_CONSOLE_MODULE
        HAS_QM MERGE_QM)
    ##构造目标项目
    SET(IS_TOOL_APP
        TRUE)
    WIS_QT_INIT_TOOL_CONSOLE_MODULE_PCH(${HAS_QM} ${MERGE_QM} "" "")
     SET(IS_TOOL_APP
        FALSE)
ENDMACRO(WIS_QT_INIT_TOOL_CONSOLE_MODULE)
#_______________________________________________________________________________
# 依赖于Qt的控制台应用程序 带预编译头
#
MACRO(WIS_QT_INIT_TOOL_CONSOLE_MODULE_PCH
        HAS_QM MERGE_QM
        PrecompiledHeader PrecompiledSource)
    ##构造目标项目
    SET(IS_TOOL_APP
        TRUE)
    WIS_QT_INIT_COMMON_LIB("CONSOLE" ${HAS_QM} ${MERGE_QM})
    ## 预编译头设置
    WIS_INIT_PCH("${PrecompiledHeader}" "${PrecompiledSource}")
    SET(IS_TOOL_APP
        FALSE)
ENDMACRO(WIS_QT_INIT_TOOL_CONSOLE_MODULE_PCH)
#_______________________________________________________________________________
# 依赖于Qt的可执行程序(非主应用程序，主要表现再Mac，上不生成.app）
#
MACRO(WIS_QT_INIT_TOOL_EXE_MODULE
        HAS_QM MERGE_QM)
    ##构造目标项目
    SET(IS_TOOL_APP
        TRUE)
    WIS_QT_INIT_TOOL_EXE_MODULE_PCH(${HAS_QM} ${MERGE_QM} "" "")
    SET(IS_TOOL_APP
        FALSE)
ENDMACRO(WIS_QT_INIT_TOOL_EXE_MODULE)
#_______________________________________________________________________________
# 依赖于Qt的可执行程序 带预编译头(非主应用程序，主要表现再Mac，上不生成.app）
#
MACRO(WIS_QT_INIT_TOOL_EXE_MODULE_PCH
        HAS_QM MERGE_QM
        PrecompiledHeader PrecompiledSource)
    ##构造目标项目
    SET(IS_TOOL_APP
        TRUE)
    WIS_QT_INIT_COMMON_LIB("EXE" ${HAS_QM} ${MERGE_QM})
    ## 预编译头设置
    WIS_INIT_PCH("${PrecompiledHeader}" "${PrecompiledSource}")
    SET(IS_TOOL_APP
        FALSE)
ENDMACRO(WIS_QT_INIT_TOOL_EXE_MODULE_PCH)
