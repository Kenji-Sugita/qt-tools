cmake_minimum_required(VERSION 3.16)

project(fileservermcp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

qt_standard_project_setup()

qt_add_executable(fileservermcp-server
    include/common/JsonRpcMessage.h
    include/server/FileRepository.h
    include/server/McpToolHandler.h
    include/server/ServerOptions.h
    include/server/SseMcpServer.h
    include/server/SseSession.h
    src/common/JsonRpcMessage.cpp
    src/server/FileRepository.cpp
    src/server/McpToolHandler.cpp
    src/server/ServerOptions.cpp
    src/server/SseMcpServer.cpp
    src/server/SseSession.cpp
    src/server/main.cpp
)

target_include_directories(fileservermcp-server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/common
        ${CMAKE_CURRENT_SOURCE_DIR}/include/server
)

target_link_libraries(fileservermcp-server
    PRIVATE
        Qt6::Core
        Qt6::Network
)

qt_add_executable(fileservermcp-client
    include/client/ClientOptions.h
    include/client/Command.h
    include/client/CommandRegistry.h
    include/client/Output.h
    include/client/Repl.h
    include/client/SseMcpClient.h
    include/common/JsonRpcMessage.h
    src/client/ClientOptions.cpp
    src/client/CommandRegistry.cpp
    src/client/Output.cpp
    src/client/Repl.cpp
    src/client/SseMcpClient.cpp
    src/client/main.cpp
    src/common/JsonRpcMessage.cpp
)

target_include_directories(fileservermcp-client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/common
        ${CMAKE_CURRENT_SOURCE_DIR}/include/client
)

target_link_libraries(fileservermcp-client
    PRIVATE
        Qt6::Core
        Qt6::Network
)
