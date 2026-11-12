#include "widget.h"

#include <QPalette>
#include <QVBoxLayout>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    colorFrame = new QFrame(this);
    colorFrame->setMinimumHeight(80);
    colorFrame->setFrameShape(QFrame::StyledPanel);
    colorFrame->setAutoFillBackground(true);

    valueLabel = new QLabel(this);

    redSlider = new QSlider(Qt::Horizontal, this);
    redSlider->setRange(0, 255);
    redSlider->setValue(128);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(colorFrame);
    layout->addWidget(redSlider);
    layout->addWidget(valueLabel);

    connect(redSlider, &QSlider::valueChanged, this, &Widget::updateColor);
    updateColor();
}

void Widget::updateColor()
{
    const int red = redSlider->value();
    QPalette palette = colorFrame->palette();
    palette.setColor(QPalette::Window, QColor(red, 80, 120));
    colorFrame->setPalette(palette);
    valueLabel->setText(QStringLiteral("red = %1").arg(red));
}
