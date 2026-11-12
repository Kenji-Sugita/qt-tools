cmake_minimum_required(VERSION 3.16)

project(Qml24Resources VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup()



qt_add_executable(Qml24Resources
    main.cpp
)

qt_add_qml_module(Qml24Resources
    URI Qml24Resources
    VERSION 1.0
    QML_FILES
        Main.qml

    RESOURCES
        images/logo.txt
        fonts/readme.txt
)

target_link_libraries(Qml24Resources
    PRIVATE
        Qt6::Quick
)

set_target_properties(Qml24Resources PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
