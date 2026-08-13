################################################################################
# wisCPPProjUtilities.cmake - part of CMake configuration of wiSCADA
#
################################################################################
# 生成 普通C++工程
#
################################################################################

#_______________________________________________________________________________
# INIT_TYPE
#    EXE        可执行程序
#    DLL        动态链接库
#    LIB        静态链接库
#
#
#
#
FUNCTION (WIS_INIT_COMMON_LIB INIT_TYPE)
    #包含头文件
    INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})
    #项目名称全打写 全小写
    IF (${INIT_TYPE} STREQUAL "EXE" OR ${INIT_TYPE} STREQUAL "CONSOLE") ##生成可执行程序
        STRING(TOUPPER "${PROJECT_NAME}" PROJECTNAMEU)
        SET(PROJECTNAMEL
                ${PROJECT_NAME})
    ELSE()
        #STRING(TOLOWER "${PROJECT_NAME}" PROJECTNAMEL)
		STRING(TOUPPER "${PROJECT_NAME}" PROJECTNAMEL)
        STRING(TOUPPER "${PROJECT_NAME}" PROJECTNAMEU)
    ENDIF()
    SOURCE_GROUP("Header Files" FILES ${HEADERS})
    SOURCE_GROUP("Source Files" FILES ${SOURCES})
    SOURCE_GROUP("Other Files"  FILES ${OTHER_FILES})
    IF( ${INIT_TYPE} STREQUAL "EXE" OR ${INIT_TYPE} STREQUAL "CONSOLE")##可执行程序(包括GUI应用程序和控制台应用程序)
        IF(APPLE) #application bundle generation
            IF(IS_TOOL_APP)
                ADD_EXECUTABLE(${PROJECTNAMEL} WIN32
                    ${HEADERS}
                    ${SOURCES}
                    ${RESOURCES}
                    ${OTHER_FILES}
					${SOURCE_GROUP_FILES}
                )
            ELSE()
                IF(NOT ${IDE_APPBUNDLE_NAME} MATCHES ${PROJECTNAMEL})
                    SET(APP_ICNS ${PROJECT_SOURCE_DIR}/${IDE_APPBUNDLE_NAME}.icns)
                ELSE()
                    SET(APP_ICNS ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.icns)
                ENDIF()
                SET(APP_PLIST ${PROJECT_SOURCE_DIR}/Info.plist)
                LIST(APPEND OTHER_FILES ${APP_ICNS} ${APP_PLIST})
                SET(BUNDLE_LOADER "${BIN_DIR}/${IDE_APPBUNDLE_NAME}") # BUNDLE_LOADER is used by plugins needing to extend the cocoa wrapper (SDLVideo).
                SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -bundle_loader \"${BUNDLE_LOADER}\"")

                # icon
                SET_SOURCE_FILES_PROPERTIES(
                        ${APP_ICNS}
                        PROPERTIES MACOSX_PACKAGE_LOCATION Resources
                )            
                # default values
                SET_SOURCE_FILES_PROPERTIES(
                        ${APP_PLIST}
                        PROPERTIES MACOSX_PACKAGE_LOCATION .
                )

                SET(MACOSX_BUNDLE_ICON_FILE ${APP_ICNS})
                SET(PRODUCT_NAME ${PROJECT_NAME}) # for info.plist
                ADD_EXECUTABLE(${PROJECTNAMEL} WIN32 MACOSX_BUNDLE
                    ${HEADERS}
                    ${SOURCES}
                    ${RESOURCES}
                    ${OTHER_FILES}
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
            ### 应用程序的名称不强制改变大小写。
            # 生成目标程序
            ADD_EXECUTABLE(${PROJECTNAMEL} ${WINDOWS}
                ${HEADERS}
                ${SOURCES}
                ${RESOURCES}
                ${OTHER_FILES}
				${SOURCE_GROUP_FILES}
            )
            IF (MSVC)
                ##需要管理员权限,放在add_executable的后面
                SET_TARGET_PROPERTIES(${PROJECTNAMEL} 
                    PROPERTIES LINK_FLAGS "/level='requireAdministrator' /uiAccess='false'")
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
    ELSEIF(${INIT_TYPE} STREQUAL "LIB")
        # Utils shared library
        ADD_LIBRARY(${PROJECTNAMEL} STATIC
            ${HEADERS}
            ${SOURCES}
			${RESOURCES}
            ${OTHER_FILES}
			${SOURCE_GROUP_FILES}
        )
    ELSE()
        # Utils shared library
        ADD_LIBRARY(${PROJECTNAMEL} SHARED
            ${HEADERS}
            ${SOURCES}
			${RESOURCES}
            ${OTHER_FILES}
			${SOURCE_GROUP_FILES}
        )
    ENDIF()
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
	
    ## 当前工程依赖的lib
    SET(LINK_DEPS
        )
    FOREACH(deps ${WIS_MODULE_DEPS})
        STRING(TOUPPER "${deps}" deps)
        LIST(APPEND LINK_DEPS "${${deps}_TARGET_LINK}")
    ENDFOREACH()

    FOREACH(deps ${DEPENDENCIES})
        LIST(APPEND LINK_DEPS "${deps}")
    ENDFOREACH()
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
    TARGET_LINK_LIBRARIES(${PROJECTNAMEL}
        ${LINK_DEPS}
    )
    STRING(LENGTH "${WIS_MODULE_DEPS}" varlen)
    IF(NOT ${varlen} MATCHES "0")
        ADD_DEPENDENCIES(${PROJECTNAMEL}
            ${WIS_MODULE_DEPS}
        )
    ENDIF()
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
    IF(WIN32)
        IF(MSVC)
        ## 设置多核编译
        SET_TARGET_PROPERTIES(${PROJECTNAMEL}
            PROPERTIES
            COMPILE_FLAGS "/MP" )  #Yes 多核编译
       
            SET_TARGET_PROPERTIES( ${PROJECTNAMEL}
                PROPERTIES
                COMPILE_FLAGS "/EHsc" ) #yes 启用C++异常
        ENDIF()
    ENDIF()

    # configuration summary
    boost_report_value(LINK_DEPS)
    wis_report_directory_property(COMPILE_DEFINITIONS)
    IF (NOT ${INIT_TYPE} MATCHES "EXE" AND NOT ${INIT_TYPE} MATCHES "CONSOLE")
        wis_target_output_name(${PROJECTNAMEL} ${PROJECTNAMEU}_TARGET_LINK)
        # # Export core target name to make it visible by backends
        SET(${PROJECTNAMEU}_TARGET_LINK ${${PROJECTNAMEU}_TARGET_LINK} PARENT_SCOPE)
    ENDIF()
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
# OTHER_FILES    其它文件
#
# DEPENDENCIES     依赖的第三方的lib
#
# WIS_MODULE_DEPS 依赖的本解决方案内的项目名称

MACRO(WIS_INIT_LIBRARY_MODULE )
    ##构造目标项目
    WIS_INIT_LIBRARY_MODULE_PCH("" "")
ENDMACRO(WIS_INIT_LIBRARY_MODULE )
#
# 带预编译头的动态链接库
#
MACRO(WIS_INIT_LIBRARY_MODULE_PCH
    PrecompiledHeader PrecompiledSource
    )
    #项目名称大写
    STRING(TOUPPER "${PROJECT_NAME}" PROJECTNAMEU)
    ## 动态库导出
    ADD_DEFINITIONS(-D${PROJECTNAMEU}_LIBRARY)
    ##构造目标项目
    WIS_INIT_COMMON_LIB("DLL")
    ## 预编译头设置
    WIS_INIT_PCH("${PrecompiledHeader}" "${PrecompiledSource}")
	
ENDMACRO(WIS_INIT_LIBRARY_MODULE_PCH )
#_______________________________________________________________________________
# 静态链接库
#
MACRO(WIS_INIT_STATIC_MODULE)
    ##构造目标项目
    WIS_INIT_STATIC_MODULE_PCH("" "")
ENDMACRO(WIS_INIT_STATIC_MODULE)
#
# 带预编译头的静态链接库
#
MACRO(WIS_INIT_STATIC_MODULE_PCH
    PrecompiledHeader PrecompiledSource)
    #项目名称大写
    STRING(TOUPPER "${PROJECT_NAME}" PROJECTNAMEU)
    ## 静态库导出
    ADD_DEFINITIONS(-D${PROJECTNAMEU}_STATIC_LIB)
    ##构造目标项目
    WIS_INIT_COMMON_LIB("LIB")
    ## 预编译头设置
    WIS_INIT_PCH("${PrecompiledHeader}" "${PrecompiledSource}")
	
ENDMACRO(WIS_INIT_STATIC_MODULE_PCH)
################################################################################
# 控制台应用程序
MACRO(WIS_INIT_CONSOLE_EXE_MODULE)
    ##构造目标项目
    WIS_INIT_CONSOLE_EXE_MODULE_PCH("" "")
ENDMACRO(WIS_INIT_CONSOLE_EXE_MODULE)
#_______________________________________________________________________________
# 可执行程序 带预编译头
#
#
MACRO(WIS_INIT_CONSOLE_EXE_MODULE_PCH
    PrecompiledHeader PrecompiledSource)
    ##构造目标项目
    WIS_INIT_COMMON_LIB("CONSOLE")
    ## 预编译头设置
    WIS_INIT_PCH("${PrecompiledHeader}" "${PrecompiledSource}")
ENDMACRO(WIS_INIT_CONSOLE_EXE_MODULE_PCH)

################################################################################
# GUI应用程序
#_______________________________________________________________________________
# 可执行程序
#
#
MACRO(WIS_INIT_EXE_MODULE)
    ##构造目标项目
    WIS_INIT_EXE_MODULE_PCH("" "")
ENDMACRO(WIS_INIT_EXE_MODULE)
#_______________________________________________________________________________
# 可执行程序 带预编译头
#
#
MACRO(WIS_INIT_EXE_MODULE_PCH
    PrecompiledHeader PrecompiledSource)
    ##构造目标项目
    WIS_INIT_COMMON_LIB("EXE")
    ## 预编译头设置
    WIS_INIT_PCH("${PrecompiledHeader}" "${PrecompiledSource}")
ENDMACRO(WIS_INIT_EXE_MODULE_PCH)
################################################################################
# Tool程序(非主应用程序，主要表现再Mac，上不生成.app）
#_______________________________________________________________________________
# 可执行程序
#
#
MACRO(WIS_INIT_TOOL_EXE_MODULE)
    ##构造目标项目
    SET(IS_TOOL_APP
        TRUE)
    WIS_INIT_TOOL_EXE_MODULE_PCH("" "")
    SET(IS_TOOL_APP
        FALSE)
ENDMACRO(WIS_INIT_TOOL_EXE_MODULE)
#_______________________________________________________________________________
# 可执行程序 带预编译头
#
#
MACRO(WIS_INIT_TOOL_EXE_MODULE_PCH
    PrecompiledHeader PrecompiledSource)
    ##构造目标项目
    SET(IS_TOOL_APP
        TRUE)
    WIS_INIT_COMMON_LIB("EXE")
    ## 预编译头设置
    WIS_INIT_PCH("${PrecompiledHeader}" "${PrecompiledSource}")
    SET(IS_TOOL_APP
        FALSE)
ENDMACRO(WIS_INIT_TOOL_EXE_MODULE_PCH)

# 可执行程序
#
#
MACRO(WIS_INIT_TOOL_CONSOLE_MODULE)
    ##构造目标项目
    SET(IS_TOOL_APP
        TRUE)
    WIS_INIT_TOOL_CONSOLE_MODULE_PCH("" "")
    SET(IS_TOOL_APP
        FALSE)
ENDMACRO(WIS_INIT_TOOL_CONSOLE_MODULE)
#_______________________________________________________________________________
# 可执行程序 带预编译头
#
#
MACRO(WIS_INIT_TOOL_CONSOLE_MODULE_PCH
    PrecompiledHeader PrecompiledSource)
    ##构造目标项目
    SET(IS_TOOL_APP
        TRUE)
    WIS_INIT_COMMON_LIB("CONSOLE")
    ## 预编译头设置
    WIS_INIT_PCH("${PrecompiledHeader}" "${PrecompiledSource}")
    SET(IS_TOOL_APP
        FALSE)
ENDMACRO(WIS_INIT_TOOL_CONSOLE_MODULE_PCH)
