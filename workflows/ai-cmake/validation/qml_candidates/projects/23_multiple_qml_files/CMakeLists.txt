cmake_minimum_required(VERSION 3.16)

project(Qml23MultipleFiles VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup()



qt_add_executable(Qml23MultipleFiles
    main.cpp
)

qt_add_qml_module(Qml23MultipleFiles
    URI Qml23MultipleFiles
    VERSION 1.0
    QML_FILES
        Main.qml
        components/StatusPanel.qml
)

target_link_libraries(Qml23MultipleFiles
    PRIVATE
        Qt6::Quick
)

set_target_properties(Qml23MultipleFiles PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
