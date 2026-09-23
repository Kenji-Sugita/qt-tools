cmake_minimum_required(VERSION 3.26)
project(solutions LANGUAGES CXX)

include(ExternalProject)

set(directory_list
    cartesianwithimage
    drag-and-drop
    fibonacci
    model-view
    multithreading
    object-browser
    sif
    siftest
    statecombobox
    tcp-local-server
    tcp-server
)

foreach(directory IN LISTS directory_list)
    ExternalProject_Add(${directory}
        SOURCE_DIR  "${CMAKE_SOURCE_DIR}/${directory}"
        BINARY_DIR  "${CMAKE_SOURCE_DIR}/${directory}/build"
        CMAKE_ARGS  -DCMAKE_BUILD_TYPE=$<IF:$<BOOL:${CMAKE_BUILD_TYPE}>,${CMAKE_BUILD_TYPE},Release>
        BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR>
        INSTALL_COMMAND ""   # インストールはしない
    )
endforeach()

add_custom_target(build_all)
add_dependencies(build_all ${directory_list})
