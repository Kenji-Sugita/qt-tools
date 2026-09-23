cmake_minimum_required(VERSION 3.16)

set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "")
project(qeyes VERSION 1.2 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_standard_project_setup()

set(app_icon_macos "${CMAKE_CURRENT_SOURCE_DIR}/assets/qeyes.icns")
set(app_icon_windows "${CMAKE_CURRENT_SOURCE_DIR}/assets/qeyes.ico")

qt_add_executable(qeyes
    MACOSX_BUNDLE
    WIN32
    main.cpp
)

if(APPLE)
    target_sources(qeyes PRIVATE "${app_icon_macos}")
    set_source_files_properties("${app_icon_macos}" PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )
    set_target_properties(qeyes PROPERTIES
        MACOSX_BUNDLE_ICON_FILE "qeyes"
    )
endif()

if(WIN32)
    target_sources(qeyes PRIVATE qeyes.rc)
endif()

target_link_libraries(qeyes PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

install(TARGETS qeyes
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
