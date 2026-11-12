#ifndef MODELFLIPPROXY_H
#define MODELFLIPPROXY_H

#include <QAbstractProxyModel>

class ModelFlipProxy : public QAbstractProxyModel
{
    Q_OBJECT

public:
    explicit ModelFlipProxy(QObject* parent = nullptr);
    ~ModelFlipProxy();

    QModelIndex mapFromSource(const QModelIndex& sourceIndex) const override;
    QModelIndex mapToSource(const QModelIndex& proxyIndex) const override;

    QModelIndex index(int, int, const QModelIndex&) const override;
    QModelIndex parent(const QModelIndex&) const override;
    int rowCount(const QModelIndex&) const override;
    int columnCount(const QModelIndex&) const override;
    QVariant data(const QModelIndex&, int) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
};
#endif
