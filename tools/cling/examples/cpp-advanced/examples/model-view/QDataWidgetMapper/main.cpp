#include "BookStore.h"
#include <QApplication>
#include <QStandardItemModel>
#include <QTableView>
#include <QHeaderView>
#include <QDebug>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    struct Data {
        const char* const title;
        const char* const author;
        const double price;
    };

    const Data data[] {
        { "Practical Qt", "Kalle Mathias Dalheimer",  45.00 },
        { "Programming with Qt", "Kalle Mathias Dalheimer",  39.95 },
        { "C++ GUI Programming with Qt 4", "Jasmin Blanchette", 59.95 },
        { "The Art of Building Qt Applications", "Daniel Molkentin",  54.95 },
        { "An Introduction to Design Patterns in C++ with Qt 4", "Alan Ezust", 54.95 },
    };

    const int rows = sizeof(data)/sizeof(Data);
    const int columns = 3;
    QStandardItemModel model(rows, columns);
    for (int row = 0; row < rows; ++row) {
        model.setData(model.index(row, 0), data[row].title);
        model.setData(model.index(row, 1), data[row].author);
        model.setData(model.index(row, 2), data[row].price);
    }

    QTableView tableView;
    tableView.setModel(&model);
    tableView.setWindowTitle("Model");
    tableView.resizeColumnsToContents();
    tableView.resize(600, 200);
    tableView.show();

    BookStore bookStore(&model);
    bookStore.show();

    return app.exec();
}
