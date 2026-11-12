cmake_minimum_required(VERSION 3.16)

project(Pattern14SqlApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Sql)

qt_standard_project_setup()

qt_add_executable(Pattern14SqlApp
    main.cpp
    repository.cpp
    repository.h
)

target_link_libraries(Pattern14SqlApp
    PRIVATE
        Qt6::Core
        Qt6::Sql
)
