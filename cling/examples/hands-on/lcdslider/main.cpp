#include <QApplication>

#include "lcdslider.h"

int main(int argc, char** argv)
{
    QApplication app{argc, argv};

    LcdSlider lcdSlider{};
    lcdSlider.show();

    return app.exec();
}
