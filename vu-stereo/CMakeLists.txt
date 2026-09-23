cmake_minimum_required(VERSION 3.16)

set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "Minimum macOS deployment version" FORCE)

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" VUSTEREO_VERSION LIMIT_COUNT 1)
string(STRIP "${VUSTEREO_VERSION}" VUSTEREO_VERSION)
if(NOT VUSTEREO_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    message(FATAL_ERROR "VERSION must contain a semantic version such as 1.2.3")
endif()

project(VuStereo VERSION "${VUSTEREO_VERSION}" LANGUAGES CXX OBJCXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg LinguistTools)

qt_standard_project_setup()

qt_add_executable(VuStereo
    include/audiolevelcontroller.h
    include/audiolevelmonitor.h
    include/dragwindow.h
    include/metersettings.h
    include/meterbasewidget.h
    include/settingsdialog.h
    include/translationmanager.h
    include/vuapplicationwindow.h
    include/vumeterpanel.h
    include/vumeterwidget.h
    images/app-icon.icns
    src/audiolevelcontroller.cpp
    src/audiolevelmonitor.mm
    src/dragwindow.cpp
    src/main.cpp
    src/meterbasewidget.cpp
    src/settingsdialog.cpp
    src/translationmanager.cpp
    src/vuapplicationwindow.cpp
    src/vumeterpanel.cpp
    src/vumeterwidget.cpp
)

target_include_directories(VuStereo
    PRIVATE
        include
)

qt_add_resources(VuStereo "vu_resources"
    PREFIX "/"
    FILES
        images/app-icon.svg
        images/vu-background.svg
        images/vu-linear-background.svg
)

qt_add_lrelease(
    TS_FILES translations/vu-stereo_ja.ts
    QM_FILES_OUTPUT_VARIABLE translation_qm_files
)

qt_add_resources(VuStereo "translation_resources"
    PREFIX "/i18n"
    BASE "${CMAKE_CURRENT_BINARY_DIR}"
    FILES
        ${translation_qm_files}
)

target_link_libraries(VuStereo
    PRIVATE
        Qt6::Widgets
        Qt6::Svg
        "-framework CoreGraphics"
        "-framework CoreMedia"
        "-framework ScreenCaptureKit"
)

set_source_files_properties(images/app-icon.icns PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
)

set_target_properties(VuStereo PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
    MACOSX_BUNDLE_ICON_FILE app-icon.icns
    MACOSX_BUNDLE_GUI_IDENTIFIER jp.co.sra.vu-stereo
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
)

add_custom_target(sign_VuStereo ALL
    COMMAND ${CMAKE_COMMAND} -DAPP_BUNDLE=$<TARGET_BUNDLE_DIR:VuStereo> -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/sign_app.cmake"
    DEPENDS VuStereo
    COMMENT "Signing VuStereo.app"
    VERBATIM
)

add_custom_target(source_release
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -DRELEASE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/release
        -DRELEASE_VERSION=${PROJECT_VERSION}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/make_source_release.cmake
    COMMENT "Creating VuStereo source release zip"
    VERBATIM
)
