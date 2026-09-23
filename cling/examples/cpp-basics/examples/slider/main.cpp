#include "slider.h"
#include <QApplication>
#include <QLCDNumber>
#include <QLayout>
#include <QDebug>

int main(int argc, char** argv)
{
  QApplication app(argc, argv);

  const auto slider = new Slider(Qt::Horizontal);
  slider->setRange(-100, 100);
  qDebug() << slider->sizePolicy();

  const auto number = new QLCDNumber;

  QObject::connect(slider, &Slider::valueChanged, number, static_cast<void (QLCDNumber::*)(int)>(&QLCDNumber::display));

  slider->setValue(42);

  QWidget top;
  const auto topLayout = new QVBoxLayout(&top);
  topLayout->addWidget(slider);
  topLayout->addWidget(number);

  top.show();

  return app.exec();
}
