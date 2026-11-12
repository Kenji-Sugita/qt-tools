cmake_minimum_required(VERSION 3.16)

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" ICPP_VERSION LIMIT_COUNT 1)
string(STRIP "${ICPP_VERSION}" ICPP_VERSION)
if(NOT ICPP_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    message(FATAL_ERROR "VERSION must contain a semantic version such as 1.2.3")
endif()

project(icpp VERSION "${ICPP_VERSION}" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTING "Build tests" ON)
if (BUILD_TESTING)
    enable_testing()
endif()

option(ICPP_USE_READLINE "Enable GNU Readline support when available" ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui HttpServer Network Widgets)

qt_standard_project_setup()

set(ICPP_HAS_READLINE FALSE)
set(ICPP_READLINE_IS_EDITLINE FALSE)

if (ICPP_USE_READLINE AND UNIX AND NOT WIN32)
    find_path(READLINE_INCLUDE_DIR NAMES readline/readline.h)
    find_library(READLINE_LIBRARY NAMES readline)

    if (READLINE_INCLUDE_DIR AND READLINE_LIBRARY)
        set(ICPP_HAS_READLINE TRUE)
    else()
        find_path(READLINE_INCLUDE_DIR NAMES editline/readline.h)
        find_library(READLINE_LIBRARY NAMES edit)

        if (READLINE_INCLUDE_DIR AND READLINE_LIBRARY)
            set(ICPP_HAS_READLINE TRUE)
            set(ICPP_READLINE_IS_EDITLINE TRUE)
        endif()
    endif()
endif()

qt_add_executable(icpp
    src/Application.cpp
    src/InterpreterProcess.cpp
    src/McpController.cpp
    src/McpHttpServer.cpp
    src/McpInterpreterSession.cpp
    src/main.cpp
    src/ReplHelpers.cpp
    src/ReplSessionCommands.cpp
    src/ReplSessionDisplay.cpp
    src/ReplSessionEdit.cpp
    src/ReplSessionFiles.cpp
    src/ReplSessionHelp.cpp
    src/ReplSessionImpl.cpp
    src/ReplSessionInterpreter.cpp
    src/ReplSessionQtFiles.cpp
    src/ReplSessionQtWidgets.cpp
    src/ReplSessionTemplate.cpp
    src/ReplSessionWorkflow.cpp
    src/repl.cpp
    include/icpp/Application.h
    include/icpp/InterpreterProcess.h
    include/icpp/LineEdit.h
    include/icpp/McpController.h
    include/icpp/McpHttpServer.h
    include/icpp/McpInterpreterSession.h
    include/icpp/ReplHelpers.h
    include/icpp/ReplSessionImpl.h
    include/icpp/repl.h
)

target_include_directories(icpp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(icpp
    PRIVATE
        ICPP_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(icpp
    PRIVATE
        Qt6::Core
        Qt6::HttpServer
        Qt6::Network
)

install(TARGETS icpp
    RUNTIME DESTINATION bin
)
install(FILES VERSION
    DESTINATION share/icpp
)

if(APPLE)
    set_target_properties(icpp PROPERTIES
        INSTALL_RPATH "@executable_path/../lib"
        INSTALL_RPATH_USE_LINK_PATH TRUE
    )
endif()

if (ICPP_HAS_READLINE)
    target_include_directories(icpp PRIVATE ${READLINE_INCLUDE_DIR})
    target_compile_definitions(icpp PRIVATE HAVE_READLINE=1)
    if (ICPP_READLINE_IS_EDITLINE)
        target_compile_definitions(icpp PRIVATE HAVE_EDITLINE_READLINE=1)
    endif()
    target_link_libraries(icpp PRIVATE ${READLINE_LIBRARY})
endif()

if (ICPP_USE_READLINE AND NOT ICPP_HAS_READLINE)
    if (WIN32)
        message(STATUS "Readline disabled on Windows; using standard input fallback.")
    else()
        message(STATUS "Readline not found; using standard input fallback.")
    endif()
endif()

if (BUILD_TESTING AND NOT WIN32)
    find_package(Qt6 REQUIRED COMPONENTS Test)

    add_test(
        NAME icpp_integration
        COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_tests.sh $<TARGET_FILE:icpp> ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_tests_properties(icpp_integration PROPERTIES TIMEOUT 180)

    qt_add_executable(icpp_mcp_http_test
        tests/tst_mcp_http.cpp
    )
    target_link_libraries(icpp_mcp_http_test
        PRIVATE
            Qt6::Core
            Qt6::Network
            Qt6::Test
    )
    add_test(
        NAME icpp_mcp_http
        COMMAND ${CMAKE_COMMAND} -E env
            ICPP_TEST_BINARY=$<TARGET_FILE:icpp>
            $<TARGET_FILE:icpp_mcp_http_test>
    )
    set_tests_properties(icpp_mcp_http PROPERTIES TIMEOUT 60)

    add_test(
        NAME icpp_source_layout
        COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_source_layout.sh ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()
