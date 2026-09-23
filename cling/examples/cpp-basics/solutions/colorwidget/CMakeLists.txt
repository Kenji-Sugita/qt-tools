cmake_minimum_required(VERSION 3.26)

project(colorwidget LANGUAGES CXX)

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
    colorwidget.cpp
    colorwidget.h
    colorwidget.ui
)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(colorwidget)
else()
    add_executable(colorwidget)
endif()

target_sources(colorwidget PRIVATE ${APP_SOURCES})
target_link_libraries(colorwidget PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(colorwidget PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

set(TS_FILES
    "${CMAKE_CURRENT_LIST_DIR}/colorwidget_ja.ts"
)

# .qm の最終配置先: a.out、.app、.exe の横
set(QM_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")

# lupdate/lrelease ターゲット
set(_LUPDATE "Qt${QT_VERSION_MAJOR}::lupdate")
set(_LRELEASE "Qt${QT_VERSION_MAJOR}::lrelease")

# 内部ターゲット: .ts 更新
add_custom_target(_update_ts
    COMMAND ${_LUPDATE} ${APP_SOURCES} -ts ${TS_FILES}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    VERBATIM
)

# ユーザー向けターゲット名: update_translations
# cmake --build build --target update_translations
# Qt 5 でも Qt 6 と同じターゲットを使えるようにする。
if(QT_VERSION_MAJOR LESS 6)
    add_custom_target(update_translations DEPENDS _update_ts)
endif()

# 内部ターゲット: .qm 生成
if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    # Qt 6: Qt の翻訳生成ターゲットを作る (名前は環境で変動するため OUTPUT_TARGETS で受け取る)
    qt_add_translations(
        TARGETS colorwidget
        SOURCE_TARGETS colorwidget
        TS_FILES ${TS_FILES}
        OUTPUT_TARGETS _I18N_TARGETS
    )

    # a.out、.app、.exe と同じディレクトリー/<base>.qm を想定
    set(QM_FILES "")
    foreach(ts IN LISTS TS_FILES)
        get_filename_component(base "${ts}" NAME_WE)
        list(APPEND QM_FILES "${CMAKE_CURRENT_BINARY_DIR}/${base}.qm")
    endforeach()

    add_custom_target(_release_qm DEPENDS ${_I18N_TARGETS})
else()
    # Qt 5: .ts -> .qm
    qt5_add_translation(QM_FILES ${TS_FILES})
    add_custom_target(_release_qm DEPENDS ${QM_FILES})
endif()

# ユーザー向けターゲット名: release_translations
# cmake --build build --target release_translations
# Qt 5 でも Qt 6 と同じターゲットを使えるようにする。
if(QT_VERSION_MAJOR LESS 6)
    add_custom_target(release_translations DEPENDS _release_qm)
endif()

# 通常ビルドで .qm も欲しい
add_dependencies(colorwidget _release_qm)

# 最終的に .qm を build/ に置く
add_custom_command(TARGET colorwidget POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${QM_FILES}
        "${QM_OUT_DIR}"
    VERBATIM
)
