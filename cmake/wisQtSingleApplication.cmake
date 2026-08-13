################################################################################
# wisQtSingleApplication.cmake - QtSingleApplication 第三方静态库
################################################################################

IF(NOT TARGET QtSingleApplication)
    SET(QT_SINGLE_APPLICATION_SOURCES
        ${THIRDPARTY_ROOT}/qt/qtlockedfile/qtlockedfile.cpp
        ${THIRDPARTY_ROOT}/qt/qtsingleapplication/qtsingleapplication.cpp
        ${THIRDPARTY_ROOT}/qt/qtsingleapplication/qtlocalpeer.cpp
    )

    IF(WIN32)
        LIST(APPEND QT_SINGLE_APPLICATION_SOURCES
            ${THIRDPARTY_ROOT}/qt/qtlockedfile/qtlockedfile_win.cpp
        )
    ELSE()
        LIST(APPEND QT_SINGLE_APPLICATION_SOURCES
            ${THIRDPARTY_ROOT}/qt/qtlockedfile/qtlockedfile_unix.cpp
        )
    ENDIF()

    SET(QT_SINGLE_APPLICATION_HEADERS
        ${THIRDPARTY_ROOT}/qt/qtlockedfile/qtlockedfile.h
        ${THIRDPARTY_ROOT}/qt/qtsingleapplication/qtlocalpeer.h
        ${THIRDPARTY_ROOT}/qt/qtsingleapplication/qtsingleapplication.h
    )

    ADD_LIBRARY(QtSingleApplication STATIC
        ${QT_SINGLE_APPLICATION_SOURCES}
        ${QT_SINGLE_APPLICATION_HEADERS}
    )

    TARGET_INCLUDE_DIRECTORIES(QtSingleApplication PUBLIC
        ${THIRDPARTY_ROOT}/qt/qtsingleapplication
        ${THIRDPARTY_ROOT}/qt/qtlockedfile
    )

    FIND_PACKAGE(Qt5 COMPONENTS Core Widgets Network REQUIRED)

    TARGET_LINK_LIBRARIES(QtSingleApplication PUBLIC
        Qt5::Core
        Qt5::Widgets
        Qt5::Network
    )

    SET_TARGET_PROPERTIES(QtSingleApplication PROPERTIES
        FOLDER "ThirdParty"
    )
ENDIF()
