cmake_minimum_required(VERSION 3.26)

project(q_variant LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(q_variant)
else()
    add_executable(q_variant)
endif()

target_sources(q_variant PRIVATE
    main.cpp
    data.cpp
    data.h
)

target_link_libraries(q_variant PRIVATE Qt${QT_VERSION_MAJOR}::Core)
