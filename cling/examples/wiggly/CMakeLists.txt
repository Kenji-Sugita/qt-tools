# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(wiggly LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_standard_project_setup()

qt_add_executable(wiggly
    main.cpp
    wigglywidget.cpp
    dialog.cpp
    dialog.h
    wigglywidget.h
)

set_target_properties(wiggly PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(wiggly PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)
