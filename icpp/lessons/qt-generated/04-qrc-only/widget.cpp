// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// qrc_resources.cpp を include し、:/data/message.txt を QLabel に表示してください。
// loadMessage() と go() 関数を用意し、main() は書かないでください。

#include <QFile>
#include <QLabel>
#include <QString>
#include <QTextStream>
#include <QVBoxLayout>
#include <QWidget>

#include "qrc_resources.cpp"

QString loadMessage() {
    QFile file(":/data/message.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "open failed";
    }
    QTextStream stream(&file);
    return stream.readAll().trimmed();
}

QWidget *go() {
    auto *widget = new QWidget;
    widget->setWindowTitle("04 .qrc Only");
    auto *label = new QLabel(loadMessage(), widget);
    auto *layout = new QVBoxLayout(widget);
    layout->addWidget(label);
    widget->resize(240, 100);
    return widget;
}
