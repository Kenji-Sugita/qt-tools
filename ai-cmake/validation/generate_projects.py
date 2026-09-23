#!/usr/bin/env python3
from pathlib import Path
import shutil


ROOT = Path(__file__).resolve().parent
PROJECTS = ROOT / "projects"


def write(path, text):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text.strip() + "\n", encoding="utf-8")


def common_main_widgets(app_class="QApplication", window_class="QWidget"):
    include = "QWidget" if window_class == "QWidget" else "QMainWindow"
    return f"""
#include <{app_class}>
#include <{include}>

int main(int argc, char *argv[])
{{
    {app_class} app(argc, argv);
    {window_class} window;
    window.setWindowTitle("validation");
    return 0;
}}
"""


def mainwindow_h(class_name="MainWindow", base="QMainWindow", extra=""):
    return f"""
#pragma once

#include <{base}>

class {class_name} : public {base}
{{
    Q_OBJECT

public:
    explicit {class_name}(QWidget *parent = nullptr);
{extra}
}};
"""


def mainwindow_cpp(class_name="MainWindow", base="QMainWindow"):
    header = "mainwindow.h" if class_name == "MainWindow" else f"{class_name.lower()}.h"
    return f"""
#include "{header}"

{class_name}::{class_name}(QWidget *parent)
    : {base}(parent)
{{
    setWindowTitle("validation");
}}
"""


def qt_ui_mainwindow():
    return """
<?xml version="1.0" encoding="UTF-8"?>
<ui version="4.0">
 <class>MainWindow</class>
 <widget class="QMainWindow" name="MainWindow">
  <property name="geometry">
   <rect>
    <x>0</x>
    <y>0</y>
    <width>320</width>
    <height>240</height>
   </rect>
  </property>
  <widget class="QWidget" name="centralwidget"/>
 </widget>
 <resources/>
 <connections/>
</ui>
"""


def write_standard_widgets(path, target, extra_cmake="", sources=None, components="Widgets"):
    sources = sources or ["main.cpp", "mainwindow.cpp", "mainwindow.h"]
    write(path / "CMakeLists.txt", f"""
cmake_minimum_required(VERSION 3.16)

project({target} VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS {components})

qt_standard_project_setup()

qt_add_executable({target}
    {" ".join(sources)}
)

{extra_cmake}

target_link_libraries({target}
    PRIVATE
        Qt6::Widgets
)

set_target_properties({target} PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
""")
    write(path / "main.cpp", """
#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return 0;
}
""")
    write(path / "mainwindow.h", mainwindow_h())
    write(path / "mainwindow.cpp", mainwindow_cpp())


def create_projects():
    if PROJECTS.exists():
        shutil.rmtree(PROJECTS)
    PROJECTS.mkdir(parents=True)

    # 1. Qt Widgets application
    p = PROJECTS / "01_widgets_app"
    write_standard_widgets(p, "Pattern01WidgetsApp")

    # 2. Qt Quick / QML application
    p = PROJECTS / "02_quick_qml_app"
    write(p / "CMakeLists.txt", """
cmake_minimum_required(VERSION 3.16)

project(Pattern02QuickApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup()

qt_add_executable(Pattern02QuickApp
    main.cpp
)

qt_add_qml_module(Pattern02QuickApp
    URI Pattern02QuickApp
    VERSION 1.0
    QML_FILES
        Main.qml
    RESOURCES
        images/logo.txt
)

target_link_libraries(Pattern02QuickApp
    PRIVATE
        Qt6::Quick
)

set_target_properties(Pattern02QuickApp PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
""")
    write(p / "main.cpp", """
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    engine.loadFromModule("Pattern02QuickApp", "Main");
    return 0;
}
""")
    write(p / "Main.qml", """
import QtQuick

Item {
    width: 320
    height: 240
}
""")
    write(p / "images/logo.txt", "validation")

    # 3. Qt console application
    p = PROJECTS / "03_console_app"
    write(p / "CMakeLists.txt", """
cmake_minimum_required(VERSION 3.16)

project(Pattern03ConsoleApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

qt_standard_project_setup()

qt_add_executable(Pattern03ConsoleApp
    main.cpp
)

target_link_libraries(Pattern03ConsoleApp
    PRIVATE
        Qt6::Core
)
""")
    write(p / "main.cpp", """
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    return 0;
}
""")

    # 4. Qt Core library
    p = PROJECTS / "04_core_library"
    write(p / "CMakeLists.txt", """
cmake_minimum_required(VERSION 3.16)

project(Pattern04CoreLibrary VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

qt_standard_project_setup()

qt_add_library(Pattern04CoreLibrary
    myobject.cpp
    myobject.h
)

target_link_libraries(Pattern04CoreLibrary
    PUBLIC
        Qt6::Core
)
""")
    write(p / "myobject.h", """
#pragma once

#include <QObject>
#include <QString>

class MyObject : public QObject
{
    Q_OBJECT

public:
    explicit MyObject(QObject *parent = nullptr);
    QString name() const;
};
""")
    write(p / "myobject.cpp", """
#include "myobject.h"

MyObject::MyObject(QObject *parent) : QObject(parent) {}

QString MyObject::name() const
{
    return QStringLiteral("validation");
}
""")

    # 5. Qt Widgets library
    p = PROJECTS / "05_widgets_library"
    write(p / "CMakeLists.txt", """
cmake_minimum_required(VERSION 3.16)

project(Pattern05WidgetsLibrary VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_library(Pattern05WidgetsLibrary
    customwidget.cpp
    customwidget.h
)

target_link_libraries(Pattern05WidgetsLibrary
    PUBLIC
        Qt6::Widgets
)
""")
    write(p / "customwidget.h", mainwindow_h("CustomWidget", "QWidget"))
    write(p / "customwidget.cpp", mainwindow_cpp("CustomWidget", "QWidget"))

    # 6. Application + common library
    p = PROJECTS / "06_app_with_library"
    write(p / "CMakeLists.txt", """
cmake_minimum_required(VERSION 3.16)

project(Pattern06AppWithLibrary VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_library(Pattern06AppCore
    appmodel.cpp
    appmodel.h
)

target_link_libraries(Pattern06AppCore
    PUBLIC
        Qt6::Core
)

qt_add_executable(Pattern06App
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_link_libraries(Pattern06App
    PRIVATE
        Pattern06AppCore
        Qt6::Widgets
)

set_target_properties(Pattern06App PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
""")
    write(p / "appmodel.h", """
#pragma once

#include <QString>

class AppModel
{
public:
    QString title() const;
};
""")
    write(p / "appmodel.cpp", """
#include "appmodel.h"

QString AppModel::title() const
{
    return QStringLiteral("validation");
}
""")
    write(p / "main.cpp", """
#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    return 0;
}
""")
    write(p / "mainwindow.h", mainwindow_h())
    write(p / "mainwindow.cpp", mainwindow_cpp())

    # 7. Multiple executables
    p = PROJECTS / "07_multiple_executables"
    write(p / "CMakeLists.txt", """
cmake_minimum_required(VERSION 3.16)

project(Pattern07Tools VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(Pattern07GuiTool
    gui_main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_link_libraries(Pattern07GuiTool
    PRIVATE
        Qt6::Widgets
)

set_target_properties(Pattern07GuiTool PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

qt_add_executable(Pattern07CliTool
    cli_main.cpp
)

target_link_libraries(Pattern07CliTool
    PRIVATE
        Qt6::Core
)
""")
    write(p / "gui_main.cpp", """
#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    return 0;
}
""")
    write(p / "cli_main.cpp", """
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    return 0;
}
""")
    write(p / "mainwindow.h", mainwindow_h())
    write(p / "mainwindow.cpp", mainwindow_cpp())

    # 8. Qt Test target
    p = PROJECTS / "08_qt_test"
    write(p / "CMakeLists.txt", """
cmake_minimum_required(VERSION 3.16)

project(Pattern08QtTest VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

find_package(Qt6 REQUIRED COMPONENTS Test)

qt_standard_project_setup()

qt_add_executable(Pattern08QtTest
    tst_validation.cpp
)

target_link_libraries(Pattern08QtTest
    PRIVATE
        Qt6::Test
)

add_test(NAME Pattern08QtTest COMMAND Pattern08QtTest)
""")
    write(p / "tst_validation.cpp", """
#include <QtTest>

class ValidationTest : public QObject
{
    Q_OBJECT

private slots:
    void passes() { QVERIFY(true); }
};

QTEST_MAIN(ValidationTest)
#include "tst_validation.moc"
""")

    # 9. Application + test
    p = PROJECTS / "09_app_with_tests"
    write(p / "CMakeLists.txt", """
cmake_minimum_required(VERSION 3.16)

project(Pattern09TestableApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Test)

qt_standard_project_setup()

qt_add_library(Pattern09AppCore
    calculator.cpp
    calculator.h
)

target_link_libraries(Pattern09AppCore
    PUBLIC
        Qt6::Core
)

qt_add_executable(Pattern09App
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_link_libraries(Pattern09App
    PRIVATE
        Pattern09AppCore
        Qt6::Widgets
)

set_target_properties(Pattern09App PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

qt_add_executable(Pattern09AppCoreTest
    tst_calculator.cpp
)

target_link_libraries(Pattern09AppCoreTest
    PRIVATE
        Pattern09AppCore
        Qt6::Test
)

add_test(NAME Pattern09AppCoreTest COMMAND Pattern09AppCoreTest)
""")
    write(p / "calculator.h", """
#pragma once

class Calculator
{
public:
    int add(int left, int right) const;
};
""")
    write(p / "calculator.cpp", """
#include "calculator.h"

int Calculator::add(int left, int right) const
{
    return left + right;
}
""")
    write(p / "main.cpp", """
#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    return 0;
}
""")
    write(p / "mainwindow.h", mainwindow_h())
    write(p / "mainwindow.cpp", mainwindow_cpp())
    write(p / "tst_calculator.cpp", """
#include <QtTest>
#include "calculator.h"

class CalculatorTest : public QObject
{
    Q_OBJECT

private slots:
    void add() { QCOMPARE(Calculator().add(1, 2), 3); }
};

QTEST_MAIN(CalculatorTest)
#include "tst_calculator.moc"
""")

    # 10. Subdirectory layout
    p = PROJECTS / "10_subdirectories"
    write(p / "CMakeLists.txt", """
cmake_minimum_required(VERSION 3.16)

project(Pattern10SubdirProject VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

find_package(Qt6 REQUIRED COMPONENTS Widgets Test)

qt_standard_project_setup()

add_subdirectory(src)
add_subdirectory(tests)
""")
    write(p / "src/CMakeLists.txt", """
qt_add_executable(Pattern10SubdirApp
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_link_libraries(Pattern10SubdirApp
    PRIVATE
        Qt6::Widgets
)
""")
    write(p / "src/main.cpp", """
#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    return 0;
}
""")
    write(p / "src/mainwindow.h", mainwindow_h())
    write(p / "src/mainwindow.cpp", mainwindow_cpp())
    write(p / "tests/CMakeLists.txt", """
qt_add_executable(Pattern10SubdirTest
    tst_validation.cpp
)

target_link_libraries(Pattern10SubdirTest
    PRIVATE
        Qt6::Test
)

add_test(NAME Pattern10SubdirTest COMMAND Pattern10SubdirTest)
""")
    write(p / "tests/tst_validation.cpp", """
#include <QtTest>

class SubdirTest : public QObject
{
    Q_OBJECT

private slots:
    void passes() { QVERIFY(true); }
};

QTEST_MAIN(SubdirTest)
#include "tst_validation.moc"
""")

    # 11. Designer .ui file
    p = PROJECTS / "11_ui_file"
    write_standard_widgets(p, "Pattern11UiApp", sources=["main.cpp", "mainwindow.cpp", "mainwindow.h", "mainwindow.ui"])
    write(p / "mainwindow.cpp", """
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    Ui::MainWindow ui;
    ui.setupUi(this);
}
""")
    write(p / "mainwindow.ui", qt_ui_mainwindow())

    # 12. Resources in Widgets app
    p = PROJECTS / "12_resources"
    write_standard_widgets(p, "Pattern12ResourceApp", extra_cmake="""
qt_add_resources(Pattern12ResourceApp "app_resources"
    PREFIX "/"
    FILES
        icons/app.txt
        images/splash.txt
)
""")
    write(p / "icons/app.txt", "icon")
    write(p / "images/splash.txt", "splash")

    # 13. Qt Network
    p = PROJECTS / "13_network"
    write(p / "CMakeLists.txt", """
cmake_minimum_required(VERSION 3.16)

project(Pattern13NetworkApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

qt_standard_project_setup()

qt_add_executable(Pattern13NetworkApp
    main.cpp
    downloader.cpp
    downloader.h
)

target_link_libraries(Pattern13NetworkApp
    PRIVATE
        Qt6::Core
        Qt6::Network
)
""")
    write(p / "downloader.h", """
#pragma once

#include <QObject>
#include <QNetworkAccessManager>

class Downloader : public QObject
{
    Q_OBJECT

public:
    explicit Downloader(QObject *parent = nullptr);

private:
    QNetworkAccessManager manager;
};
""")
    write(p / "downloader.cpp", """
#include "downloader.h"

Downloader::Downloader(QObject *parent) : QObject(parent) {}
""")
    write(p / "main.cpp", """
#include <QCoreApplication>
#include "downloader.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Downloader downloader;
    return 0;
}
""")

    # 14. Qt SQL
    p = PROJECTS / "14_sql"
    write(p / "CMakeLists.txt", """
cmake_minimum_required(VERSION 3.16)

project(Pattern14SqlApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Sql)

qt_standard_project_setup()

qt_add_executable(Pattern14SqlApp
    main.cpp
    repository.cpp
    repository.h
)

target_link_libraries(Pattern14SqlApp
    PRIVATE
        Qt6::Core
        Qt6::Sql
)
""")
    write(p / "repository.h", """
#pragma once

#include <QSqlDatabase>

class Repository
{
public:
    QSqlDatabase database() const;
};
""")
    write(p / "repository.cpp", """
#include "repository.h"

QSqlDatabase Repository::database() const
{
    return QSqlDatabase();
}
""")
    write(p / "main.cpp", """
#include <QCoreApplication>
#include "repository.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Repository repository;
    return repository.database().isValid() ? 0 : 0;
}
""")

    # 15. Qt Concurrent
    p = PROJECTS / "15_concurrent"
    write(p / "CMakeLists.txt", """
cmake_minimum_required(VERSION 3.16)

project(Pattern15ConcurrentApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Concurrent)

qt_standard_project_setup()

qt_add_executable(Pattern15ConcurrentApp
    main.cpp
    worker.cpp
    worker.h
)

target_link_libraries(Pattern15ConcurrentApp
    PRIVATE
        Qt6::Core
        Qt6::Concurrent
)
""")
    write(p / "worker.h", """
#pragma once

class Worker
{
public:
    int calculate() const;
};
""")
    write(p / "worker.cpp", """
#include "worker.h"

int Worker::calculate() const
{
    return 1;
}
""")
    write(p / "main.cpp", """
#include <QCoreApplication>
#include <QtConcurrent>
#include "worker.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Worker worker;
    auto future = QtConcurrent::run([&worker] { return worker.calculate(); });
    future.waitForFinished();
    return future.result() == 1 ? 0 : 1;
}
""")

    # 16. OpenGL Widgets
    p = PROJECTS / "16_opengl"
    write(p / "CMakeLists.txt", """
cmake_minimum_required(VERSION 3.16)

project(Pattern16OpenGLApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)

qt_standard_project_setup()

qt_add_executable(Pattern16OpenGLApp
    main.cpp
    glwidget.cpp
    glwidget.h
)

target_link_libraries(Pattern16OpenGLApp
    PRIVATE
        Qt6::Widgets
        Qt6::OpenGLWidgets
)

set_target_properties(Pattern16OpenGLApp PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
""")
    write(p / "glwidget.h", """
#pragma once

#include <QOpenGLWidget>

class GLWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit GLWidget(QWidget *parent = nullptr);
};
""")
    write(p / "glwidget.cpp", """
#include "glwidget.h"

GLWidget::GLWidget(QWidget *parent) : QOpenGLWidget(parent) {}
""")
    write(p / "main.cpp", """
#include <QApplication>
#include "glwidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    GLWidget widget;
    return 0;
}
""")

    # 17. Translations
    p = PROJECTS / "17_translations"
    write_standard_widgets(p, "Pattern17TranslatedApp", components="Widgets LinguistTools", extra_cmake="""
qt_add_translations(Pattern17TranslatedApp
    TS_FILES
        translations/Pattern17TranslatedApp_ja.ts
)
""")
    write(p / "translations/Pattern17TranslatedApp_ja.ts", """
<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ja_JP">
</TS>
""")

    # 18. install()
    p = PROJECTS / "18_install"
    write_standard_widgets(p, "Pattern18InstallableApp", extra_cmake="""
install(TARGETS Pattern18InstallableApp
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
""")

    # 19. macOS / Windows GUI properties
    p = PROJECTS / "19_gui_properties"
    write_standard_widgets(p, "Pattern19GuiApp")

    # 20. Qt 6.3 compatibility style
    p = PROJECTS / "20_qt62_compat"
    write(p / "CMakeLists.txt", """
cmake_minimum_required(VERSION 3.16)

project(Pattern20Qt62Compat VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(Pattern20Qt62Compat
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

target_link_libraries(Pattern20Qt62Compat
    PRIVATE
        Qt6::Widgets
)
""")
    write(p / "main.cpp", """
#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    return 0;
}
""")
    write(p / "mainwindow.h", mainwindow_h())
    write(p / "mainwindow.cpp", """
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    Ui::MainWindow ui;
    ui.setupUi(this);
}
""")
    write(p / "mainwindow.ui", qt_ui_mainwindow())

    # 21. Versioned Qt CMake commands
    p = PROJECTS / "21_versioned_qt_commands"
    write(p / "CMakeLists.txt", """
cmake_minimum_required(VERSION 3.16)

project(Pattern21VersionedQtApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QT_NO_CREATE_VERSIONLESS_FUNCTIONS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt6_standard_project_setup()

qt6_add_executable(Pattern21VersionedQtApp
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_link_libraries(Pattern21VersionedQtApp
    PRIVATE
        Qt6::Widgets
)
""")
    write(p / "main.cpp", """
#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    return 0;
}
""")
    write(p / "mainwindow.h", mainwindow_h())
    write(p / "mainwindow.cpp", mainwindow_cpp())


if __name__ == "__main__":
    create_projects()
    print(f"Generated validation projects in {PROJECTS}")
