#include <QApplication>
#include <QStandardItemModel>
#include <QTableView>
#include <QTreeView>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QStandardItemModel model(10, 10);
    for (int topRow = 0; topRow < 5; ++topRow) {
        for (int topColumn = 0; topColumn < 10; ++topColumn) {
            model.setData(model.index(topRow, topColumn), QString("(%1, %2)").arg(topRow).arg(topColumn));
        }
        const QModelIndex index = model.index(topRow, 0);

        model.insertRows(0, 10, index);
        model.insertColumns(0, 10, index);

        for (int row = 0; row < 10; ++row) {
            for (int col = 0; col < 10; ++col) {
                model.setData(model.index(row, col, index), row * col);
            }
        }
    }

    QTableView table;
    table.setModel(&model);
    table.show();

    QTreeView tree;
    tree.setModel(&model);
    tree.show();

    return app.exec();
}
