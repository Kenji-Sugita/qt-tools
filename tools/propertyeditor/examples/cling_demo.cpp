#pragma cling load("/Users/sugita/src/qt/tools/propertyeditor/build/property_editor.dylib")
#include "demo_widget_selector.h"

auto *windowWidget = new QWidget;
DemoWidgetSelector::setupDemoWindow(windowWidget);
windowWidget->show();
