#include <QSlider>
#include <QVBoxLayout>
#include <QDebug>

QSlider* create_slider() {
    auto window = new QWidget{};
    auto topLayout = new QVBoxLayout{window};

    auto slider = new QSlider(Qt::Horizontal);
    slider->setMinimumWidth(200);
    topLayout->addWidget(slider);
    topLayout->setAlignment(slider, Qt::AlignHCenter);
    QObject::connect(slider, &QSlider::valueChanged, [](int value) { qDebug() << value; });

    window->show();
    window->raise();
    window->activateWindow();

    return slider;
}

auto slider = create_slider();
