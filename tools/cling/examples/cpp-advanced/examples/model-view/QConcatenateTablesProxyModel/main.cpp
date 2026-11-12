#include <QApplication>
#include <QSplitter>
#include <QListView>
#include <QTableView>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QConcatenateTablesProxyModel>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // List
    QStringList stringList;
    for (int row = 0; row < 10; ++row) {
        stringList << QString("Item %0").arg(row);
    }
    QStringListModel listModel;
    listModel.setStringList(stringList);

    stringList.clear();
    for (int row = 0; row < 10; ++row) {
        stringList << QString("Extra %0").arg(row);
    }
    QStringListModel extraListModel;
    extraListModel.setStringList(stringList);

    QConcatenateTablesProxyModel allListModels;
    allListModels.addSourceModel(&listModel);
    allListModels.addSourceModel(&extraListModel);

    const auto listView = new QListView;
    listView->setModel(&allListModels);

    // Table
    QStandardItemModel tableModel;
    for (int row = 0; row < 10; ++row) {
        tableModel.insertRow(row, { new QStandardItem(QString("Item %0, 0").arg(row)),
                                    new QStandardItem(QString("Item %0, 1").arg(row)) });
        tableModel.setHeaderData(row, Qt::Vertical, QString("%1").arg(row));
    }
    tableModel.setHeaderData(0, Qt::Horizontal, "A");
    tableModel.setHeaderData(1, Qt::Horizontal, "B");

    QStandardItemModel extraTableModel;
    for (int row = 0; row < 10; ++row) {
        extraTableModel.insertRow(row, { new QStandardItem(QString("Extra %0, 0").arg(row)),
                                         new QStandardItem(QString("Extra %0, 1").arg(row)),
                                         new QStandardItem(QString("Extra %0, 2").arg(row)) });
        extraTableModel.setHeaderData(row, Qt::Vertical, QString("Extra %1").arg(row));
    }
    extraTableModel.setHeaderData(0, Qt::Horizontal, "Extra A");
    extraTableModel.setHeaderData(1, Qt::Horizontal, "Extra B");
    extraTableModel.setHeaderData(2, Qt::Horizontal, "Extra C");

    QConcatenateTablesProxyModel allTableModels;
    allTableModels.addSourceModel(&tableModel);
    allTableModels.addSourceModel(&extraTableModel);

    const auto tableView = new QTableView;
    tableView->setModel(&allTableModels);

    // All models
    QConcatenateTablesProxyModel allModels;
    allModels.addSourceModel(&listModel);
    allModels.addSourceModel(&extraListModel);
    allModels.addSourceModel(&tableModel);
    allModels.addSourceModel(&extraTableModel);

    const auto tableViewForAll = new QTableView;
    tableViewForAll->setModel(&allModels);

    // The list and the table.
    QSplitter splitter;
    splitter.addWidget(listView);
    splitter.addWidget(tableView);
    splitter.addWidget(tableViewForAll);
    splitter.show();

    return app.exec();
}
