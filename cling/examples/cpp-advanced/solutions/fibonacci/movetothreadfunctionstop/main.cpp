#include <QApplication>
#include "fibwidget.h"

int main(int argc, char** argv)
{
    qSetMessagePattern("%{file}:%{line} %{function} %{message}");

    QApplication app(argc, argv);

    FibWidget widget;
    widget.show();

    return app.exec();
}
