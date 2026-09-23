#include "ObjectBrowserModel.h"
#include <QApplication>
#include <QWidget>

ObjectBrowserModel::ObjectBrowserModel(QWidget* widget, QObject* parent)
    : QAbstractTableModel(parent), widget(widget)
{
}

QList<QObject*> ObjectBrowserModel::children(QObject* parent) const
{
    // TODO: Return the list of all the children of parent
    return QList<QObject*>();
}

QString ObjectBrowserModel::label(const QObject* object, const int column) const
{
    // Return the content for a given column
    switch (column) {
    case 0: // TODO: return class name
    case 1: // TODO: return object name
    case 2: // TODO: return pointer address
        return "Hello World";
    }
    return QString();
}

int ObjectBrowserModel::columnCount(const QModelIndex& parent) const
{
    // TODO: implement
    // Hint: Return the number of columns
    Q_ASSERT(!parent.isValid());
    return 0;
}

int ObjectBrowserModel::rowCount(const QModelIndex& parent) const
{
    // TODO: implement
    // Hint: Your children method above might be utterly useful now
    Q_ASSERT(!parent.isValid());
    return 0;
}

QVariant ObjectBrowserModel::data(const QModelIndex& index, int role) const
{
    // TODO: implement
    // Hint: Your label method might come in handy now.
    Q_ASSERT(index.isValid());
    Q_ASSERT(hasIndex(index.row(), index.column()));
    return QVariant();
}

QVariant ObjectBrowserModel::headerData(const int section, const Qt::Orientation orientation, const int role) const
{
    // TODO: implement
    return QAbstractTableModel::headerData(section, orientation, role);
}
