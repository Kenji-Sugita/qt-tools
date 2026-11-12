cmake_minimum_required(VERSION 3.16)

project(Qml32Install VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup()



qt_add_executable(Qml32Install
    main.cpp
)

qt_add_qml_module(Qml32Install
    URI Qml32Install
    VERSION 1.0
    QML_FILES
        Main.qml
)

target_link_libraries(Qml32Install
    PRIVATE
        Qt6::Quick
)

set_target_properties(Qml32Install PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)


install(TARGETS Qml32Install
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
