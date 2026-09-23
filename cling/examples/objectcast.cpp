#include <QSlider>

QObject* create_slider()
{
    auto slider = new QSlider{Qt::Horizontal};

    return slider;
}

auto object = create_slider();
