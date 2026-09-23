cmake_minimum_required(VERSION 3.16)

project(Pattern13NetworkApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

qt_standard_project_setup()

qt_add_executable(Pattern13NetworkApp
    main.cpp
    downloader.cpp
    downloader.h
)

target_link_libraries(Pattern13NetworkApp
    PRIVATE
        Qt6::Core
        Qt6::Network
)
