#include "slider.h"

#include <QSlider>
#include <QLabel>
#include <QBoxLayout>

Slider::Slider(Qt::Orientation orientation, QWidget* parent)
    : QWidget(parent)
{
    slider = new QSlider(orientation);
    display = new QLabel;
    display->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    connect(slider, &QSlider::valueChanged, display, QOverload<int>::of(&QLabel::setNum));
    connect(slider, &QSlider::valueChanged, this, &Slider::valueChanged);

    QBoxLayout* topLayout = nullptr;
    if (orientation == Qt::Horizontal) {
        topLayout = new QHBoxLayout(this);
    } else {
        topLayout = new QVBoxLayout(this);
    }

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    topLayout->setContentsMargins(0, 0, 0, 0);

    topLayout->addWidget(slider);
    topLayout->addWidget(display);
}

void Slider::setValue(int value)
{
    slider->setValue(value);
}

void Slider::setRange(int min, int max)
{
    slider->setRange(min, max);
    display->setFixedWidth(maxWidth(minimum(), maximum()));
}

void Slider::setMinimum(int min)
{
    setRange(min, maximum());
}

void Slider::setMaximum(int max)
{
    setRange(minimum(), max);
}

int Slider::value() const
{
    return slider->value();
}

int Slider::minimum() const
{
    return slider->minimum();
}

int Slider::maximum() const
{
    return slider->maximum();
}

int Slider::maxWidth(int min, int max) const
{
    int maxValue = qMax(qAbs(min), qAbs(max));

    // Count the amount of digits.
    int digits = 0;
    while (maxValue >= 1) {
        ++digits;
        maxValue /= 10;
    }

    // Find the maximum width of any digit.
    int maxWidth = 0;
    for (int i = 0; i < 10; ++i) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        maxWidth = qMax(maxWidth, fontMetrics().horizontalAdvance(QString::number(i)));
#else
        maxWidth = qMax(maxWidth, fontMetrics().width(QString::number(i)));
#endif
    }

    // If min or max is negative, reserve space for a minus-sign
    if (min < 0 || max < 0) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        return maxWidth * digits + fontMetrics().horizontalAdvance('-');
#else
        return maxWidth * digits + fontMetrics().width('-');
#endif
    } else {
        return maxWidth * digits;
    }
}

