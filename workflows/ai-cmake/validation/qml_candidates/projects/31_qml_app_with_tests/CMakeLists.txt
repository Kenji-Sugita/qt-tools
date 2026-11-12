cmake_minimum_required(VERSION 3.16)

project(Qml31AppWithTests VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

find_package(Qt6 REQUIRED COMPONENTS Quick Test)

qt_standard_project_setup()

qt_add_library(Qml31Core
    formatter.cpp
    formatter.h
)
target_link_libraries(Qml31Core PUBLIC Qt6::Core)

qt_add_executable(Qml31App
    main.cpp
)
qt_add_qml_module(Qml31App
    URI Qml31App
    VERSION 1.0
    QML_FILES
        Main.qml
)
target_link_libraries(Qml31App PRIVATE Qt6::Quick Qml31Core)

set_target_properties(Qml31App PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

qt_add_executable(Qml31CoreTest
    tst_formatter.cpp
)
target_link_libraries(Qml31CoreTest PRIVATE Qt6::Test Qml31Core)
add_test(NAME Qml31CoreTest COMMAND Qml31CoreTest)
