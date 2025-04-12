cmake_minimum_required(VERSION 3.16)

project(IMClient VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 匹配 protobuf 文件
file(GLOB PB_FILES "./proto/*.proto")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network WebSockets Protobuf Multimedia)

set(PROJECT_SOURCES
        main.cpp
        mainwidget.cpp
        mainwidget.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(IMClient
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        model/data.h
        common.h
        resource.qrc
        sessionfriendarea.h sessionfriendarea.cpp
        debug.h
        messageshowarea.h messageshowarea.cpp
        messageeditarea.h messageeditarea.cpp
        selfinfowidget.h selfinfowidget.cpp
        userinfowidget.h userinfowidget.cpp
        sessiondetailwidget.h sessiondetailwidget.cpp
        choosefrienddialog.h choosefrienddialog.cpp
        groupsessiondetailwidget.h groupsessiondetailwidget.cpp
        addfrienddialog.h addfrienddialog.cpp
        historymessagewidget.h historymessagewidget.cpp
        loginwidget.h loginwidget.cpp
        phoneloginwidget.h phoneloginwidget.cpp
        toast.h toast.cpp
        proto/base.proto proto/file.proto proto/friend.proto proto/gateway.proto proto/message_storage.proto proto/message_transmit.proto proto/notify.proto proto/speech_recognition.proto proto/user.proto
        model/datacenter.h model/datacenter.cpp
        network/netclient.h network/netclient.cpp
        verifycodewidget.h verifycodewidget.cpp
        soundrecorder.h soundrecorder.cpp
        imageviewer.h imageviewer.cpp
    )

# 引入 protobuf
qt_add_protobuf(IMClient PROTO_FILES ${PB_FILES})

# Define target properties for Android with Qt 6 as:
#    set_property(TARGET IMClient APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(IMClient SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(IMClient
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(IMClient PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt6::Network Qt6::WebSockets Qt6::Protobuf Qt6::Multimedia)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.IMClient)
endif()
set_target_properties(IMClient PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS IMClient
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(IMClient)
endif()
