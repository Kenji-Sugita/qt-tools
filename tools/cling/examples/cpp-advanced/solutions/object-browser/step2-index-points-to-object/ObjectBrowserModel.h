#ifndef OBJECTBROWSERMODEL_H
#define OBJECTBROWSERMODEL_H

#include <QAbstractTableModel>

class ObjectBrowserModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit ObjectBrowserModel(QWidget* widget, QObject* parent = nullptr);
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex& index) const;

protected:
    QList<QObject*> children(QObject* parent) const;
    QString label(const QObject* widget, int column) const;
    QObject* qobjectFromModelIndex(const QModelIndex& index) const;

private:
    QWidget* widget;
};
#endif
