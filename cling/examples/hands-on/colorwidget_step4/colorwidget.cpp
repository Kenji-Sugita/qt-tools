#include <QPalette>
#include <QSignalBlocker>

#include "colorwidget.h"

ColorWidget::ColorWidget(QWidget* parent)
    : QWidget{parent}
{
    setupUi(this);

    setContentsMargins(0, 0, 0, 0);

    colorFrame->setAutoFillBackground(true);

    connect(redSlider, &QSlider::valueChanged, this, &ColorWidget::updateColor);
    connect(greenSlider, &QSlider::valueChanged, this, &ColorWidget::updateColor);
    connect(blueSlider, &QSlider::valueChanged, this, &ColorWidget::updateColor);

    setColor(QColor{redSlider->value(), greenSlider->value(), blueSlider->value()});
}

void ColorWidget::setColor(const QColor& color)
{
    if (color == m_color) {
        return;
    }

    applyToFrame(color);
    syncSlidersFromColor(color);

    m_color = color;
    emit colorChanged(m_color);
}

void ColorWidget::updateColor()
{
    const QColor color{redSlider->value(), greenSlider->value(), blueSlider->value()};
    setColor(color);
}

void ColorWidget::applyToFrame(const QColor& color)
{
    QPalette palette{colorFrame->palette()};
    palette.setColor(QPalette::Window, color);
    colorFrame->setPalette(palette);
}

void ColorWidget::syncSlidersFromColor(const QColor& color)
{
    QSignalBlocker redBlocker(redSlider);
    QSignalBlocker greenBlocker(greenSlider);
    QSignalBlocker blueBlocker(blueSlider);

    redSlider->setValue(color.red());
    greenSlider->setValue(color.green());
    blueSlider->setValue(color.blue());
}
