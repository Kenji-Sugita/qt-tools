#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QHBoxLayout>

#include "colorwidget.h"

int main(int argc, char** argv)
{
    QApplication app{argc, argv};

    QTranslator translator{};

    if (translator.load("colorwidget_" + QLocale::system().bcp47Name())) {
        if (!app.installTranslator(&translator)) {
            qWarning() << "Cannot install the translator";
        }
    } else {
        qWarning() << "Cannot load the translation file";
    }

    auto window = new QWidget{};
    auto windowLayout = new QHBoxLayout{window};

    auto colorWidget = new ColorWidget{};
    windowLayout->addWidget(colorWidget);

    auto colorWidget2 = new ColorWidget{};
    windowLayout->addWidget(colorWidget2);

    QObject::connect(colorWidget, &ColorWidget::colorChanged,
                     colorWidget2, &ColorWidget::setColor);
    QObject::connect(colorWidget2, &ColorWidget::colorChanged,
                     colorWidget, &ColorWidget::setColor);

    window->show();

    return app.exec();
}
