// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// ui_form.h で作った button の clicked を Widget::updateTitle slot に接続してください。
// go() 関数と moc_widget.cpp の include を用意し、main() は書かないでください。

#include "widget.h"

#include "ui_form.h"

#include <QPushButton>

Widget *go() {
    auto *widget = new Widget;
    Ui::Form ui;
    ui.setupUi(widget);
    QObject::connect(widget->findChild<QPushButton *>("button"),
                     &QPushButton::clicked,
                     widget,
                     &Widget::updateTitle);
    return widget;
}

#include "moc_widget.cpp"

