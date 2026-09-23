#include <QApplication>
#include <QTranslator>
#include <QLocale>

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

    ColorWidget top{nullptr};
    top.show();

    return app.exec();
}
