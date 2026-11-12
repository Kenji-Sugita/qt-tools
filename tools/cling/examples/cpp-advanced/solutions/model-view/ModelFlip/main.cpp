#include <QApplication>
#include <QStandardItemModel>
#include <QSplitter>
#include <QTableView>

#include "ModelFlipProxy.h"
#include <qsplitter.h>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // Set up the model
    const int NumberOfRows = 10;
    const int NumberOfColumns = 5;
    QStandardItemModel model;
    model.insertRows(0, NumberOfRows, QModelIndex());
    model.insertColumns(0, NumberOfColumns, QModelIndex());
    for (int row = 0; row < NumberOfRows; ++ row)
        for (int column = 0; column < NumberOfColumns; ++ column) {
            QModelIndex index = model.index(row, column, QModelIndex());
            model.setData(index, 10 * row + column);
        }

    // Set up the proxy
    ModelFlipProxy proxy;
    proxy.setSourceModel(&model);

    // Views
    QSplitter splitter;
    const auto left  = new QTableView(&splitter);
    const auto right = new QTableView(&splitter);

    // Connect view and model
    left->setModel(&model);
    right->setModel(&proxy);

    splitter.show();

    return app.exec();
}
