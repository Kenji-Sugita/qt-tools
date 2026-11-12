// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// QPushButton をクリックすると QLabel の Count が増える Widget を実装してください。
// lambda connect と go() 関数を用意し、main() は書かないでください。

#include "widget.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

Widget::Widget(QWidget *parent) : QWidget(parent) {
    setWindowTitle("03 Lambda Connect");
    label = new QLabel("Count: 0", this);
    label->setAlignment(Qt::AlignCenter);

    auto *button = new QPushButton("Count up", this);

    QObject::connect(button, &QPushButton::clicked, this, [this]() {
        ++count;
        label->setText(QString("Count: %1").arg(count));
    });

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(label);
    layout->addWidget(button, 0, Qt::AlignCenter);

    resize(280, 140);
}

Widget *go() {
    return new Widget;
}
