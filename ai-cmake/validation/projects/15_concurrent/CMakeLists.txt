cmake_minimum_required(VERSION 3.16)

project(Pattern15ConcurrentApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Concurrent)

qt_standard_project_setup()

qt_add_executable(Pattern15ConcurrentApp
    main.cpp
    worker.cpp
    worker.h
)

target_link_libraries(Pattern15ConcurrentApp
    PRIVATE
        Qt6::Core
        Qt6::Concurrent
)
