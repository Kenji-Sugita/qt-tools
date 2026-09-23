cmake_minimum_required(VERSION 3.26)

project(colorwidget LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets LinguistTools)

qt_standard_project_setup()

qt_add_executable(colorwidget
    main.cpp
    colorwidget.cpp
    colorwidget.h
    colorwidget.ui
)

target_link_libraries(colorwidget PRIVATE Qt6::Widgets)

set_target_properties(colorwidget PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

set(TS_FILES
    "${CMAKE_CURRENT_LIST_DIR}/colorwidget_ja.ts"
)

qt_add_translations(
    TARGETS colorwidget
    TS_FILES ${TS_FILES}
)
