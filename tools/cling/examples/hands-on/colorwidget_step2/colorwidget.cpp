#include <QPalette>

#include "colorwidget.h"

ColorWidget::ColorWidget(QWidget* parent)
    : QWidget{parent}
{
    setupUi(this);

    colorFrame->setAutoFillBackground(true);
    updateColor();

    connect(redSlider, &QSlider::valueChanged, this, &ColorWidget::updateColor);
    connect(greenSlider, &QSlider::valueChanged, this, &ColorWidget::updateColor);
    connect(blueSlider, &QSlider::valueChanged, this, &ColorWidget::updateColor);
}

void ColorWidget::updateColor()
{
    const QColor color{redSlider->value(), greenSlider->value(), blueSlider->value()};
    QPalette palette{colorFrame->palette()};
    palette.setColor(QPalette::Window, color);
    colorFrame->setPalette(palette);
}
