#include <QApplication>
#include "colorwidget.h"

int main(int argc, char** argv)
{
    QApplication app{argc, argv};

    ColorWidget top{nullptr};
    top.show();

    return app.exec();
}
