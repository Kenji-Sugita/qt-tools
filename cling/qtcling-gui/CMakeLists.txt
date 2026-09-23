cmake_minimum_required(VERSION 3.20)

project(qtcling-gui LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC OFF)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Cling REQUIRED CONFIG)

if(NOT LLVM_BUILD_BINARY_DIR)
  message(FATAL_ERROR "LLVM_BUILD_BINARY_DIR is not available from LLVMConfig.cmake")
endif()

add_executable(qtcling-gui main.cpp)

target_link_libraries(qtcling-gui PRIVATE
  Qt6::Widgets
  clingInterpreter
  clingMetaProcessor
  LLVMSupport
)

target_include_directories(qtcling-gui PRIVATE
  ${CLING_INCLUDE_DIRS}
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
)

target_compile_definitions(qtcling-gui PRIVATE
  QTCLING_DEFAULT_CLING_ROOT="${LLVM_BUILD_BINARY_DIR}"
  QTCLING_DEFAULT_RESOURCE_DIR="${LLVM_BUILD_BINARY_DIR}/lib/clang/${LLVM_VERSION_MAJOR}"
  QTCLING_DEFAULT_QT_ROOT="${Qt6_DIR}/../../.."
)

set_target_properties(qtcling-gui PROPERTIES ENABLE_EXPORTS 1)

if(MSVC)
  set_target_properties(qtcling-gui PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS 1)
endif()
