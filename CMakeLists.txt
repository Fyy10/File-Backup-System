cmake_minimum_required(VERSION 3.5)

project(File-Backup-System VERSION 0.1 LANGUAGES CXX)

# add libraries
add_subdirectory(core)
add_subdirectory(extensions)
add_subdirectory(net)
# resource dir
add_subdirectory(res)
# include headers
include_directories(
    core
    extensions
    net
)
set(FBS_LIBS
    core
    extensions
    net
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# the default is /usr/local to prevent destroying the system using `sudo make install`
# set(CMAKE_INSTALL_PREFIX /usr)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check https://doc.qt.io/qt/deployment-android.html for more information.
# They need to be set before the find_package( ...) calls below.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        loginwindow.cpp
        loginwindow.h
        loginwindow.ui
        signwindow.cpp
        signwindow.h
        signwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(File-Backup-System
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(File-Backup-System SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(File-Backup-System
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(File-Backup-System PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${FBS_LIBS})

set_target_properties(File-Backup-System PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(File-Backup-System)
endif()

# install binary
install(
    TARGETS File-Backup-System
    RUNTIME DESTINATION bin
)