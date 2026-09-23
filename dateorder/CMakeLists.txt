cmake_minimum_required(VERSION 3.16)

project(dateorder LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(younger
    main.cpp
)

add_custom_command(TARGET younger POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        $<TARGET_FILE_NAME:younger>
        $<TARGET_FILE_DIR:younger>/older
    VERBATIM
)
