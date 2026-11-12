#include "fibwidget.h"
#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    FibWidget widget;
    widget.show();

    return app.exec();
}
