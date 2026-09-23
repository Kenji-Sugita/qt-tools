cmake_minimum_required(VERSION 3.16)

project(Pattern08QtTest VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

find_package(Qt6 REQUIRED COMPONENTS Test)

qt_standard_project_setup()

qt_add_executable(Pattern08QtTest
    tst_validation.cpp
)

target_link_libraries(Pattern08QtTest
    PRIVATE
        Qt6::Test
)

add_test(NAME Pattern08QtTest COMMAND Pattern08QtTest)
