#pragma once

#include "propertyeditorglobal.h"

#include <QtWidgets/QDialog>

class ObjectTreeWidget;
class QShowEvent;

class PROPERTY_EDITOR_EXPORT ObjectTreeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ObjectTreeDialog(QWidget *parentWidget = nullptr);

    void setRootObject(QObject *rootObject);
    QObject *rootObject() const;

    void setCurrentObject(QObject *currentObject);
    QObject *currentObject() const;

    ObjectTreeWidget *objectTreeWidget() const;

protected:
    void showEvent(QShowEvent *showEvent) override;

private:
    ObjectTreeWidget *treeWidget = nullptr;
    bool hasAppliedInitialColumnWidths = false;
};
