#include "propertyeditor.h"

#include "objectpropertyeditorwidget.h"

#include <QtWidgets/QVBoxLayout>

PropertyEditor::PropertyEditor(QWidget *parentWidget)
    : QWidget(parentWidget)
{
    verticalLayout = new QVBoxLayout(this);
    verticalLayout->setContentsMargins(0, 0, 0, 0);

    propertyEditorWidget = new ObjectPropertyEditorWidget(this);
    verticalLayout->addWidget(propertyEditorWidget);
}

void PropertyEditor::setInspectedObject(QObject *targetObject)
{
    if (propertyEditorWidget != nullptr) {
        propertyEditorWidget->setInspectedObject(targetObject);
    }
}

QObject *PropertyEditor::inspectedObject() const
{
    return propertyEditorWidget != nullptr ? propertyEditorWidget->inspectedObject() : nullptr;
}
