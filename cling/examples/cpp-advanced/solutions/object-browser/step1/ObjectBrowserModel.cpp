#include "ObjectBrowserModel.h"
#include <QApplication>
#include <QTextStream>
#include <QWidget>

ObjectBrowserModel::ObjectBrowserModel(QWidget* widget, QObject* parent)
    : QAbstractTableModel(parent), widget(widget)
{
}

QList<QObject*> ObjectBrowserModel::children(QObject* parent) const
{
    // TODO: Return the list of all the children of parent
    const QList<QObject*> result = parent->children();
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
    // TODO: implement
    // Hint: Return the number of columns
    Q_ASSERT(!parent.isValid());
    return 3;
}

int ObjectBrowserModel::rowCount(const QModelIndex& parent) const
{
    // TODO: implement
    // Hint: Your children method above might be utterly useful now
    Q_ASSERT(!parent.isValid());
    return children(widget).count();
}

QVariant ObjectBrowserModel::data(const QModelIndex& index, int role) const
{
    // TODO: implement
    // Hint: Your label method might come in handy now.
    Q_ASSERT(index.isValid());
    Q_ASSERT(hasIndex(index.row(), index.column()));
    if (role != Qt::DisplayRole ) {
        return QVariant();
    }

    QObject* const object = children(widget).at(index.row());
    return label(object, index.column());
}

QVariant ObjectBrowserModel::headerData(const int section, const Qt::Orientation orientation, const int role) const
{
    // TODO: implement
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    switch (section) {
    case 0: return "Class Name";
    case 1: return "Object Name";
    case 2: return "Address";
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}
