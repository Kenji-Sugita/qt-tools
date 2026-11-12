#include "objecttreewidget.h"

#include <QtCore/QSignalBlocker>
#include <QtCore/QVariant>
#include <QtGui/QFontMetrics>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QTreeWidgetItem>
#include <QtWidgets/QVBoxLayout>

namespace {

QString objectTreeLabel(const QObject *targetObject)
{
    if (targetObject == nullptr) {
        return QStringLiteral("<null>");
    }

    const QString objectName = targetObject->objectName().trimmed();
    if (!objectName.isEmpty()) {
        return objectName;
    }

    return QStringLiteral("<unnamed>");
}

QString objectTreeTypeName(const QObject *targetObject)
{
    return targetObject != nullptr ? QString::fromLatin1(targetObject->metaObject()->className())
                                   : QStringLiteral("QObject");
}

QString objectBreadcrumbSegment(const QObject *targetObject)
{
    if (targetObject == nullptr) {
        return QStringLiteral("QObject(<null>)");
    }

    return QStringLiteral("%1(%2)")
        .arg(objectTreeTypeName(targetObject), objectTreeLabel(targetObject));
}

QString fullBreadcrumbPath(const QObject *targetObject)
{
    QStringList pathSegments;
    for (const QObject *currentObject = targetObject;
         currentObject != nullptr;
         currentObject = currentObject->parent()) {
        pathSegments.prepend(objectBreadcrumbSegment(currentObject));
    }

    return pathSegments.join(QStringLiteral(" > "));
}

} // namespace

ObjectTreeWidget::ObjectTreeWidget(QWidget *parentWidget)
    : QWidget(parentWidget)
{
    auto *verticalLayout = new QVBoxLayout(this);

    breadcrumbValueLabel = new QLabel(this);
    breadcrumbValueLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    breadcrumbValueLabel->setWordWrap(false);

    filterLineEdit = new QLineEdit(this);
    filterLineEdit->setClearButtonEnabled(true);
    filterLineEdit->setPlaceholderText(
        QStringLiteral("Filter by object name or class name"));

    treeWidget = new QTreeWidget(this);
    treeWidget->setColumnCount(2);
    treeWidget->setHeaderLabels({QStringLiteral("Object Name"), QStringLiteral("Class Name")});
    treeWidget->header()->setStretchLastSection(false);
    treeWidget->header()->setSectionResizeMode(0, QHeaderView::Interactive);
    treeWidget->header()->setSectionResizeMode(1, QHeaderView::Interactive);

    verticalLayout->addWidget(breadcrumbValueLabel);
    verticalLayout->addWidget(filterLineEdit);
    verticalLayout->addWidget(treeWidget);

    connect(treeWidget, &QTreeWidget::currentItemChanged,
            this, &ObjectTreeWidget::handleCurrentItemChanged);
    connect(filterLineEdit, &QLineEdit::textChanged, this, &ObjectTreeWidget::handleFilterTextChanged);
}

void ObjectTreeWidget::setRootObject(QObject *rootObject)
{
    rootObjectPointer = rootObject;
    treeWidget->clear();

    if (rootObject != nullptr) {
        populateTree(rootObject, nullptr);
        treeWidget->expandAll();
    }

    applyFilter(filterLineEdit->text());
    selectCurrentObject();
    updateBreadcrumbLabel();
}

QObject *ObjectTreeWidget::rootObject() const
{
    return rootObjectPointer;
}

void ObjectTreeWidget::setCurrentObject(QObject *currentObject)
{
    currentObjectPointer = currentObject;
    selectCurrentObject();
    updateBreadcrumbLabel();
}

QObject *ObjectTreeWidget::currentObject() const
{
    return currentObjectPointer;
}

void ObjectTreeWidget::resizeEvent(QResizeEvent *resizeEvent)
{
    QWidget::resizeEvent(resizeEvent);
    updateBreadcrumbLabel();
}

void ObjectTreeWidget::handleCurrentItemChanged(QTreeWidgetItem *currentItem, QTreeWidgetItem *)
{
    if (currentItem == nullptr) {
        currentObjectPointer.clear();
        updateBreadcrumbLabel();
        return;
    }

    const quintptr objectAddress = currentItem->data(0, Qt::UserRole).value<quintptr>();
    auto *targetObject = reinterpret_cast<QObject *>(objectAddress);
    currentObjectPointer = targetObject;
    updateBreadcrumbLabel();
    if (targetObject != nullptr) {
        emit objectActivated(targetObject);
    }
}

void ObjectTreeWidget::handleFilterTextChanged(const QString &filterText)
{
    applyFilter(filterText);
    selectCurrentObject();
}

void ObjectTreeWidget::applyFilter(const QString &filterText)
{
    for (int itemIndex = 0; itemIndex < treeWidget->topLevelItemCount(); ++itemIndex) {
        filterTreeItem(treeWidget->topLevelItem(itemIndex), filterText.trimmed());
    }
}

void ObjectTreeWidget::updateBreadcrumbLabel()
{
    const QString fullPath = fullBreadcrumbPath(currentObjectPointer);
    breadcrumbValueLabel->setToolTip(fullPath);

    if (fullPath.isEmpty()) {
        breadcrumbValueLabel->clear();
        return;
    }

    QString displayPath = fullPath;
    const int availableWidth = std::max(120, breadcrumbValueLabel->width() - 8);
    const QString separator = QStringLiteral(" > ");
    const QStringList pathSegments = fullPath.split(separator);
    const QFontMetrics fontMetrics(breadcrumbValueLabel->font());

    if (fontMetrics.horizontalAdvance(displayPath) > availableWidth && !pathSegments.isEmpty()) {
        QStringList visibleSegments;
        for (int segmentIndex = pathSegments.size() - 1; segmentIndex >= 0; --segmentIndex) {
            visibleSegments.prepend(pathSegments.at(segmentIndex));
            const QString candidateText = QStringLiteral("... > ") + visibleSegments.join(separator);
            if (fontMetrics.horizontalAdvance(candidateText) > availableWidth) {
                visibleSegments.removeFirst();
                break;
            }
        }

        displayPath = visibleSegments.size() == pathSegments.size()
            ? fullPath
            : QStringLiteral("... > ") + visibleSegments.join(separator);
        displayPath = fontMetrics.elidedText(displayPath, Qt::ElideRight, availableWidth);
    }

    breadcrumbValueLabel->setText(displayPath);
}

bool ObjectTreeWidget::filterTreeItem(QTreeWidgetItem *treeItem, const QString &filterText)
{
    if (treeItem == nullptr) {
        return false;
    }

    bool hasVisibleChild = false;
    for (int childIndex = 0; childIndex < treeItem->childCount(); ++childIndex) {
        hasVisibleChild = filterTreeItem(treeItem->child(childIndex), filterText) || hasVisibleChild;
    }

    const bool hasFilter = !filterText.isEmpty();
    const bool itemMatches = !hasFilter
        || treeItem->text(0).contains(filterText, Qt::CaseInsensitive)
        || treeItem->text(1).contains(filterText, Qt::CaseInsensitive);
    const bool shouldShow = itemMatches || hasVisibleChild || !hasFilter;
    treeItem->setHidden(!shouldShow);
    if (hasFilter && hasVisibleChild) {
        treeItem->setExpanded(true);
    }

    return shouldShow;
}

QTreeWidgetItem *ObjectTreeWidget::findTreeItem(QObject *targetObject, QTreeWidgetItem *parentItem) const
{
    if (targetObject == nullptr) {
        return nullptr;
    }

    const int childCount = parentItem != nullptr ? parentItem->childCount()
                                                 : treeWidget->topLevelItemCount();
    for (int itemIndex = 0; itemIndex < childCount; ++itemIndex) {
        QTreeWidgetItem *candidateItem =
            parentItem != nullptr ? parentItem->child(itemIndex) : treeWidget->topLevelItem(itemIndex);
        const quintptr objectAddress = candidateItem->data(0, Qt::UserRole).value<quintptr>();
        if (reinterpret_cast<QObject *>(objectAddress) == targetObject) {
            return candidateItem;
        }

        if (QTreeWidgetItem *childMatch = findTreeItem(targetObject, candidateItem)) {
            return childMatch;
        }
    }

    return nullptr;
}

void ObjectTreeWidget::populateTree(QObject *targetObject, QTreeWidgetItem *parentItem)
{
    if (targetObject == nullptr) {
        return;
    }

    auto *treeItem =
        new QTreeWidgetItem({objectTreeLabel(targetObject), objectTreeTypeName(targetObject)});
    treeItem->setData(0, Qt::UserRole, QVariant::fromValue(reinterpret_cast<quintptr>(targetObject)));
    treeItem->setToolTip(0, QStringLiteral("%1 (%2)")
                                .arg(objectTreeLabel(targetObject), objectTreeTypeName(targetObject)));

    if (parentItem != nullptr) {
        parentItem->addChild(treeItem);
    } else {
        treeWidget->addTopLevelItem(treeItem);
    }

    const QObjectList childObjects = targetObject->children();
    for (QObject *childObject : childObjects) {
        populateTree(childObject, treeItem);
    }
}

void ObjectTreeWidget::selectCurrentObject()
{
    const QSignalBlocker signalBlocker(treeWidget);
    treeWidget->setCurrentItem(nullptr);

    QTreeWidgetItem *targetItem = findTreeItem(currentObjectPointer);
    if (targetItem == nullptr || targetItem->isHidden()) {
        return;
    }

    for (QTreeWidgetItem *parentItem = targetItem->parent(); parentItem != nullptr; parentItem = parentItem->parent()) {
        parentItem->setExpanded(true);
    }

    treeWidget->setCurrentItem(targetItem);
    treeWidget->scrollToItem(targetItem);
}
