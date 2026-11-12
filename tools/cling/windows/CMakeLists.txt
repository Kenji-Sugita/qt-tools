cmake_minimum_required(VERSION 3.21)
project(qtcling_windows_runtime LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_library(qtcling-win SHARED ../src/qtguiwin.cpp)
target_compile_features(qtcling-win PRIVATE cxx_std_17)
target_link_libraries(qtcling-win PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
set_target_properties(qtcling-win PROPERTIES
  PREFIX ""
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
