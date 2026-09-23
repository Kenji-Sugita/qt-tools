#pragma once

#include "propertyeditorglobal.h"

#include <QtWidgets/QWidget>

class QObject;
class QVBoxLayout;
class ObjectPropertyEditorWidget;

class PROPERTY_EDITOR_EXPORT PropertyEditor : public QWidget
{
    Q_OBJECT

public:
    explicit PropertyEditor(QWidget *parentWidget = nullptr);

    void setInspectedObject(QObject *targetObject);
    QObject *inspectedObject() const;

private:
    ObjectPropertyEditorWidget *propertyEditorWidget = nullptr;
    QVBoxLayout *verticalLayout = nullptr;
};
