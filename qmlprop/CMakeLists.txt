cmake_minimum_required(VERSION 3.21)

project(qmlprop VERSION 2.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Gui Qml Quick)

qt_standard_project_setup()

qt_add_executable(qmlprop
    src/commandlineoptions.cpp
    src/commandlineoptions.h
    src/importprofile.h
    src/main.cpp
    src/qmlattachedtypecatalog.cpp
    src/qmlattachedtypecatalog.h
    src/metaobjectprinter.cpp
    src/metaobjectprinter.h
    src/qmlmodulecatalog.cpp
    src/qmlmodulecatalog.h
    src/qmltypeinspector.cpp
    src/qmltypeinspector.h
    src/qmltypenamecatalog.cpp
    src/qmltypenamecatalog.h
)

target_link_libraries(qmlprop PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
)

install(TARGETS qmlprop)
