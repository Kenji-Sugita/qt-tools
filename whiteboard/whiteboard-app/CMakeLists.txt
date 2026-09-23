cmake_minimum_required(VERSION 3.16)

if(APPLE AND NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build macOS universal binaries" FORCE)
endif()

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" WHITEBOARD_APP_VERSION LIMIT_COUNT 1)
string(STRIP "${WHITEBOARD_APP_VERSION}" WHITEBOARD_APP_VERSION)
if(NOT WHITEBOARD_APP_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    message(FATAL_ERROR "VERSION must contain a semantic version such as 1.2.3")
endif()

project(WhiteboardApp VERSION "${WHITEBOARD_APP_VERSION}" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

find_package(Qt6 REQUIRED COMPONENTS Core Gui HttpServer Network Svg Widgets Test LinguistTools)

qt_standard_project_setup()

qt_add_library(WhiteboardCore
    src/BoardDocument.cpp
    src/BoardDocumentExchange.cpp
    src/BoardEnums.cpp
    src/BoardModel.cpp
    src/BoardSettings.cpp
    src/DrawingElement.cpp
    src/DrawingRenderer.cpp
    src/ImageExporter.cpp
    src/ImageImporter.cpp
    src/JsonUtil.cpp
    src/Page.cpp
    include/BoardDocument.h
    include/BoardDocumentExchange.h
    include/BoardEnums.h
    include/BoardModel.h
    include/BoardSettings.h
    include/DrawingElement.h
    include/DrawingRenderer.h
    include/ImageExporter.h
    include/ImageImporter.h
    include/JsonUtil.h
    include/Page.h
)

target_include_directories(WhiteboardCore
    PUBLIC
        include
)

target_link_libraries(WhiteboardCore
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Svg
)

if(WIN32)
    set_target_properties(WhiteboardCore PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
endif()

qt_add_library(WhiteboardPlatform STATIC
    src/PlatformColorDialog.cpp
    include/PlatformColorDialog.h
)

target_include_directories(WhiteboardPlatform
    PUBLIC
        include
)

target_link_libraries(WhiteboardPlatform
    PUBLIC
        Qt6::Widgets
)

qt_add_executable(WhiteboardApp
    assets/appicon/WhiteboardApp.icns
    src/CanvasWidget.cpp
    src/CenteredComboBox.cpp
    src/FloatingActionBubbleLabel.cpp
    src/FloatingActionChildButton.cpp
    src/FloatingActionButton.cpp
    src/ImageExportDialog.cpp
    src/MainWindow.cpp
    src/PersistentToolBar.cpp
    src/WhiteboardMcpController.cpp
    src/WhiteboardMcpHttpServer.cpp
    src/main.cpp
    include/CanvasWidget.h
    include/CenteredComboBox.h
    include/FloatingActionBubbleLabel.h
    include/FloatingActionChildButton.h
    include/FloatingActionButton.h
    include/ImageExportDialog.h
    include/MainWindow.h
    include/PersistentToolBar.h
    include/WhiteboardMcpController.h
    include/WhiteboardMcpHttpServer.h
)

set_source_files_properties(assets/appicon/WhiteboardApp.icns PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources
)

qt_add_resources(WhiteboardApp "whiteboard_icons"
    PREFIX "/"
    FILES
        assets/appicon/whiteboard-app-icon.png
        assets/icons/arc.svg
        assets/icons/bezier.svg
        assets/icons/bring-forward.svg
        assets/icons/circle.svg
        assets/icons/color.svg
        assets/icons/copy.svg
        assets/icons/ellipse.svg
        assets/icons/eraser.svg
        assets/icons/fill-color.svg
        assets/icons/font.svg
        assets/icons/group.svg
        assets/icons/line.svg
        assets/icons/page-add.svg
        assets/icons/page-delete.svg
        assets/icons/page-lock.svg
        assets/icons/page-next.svg
        assets/icons/page-prev.svg
        assets/icons/page-unlock.svg
        assets/icons/paste.svg
        assets/icons/pen.svg
        assets/icons/polyline.svg
        assets/icons/quick-actions.svg
        assets/icons/rectangle.svg
        assets/icons/redo.svg
        assets/icons/reset-style.svg
        assets/icons/rounded-rectangle.svg
        assets/icons/scale.svg
        assets/icons/select.svg
        assets/icons/send-backward.svg
        assets/icons/stroke-width.svg
        assets/icons/text.svg
        assets/icons/undo.svg
        assets/icons/ungroup.svg
        assets/icons/zoom-in.svg
        assets/icons/zoom-out.svg
)

qt_add_translations(WhiteboardApp
    TS_FILES
        translations/whiteboard_ja.ts
)

target_link_libraries(WhiteboardApp
    PRIVATE
        WhiteboardCore
        WhiteboardPlatform
        Qt6::HttpServer
        Qt6::Network
        Qt6::Widgets
)

target_compile_definitions(WhiteboardApp
    PRIVATE
        WHITEBOARD_APP_VERSION="${PROJECT_VERSION}"
)

set_target_properties(WhiteboardApp PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
    MACOSX_BUNDLE_ICON_FILE WhiteboardApp.icns
)

install(TARGETS WhiteboardApp
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

install(FILES VERSION
    DESTINATION share/WhiteboardApp
)

function(add_whiteboard_core_test target source)
    qt_add_executable(${target}
        ${source}
    )
    target_link_libraries(${target}
        PRIVATE
            WhiteboardCore
            Qt6::Test
    )
    add_test(NAME ${target} COMMAND ${target})
endfunction()

function(add_whiteboard_canvas_test target source)
    qt_add_executable(${target}
        ${source}
        src/CanvasWidget.cpp
        include/CanvasWidget.h
    )
    target_link_libraries(${target}
        PRIVATE
            WhiteboardCore
            Qt6::Widgets
            Qt6::Test
    )
    add_test(NAME ${target} COMMAND ${target})
endfunction()

function(add_whiteboard_widget_test target source)
    qt_add_executable(${target}
        ${source}
        src/FloatingActionButton.cpp
        include/FloatingActionButton.h
    )
    target_link_libraries(${target}
        PRIVATE
            Qt6::Widgets
            Qt6::Test
    )
    target_include_directories(${target}
        PRIVATE
            include
    )
    add_test(NAME ${target} COMMAND ${target})
endfunction()

function(add_whiteboard_image_export_dialog_test target source)
    qt_add_executable(${target}
        ${source}
        src/ImageExportDialog.cpp
        include/ImageExportDialog.h
    )
    target_link_libraries(${target}
        PRIVATE
            WhiteboardCore
            Qt6::Widgets
            Qt6::Test
    )
    add_test(NAME ${target} COMMAND ${target})
endfunction()

function(add_whiteboard_mcp_test target source)
    qt_add_executable(${target}
        ${source}
        src/CanvasWidget.cpp
        src/WhiteboardMcpController.cpp
        include/CanvasWidget.h
        include/WhiteboardMcpController.h
    )
    target_link_libraries(${target}
        PRIVATE
            WhiteboardCore
            Qt6::Widgets
            Qt6::Test
    )
    target_compile_definitions(${target}
        PRIVATE
            WHITEBOARD_APP_VERSION="${PROJECT_VERSION}"
    )
    add_test(NAME ${target} COMMAND ${target})
endfunction()

function(add_whiteboard_mcp_http_test target source)
    qt_add_executable(${target}
        ${source}
        src/CanvasWidget.cpp
        src/WhiteboardMcpController.cpp
        src/WhiteboardMcpHttpServer.cpp
        include/CanvasWidget.h
        include/WhiteboardMcpController.h
        include/WhiteboardMcpHttpServer.h
    )
    target_link_libraries(${target}
        PRIVATE
            WhiteboardCore
            Qt6::HttpServer
            Qt6::Network
            Qt6::Widgets
            Qt6::Test
    )
    target_compile_definitions(${target}
        PRIVATE
            WHITEBOARD_APP_VERSION="${PROJECT_VERSION}"
    )
    add_test(NAME ${target} COMMAND ${target})
endfunction()

add_whiteboard_core_test(WhiteboardCoreDrawingTest tests/tst_boardmodel_drawing.cpp)
add_whiteboard_core_test(WhiteboardCoreStyleTest tests/tst_boardmodel_style.cpp)
add_whiteboard_core_test(WhiteboardCoreCurveTest tests/tst_boardmodel_curve.cpp)
add_whiteboard_core_test(WhiteboardCoreSelectionTest tests/tst_boardmodel_selection.cpp)
add_whiteboard_core_test(WhiteboardCoreDocumentTest tests/tst_boardmodel_document.cpp)
add_whiteboard_core_test(WhiteboardDocumentExchangeTest tests/tst_boarddocumentexchange.cpp)
add_whiteboard_core_test(WhiteboardImageExporterTest tests/tst_imageexporter.cpp)
add_whiteboard_core_test(WhiteboardImageImporterTest tests/tst_imageimporter.cpp)

add_whiteboard_widget_test(WhiteboardFloatingActionButtonTest tests/tst_floatingactionbutton.cpp)

qt_add_executable(WhiteboardPersistentToolBarTest
    tests/tst_persistenttoolbar.cpp
    src/PersistentToolBar.cpp
    include/PersistentToolBar.h
)
target_link_libraries(WhiteboardPersistentToolBarTest
    PRIVATE
        Qt6::Widgets
        Qt6::Test
)
target_include_directories(WhiteboardPersistentToolBarTest
    PRIVATE
        include
)
add_test(NAME WhiteboardPersistentToolBarTest COMMAND WhiteboardPersistentToolBarTest)

add_whiteboard_image_export_dialog_test(WhiteboardImageExportDialogTest tests/tst_imageexportdialog.cpp)
add_whiteboard_canvas_test(WhiteboardCanvasCurveTest tests/tst_canvaswidget_curve.cpp)
add_whiteboard_canvas_test(WhiteboardCanvasShapeTextTest tests/tst_canvaswidget_shape_text.cpp)
add_whiteboard_canvas_test(WhiteboardCanvasSelectionTest tests/tst_canvaswidget_selection.cpp)
add_whiteboard_mcp_test(WhiteboardMcpTest tests/tst_whiteboard_mcp.cpp)
add_whiteboard_mcp_http_test(WhiteboardMcpHttpTest tests/tst_whiteboard_mcp_http.cpp)
