# CMakeLists.txt 生成・修正用 Codex 指示

## 共通方針

Qt 6 用の `CMakeLists.txt` を作成または修正するときは、現代的な Qt CMake の
書き方を使用してください。

必須ルール:

- `find_package(Qt6 REQUIRED COMPONENTS ...)` を使用してください。
- 最初の `find_package(Qt6 ...)` の直後に `qt_standard_project_setup()` を呼び出
  してください。
- `qt_standard_project_setup()` は、`qt_add_executable()`、`add_executable()`、
  `qt_add_library()`、`add_library()` などのターゲット定義より前に呼び出してくだ
  さい。
- Qt アプリケーションのターゲットには、できるだけ `qt_add_executable()` を使用し
  てください。
- Qt ライブラリのターゲットには、必要に応じて `qt_add_library()` を使用してくださ
  い。
- Qt モジュールのリンクには、`Qt6::Widgets`、`Qt6::Gui`、`Qt6::Core`、
  `Qt6::Quick` などのインポートターゲットを使用してください。
- Qt 6.3 より古い Qt との互換性が明示的に必要でない限り、`CMAKE_AUTOMOC` や
  `CMAKE_AUTOUIC` を手動で設定しないでください。
- 指示されていない外部依存、ビルドオプション、警告オプション、インストール設定を
  勝手に追加しないでください。

## 既存 CMakeLists.txt 修正時の方針

- 既存のターゲット名、プロジェクト名、ディレクトリ構成、ソース構成をできるだけ維
  持してください。
- 既存コードの QML 読み込み URL、リソースパス、生成物名を壊さないでください。
- 古い書き方を現代化する場合でも、動作変更が起きる可能性のある変更は事前に説明し
  てください。
- ビルドエラー修正では、原因に直接関係する最小限の変更を優先してください。

## 非推奨の書き方

新規作成する `CMakeLists.txt` では、原則として次の書き方を避けてください。

- グローバルな `include_directories()`
- グローバルな `link_libraries()`
- 不必要な `file(GLOB ...)`
- Qt 6.3 以降を前提にできる場合の手動 `CMAKE_AUTOMOC` / `CMAKE_AUTOUIC`
- Qt モジュール名を文字列で直接リンクする古い書き方
- QML ファイルを単に `.qrc` に入れて `qt_add_executable()` のソースへ追加するだ
  けの構成

## パターン 1: Qt Widgets アプリケーション

用途:

- `QApplication` と QWidget 系クラスを使う GUI アプリケーション。

方針:

- `find_package(Qt6 REQUIRED COMPONENTS Widgets)` を使う。
- `qt_add_executable()` を使う。
- `Qt6::Widgets` をリンクする。
- GUI アプリケーションでは `WIN32_EXECUTABLE` と `MACOSX_BUNDLE` を設定する。

例:

```cmake
cmake_minimum_required(VERSION 3.16)

project(MyWidgetsApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(MyWidgetsApp
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_link_libraries(MyWidgetsApp
    PRIVATE
        Qt6::Widgets
)

set_target_properties(MyWidgetsApp PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
```

## パターン 2: Qt Quick / QML アプリケーション

用途:

- `QQmlApplicationEngine` と QML を使うアプリケーション。

方針:

- 新規の Qt Quick アプリケーションでは、可能であれば `qt_add_qml_module()` を使
  う。
- `main.cpp` で `engine.loadFromModule("module", "Main")` を使う場合は
  `qt_add_qml_module()` を使う。
- 既存コードが `qrc:/...` URL で QML を読み込む場合は `qt_add_resources()` を使
  う。
- `.qrc` ファイルを `qt_add_executable()` のソースに入れるだけの構成は避ける。

例:

```cmake
cmake_minimum_required(VERSION 3.16)

project(MyQuickApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup()

qt_add_executable(MyQuickApp
    main.cpp
)

qt_add_qml_module(MyQuickApp
    URI MyQuickApp
    VERSION 1.0
    QML_FILES
        Main.qml
    RESOURCES
        images/logo.png
)

target_link_libraries(MyQuickApp
    PRIVATE
        Qt6::Quick
)

set_target_properties(MyQuickApp PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
```

`qrc:/...` 互換構成の例:

```cmake
qt_add_executable(layouts
    main.cpp
)

qt_add_resources(layouts "layouts_resources"
    PREFIX "/qt/qml/layouts"
    FILES
        layouts.qml
)
```

この場合、`main.cpp` の読み込み先と `PREFIX` を一致させてください。

```cpp
engine.load(QUrl(QStringLiteral("qrc:/qt/qml/layouts/layouts.qml")));
```

## パターン 3: Qt コンソールアプリケーション

用途:

- `QCoreApplication` を使うコマンドラインアプリケーション。

方針:

- `find_package(Qt6 REQUIRED COMPONENTS Core)` を使う。
- `Qt6::Core` をリンクする。
- `WIN32_EXECUTABLE` と `MACOSX_BUNDLE` は設定しない。

例:

```cmake
cmake_minimum_required(VERSION 3.16)

project(MyConsoleApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

qt_standard_project_setup()

qt_add_executable(MyConsoleApp
    main.cpp
)

target_link_libraries(MyConsoleApp
    PRIVATE
        Qt6::Core
)
```

## パターン 4: Qt Core ライブラリ

用途:

- QObject、QString、QFile、QTimer など Qt Core を使う共通ライブラリ。

方針:

- `find_package(Qt6 REQUIRED COMPONENTS Core)` を使う。
- ライブラリターゲットには `qt_add_library()` を使う。
- 公開ヘッダが Qt Core 型を公開 API に含む場合は `Qt6::Core` を `PUBLIC` でリンク
  する。

例:

```cmake
cmake_minimum_required(VERSION 3.16)

project(MyCoreLibrary VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

qt_standard_project_setup()

qt_add_library(MyCoreLibrary
    myobject.cpp
    myobject.h
)

target_link_libraries(MyCoreLibrary
    PUBLIC
        Qt6::Core
)
```

## パターン 5: Qt Widgets ライブラリ

用途:

- QWidget 派生クラスや Widgets 部品をまとめるライブラリ。

方針:

- `find_package(Qt6 REQUIRED COMPONENTS Widgets)` を使う。
- 公開 API が QWidget 系を含む場合は `Qt6::Widgets` を `PUBLIC` でリンクする。

例:

```cmake
cmake_minimum_required(VERSION 3.16)

project(MyWidgetsLibrary VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_library(MyWidgetsLibrary
    customwidget.cpp
    customwidget.h
)

target_link_libraries(MyWidgetsLibrary
    PUBLIC
        Qt6::Widgets
)
```

## パターン 6: アプリケーション + 共通ライブラリ

用途:

- アプリケーション本体と共通処理を別ターゲットに分ける構成。

方針:

- 共通処理はライブラリターゲットにする。
- アプリケーションターゲットは共通ライブラリをリンクする。
- 共通ライブラリの依存は、そのライブラリ側の `target_link_libraries()` に書く。

例:

```cmake
cmake_minimum_required(VERSION 3.16)

project(MyAppWithLibrary VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_library(MyAppCore
    appmodel.cpp
    appmodel.h
)

target_link_libraries(MyAppCore
    PUBLIC
        Qt6::Core
)

qt_add_executable(MyApp
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_link_libraries(MyApp
    PRIVATE
        MyAppCore
        Qt6::Widgets
)

set_target_properties(MyApp PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
```

## パターン 7: 複数実行ファイルを持つプロジェクト

用途:

- 1 つのプロジェクト内に GUI アプリ、CLI ツール、補助ツールなどを含む構成。

方針:

- 実行ファイルごとに別ターゲットを定義する。
- 各ターゲットに必要な Qt モジュールだけをリンクする。
- GUI ターゲットだけに `WIN32_EXECUTABLE` と `MACOSX_BUNDLE` を設定する。

例:

```cmake
cmake_minimum_required(VERSION 3.16)

project(MyTools VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(MyGuiTool
    gui_main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_link_libraries(MyGuiTool
    PRIVATE
        Qt6::Widgets
)

set_target_properties(MyGuiTool PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

qt_add_executable(MyCliTool
    cli_main.cpp
)

target_link_libraries(MyCliTool
    PRIVATE
        Qt6::Core
)
```

## パターン 8: Qt Test を使うテストターゲット

用途:

- Qt Test で単体テストを作る構成。

方針:

- `enable_testing()` を呼び出す。
- `find_package(Qt6 REQUIRED COMPONENTS Test)` を使う。
- テストはアプリケーション本体とは別ターゲットにする。
- `add_test()` を使って CTest に登録する。

例:

```cmake
cmake_minimum_required(VERSION 3.16)

project(MyQtTest VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

find_package(Qt6 REQUIRED COMPONENTS Test)

qt_standard_project_setup()

qt_add_executable(MyQtTest
    tst_mytest.cpp
)

target_link_libraries(MyQtTest
    PRIVATE
        Qt6::Test
)

add_test(NAME MyQtTest COMMAND MyQtTest)
```

## パターン 9: アプリケーション + テスト構成

用途:

- アプリケーション本体とテストを同じプロジェクトに含める構成。

方針:

- テスト対象のロジックはライブラリに分離する。
- アプリケーションとテストは、そのライブラリをそれぞれリンクする。
- `enable_testing()` はルート側で呼び出す。

例:

```cmake
cmake_minimum_required(VERSION 3.16)

project(MyTestableApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

find_package(Qt6 REQUIRED COMPONENTS Widgets Test)

qt_standard_project_setup()

qt_add_library(MyAppCore
    calculator.cpp
    calculator.h
)

target_link_libraries(MyAppCore
    PUBLIC
        Qt6::Core
)

qt_add_executable(MyTestableApp
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_link_libraries(MyTestableApp
    PRIVATE
        MyAppCore
        Qt6::Widgets
)

set_target_properties(MyTestableApp PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

qt_add_executable(MyAppCoreTest
    tst_calculator.cpp
)

target_link_libraries(MyAppCoreTest
    PRIVATE
        MyAppCore
        Qt6::Test
)

add_test(NAME MyAppCoreTest COMMAND MyAppCoreTest)
```

## パターン 10: サブディレクトリ構成

用途:

- `src`、`tests` など複数ディレクトリに分かれたプロジェクト。

方針:

- ルートの `CMakeLists.txt` は全体設定と `add_subdirectory()` を中心にする。
- 各ターゲットの詳細は、できるだけ各サブディレクトリの `CMakeLists.txt` に置く。
- `find_package(Qt6 ...)` と `qt_standard_project_setup()` は、原則としてルート側に
  置く。

ルートの例:

```cmake
cmake_minimum_required(VERSION 3.16)

project(MySubdirProject VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

find_package(Qt6 REQUIRED COMPONENTS Widgets Test)

qt_standard_project_setup()

add_subdirectory(src)
add_subdirectory(tests)
```

`src/CMakeLists.txt` の例:

```cmake
qt_add_executable(MySubdirApp
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_link_libraries(MySubdirApp
    PRIVATE
        Qt6::Widgets
)
```

`tests/CMakeLists.txt` の例:

```cmake
qt_add_executable(MySubdirTest
    tst_mytest.cpp
)

target_link_libraries(MySubdirTest
    PRIVATE
        Qt6::Test
)

add_test(NAME MySubdirTest COMMAND MySubdirTest)
```

## パターン 11: Qt Designer の .ui ファイルを使う Widgets アプリ

用途:

- Qt Designer で作成した `.ui` ファイルを使う Widgets アプリケーション。

方針:

- Qt 6.3 以降では `qt_standard_project_setup()` により AUTOUIC が有効になるため、
  原則として `CMAKE_AUTOUIC` を手動設定しない。
- `.ui` ファイルはターゲットのソースに含める。

例:

```cmake
cmake_minimum_required(VERSION 3.16)

project(MyUiApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(MyUiApp
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

target_link_libraries(MyUiApp
    PRIVATE
        Qt6::Widgets
)

set_target_properties(MyUiApp PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
```

## パターン 12: リソースファイルを使う Widgets アプリ

用途:

- アイコン、画像、テキストファイルなどを Qt リソースとして埋め込む Widgets アプリ
  ケーション。

方針:

- 新規構成では `.qrc` を手書きするより、必要に応じて `qt_add_resources()` を使う。
- 既存の `.qrc` がある場合は、既存構成を壊さない範囲で維持してよい。
- C++ 側の `:/...` パスと `PREFIX` を一致させる。

例:

```cmake
cmake_minimum_required(VERSION 3.16)

project(MyResourceApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(MyResourceApp
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

qt_add_resources(MyResourceApp "app_resources"
    PREFIX "/"
    FILES
        icons/app.png
        images/splash.png
)

target_link_libraries(MyResourceApp
    PRIVATE
        Qt6::Widgets
)

set_target_properties(MyResourceApp PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
```

## パターン 13: Qt Network を使うアプリケーション

用途:

- `QNetworkAccessManager` など Qt Network を使うアプリケーション。

方針:

- `Network` コンポーネントを `find_package()` に追加する。
- `Qt6::Network` をリンクする。
- GUI が必要な場合は `Widgets` または `Quick` も追加する。

例:

```cmake
cmake_minimum_required(VERSION 3.16)

project(MyNetworkApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

qt_standard_project_setup()

qt_add_executable(MyNetworkApp
    main.cpp
    downloader.cpp
    downloader.h
)

target_link_libraries(MyNetworkApp
    PRIVATE
        Qt6::Core
        Qt6::Network
)
```

## パターン 14: Qt SQL を使うアプリケーション

用途:

- `QSqlDatabase` など Qt SQL を使うアプリケーション。

方針:

- `Sql` コンポーネントを `find_package()` に追加する。
- `Qt6::Sql` をリンクする。
- データベースドライバの配置や配布は、必要になったときに別途扱う。

例:

```cmake
cmake_minimum_required(VERSION 3.16)

project(MySqlApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Sql)

qt_standard_project_setup()

qt_add_executable(MySqlApp
    main.cpp
    repository.cpp
    repository.h
)

target_link_libraries(MySqlApp
    PRIVATE
        Qt6::Core
        Qt6::Sql
)
```

## パターン 15: Qt Concurrent を使うアプリケーション

用途:

- `QtConcurrent` を使う並列処理アプリケーション。

方針:

- `Concurrent` コンポーネントを `find_package()` に追加する。
- `Qt6::Concurrent` をリンクする。
- GUI アプリの場合でも、並列処理の依存は使うターゲットにだけリンクする。

例:

```cmake
cmake_minimum_required(VERSION 3.16)

project(MyConcurrentApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Concurrent)

qt_standard_project_setup()

qt_add_executable(MyConcurrentApp
    main.cpp
    worker.cpp
    worker.h
)

target_link_libraries(MyConcurrentApp
    PRIVATE
        Qt6::Core
        Qt6::Concurrent
)
```

## パターン 16: OpenGL / Qt OpenGL を使うアプリケーション

用途:

- `QOpenGLWidget` や Qt OpenGL 関連クラスを使うアプリケーション。

方針:

- Widgets の `QOpenGLWidget` を使う場合は `OpenGLWidgets` を追加する。
- 必要に応じて `OpenGL` も追加する。
- `Qt6::OpenGLWidgets` と必要な Qt モジュールをリンクする。

例:

```cmake
cmake_minimum_required(VERSION 3.16)

project(MyOpenGLApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)

qt_standard_project_setup()

qt_add_executable(MyOpenGLApp
    main.cpp
    glwidget.cpp
    glwidget.h
)

target_link_libraries(MyOpenGLApp
    PRIVATE
        Qt6::Widgets
        Qt6::OpenGLWidgets
)

set_target_properties(MyOpenGLApp PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
```

## パターン 17: 翻訳ファイルを扱うプロジェクト

用途:

- Qt Linguist の `.ts` 翻訳ファイルを扱うプロジェクト。

方針:

- `LinguistTools` コンポーネントを追加する。
- Qt 6 の翻訳支援コマンドを使う。
- 既存プロジェクトの翻訳更新・生成フローがある場合は、それを優先する。

例:

```cmake
cmake_minimum_required(VERSION 3.16)

project(MyTranslatedApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets LinguistTools)

qt_standard_project_setup()

qt_add_executable(MyTranslatedApp
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

qt_add_translations(MyTranslatedApp
    TS_FILES
        translations/MyTranslatedApp_ja.ts
)

target_link_libraries(MyTranslatedApp
    PRIVATE
        Qt6::Widgets
)

set_target_properties(MyTranslatedApp PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
```

## パターン 18: install() を含む構成

用途:

- ビルド成果物をインストール対象にするプロジェクト。

方針:

- 指示がある場合だけ `install()` を追加する。
- GUI アプリでは `BUNDLE`、通常実行ファイルでは `RUNTIME` を指定する。
- ライブラリでは `LIBRARY`、`ARCHIVE`、`RUNTIME` の出力先を必要に応じて指定する。
- Qt のデプロイ処理は、必要が明示された場合だけ追加する。

例:

```cmake
cmake_minimum_required(VERSION 3.16)

project(MyInstallableApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(MyInstallableApp
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_link_libraries(MyInstallableApp
    PRIVATE
        Qt6::Widgets
)

set_target_properties(MyInstallableApp PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

install(TARGETS MyInstallableApp
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
```

## パターン 19: macOS / Windows GUI アプリ用プロパティ

用途:

- GUI アプリケーションとして起動する Widgets または Quick アプリケーション。

方針:

- GUI アプリには `WIN32_EXECUTABLE ON` と `MACOSX_BUNDLE ON` を設定する。
- コンソールアプリやテストターゲットには設定しない。
- 既存プロジェクトで明示的に無効化されている場合は、既存挙動を尊重する。

例:

```cmake
set_target_properties(MyGuiApp PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
```

## パターン 20: Qt 6.3 未満に対応する構成

用途:

- Qt 6.2 など、`qt_standard_project_setup()` が使えない環境を明示的にサポートする
  場合。

方針:

- Qt 6.3 未満の互換性が明示されている場合だけ、この構成を使う。
- `qt_standard_project_setup()` は使わない。
- 必要な場合に限り、`CMAKE_AUTOMOC`、`CMAKE_AUTOUIC`、`CMAKE_AUTORCC` を設定する。

例:

```cmake
cmake_minimum_required(VERSION 3.16)

project(MyQt62App VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(MyQt62App
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

target_link_libraries(MyQt62App
    PRIVATE
        Qt6::Widgets
)
```

## パターン 21: バージョンなし Qt CMake コマンドが無効な環境

用途:

- `QT_NO_CREATE_VERSIONLESS_FUNCTIONS` などにより、`qt_add_executable()` や
  `qt_standard_project_setup()` などのバージョンなし Qt CMake コマンドが使えない
  環境。

方針:

- バージョンなしコマンドが無効化されていることが明示されている場合だけ、この構成を
  使う。
- `qt_standard_project_setup()` の代わりに `qt6_standard_project_setup()` を使う。
- `qt_add_executable()` の代わりに `qt6_add_executable()` を使う。
- `qt_add_qml_module()` の代わりに `qt6_add_qml_module()` を使う。

例:

```cmake
cmake_minimum_required(VERSION 3.16)

project(MyVersionedQtApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt6_standard_project_setup()

qt6_add_executable(MyVersionedQtApp
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_link_libraries(MyVersionedQtApp
    PRIVATE
        Qt6::Widgets
)
```

## パターン 22: Qt Quick Controls アプリケーション

用途:

- `ApplicationWindow`、`Button`、`Label` など Qt Quick Controls を使う QML アプリ
  ケーション。

方針:

- `find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2)` を使う。
- `qt_add_qml_module()` で QML ファイルを登録する。
- C++ 側は `QGuiApplication` と `QQmlApplicationEngine` を使う。
- GUI アプリとして `WIN32_EXECUTABLE` と `MACOSX_BUNDLE` を設定する。

例:

```cmake
find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2)

qt_standard_project_setup()

qt_add_executable(MyControlsApp
    main.cpp
)

qt_add_qml_module(MyControlsApp
    URI MyControlsApp
    VERSION 1.0
    QML_FILES
        Main.qml
)

target_link_libraries(MyControlsApp
    PRIVATE
        Qt6::Quick
)

set_target_properties(MyControlsApp PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
```

## パターン 23: 複数 QML ファイルを持つアプリケーション

用途:

- `Main.qml` 以外に、`components/` などへ分けた QML 部品を持つアプリケーション。

方針:

- すべての QML ファイルを `qt_add_qml_module()` の `QML_FILES` に明示的に列挙す
  る。
- QML ファイルのディレクトリ構成を保ったまま登録する。

例:

```cmake
qt_add_qml_module(MyMultiQmlApp
    URI MyMultiQmlApp
    VERSION 1.0
    QML_FILES
        Main.qml
        components/StatusPanel.qml
        components/ToolBar.qml
)
```

## パターン 24: QML リソースを含むアプリケーション

用途:

- QML から画像、フォント、JSON などの補助ファイルを参照するアプリケーション。

方針:

- QML ファイルは `QML_FILES` に、画像やフォントなどは `RESOURCES` に列挙する。
- `.qrc` を新規に手書きするより、`qt_add_qml_module()` の `RESOURCES` を優先す
  る。

例:

```cmake
qt_add_qml_module(MyQmlResourceApp
    URI MyQmlResourceApp
    VERSION 1.0
    QML_FILES
        Main.qml
    RESOURCES
        images/logo.png
        fonts/Inter-Regular.ttf
)
```

## パターン 25: C++ バックエンドを QML に公開するアプリケーション

用途:

- `QObject` 派生クラスを QML から使うアプリケーション。

方針:

- C++ バックエンドの `.cpp` / `.h` を実行ファイルターゲットに含める。
- `qmlRegisterType()` を使う場合は `Qml` コンポーネントも追加する。
- QML から見える URI と C++ 側の登録 URI を一致させる。

例:

```cmake
find_package(Qt6 REQUIRED COMPONENTS Quick Qml)

qt_add_executable(MyBackendApp
    main.cpp
    backend.cpp
    backend.h
)

qt_add_qml_module(MyBackendApp
    URI MyBackendApp
    VERSION 1.0
    QML_FILES
        Main.qml
)

target_link_libraries(MyBackendApp
    PRIVATE
        Qt6::Quick
        Qt6::Qml
)
```

## パターン 26: QML singleton を使うアプリケーション

用途:

- テーマ、設定値、定数などを QML singleton として共有するアプリケーション。

方針:

- singleton QML ファイルに `pragma Singleton` を書く。
- CMake 側では `set_source_files_properties(... QT_QML_SINGLETON_TYPE TRUE)` を指定
  する。

例:

```cmake
set_source_files_properties(Theme.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(MySingletonApp
    URI MySingletonApp
    VERSION 1.0
    QML_FILES
        Main.qml
        Theme.qml
)
```

## パターン 27: 既存 qrc:/ 読み込みを維持する QML アプリケーション

用途:

- 既存コードが `engine.load(QUrl("qrc:/..."))` で QML を読み込んでいるアプリケー
  ション。

方針:

- 既存の QML 読み込み URL を壊さない。
- `qt_add_resources()` の `PREFIX` と C++ 側の `qrc:/...` URL を一致させる。
- 既存 qmake / `.qrc` からの移行では、このパターンを優先してよい。

例:

```cmake
qt_add_resources(MyLegacyQmlApp "qml_resources"
    PREFIX "/qt/qml/MyLegacyQmlApp"
    FILES
        Main.qml
)
```

```cpp
engine.load(QUrl(QStringLiteral("qrc:/qt/qml/MyLegacyQmlApp/Main.qml")));
```

## パターン 28: QML モジュールをライブラリ化する構成

用途:

- アプリ本体とは別に、再利用する QML 部品を QML モジュールとして分ける構成。

方針:

- 再利用 QML モジュールは `qt_add_library()` と `qt_add_qml_module()` で定義する。
- アプリターゲットは、その QML モジュールターゲットをリンクする。
- 複数モジュールを同じディレクトリに出す場合は、必要に応じて `OUTPUT_DIRECTORY`
  を指定する。

例:

```cmake
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

target_link_libraries(MyQmlApp
    PRIVATE
        ReusableUi
)
```

## パターン 29: 複数 QML モジュールを持つ構成

用途:

- `AppTheme`、`AppComponents`、アプリ本体など、複数 URI の QML モジュールを持つ構
  成。

方針:

- QML モジュールごとにターゲットと URI を分ける。
- 同じビルドディレクトリで複数 QML モジュールを定義する場合は、`OUTPUT_DIRECTORY`
  を URI に合わせて分ける。
- モジュール間の依存は `target_link_libraries()` で表現する。

例:

```cmake
qt_add_library(AppThemeModule STATIC)
qt_add_qml_module(AppThemeModule
    URI AppTheme
    VERSION 1.0
    OUTPUT_DIRECTORY AppTheme
    QML_FILES
        ThemePalette.qml
)

qt_add_library(AppComponentsModule STATIC)
qt_add_qml_module(AppComponentsModule
    URI AppComponents
    VERSION 1.0
    OUTPUT_DIRECTORY AppComponents
    QML_FILES
        InfoCard.qml
)
target_link_libraries(AppComponentsModule
    PRIVATE
        AppThemeModule
)
```

## パターン 30: C++ 型を含む QML モジュール

用途:

- QML モジュール内に C++ 型を含め、QML から直接利用する構成。

方針:

- C++ 型を `qt_add_executable()` または `qt_add_library()` のソースに含める。
- ヘッダでは `QML_ELEMENT` などの登録マクロを使う。
- `QML_ELEMENT` を使う場合は `<QtQmlIntegration/qqmlintegration.h>` を include す
  る。

例:

```cmake
qt_add_executable(MyCppQmlModuleApp
    main.cpp
    counter.cpp
    counter.h
)

qt_add_qml_module(MyCppQmlModuleApp
    URI MyCppQmlModuleApp
    VERSION 1.0
    QML_FILES
        Main.qml
)
```

## パターン 31: QML アプリケーション + テスト構成

用途:

- QML アプリ本体と、C++ 側のロジックテストを同じプロジェクトに含める構成。

方針:

- テスト対象のロジックはライブラリに分離する。
- QML アプリと Qt Test ターゲットは、そのライブラリをそれぞれリンクする。
- `enable_testing()` と `add_test()` を使う。

例:

```cmake
enable_testing()

find_package(Qt6 REQUIRED COMPONENTS Quick Test)

qt_add_library(MyQmlAppCore
    formatter.cpp
    formatter.h
)
target_link_libraries(MyQmlAppCore
    PUBLIC
        Qt6::Core
)

qt_add_executable(MyQmlAppTest
    tst_formatter.cpp
)
target_link_libraries(MyQmlAppTest
    PRIVATE
        MyQmlAppCore
        Qt6::Test
)
add_test(NAME MyQmlAppTest COMMAND MyQmlAppTest)
```

## パターン 32: QML アプリケーションの install() 構成

用途:

- QML アプリケーションの実行ファイルをインストール対象にする構成。

方針:

- 指示がある場合だけ `install()` を追加する。
- GUI アプリでは `BUNDLE`、通常実行ファイルでは `RUNTIME` を指定する。
- QML モジュールや Qt のデプロイ処理は、必要が明示された場合に追加する。

例:

```cmake
install(TARGETS MyQmlInstallApp
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
```

## パターン 33: Qt 6.3 未満向け QML 構成

用途:

- Qt 6.2 など、`qt_standard_project_setup()` や QML module 周りの新しい方針を使い
  にくい環境を明示的にサポートする場合。

方針:

- Qt 6.3 未満の互換性が明示されている場合だけ使う。
- 必要な場合に限り `CMAKE_AUTOMOC`、`CMAKE_AUTORCC` を設定する。
- 既存 `qrc:/...` 読み込みと同じく、`qt_add_resources()` で QML を登録する構成を
  優先する。

例:

```cmake
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

add_executable(MyQt62QmlApp
    main.cpp
)

qt_add_resources(MyQt62QmlApp "qml_resources"
    PREFIX "/qt/qml/MyQt62QmlApp"
    FILES
        Main.qml
)

target_link_libraries(MyQt62QmlApp
    PRIVATE
        Qt6::Quick
)
```

## Qt モジュール選択の方針

- QWidget を使う場合は `Widgets` を指定してください。
- `QGuiApplication`、画像、フォント、低レベル GUI 機能を使う場合は `Gui` を指定して
  ください。
- QObject、QString、QFile、QTimer など Qt Core だけで足りる場合は `Core` を指定し
  てください。
- `QQmlApplicationEngine` や Qt Quick を使う場合は `Quick` を指定してください。
- Qt Quick Controls を使う場合は `QuickControls2` も指定してください。
- ネットワーク機能を使う場合は `Network` を指定してください。
- SQL を使う場合は `Sql` を指定してください。
- Qt Test を使う場合は `Test` を指定してください。
- `QtConcurrent` を使う場合は `Concurrent` を指定してください。
- `QOpenGLWidget` を使う場合は `OpenGLWidgets` を指定してください。

## ソースファイル列挙の方針

- 小規模または新規プロジェクトでは、ソースファイルは明示的に列挙してください。
- 既存プロジェクトが `file(GLOB ...)` を使っている場合は、既存方針に合わせても構い
  ません。
- `file(GLOB CONFIGURE_DEPENDS ...)` を使う場合は、CMake 再構成の挙動を理解した上
  で使ってください。
- ヘッダファイルはビルドに必須でない場合でも、IDE 表示のためにターゲットのソース
  へ含めて構いません。

## QML 読み込み方法と CMake の対応

- `engine.loadFromModule("module", "Main")` を使う場合は `qt_add_qml_module()` を
  使ってください。
- `engine.load(QUrl("qrc:/..."))` を使う場合は `qt_add_resources()` を使ってくださ
  い。
- 既存コードがある場合、CMake 側だけを変更して読み込み URL を壊さないでください。
- QML に画像、フォント、JSON などの関連ファイルがある場合は、`qt_add_qml_module()`
  の `RESOURCES` または `qt_add_resources()` の `FILES` に含めてください。

## 外部ライブラリを使う場合の方針

- Qt 以外の外部依存は、ユーザーの指示または既存プロジェクトの依存がある場合だけ追
  加してください。
- CMake パッケージが提供されている場合は、原則として `find_package()` とインポート
  ターゲットを使ってください。
- 依存のリンクは `target_link_libraries()` でターゲット単位に指定してください。
- グローバルな include path や link path は避けてください。

## FetchContent を使う場合の方針

- `FetchContent` は、ユーザーが明示的に依存取得を求めた場合、または既存プロジェク
  トがその方針を採用している場合だけ使ってください。
- 既にパッケージマネージャやサブモジュールが使われている場合は、既存方針を優先して
  ください。
- ネットワーク取得を前提にする変更は、実装前に必要性を説明してください。

## 生成後の確認項目

- `cmake_minimum_required()` があること。
- `project()` があり、必要な言語が指定されていること。
- 必要な Qt モジュールが `find_package(Qt6 REQUIRED COMPONENTS ...)` に含まれてい
  ること。
- 最初の `find_package(Qt6 ...)` の後、ターゲット定義より前に
  `qt_standard_project_setup()` があること。
- 各ターゲットが必要な Qt インポートターゲットをリンクしていること。
- GUI アプリだけに `WIN32_EXECUTABLE` と `MACOSX_BUNDLE` が設定されていること。
- QML の読み込み方法と CMake の登録方法が一致していること。
- テストターゲットがある場合は `enable_testing()` と `add_test()` があること。
