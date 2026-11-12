#include "widget.h"

#include <QFile>
#include <QLabel>
#include <QTextStream>
#include <QVBoxLayout>

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
    auto *label = new QLabel(resourceText(), widget);
    auto *layout = new QVBoxLayout(widget);
    layout->addWidget(label);
    return widget;
}

#include "moc_widget.cpp"

