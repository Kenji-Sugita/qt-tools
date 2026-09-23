cmake_minimum_required(VERSION 3.26)
project(object-browser LANGUAGES CXX)

include(ExternalProject)

set(directory_list
    step1
    step2-index-points-to-object
    step2-index-points-to-parent
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
