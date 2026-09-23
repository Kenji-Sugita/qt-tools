#include <QApplication>
#include <QLCDNumber>
#include <QSlider>
#include <QLayout>

int main(int argc, char** argv)
{
    QApplication app{argc, argv};

    QWidget topWidget{};
    const auto topLayout = new QVBoxLayout{&topWidget};

    const auto lcdNumber = new QLCDNumber{&topWidget};
    topLayout->addWidget(lcdNumber);

    const auto slider = new QSlider{&topWidget};
    slider->setOrientation(Qt::Horizontal);
    topLayout->addWidget(slider);

    QObject::connect(slider, &QSlider::valueChanged, lcdNumber, qOverload<int>(&QLCDNumber::display));

    topWidget.show();

    return app.exec();
}
