cmake_minimum_required(VERSION 3.16)

project(tl LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTING "Build tests" ON)
if (BUILD_TESTING)
    enable_testing()
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Network)

qt_standard_project_setup()

qt_add_executable(tl
    src/clioptionsparser.cpp
    src/googleapiclient.cpp
    src/inputresolver.cpp
    src/main.cpp
    src/tlapplication.cpp
    src/translationservice.cpp
)

target_include_directories(tl PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(tl PRIVATE
    Qt6::Core
    Qt6::Network
)

if (BUILD_TESTING)
    add_test(
        NAME tl_cli
        COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_tests.sh $<TARGET_FILE:tl>
    )
endif()
