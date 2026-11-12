#include <QApplication>
#include <QAbstractListModel>
#include <QListView>
#include <QTableView>
#include <QTreeView>
#include <QMimeData>
#include <QDebug>

class StringListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit StringListModel(const QStringList& list, QObject* parent = nullptr)
        : QAbstractListModel(parent), list(list) {
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        Q_ASSERT(!parent.isValid());
        return list.count();
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!index.isValid()) {
            return QVariant();
        }

        if (!hasIndex(index.row(), index.column())) {
            return QVariant();
        }

        if (role == Qt::DisplayRole) {
            return list.at(index.row());
        }

        return QVariant();
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (role != Qt::DisplayRole) {
            return QVariant();
        }

        if (orientation == Qt::Horizontal) {
            Q_ASSERT(section == 0);
            return QString("Country");
        } else {
            return QString("Country %1").arg(section);
        }
    }

    Qt::ItemFlags flags(const QModelIndex &index) const override {
        if (!index.isValid()) {
            return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
        }

        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override {
        if (index.isValid() && role == Qt::EditRole) {
            list.replace(index.row(), value.toString());
            emit dataChanged(index, index);
            return true;
        }

        return false;
    }

    bool insertRows(int position, int rows, const QModelIndex& parent) override {
        Q_ASSERT(!parent.isValid());

        beginInsertRows(QModelIndex(), position, position + rows - 1);
        for (int row = 0; row < rows; ++row) {
            list.insert(position, "");
        }
        endInsertRows();

        return true;
    }

    bool removeRows(int position, int rows, const QModelIndex& parent) override {
        Q_ASSERT(!parent.isValid());

        beginRemoveRows(QModelIndex(), position, position + rows - 1);
        for (int row = 0; row < rows; ++row) {
            list.removeAt(position);
        }
        endRemoveRows();

        return true;
    }

    QStringList mimeTypes() const override {
        return { countryListMimeTypeName };
    }

    QMimeData* mimeData(const QModelIndexList& indexes) const override {
        QStringList list;
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
        for (const QModelIndex& index : std::as_const(indexes)) {
#else
        for (const QModelIndex& index : qAsConst(indexes)) {
#endif
            list << data(index).toString();
        }

        const auto mimeData = new QMimeData();
        mimeData->setData(countryListMimeTypeName, list.join(",").toUtf8());  // To comma-separated country names
        return mimeData;
    }

    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override {
        if  (canDropMimeData(data, action, row, column, parent)) {
            const QStringList items = QString(data->data(countryListMimeTypeName)).split(",");  // From comma-separated country names

            if (parent.isValid() && items.count() == 1) {  // Drop just one item on an item
                setData(parent, items.at(0));
            } else {
                int insertAtRow;
                if (parent.isValid()) {                    // Drop multiple items on an item
                    insertAtRow = parent.row();
                } else {
                    if (row < 0) {                         // Drop on parent
                        insertAtRow = rowCount();
                    } else {                               // Drop between items, above the first item, or below the last item
                        insertAtRow = row;
                    }
                }
                insertRows(insertAtRow, items.count(), QModelIndex());
                for (int i = 0; i < items.count(); ++i) {
                    list.replace(insertAtRow + i, items.at(i));
                }
                const QModelIndex startIndex = index(insertAtRow, 0);
                const QModelIndex endIndex = index(insertAtRow + items.count() - 1, 0);
                emit dataChanged(startIndex, endIndex);
            }

            return true;
        }

        return false;
    }

private:
    QStringList list;
    const QString countryListMimeTypeName = "x-text/x-plain";
};

QStringList availableCountryNames()
{
    static QStringList countries;

    if (countries.isEmpty()) {
        for (int countryIndex = QLocale::AnyCountry + 1; countryIndex <= QLocale::LastCountry; ++countryIndex) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
            countries << QLocale::territoryToString(static_cast<QLocale::Country>(countryIndex));
#else
            countries << QLocale::countryToString(static_cast<QLocale::Country>(countryIndex));
#endif
        }
        countries.sort();
    }

    return countries;
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    StringListModel model(availableCountryNames());

    QListView list;
    list.setSelectionMode(QAbstractItemView::MultiSelection);
    list.setModel(&model);
    list.setWindowTitle("QListView");
    list.setDragEnabled(true);
    list.setAcceptDrops(true);
    list.show();

    QTableView table;
    table.setSelectionMode(QAbstractItemView::MultiSelection);
    table.setModel(&model);
    table.setDragEnabled(true);
    table.setAcceptDrops(true);
    table.setWindowTitle("QTableView");
    table.show();

    QTreeView tree;
    tree.setSelectionMode(QAbstractItemView::MultiSelection);
    tree.setModel(&model);
    tree.setWindowTitle("QTreeView");
    tree.setDragEnabled(true);
    tree.setAcceptDrops(true);
    tree.show();

    return app.exec();
}

#include "main.moc"
