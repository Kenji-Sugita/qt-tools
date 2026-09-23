// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// QTableWidget を作成し、数行のデータを表示してください。
// showTable() 関数を用意し、main() は書かないでください。

#include <QTableWidget>
#include <QTableWidgetItem>

QTableWidget* showTable()
{
    auto* table = new QTableWidget(3, 2);
    table->setAttribute(Qt::WA_DeleteOnClose);
    table->setWindowTitle("icpp table sample");
    table->setHorizontalHeaderLabels(QStringList{"Name", "Score"});

    const QList<QPair<QString, QString>> rows{
        {"Ada", "98"},
        {"Grace", "95"},
        {"Linus", "91"},
    };

    for (int row = 0; row < rows.size(); ++row) {
        table->setItem(row, 0, new QTableWidgetItem(rows.at(row).first));
        table->setItem(row, 1, new QTableWidgetItem(rows.at(row).second));
    }

    table->resize(360, 180);
    table->show();
    table->raise();
    return table;
}
