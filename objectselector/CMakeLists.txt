cmake_minimum_required(VERSION 3.16)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" OBJECTSELECTOR_VERSION)
string(STRIP "${OBJECTSELECTOR_VERSION}" OBJECTSELECTOR_VERSION)

if(NOT OBJECTSELECTOR_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    message(FATAL_ERROR "VERSION must contain a semantic version (major.minor.patch).")
endif()

project(objectselector VERSION ${OBJECTSELECTOR_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

set(ICPP_PREFIX "" CACHE PATH "Install prefix of icpp used as the default install destination")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(_objectselector_default_install_prefix "")

    if(ICPP_PREFIX)
        set(_objectselector_default_install_prefix "${ICPP_PREFIX}")
    else()
        find_program(ICPP_EXECUTABLE NAMES icpp)
        if(ICPP_EXECUTABLE)
            get_filename_component(_icpp_bin_dir "${ICPP_EXECUTABLE}" DIRECTORY)
            get_filename_component(_objectselector_default_install_prefix "${_icpp_bin_dir}" DIRECTORY)
        endif()
    endif()

    if(_objectselector_default_install_prefix)
        set(CMAKE_INSTALL_PREFIX "${_objectselector_default_install_prefix}" CACHE PATH
            "Install path prefix, prepended onto install directories." FORCE)
    endif()
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Test)
qt_standard_project_setup()

add_library(objectselector SHARED
    src/objectselectorglobal.h
    src/objectpicker.cpp
    src/objectpicker.h
    src/objecthighlighter.cpp
    src/objecthighlighter.h
    src/objecttreewidget.cpp
    src/objecttreewidget.h
    src/objecttreedialog.cpp
    src/objecttreedialog.h
)

target_include_directories(objectselector
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(objectselector
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

target_compile_definitions(objectselector
    PRIVATE
        OBJECT_SELECTOR_LIBRARY
)

if(APPLE)
    set_target_properties(objectselector PROPERTIES
        OUTPUT_NAME "objectselector"
        PREFIX ""
        INSTALL_RPATH "@loader_path;@loader_path/../lib"
        INSTALL_RPATH_USE_LINK_PATH TRUE
        BUILD_WITH_INSTALL_RPATH OFF
    )
else()
    set_target_properties(objectselector PROPERTIES
        OUTPUT_NAME "objectselector"
        PREFIX ""
    )
endif()

add_executable(objectpicker_demo
    examples/objectpicker_demo.cpp
)

if(APPLE)
    set_target_properties(objectpicker_demo PROPERTIES
        INSTALL_RPATH "@executable_path/../lib"
        INSTALL_RPATH_USE_LINK_PATH TRUE
        BUILD_WITH_INSTALL_RPATH OFF
    )
endif()

target_link_libraries(objectpicker_demo
    PRIVATE
        objectselector
)

add_executable(objecttree_demo
    examples/objecttree_demo.cpp
)

if(APPLE)
    set_target_properties(objecttree_demo PROPERTIES
        INSTALL_RPATH "@executable_path/../lib"
        INSTALL_RPATH_USE_LINK_PATH TRUE
        BUILD_WITH_INSTALL_RPATH OFF
    )
endif()

target_link_libraries(objecttree_demo
    PRIVATE
        objectselector
)

install(TARGETS
    objectselector
    objectpicker_demo
    objecttree_demo
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES
    src/objecthighlighter.h
    src/objectpicker.h
    src/objectselectorglobal.h
    src/objecttreedialog.h
    src/objecttreewidget.h
    DESTINATION include
)

include(CTest)

if(BUILD_TESTING)
    add_executable(objectpicker_test
        tests/objectpicker_test.cpp
    )

    target_link_libraries(objectpicker_test
        PRIVATE
            objectselector
            Qt6::Test
    )

    add_test(NAME objectpicker_test COMMAND objectpicker_test)

    add_executable(objecthighlighter_test
        tests/objecthighlighter_test.cpp
    )

    target_link_libraries(objecthighlighter_test
        PRIVATE
            objectselector
            Qt6::Test
    )

    add_test(NAME objecthighlighter_test COMMAND objecthighlighter_test)
endif()
