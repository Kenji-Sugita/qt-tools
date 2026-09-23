#include "SortedTableView.h"
#include <QApplication>
#include <QStandardItemModel>

static const struct ModelData {
    const QString name;
    const QString nickname;
    const int age;
} Dudes[] {
    { "M. K. Dalheimer", "Pointy Haired Boss", 33 },
    { "Jesper K. Pedersen",  "Blackie", 23 },
    { "Mirko Boehm", "Miroslav", 26 },
    { "David Faure",  "dfaure", 9 },
};

static const int NoOfDudes = sizeof(Dudes)/sizeof(Dudes[0]);

int main(int argc,  char** argv)
{
    QApplication app(argc, argv);

    QStandardItemModel model;
    const int rows = NoOfDudes;
    const int columns = 3;
    model.insertRows(0, rows, QModelIndex());
    model.insertColumns(0, columns, QModelIndex());
    for (int row = 0; row < rows; ++ row) {
        model.setData(model.index(row, 0, QModelIndex()), Dudes[row].name);
        model.setData(model.index(row, 1, QModelIndex()), Dudes[row].nickname);
        model.setData(model.index(row, 2, QModelIndex()), Dudes[row].age);
    }
    model.setHeaderData(0, Qt::Horizontal, "Name");
    model.setHeaderData(1, Qt::Horizontal, "Nickname");
    model.setHeaderData(2, Qt::Horizontal, "Age");

    SortedTableView table;
    table.setModel(&model);
    table.resize(500, 300);
    table.show();

    return app.exec();
}
