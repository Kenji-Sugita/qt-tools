#include "ObjectBrowserModel.h"
#include <QApplication>
#include <QWidget>
#include <QTextStream>

ObjectBrowserModel::ObjectBrowserModel(QWidget* widget, QObject* parent)
    : QAbstractItemModel(parent), widget(widget)
{
}

QList<QObject*> ObjectBrowserModel::children(QObject* parent) const
{
    // TODO: Return the list of all the children of parent
    QList<QObject*> result;
    if (parent == 0) {
        result = widget->children();
    } else {
        result = parent->children();
    }
    return result;
}

QString ObjectBrowserModel::label(const QObject* object, const int column) const
{
    // Return the content for a given column
    switch (column) {
    case 0: return object->metaObject()->className();
    case 1: return object->objectName();
    case 2: {
        QString address;
        QTextStream(&address) << object;
        return address;
        }
    }

    return QString();
}

int ObjectBrowserModel::columnCount(const QModelIndex& parent) const
{
    // Return the number of columns
    return 3;
}

int ObjectBrowserModel::rowCount(const QModelIndex& parent) const
{
    // TODO: implement
    // Hint: Your children method above might be utterly useful now
    return children(qobjectFromModelIndex(parent)).count();
}

QVariant ObjectBrowserModel::data(const QModelIndex& index, int role) const
{
    Q_ASSERT(index.isValid());
    Q_ASSERT(hasIndex(index.row(), index.column(), index.parent()));
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    QObject* const object = qobjectFromModelIndex(index);
    return label(object, index.column());
}

QVariant ObjectBrowserModel::headerData(const int section, const Qt::Orientation orientation, const int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QAbstractItemModel::headerData(section, orientation, role);
    }

    switch (section) {
    case 0: return "Class Name";
    case 1: return "Object Name";
    case 2: return "Address";
    }

    return QVariant();
}

QModelIndex ObjectBrowserModel::index(int row, int column, const QModelIndex& parent) const
{
    // TODO: Implement. Hint: use QAbstractItemModel::createIndex.
    return createIndex(row, column, children(qobjectFromModelIndex(parent)).at(row));
}

QModelIndex ObjectBrowserModel::parent(const QModelIndex& index) const
{
    QObject* const object = qobjectFromModelIndex(index);
    QObject* const parent = object->parent();

    if (parent == 0) {
        return QModelIndex();
    }

    QObject* const grandParent = parent->parent();
    const int row = children(grandParent).indexOf(parent);
    return createIndex(row, 0, parent);
}

/*
 * The internal pointer of the model index points to the object that the model index is an index for.
 */
QObject* ObjectBrowserModel::qobjectFromModelIndex(const QModelIndex& index) const
{
    // TODO: Map from model index to the associated widget.
    // You did start out by writing down exactly what goes into the internal-pointer, right?
    // If not, please do read the object assignment yet another time!
    if (index.isValid()) {
        return reinterpret_cast<QObject*>(index.internalPointer());
    }

    return widget; // This is the root.
}
