cmake_minimum_required(VERSION 3.24)

project(sequential VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Gui Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Concurrent)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(sequential)
else()
    add_executable(sequential)
endif()

target_sources(sequential PRIVATE
    main.cpp
)

target_link_libraries(sequential PRIVATE Qt${QT_VERSION_MAJOR}::Gui
                                      Qt${QT_VERSION_MAJOR}::Concurrent)
