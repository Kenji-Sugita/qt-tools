// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// ui_form.h で form.ui を setupUi し、ラベル/ボタン/スライダーを操作してください。
// go() 関数を用意し、main() は書かないでください。

#include "widget.h"

#include <QLabel>
#include <QPushButton>
#include <QSlider>

#include "ui_form.h"

Widget::Widget(QWidget *parent) : QWidget(parent) {
    Ui::Form ui;
    ui.setupUi(this);

    label = findChild<QLabel *>("label");
    button = findChild<QPushButton *>("button");
    slider = findChild<QSlider *>("slider");

    slider->setRange(0, 100);
    slider->setValue(25);

    QObject::connect(button, &QPushButton::clicked, this, [this]() {
        label->setText("Button clicked");
    });
    QObject::connect(slider, &QSlider::valueChanged, this, [this](int value) {
        label->setText(QString("Slider: %1").arg(value));
    });

    resize(320, 160);
}

Widget *go() {
    return new Widget;
}
