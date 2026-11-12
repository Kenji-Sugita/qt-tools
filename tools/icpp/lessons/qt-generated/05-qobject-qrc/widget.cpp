// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// qrc_resources.cpp を include し、Q_OBJECT 付き Widget に resource の文字列を表示してください。
// go() 関数と moc_widget.cpp の include を用意し、main() は書かないでください。

#include "widget.h"

#include <QFile>
#include <QLabel>
#include <QTextStream>
#include <QVBoxLayout>

#include "qrc_resources.cpp"

QString loadMessage() {
    QFile file(":/data/message.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "open failed";
    }
    QTextStream stream(&file);
    return stream.readAll().trimmed();
}

Widget *go() {
    auto *widget = new Widget;
    widget->setWindowTitle("05 Q_OBJECT + .qrc");
    auto *label = new QLabel(loadMessage(), widget);
    auto *layout = new QVBoxLayout(widget);
    layout->addWidget(label);
    widget->resize(260, 100);
    return widget;
}

#include "moc_widget.cpp"
