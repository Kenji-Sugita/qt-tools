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

