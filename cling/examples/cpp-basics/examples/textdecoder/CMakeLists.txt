cmake_minimum_required(VERSION 3.26)

project(textdecoder LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(textdecoder)
else()
    add_executable(textdecoder)
endif()

target_sources(textdecoder PRIVATE
    main.cpp
)

if(QT_VERSION_MAJOR EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Core5Compat)
    target_link_libraries(textdecoder PRIVATE Qt6::Core Qt6::Core5Compat)
else()
    target_link_libraries(textdecoder PRIVATE Qt5::Core)
endif()
