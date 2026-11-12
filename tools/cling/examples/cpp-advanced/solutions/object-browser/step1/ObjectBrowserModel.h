#ifndef OBJECTBROWSERMODEL_H
#define OBJECTBROWSERMODEL_H

#include <QAbstractTableModel>

class ObjectBrowserModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ObjectBrowserModel(QWidget* widget, QObject* parent = nullptr);
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

protected:
    QList<QObject*> children(QObject* parent) const;
    QString label(const QObject* widget, int column) const;

private:
    QWidget* widget;
};

#endif

