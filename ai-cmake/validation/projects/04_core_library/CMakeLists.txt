cmake_minimum_required(VERSION 3.16)

project(Pattern04CoreLibrary VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

qt_standard_project_setup()

qt_add_library(Pattern04CoreLibrary
    myobject.cpp
    myobject.h
)

target_link_libraries(Pattern04CoreLibrary
    PUBLIC
        Qt6::Core
)
