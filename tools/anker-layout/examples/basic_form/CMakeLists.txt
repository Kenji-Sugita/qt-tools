if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    cmake_minimum_required(VERSION 3.16)

    project(AnkerBasicForm VERSION 0.1.0 LANGUAGES CXX)

    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    find_package(Qt6 REQUIRED COMPONENTS Widgets)

    qt_standard_project_setup()
endif()

set(ANKER_LAYOUT_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)

if(NOT TARGET AnkerLayout)
    qt_add_library(AnkerLayout
        ${ANKER_LAYOUT_ROOT}/include/AnkerFormAttachment.h
        ${ANKER_LAYOUT_ROOT}/include/AnkerFormConstraints.h
        ${ANKER_LAYOUT_ROOT}/include/AnkerFormLayout.h
        ${ANKER_LAYOUT_ROOT}/src/AnkerFormAttachment.cpp
        ${ANKER_LAYOUT_ROOT}/src/AnkerFormLayout.cpp
    )

    target_include_directories(AnkerLayout
        PUBLIC
            ${ANKER_LAYOUT_ROOT}/include
    )

    target_link_libraries(AnkerLayout
        PUBLIC
            Qt6::Widgets
    )
endif()

qt_add_executable(anker_basic_form
    main.cpp
)

target_link_libraries(anker_basic_form
    PRIVATE
        AnkerLayout
)

set_target_properties(anker_basic_form PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
