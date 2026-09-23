#include <QLCDNumber>
#include <QSlider>
#include <QLayout>

#include "lcdslider.h"

LcdSlider::LcdSlider(QWidget* parent)
    : QWidget{parent}
{
    const auto topLayout = new QVBoxLayout{this};

    const auto lcdNumber = new QLCDNumber{};
    topLayout->addWidget(lcdNumber);

    const auto slider = new QSlider{};
    slider->setOrientation(Qt::Horizontal);
    topLayout->addWidget(slider);

    connect(slider, &QSlider::valueChanged, lcdNumber, qOverload<int>(&QLCDNumber::display));
}
