#include "objecttreedialog.h"

#include "objecttreewidget.h"

#include <QtGui/QShowEvent>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>

ObjectTreeDialog::ObjectTreeDialog(QWidget *parentWidget)
    : QDialog(parentWidget)
{
    setWindowTitle(QStringLiteral("Object Tree"));
    resize(420, 560);

    auto *verticalLayout = new QVBoxLayout(this);
    treeWidget = new ObjectTreeWidget(this);
    verticalLayout->addWidget(treeWidget);
}

void ObjectTreeDialog::setRootObject(QObject *rootObject)
{
    treeWidget->setRootObject(rootObject);
}

QObject *ObjectTreeDialog::rootObject() const
{
    return treeWidget->rootObject();
}

void ObjectTreeDialog::setCurrentObject(QObject *currentObject)
{
    treeWidget->setCurrentObject(currentObject);
}

QObject *ObjectTreeDialog::currentObject() const
{
    return treeWidget->currentObject();
}

ObjectTreeWidget *ObjectTreeDialog::objectTreeWidget() const
{
    return treeWidget;
}

void ObjectTreeDialog::showEvent(QShowEvent *showEvent)
{
    QDialog::showEvent(showEvent);

    if (hasAppliedInitialColumnWidths) {
        return;
    }

    QTreeWidget *internalTreeWidget = treeWidget->findChild<QTreeWidget *>();
    if (internalTreeWidget == nullptr) {
        return;
    }

    const int availableWidth = internalTreeWidget->viewport()->width();
    if (availableWidth <= 0) {
        return;
    }

    const int firstColumnWidth = availableWidth / 2;
    internalTreeWidget->header()->resizeSection(0, firstColumnWidth);
    internalTreeWidget->header()->resizeSection(1, std::max(0, availableWidth - firstColumnWidth));
    hasAppliedInitialColumnWidths = true;
}
