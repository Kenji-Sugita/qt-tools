cmake_minimum_required(VERSION 3.16)

project(Qml26Singleton VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup()


set_source_files_properties(Theme.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)


qt_add_executable(Qml26Singleton
    main.cpp
)

qt_add_qml_module(Qml26Singleton
    URI Qml26Singleton
    VERSION 1.0
    QML_FILES
        Main.qml
        Theme.qml
)

target_link_libraries(Qml26Singleton
    PRIVATE
        Qt6::Quick
)

set_target_properties(Qml26Singleton PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
