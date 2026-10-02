cmake_minimum_required(VERSION 3.16)

if(APPLE AND NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build universal macOS binaries" FORCE)
endif()

project(LaserPointer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

if(WIN32)
    set(APP_ICON_RESOURCE_WINDOWS assets/LaserPointer.rc)
endif()

qt_add_executable(LaserPointer
    assets/LaserPointer.icns
    ${APP_ICON_RESOURCE_WINDOWS}
    src/LaserPointerMenuBuilder.cpp
    src/LaserPointerSettings.cpp
    src/LaserPointerWidget.cpp
    src/TrailSpotWidget.cpp
    src/main.cpp
    include/LaserPointerMenuBuilder.h
    include/LaserPointerSettings.h
    include/LaserPointerWidget.h
    include/TrailSpotWidget.h
)

qt_add_resources(LaserPointer "app_resources"
    PREFIX
        "/"
    FILES
        assets/laser-pointer.png
)

target_include_directories(LaserPointer
    PRIVATE
        include
)

target_link_libraries(LaserPointer
    PRIVATE
        Qt6::Widgets
)

set_source_files_properties(assets/LaserPointer.icns PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources
)

set_target_properties(LaserPointer PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
    MACOSX_BUNDLE_ICON_FILE LaserPointer.icns
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/assets/MacOSXBundleInfo.plist.in"
)
