cmake_minimum_required(VERSION 3.21)

project(SeminarTimer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

set(app_icon_macos "${CMAKE_CURRENT_SOURCE_DIR}/SeminarTimer.icns")
set_source_files_properties(${app_icon_macos} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

qt_add_executable(SeminarTimer
    MACOSX_BUNDLE
    main.cpp
    ${app_icon_macos}
)

set_target_properties(SeminarTimer PROPERTIES
    OUTPUT_NAME "SeminarTimer"
    MACOSX_BUNDLE_BUNDLE_NAME "SeminarTimer"
    MACOSX_BUNDLE_ICON_FILE "SeminarTimer.icns"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.sugita.seminartimer"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
)

target_link_libraries(SeminarTimer PRIVATE Qt6::Widgets)
