#include <QApplication>
#include <QAbstractListModel>
#include <QListView>
#include <QTableView>
#include <QTreeView>
#include <QDebug>

class StringListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit StringListModel(const QStringList& list, QObject* parent = nullptr)
        : QAbstractListModel(parent), list(list) {}

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        Q_ASSERT(!parent.isValid());

        return list.count();
    }

    QVariant data(const QModelIndex& index, int role) const override {
        Q_ASSERT(index.isValid());
        Q_ASSERT(hasIndex(index.row(), index.column(), QModelIndex()));

        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return list.at(index.row());
        }
        return QVariant();
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (role != Qt::DisplayRole) {
            return QAbstractListModel::headerData(section, orientation, role);
        }

        if (orientation == Qt::Horizontal) {
            Q_ASSERT(section == 0);
            return QString("Counry");
        } else {
            return QString("Country %1").arg(section + 1);
        }
    }

    Qt::ItemFlags flags(const QModelIndex& index) const override {
        Q_ASSERT(index.isValid());

        return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
    }

    bool setData(const QModelIndex& index, const QVariant& value, int role) override {
        Q_ASSERT(index.isValid());

        if (role == Qt::EditRole) {
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

private:
    QStringList list;
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
    list.setModel(&model);
    list.setWindowTitle("QListView");
    list.show();

    QTableView table;
    table.setModel(&model);
    table.setWindowTitle("QTableView");

    QTreeView tree;
    tree.setModel(&model);
    tree.setWindowTitle("QTreeView");
    tree.show();

    table.show();

    return app.exec();
}

#include "main.moc"
