#pragma once

#include "objectselectorglobal.h"

#include <QtCore/QPointer>
#include <QtWidgets/QWidget>

class QLabel;
class QLineEdit;
class QResizeEvent;
class QTreeWidget;
class QTreeWidgetItem;

class OBJECT_SELECTOR_EXPORT ObjectTreeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ObjectTreeWidget(QWidget *parentWidget = nullptr);

    void setRootObject(QObject *rootObject);
    QObject *rootObject() const;

    void setCurrentObject(QObject *currentObject);
    QObject *currentObject() const;

signals:
    void objectActivated(QObject *targetObject);

protected:
    void resizeEvent(QResizeEvent *resizeEvent) override;

private slots:
    void handleCurrentItemChanged(QTreeWidgetItem *currentItem, QTreeWidgetItem *previousItem);
    void handleFilterTextChanged(const QString &filterText);

private:
    void applyFilter(const QString &filterText);
    void updateBreadcrumbLabel();
    bool filterTreeItem(QTreeWidgetItem *treeItem, const QString &filterText);
    QTreeWidgetItem *findTreeItem(QObject *targetObject, QTreeWidgetItem *parentItem = nullptr) const;
    void populateTree(QObject *targetObject, QTreeWidgetItem *parentItem);
    void selectCurrentObject();

    QLabel *breadcrumbValueLabel = nullptr;
    QLineEdit *filterLineEdit = nullptr;
    QTreeWidget *treeWidget = nullptr;
    QPointer<QObject> rootObjectPointer;
    QPointer<QObject> currentObjectPointer;
};
