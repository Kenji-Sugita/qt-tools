#pragma cling load("/Users/sugita/src/qt/tools/propertyeditor/build/property_editor.dylib")
#pragma cling load("/Users/sugita/src/qt/tools/propertyeditor/examples/colorwidget/build/libcolorwidgetplugin.dylib")
#include "colorwidget/colorwidget.h"
#include "../src/propertyeditor.h"

auto colorWidget = new ColorWidget{};
colorWidget->show();

auto *propertyEditorHost = new PropertyEditor;
propertyEditorHost->resize(600, 420);
propertyEditorHost->setInspectedObject(colorWidget);
propertyEditorHost->show();
