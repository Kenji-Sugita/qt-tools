#include <QApplication>
#include <QTableView>
#include <QTableWidgetItem>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QTableWidget table(100, 5);
    for (int row = 0; row < table.rowCount(); ++row) {
        for (int column = 0; column < table.columnCount(); ++column) {
            table.setItem(row, column, new QTableWidgetItem(QString::number(row * column)));
        }
    }

    table.show();

    return app.exec();
}
