#pragma cling load("/Users/sugita/src/qt/tools/propertyeditor/build/property_editor.dylib")
#include "../src/propertyeditor.h"
#include <QVBoxLayout>
#include <QDialog>

auto *dialog = new QDialog();
dialog->resize(400, 300);
dialog->show();

auto *propertyEditorHost = new PropertyEditor;
propertyEditorHost->resize(600, 420);
propertyEditorHost->setInspectedObject(dialog);
propertyEditorHost->show();
