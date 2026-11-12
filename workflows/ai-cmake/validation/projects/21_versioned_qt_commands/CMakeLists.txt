cmake_minimum_required(VERSION 3.16)

project(Pattern21VersionedQtApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QT_NO_CREATE_VERSIONLESS_FUNCTIONS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt6_standard_project_setup()

qt6_add_executable(Pattern21VersionedQtApp
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_link_libraries(Pattern21VersionedQtApp
    PRIVATE
        Qt6::Widgets
)
