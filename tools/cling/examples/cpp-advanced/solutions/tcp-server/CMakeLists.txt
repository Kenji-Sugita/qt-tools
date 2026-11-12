cmake_minimum_required(VERSION 3.26)

project(tcp-server LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(tcp-server)
else()
    add_executable(tcp-server)
endif()

target_sources(tcp-server PRIVATE
    main.cpp
    connectionwindow.cpp
    connectionwindow.h
    server.cpp
    server.h
)

target_link_libraries(tcp-server PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
                                               Qt${QT_VERSION_MAJOR}::Network)

set_target_properties(tcp-server PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
