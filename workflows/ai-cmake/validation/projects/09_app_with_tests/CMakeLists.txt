cmake_minimum_required(VERSION 3.16)

project(Pattern09TestableApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Test)

qt_standard_project_setup()

qt_add_library(Pattern09AppCore
    calculator.cpp
    calculator.h
)

target_link_libraries(Pattern09AppCore
    PUBLIC
        Qt6::Core
)

qt_add_executable(Pattern09App
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_link_libraries(Pattern09App
    PRIVATE
        Pattern09AppCore
        Qt6::Widgets
)

set_target_properties(Pattern09App PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

qt_add_executable(Pattern09AppCoreTest
    tst_calculator.cpp
)

target_link_libraries(Pattern09AppCoreTest
    PRIVATE
        Pattern09AppCore
        Qt6::Test
)

add_test(NAME Pattern09AppCoreTest COMMAND Pattern09AppCoreTest)
