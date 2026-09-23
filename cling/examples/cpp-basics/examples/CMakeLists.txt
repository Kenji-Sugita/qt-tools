# 各サブディレクトリーを ExternalProject として独立に configure/build します。
# 各サブプロジェクトのビルドディレクトリーは <subdir>/build に固定し、成果物を
# サブディレクトリーに集約します。マルチコンフィグでは --config で構成を指定し、
# シングルコンフィグでは CMAKE_BUILD_TYPE を外部に渡します。ターゲットをビルド
# すると、directory_list に列挙したサブプロジェクトをまとめてビルドします。

cmake_minimum_required(VERSION 3.26)

project(solutions LANGUAGES CXX)

include(ExternalProject)

set(directory_list
    codec
    completer
    containers
    defaultvaluesinsignals
    delayedinit
    dialog
    dynamichelp
    dynamictranslation
    eventfilter
    helloworld
    inputmasks
    metainfo
    moving
    paintprogram
    qtextstream
    quotebutton
    qvariant
    resources
    showhide
    signalslots
    sizepolicy
    slider
    stopwatch
    syntheticevents
    textdecoder
    updating
    widgetgrouping
    widgetprinting
)

set(external_cmake_args "")
set(external_build_command "")

# マルチコンフィグ判定
if(CMAKE_CONFIGURATION_TYPES)
    set(external_build_command
        ${CMAKE_COMMAND} --build <BINARY_DIR> --config $<CONFIG>
    )
else()
    # シングルコンフィグ: 未指定なら Release
    set(build_type "${CMAKE_BUILD_TYPE}")
    if(NOT build_type)
        set(build_type "Release")
    endif()

    set(external_cmake_args
        -DCMAKE_BUILD_TYPE:STRING=${build_type}
    )

    set(external_build_command
        ${CMAKE_COMMAND} --build <BINARY_DIR>
    )
endif()

foreach(directory IN LISTS directory_list)
    ExternalProject_Add(${directory}
        SOURCE_DIR  "${CMAKE_SOURCE_DIR}/${directory}"
        BINARY_DIR  "${CMAKE_SOURCE_DIR}/${directory}/build"
        CMAKE_ARGS  ${external_cmake_args}
        BUILD_COMMAND ${external_build_command}
        INSTALL_COMMAND ""   # インストールはしない
    )
endforeach()
