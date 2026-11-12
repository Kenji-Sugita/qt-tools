#!/usr/bin/env python3
from pathlib import Path
import shutil


ROOT = Path(__file__).resolve().parent
PROJECTS = ROOT / "qml_candidates" / "projects"


def write(path, text):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text.strip() + "\n", encoding="utf-8")


def write_quick_main(path, module, main_type="Main"):
    write(path / "main.cpp", f"""
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    engine.loadFromModule("{module}", "{main_type}");
    return engine.rootObjects().isEmpty() ? 1 : 0;
}}
""")


def basic_quick_cmake(project, target, components="Quick", module=None, qml_files=None,
                      resources=None, extra_before="", extra_after="", extra_sources=None,
                      executable=True):
    module = module or target
    qml_files = qml_files or ["Main.qml"]
    resources = resources or []
    extra_sources = extra_sources or []
    add_target = "qt_add_executable" if executable else "qt_add_library"
    source_lines = "\n    ".join(["main.cpp"] + extra_sources) if executable else "\n    ".join(extra_sources)
    qml_lines = "\n        ".join(qml_files)
    resource_block = ""
    if resources:
        resource_lines = "\n        ".join(resources)
        resource_block = f"""
    RESOURCES
        {resource_lines}
"""
    return f"""
cmake_minimum_required(VERSION 3.16)

project({project} VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS {components})

qt_standard_project_setup()

{extra_before}

{add_target}({target}
    {source_lines}
)

qt_add_qml_module({target}
    URI {module}
    VERSION 1.0
    QML_FILES
        {qml_lines}
{resource_block})

target_link_libraries({target}
    PRIVATE
        Qt6::Quick
)

{f'''set_target_properties({target} PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)''' if executable else ''}

{extra_after}
"""


def create_projects():
    if PROJECTS.exists():
        shutil.rmtree(PROJECTS)
    PROJECTS.mkdir(parents=True)

    # 22. Quick Controls application
    p = PROJECTS / "22_quick_controls"
    write(p / "CMakeLists.txt", basic_quick_cmake("Qml22QuickControls", "Qml22QuickControls",
                                                   components="Quick QuickControls2"))
    write_quick_main(p, "Qml22QuickControls")
    write(p / "Main.qml", """
import QtQuick
import QtQuick.Controls

ApplicationWindow {
    width: 320
    height: 240
    visible: false

    Button {
        text: "OK"
        anchors.centerIn: parent
    }
}
""")

    # 23. Multiple QML files
    p = PROJECTS / "23_multiple_qml_files"
    write(p / "CMakeLists.txt", basic_quick_cmake("Qml23MultipleFiles", "Qml23MultipleFiles",
                                                   qml_files=["Main.qml", "components/StatusPanel.qml"]))
    write_quick_main(p, "Qml23MultipleFiles")
    write(p / "Main.qml", """
import QtQuick
import Qml23MultipleFiles

Item {
    width: 320
    height: 240
    StatusPanel { anchors.centerIn: parent }
}
""")
    write(p / "components/StatusPanel.qml", """
import QtQuick

Rectangle {
    width: 120
    height: 40
    color: "steelblue"
}
""")

    # 24. QML resources
    p = PROJECTS / "24_qml_resources"
    write(p / "CMakeLists.txt", basic_quick_cmake("Qml24Resources", "Qml24Resources",
                                                   resources=["images/logo.txt", "fonts/readme.txt"]))
    write_quick_main(p, "Qml24Resources")
    write(p / "Main.qml", """
import QtQuick

Item {
    width: 320
    height: 240
}
""")
    write(p / "images/logo.txt", "logo")
    write(p / "fonts/readme.txt", "font placeholder")

    # 25. C++ backend registered from main.cpp
    p = PROJECTS / "25_cpp_backend_registered"
    write(p / "CMakeLists.txt", basic_quick_cmake("Qml25CppBackend", "Qml25CppBackend",
                                                   components="Quick Qml",
                                                   extra_sources=["backend.cpp", "backend.h"]))
    write(p / "backend.h", """
#pragma once

#include <QObject>
#include <QString>

class Backend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString message READ message CONSTANT)

public:
    explicit Backend(QObject *parent = nullptr);
    QString message() const;
};
""")
    write(p / "backend.cpp", """
#include "backend.h"

Backend::Backend(QObject *parent) : QObject(parent) {}

QString Backend::message() const
{
    return QStringLiteral("backend");
}
""")
    write(p / "main.cpp", """
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <qqml.h>
#include "backend.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    qmlRegisterType<Backend>("Qml25CppBackend", 1, 0, "Backend");
    QQmlApplicationEngine engine;
    engine.loadFromModule("Qml25CppBackend", "Main");
    return engine.rootObjects().isEmpty() ? 1 : 0;
}
""")
    write(p / "Main.qml", """
import QtQuick
import Qml25CppBackend

Item {
    width: 320
    height: 240
    property var backend: Backend {}
    property string text: backend.message
}
""")

    # 26. QML singleton
    p = PROJECTS / "26_qml_singleton"
    write(p / "CMakeLists.txt", basic_quick_cmake("Qml26Singleton", "Qml26Singleton",
                                                   qml_files=["Main.qml", "Theme.qml"],
                                                   extra_before="""
set_source_files_properties(Theme.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)
"""))
    write_quick_main(p, "Qml26Singleton")
    write(p / "Main.qml", """
import QtQuick
import Qml26Singleton

Item {
    width: 320
    height: 240
    property color accent: Theme.accent
}
""")
    write(p / "Theme.qml", """
pragma Singleton
import QtQuick

QtObject {
    readonly property color accent: "tomato"
}
""")

    # 27. Existing qrc:/ URL compatibility
    p = PROJECTS / "27_qrc_compat"
    write(p / "CMakeLists.txt", """
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
""")
    write(p / "main.cpp", """
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QUrl>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/LegacyApp/Main.qml")));
    return engine.rootObjects().isEmpty() ? 1 : 0;
}
""")
    write(p / "Main.qml", """
import QtQuick

Item {
    width: 320
    height: 240
}
""")

    # 28. QML module as a library, linked by app
    p = PROJECTS / "28_qml_module_library"
    write(p / "CMakeLists.txt", """
cmake_minimum_required(VERSION 3.16)

project(Qml28ModuleLibrary VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup()

qt_add_library(ReusableUi STATIC)

qt_add_qml_module(ReusableUi
    URI ReusableUi
    VERSION 1.0
    OUTPUT_DIRECTORY ReusableUi
    QML_FILES
        ReusableButton.qml
)

target_link_libraries(ReusableUi
    PRIVATE
        Qt6::Quick
)

qt_add_executable(Qml28App
    main.cpp
)

qt_add_qml_module(Qml28App
    URI Qml28App
    VERSION 1.0
    QML_FILES
        Main.qml
)

target_link_libraries(Qml28App
    PRIVATE
        Qt6::Quick
        ReusableUi
)

set_target_properties(Qml28App PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
""")
    write_quick_main(p, "Qml28App")
    write(p / "Main.qml", """
import QtQuick
import ReusableUi

Item {
    width: 320
    height: 240
    ReusableButton { anchors.centerIn: parent }
}
""")
    write(p / "ReusableButton.qml", """
import QtQuick

Rectangle {
    width: 80
    height: 30
    color: "seagreen"
}
""")

    # 29. Multiple QML modules
    p = PROJECTS / "29_multiple_qml_modules"
    write(p / "CMakeLists.txt", """
cmake_minimum_required(VERSION 3.16)

project(Qml29MultipleModules VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup()

qt_add_library(ThemeModule STATIC)
qt_add_qml_module(ThemeModule
    URI AppTheme
    VERSION 1.0
    OUTPUT_DIRECTORY AppTheme
    QML_FILES
        ThemePalette.qml
)
target_link_libraries(ThemeModule PRIVATE Qt6::Quick)

qt_add_library(ComponentModule STATIC)
qt_add_qml_module(ComponentModule
    URI AppComponents
    VERSION 1.0
    OUTPUT_DIRECTORY AppComponents
    QML_FILES
        InfoCard.qml
)
target_link_libraries(ComponentModule PRIVATE Qt6::Quick ThemeModule)

qt_add_executable(Qml29App
    main.cpp
)
qt_add_qml_module(Qml29App
    URI Qml29App
    VERSION 1.0
    QML_FILES
        Main.qml
)
target_link_libraries(Qml29App PRIVATE Qt6::Quick ThemeModule ComponentModule)

set_target_properties(Qml29App PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
""")
    write_quick_main(p, "Qml29App")
    write(p / "Main.qml", """
import QtQuick
import AppComponents

Item {
    width: 320
    height: 240
    InfoCard { anchors.centerIn: parent }
}
""")
    write(p / "ThemePalette.qml", """
pragma Singleton
import QtQuick

QtObject {
    readonly property color panel: "slategray"
}
""")
    write(p / "InfoCard.qml", """
import QtQuick

Rectangle {
    width: 120
    height: 60
    color: "slategray"
}
""")

    # 30. C++ type inside QML module
    p = PROJECTS / "30_cpp_type_in_qml_module"
    write(p / "CMakeLists.txt", basic_quick_cmake("Qml30CppTypeModule", "Qml30CppTypeModule",
                                                   qml_files=["Main.qml"],
                                                   extra_sources=["counter.cpp", "counter.h"]))
    write_quick_main(p, "Qml30CppTypeModule")
    write(p / "counter.h", """
#pragma once

#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>

class Counter : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(int value READ value CONSTANT)

public:
    explicit Counter(QObject *parent = nullptr);
    int value() const;
};
""")
    write(p / "counter.cpp", """
#include "counter.h"

Counter::Counter(QObject *parent) : QObject(parent) {}

int Counter::value() const
{
    return 42;
}
""")
    write(p / "Main.qml", """
import QtQuick
import Qml30CppTypeModule

Item {
    width: 320
    height: 240
    property var counter: Counter {}
    property int answer: counter.value
}
""")

    # 31. QML app with C++ Qt Test
    p = PROJECTS / "31_qml_app_with_tests"
    write(p / "CMakeLists.txt", """
cmake_minimum_required(VERSION 3.16)

project(Qml31AppWithTests VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

find_package(Qt6 REQUIRED COMPONENTS Quick Test)

qt_standard_project_setup()

qt_add_library(Qml31Core
    formatter.cpp
    formatter.h
)
target_link_libraries(Qml31Core PUBLIC Qt6::Core)

qt_add_executable(Qml31App
    main.cpp
)
qt_add_qml_module(Qml31App
    URI Qml31App
    VERSION 1.0
    QML_FILES
        Main.qml
)
target_link_libraries(Qml31App PRIVATE Qt6::Quick Qml31Core)

set_target_properties(Qml31App PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

qt_add_executable(Qml31CoreTest
    tst_formatter.cpp
)
target_link_libraries(Qml31CoreTest PRIVATE Qt6::Test Qml31Core)
add_test(NAME Qml31CoreTest COMMAND Qml31CoreTest)
""")
    write_quick_main(p, "Qml31App")
    write(p / "formatter.h", """
#pragma once

#include <QString>

class Formatter
{
public:
    QString label(int value) const;
};
""")
    write(p / "formatter.cpp", """
#include "formatter.h"

QString Formatter::label(int value) const
{
    return QStringLiteral("value:%1").arg(value);
}
""")
    write(p / "tst_formatter.cpp", """
#include <QtTest>
#include "formatter.h"

class FormatterTest : public QObject
{
    Q_OBJECT

private slots:
    void label() { QCOMPARE(Formatter().label(3), QStringLiteral("value:3")); }
};

QTEST_MAIN(FormatterTest)
#include "tst_formatter.moc"
""")
    write(p / "Main.qml", """
import QtQuick

Item {
    width: 320
    height: 240
}
""")

    # 32. QML app install
    p = PROJECTS / "32_qml_install"
    write(p / "CMakeLists.txt", basic_quick_cmake("Qml32Install", "Qml32Install",
                                                   extra_after="""
install(TARGETS Qml32Install
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
"""))
    write_quick_main(p, "Qml32Install")
    write(p / "Main.qml", """
import QtQuick

Item {
    width: 320
    height: 240
}
""")

    # 33. Qt 6.3 compatibility QML resource style
    p = PROJECTS / "33_qml_qt62_compat"
    write(p / "CMakeLists.txt", """
cmake_minimum_required(VERSION 3.16)

project(Qml33Qt62Compat VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

add_executable(Qml33Qt62Compat
    main.cpp
)

qt_add_resources(Qml33Qt62Compat "qml_resources"
    PREFIX "/qt/qml/Qml33Qt62Compat"
    FILES
        Main.qml
)

target_link_libraries(Qml33Qt62Compat
    PRIVATE
        Qt6::Quick
)
""")
    write(p / "main.cpp", """
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QUrl>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/Qml33Qt62Compat/Main.qml")));
    return engine.rootObjects().isEmpty() ? 1 : 0;
}
""")
    write(p / "Main.qml", """
import QtQuick

Item {
    width: 320
    height: 240
}
""")


if __name__ == "__main__":
    create_projects()
    print(f"Generated QML candidate projects in {PROJECTS}")
