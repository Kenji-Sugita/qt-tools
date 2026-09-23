cmake_minimum_required(VERSION 3.16)

project(Qml27QrcCompat VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup()

qt_add_executable(Qml27QrcCompat
    main.cpp
)

qt_add_resources(Qml27QrcCompat "qml_resources"
    PREFIX "/qt/qml/LegacyApp"
    FILES
        Main.qml
)

target_link_libraries(Qml27QrcCompat
    PRIVATE
        Qt6::Quick
)
