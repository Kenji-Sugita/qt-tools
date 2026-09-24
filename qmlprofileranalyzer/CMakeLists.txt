cmake_minimum_required(VERSION 3.16)

project(qmlprofileranalyzer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Test Xml)

add_executable(qmlprofileranalyzer
    src/main.cpp
    src/qtdreader.cpp
    src/analyzer.cpp
    src/report.cpp
)

target_include_directories(qmlprofileranalyzer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(qmlprofileranalyzer PRIVATE
    Qt6::Core
    Qt6::Xml
)

qt_standard_project_setup(REQUIRES 6.8)

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/demos/localizedclock/main.cpp")
    qt_add_executable(localizedclockdemo
        demos/localizedclock/main.cpp
    )

    target_link_libraries(localizedclockdemo PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
    )

    set_target_properties(localizedclockdemo PROPERTIES
        WIN32_EXECUTABLE FALSE
        MACOSX_BUNDLE FALSE
    )

    target_compile_definitions(localizedclockdemo PRIVATE
        QT_QML_DEBUG
        LOCALIZEDCLOCKDEMO_QML_PATH="${CMAKE_CURRENT_SOURCE_DIR}/demos/localizedclock/Main.qml"
    )
endif()

enable_testing()

add_executable(test_qtdreader
    tests/test_qtdreader.cpp
    src/qtdreader.cpp
    src/analyzer.cpp
    src/report.cpp
)

target_include_directories(test_qtdreader PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(test_qtdreader PRIVATE
    TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/data"
)

target_link_libraries(test_qtdreader PRIVATE
    Qt6::Core
    Qt6::Test
    Qt6::Xml
)

add_test(NAME test_qtdreader COMMAND test_qtdreader)
