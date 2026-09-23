// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// ui_form.h と qrc_resources.cpp を使い、resource の文字列を form の label に表示してください。
// go() 関数と moc_widget.cpp の include を用意し、main() は書かないでください。

#include "widget.h"

#include "ui_form.h"

#include <QFile>
#include <QLabel>
#include <QTextStream>

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
    Ui::Form ui;
    ui.setupUi(widget);
    if (auto *label = widget->findChild<QLabel *>("label")) {
        label->setText(loadMessage());
    }
    return widget;
}

#include "moc_widget.cpp"
