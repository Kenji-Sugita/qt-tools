cmake_minimum_required(VERSION 3.16)

project(Qml28ModuleLibrary VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup()

qt_add_library(ReusableUi STATIC)

qt_add_qml_module(ReusableUi
    URI ReusableUi
    VERSION 1.0
    OUTPUT_DIRECTORY ReusableUi
    QML_FILES
        ReusableButton.qml
)

target_link_libraries(ReusableUi
    PRIVATE
        Qt6::Quick
)

qt_add_executable(Qml28App
    main.cpp
)

qt_add_qml_module(Qml28App
    URI Qml28App
    VERSION 1.0
    QML_FILES
        Main.qml
)

target_link_libraries(Qml28App
    PRIVATE
        Qt6::Quick
        ReusableUi
)

set_target_properties(Qml28App PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
