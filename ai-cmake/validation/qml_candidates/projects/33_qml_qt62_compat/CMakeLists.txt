cmake_minimum_required(VERSION 3.16)

project(Qml33Qt62Compat VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

add_executable(Qml33Qt62Compat
    main.cpp
)

qt_add_resources(Qml33Qt62Compat "qml_resources"
    PREFIX "/qt/qml/Qml33Qt62Compat"
    FILES
        Main.qml
)

target_link_libraries(Qml33Qt62Compat
    PRIVATE
        Qt6::Quick
)
