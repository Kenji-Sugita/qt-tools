cmake_minimum_required(VERSION 3.16)

project(Qml29MultipleModules VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup()

qt_add_library(ThemeModule STATIC)
qt_add_qml_module(ThemeModule
    URI AppTheme
    VERSION 1.0
    OUTPUT_DIRECTORY AppTheme
    QML_FILES
        ThemePalette.qml
)
target_link_libraries(ThemeModule PRIVATE Qt6::Quick)

qt_add_library(ComponentModule STATIC)
qt_add_qml_module(ComponentModule
    URI AppComponents
    VERSION 1.0
    OUTPUT_DIRECTORY AppComponents
    QML_FILES
        InfoCard.qml
)
target_link_libraries(ComponentModule PRIVATE Qt6::Quick ThemeModule)

qt_add_executable(Qml29App
    main.cpp
)
qt_add_qml_module(Qml29App
    URI Qml29App
    VERSION 1.0
    QML_FILES
        Main.qml
)
target_link_libraries(Qml29App PRIVATE Qt6::Quick ThemeModule ComponentModule)

set_target_properties(Qml29App PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
