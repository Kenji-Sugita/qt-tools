cmake_minimum_required(VERSION 3.26)

project(tcp-local-server LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(tcp-local-server)
else()
    add_executable(tcp-local-server)
endif()

target_sources(tcp-local-server PRIVATE
    main.cpp
    connectionwindow.cpp
    connectionwindow.h
    server.cpp
    server.h
)

target_link_libraries(tcp-local-server PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
                                               Qt${QT_VERSION_MAJOR}::Network)

set_target_properties(tcp-local-server PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
