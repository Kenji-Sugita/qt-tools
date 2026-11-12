#include <QApplication>

#include "widget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Widget widget;
    widget.resize(320, 180);
    widget.show();

    return app.exec();
}

