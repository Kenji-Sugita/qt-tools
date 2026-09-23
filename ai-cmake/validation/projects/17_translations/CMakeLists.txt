cmake_minimum_required(VERSION 3.16)

project(Pattern17TranslatedApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets LinguistTools)

qt_standard_project_setup()

qt_add_executable(Pattern17TranslatedApp
    main.cpp mainwindow.cpp mainwindow.h
)


qt_add_translations(Pattern17TranslatedApp
    TS_FILES
        translations/Pattern17TranslatedApp_ja.ts
)


target_link_libraries(Pattern17TranslatedApp
    PRIVATE
        Qt6::Widgets
)

set_target_properties(Pattern17TranslatedApp PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
