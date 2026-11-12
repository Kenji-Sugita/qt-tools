cmake_minimum_required(VERSION 3.16)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" PROPERTY_EDITOR_VERSION)
string(STRIP "${PROPERTY_EDITOR_VERSION}" PROPERTY_EDITOR_VERSION)

project(property_editor VERSION ${PROPERTY_EDITOR_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

set(ICPP_PREFIX "" CACHE PATH "Install prefix of icpp used as the default install destination")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(_property_editor_default_install_prefix "")

    if(ICPP_PREFIX)
        set(_property_editor_default_install_prefix "${ICPP_PREFIX}")
    else()
        find_program(ICPP_EXECUTABLE NAMES icpp)
        if(ICPP_EXECUTABLE)
            get_filename_component(_icpp_bin_dir "${ICPP_EXECUTABLE}" DIRECTORY)
            get_filename_component(_property_editor_default_install_prefix "${_icpp_bin_dir}" DIRECTORY)
        endif()
    endif()

    if(_property_editor_default_install_prefix)
        set(CMAKE_INSTALL_PREFIX "${_property_editor_default_install_prefix}" CACHE PATH
            "Install path prefix, prepended onto install directories." FORCE)
    endif()
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Test)
qt_standard_project_setup()

find_package(Qt6 QUIET COMPONENTS OpenGLWidgets QuickWidgets WebEngineWidgets)

add_library(property_editor SHARED
    src/objecthighlighter.cpp
    src/objecthighlighter.h
    src/objectpicker.cpp
    src/objectpicker.h
    src/objecttreedialog.cpp
    src/objecttreedialog.h
    src/objecttreewidget.cpp
    src/objecttreewidget.h
    src/propertyeditor.cpp
    src/propertyeditorglobal.h
    src/objectpropertyeditorwidget.cpp
    src/objectpropertyeditorwidget.h
)

target_include_directories(property_editor
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(property_editor
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

target_compile_definitions(property_editor
    PRIVATE
        PROPERTY_EDITOR_LIBRARY
)

if(APPLE)
    set_target_properties(property_editor PROPERTIES
        OUTPUT_NAME "property_editor"
        PREFIX ""
        INSTALL_RPATH "@loader_path;@loader_path/../lib"
        INSTALL_RPATH_USE_LINK_PATH TRUE
        BUILD_WITH_INSTALL_RPATH OFF
    )
else()
    set_target_properties(property_editor PROPERTIES
        OUTPUT_NAME "property_editor"
        PREFIX ""
    )
endif()

add_executable(property_editor_demo
    examples/demo_main.cpp
)

if(APPLE)
    set_target_properties(property_editor_demo PROPERTIES
        INSTALL_RPATH "@executable_path/../lib"
        INSTALL_RPATH_USE_LINK_PATH TRUE
        BUILD_WITH_INSTALL_RPATH OFF
    )
endif()

target_link_libraries(property_editor_demo
    PRIVATE
        property_editor
)

if(TARGET Qt6::OpenGLWidgets)
    target_link_libraries(property_editor_demo PRIVATE Qt6::OpenGLWidgets)
    target_compile_definitions(property_editor_demo PRIVATE HAVE_QT_OPENGLWIDGETS)
endif()

if(TARGET Qt6::QuickWidgets)
    target_link_libraries(property_editor_demo PRIVATE Qt6::QuickWidgets)
    target_compile_definitions(property_editor_demo PRIVATE HAVE_QT_QUICKWIDGETS)
endif()

if(TARGET Qt6::WebEngineWidgets)
    target_link_libraries(property_editor_demo PRIVATE Qt6::WebEngineWidgets)
    target_compile_definitions(property_editor_demo PRIVATE HAVE_QT_WEBENGINEWIDGETS)
endif()

install(TARGETS
    property_editor
    property_editor_demo
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES
    src/objectpicker.h
    src/objecttreedialog.h
    src/objecttreewidget.h
    src/propertyeditor.h
    src/propertyeditorglobal.h
    DESTINATION include
)

install(FILES
    VERSION
    DESTINATION .
)

include(CTest)

if(BUILD_TESTING)
    add_executable(property_editor_test
        tests/objectpropertyeditorwidget_test.cpp
    )

    target_link_libraries(property_editor_test
        PRIVATE
            property_editor
            Qt6::Test
    )

    add_test(NAME property_editor_test COMMAND property_editor_test)

endif()
