cmake_minimum_required(VERSION 3.16)

project(AnkerLayout VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

find_package(Qt6 REQUIRED COMPONENTS Widgets Test)

qt_standard_project_setup()

qt_add_library(AnkerLayout
    include/AnkerFormAttachment.h
    include/AnkerFormConstraints.h
    include/AnkerFormLayout.h
    src/AnkerFormAttachment.cpp
    src/AnkerFormLayout.cpp
)

target_include_directories(AnkerLayout
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(AnkerLayout
    PUBLIC
        Qt6::Widgets
)

add_subdirectory(examples/basic_form)
add_subdirectory(examples/relative_widgets)
add_subdirectory(examples/position_anchors)
add_subdirectory(examples/opposite_edges)
add_subdirectory(examples/resizable_dialog)
add_subdirectory(examples/full_attachments)
add_subdirectory(tests)
