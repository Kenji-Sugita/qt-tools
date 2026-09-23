#include "widget.h"

#include "ui_form.h"

#include <QFile>
#include <QLabel>
#include <QTextStream>

#include "qrc_resources.cpp"

QString resourceText() {
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
        label->setText(resourceText());
    }
    return widget;
}

#include "moc_widget.cpp"

