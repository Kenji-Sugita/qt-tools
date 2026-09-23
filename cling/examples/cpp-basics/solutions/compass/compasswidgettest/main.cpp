#include "compasswidget.h"
#include "compasswidget2.h"
#include <QApplication>
#include <QLabel>
#include <QLayout>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QWidget top;

    CompassWidget* const compassWidget = new CompassWidget(&top);
    CompassWidget2* const compassWidget2 = new CompassWidget2(&top);

    QObject::connect(compassWidget, &CompassWidget::directionChanged,
                     compassWidget2, &CompassWidget2::setDirection);
    QObject::connect(compassWidget2, &CompassWidget2::directionChanged,
                     compassWidget, &CompassWidget::setDirection);

    compassWidget2->setFixedSize(4 * compassWidget2->sizeHint());

    QHBoxLayout* const compassLayout = new QHBoxLayout;
    compassLayout->addStretch();
    compassLayout->addWidget(compassWidget);
    compassLayout->addStretch();

    QHBoxLayout* const compassLayout2 = new QHBoxLayout;
    compassLayout2->addStretch();
    compassLayout2->addWidget(compassWidget2);
    compassLayout2->addStretch();

    QVBoxLayout* const topLayout = new QVBoxLayout(&top);
    topLayout->addLayout(compassLayout);
    topLayout->addLayout(compassLayout2);
    topLayout->addStretch();

    top.show();

    return app.exec();
}
