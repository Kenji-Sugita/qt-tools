cmake_minimum_required(VERSION 3.16)

project(cutter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Gui)

qt_standard_project_setup()

qt_add_executable(cutter
    main.cpp
)

target_link_libraries(cutter PRIVATE
    Qt6::Core
    Qt6::Gui
)
