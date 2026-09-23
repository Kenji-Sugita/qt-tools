# Qt の翻訳 (.ts/.qm)を CMake のビルドに組み込みつつ、.qm を build ディレクト
# リー直下に生成し、さらに build 側へコピーした .qrc を AUTORCC で取り込む

cmake_minimum_required(VERSION 3.26)

project(dynamictranslation LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(APP_SOURCES
    main.cpp
    harness.cpp
    harness.h
)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(dynamictranslation)
else()
    add_executable(dynamictranslation)
endif()

target_sources(dynamictranslation PRIVATE ${APP_SOURCES})
target_link_libraries(dynamictranslation PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(dynamictranslation PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

set(TS_FILES
    "${CMAKE_CURRENT_LIST_DIR}/dynamictranslation_ja.ts"
    "${CMAKE_CURRENT_LIST_DIR}/dynamictranslation_en.ts"
)

# .qm の生成先: build ディレクトリー直下
set(QM_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")

set(_LUPDATE  "Qt${QT_VERSION_MAJOR}::lupdate")
set(_LRELEASE "Qt${QT_VERSION_MAJOR}::lrelease")

# どちらの分岐でも QM_FILES (=生成される .qm のフルパス一覧)を作る
set(QM_FILES "")

if(QT_VERSION_MAJOR GREATER_EQUAL 6 AND COMMAND qt_add_translations)
    # Qt 6.2+ : .qrc 方式を維持するため、.qm の自動リソース埋め込みはしない
    qt_add_translations(dynamictranslation
        TS_FILES ${TS_FILES}
        QM_FILES_OUTPUT_VARIABLE QM_FILES
        # (既定で .qm は CMAKE_CURRENT_BINARY_DIR に出る)
    )
else()
    # Qt5 / Qt6.0-6.1 : 手動で update/release を作る
    add_custom_target(update_translations
        COMMAND ${_LUPDATE} ${APP_SOURCES} -ts ${TS_FILES}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        VERBATIM
    )

    foreach(ts IN LISTS TS_FILES)
    	# .qm を求め QM_FILES に追加する
        get_filename_component(base "${ts}" NAME_WE)
        set(qm_out "${QM_OUT_DIR}/${base}.qm")
        list(APPEND QM_FILES "${qm_out}")

        # .ts の変更時に .qm を生成
        add_custom_command(
            OUTPUT "${qm_out}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${QM_OUT_DIR}"
            COMMAND ${_LRELEASE} "${ts}" -qm "${qm_out}"
            DEPENDS "${ts}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            VERBATIM
        )
    endforeach()

    add_custom_target(release_translations DEPENDS ${QM_FILES})
endif()

# build 側へ .qrc をコピーし、それを AUTORCC で取り込む
# 重要: .qrc の生成 (コピー) 自体を .qm に依存させる
set(SRC_QRC    "${CMAKE_CURRENT_SOURCE_DIR}/dynamictranslation.qrc")
set(COPIED_QRC "${CMAKE_CURRENT_BINARY_DIR}/dynamictranslation.qrc")

add_custom_command(
    OUTPUT  "${COPIED_QRC}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SRC_QRC}" "${COPIED_QRC}"
    DEPENDS "${SRC_QRC}" ${QM_FILES}
    VERBATIM
)

add_custom_target(copy_qrc DEPENDS "${COPIED_QRC}")

# AUTORCC の入力として build 側の .qrc を追加
target_sources(dynamictranslation PRIVATE "${COPIED_QRC}")

# 通常ビルドで必ず回す
add_dependencies(dynamictranslation copy_qrc)

add_dependencies(dynamictranslation release_translations)
