// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// QLabel と QPushButton を QVBoxLayout で並べる Widget を実装してください。
// go() 関数を用意し、main() は書かないでください。

#include "widget.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

Widget::Widget(QWidget *parent) : QWidget(parent) {
    setWindowTitle("02 Header + Source");

    auto *label = new QLabel("A label and a button", this);
    label->setAlignment(Qt::AlignCenter);

    auto *button = new QPushButton("Button", this);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(label);
    layout->addWidget(button, 0, Qt::AlignCenter);

    resize(280, 140);
}

Widget *go() {
    return new Widget;
}
