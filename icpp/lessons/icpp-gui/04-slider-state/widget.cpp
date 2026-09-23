// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// QSlider の値が変わると QLabel に現在値を表示する Widget を実装してください。
// updateLabel() と go() 関数を用意し、main() は書かないでください。

#include "widget.h"

#include <QVBoxLayout>

Widget::Widget(QWidget *parent) : QWidget(parent) {
    setWindowTitle("04 Slider State");

    label = new QLabel(this);
    label->setAlignment(Qt::AlignCenter);

    slider = new QSlider(Qt::Horizontal, this);
    slider->setRange(0, 100);
    slider->setValue(50);

    QObject::connect(slider, &QSlider::valueChanged, this, [this](int value) {
        updateLabel(value);
    });

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(label);
    layout->addWidget(slider);

    updateLabel(slider->value());
    resize(320, 130);
}

void Widget::updateLabel(int value) {
    label->setText(QString("Value: %1").arg(value));
}

Widget *go() {
    return new Widget;
}
