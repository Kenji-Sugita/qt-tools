#include "colortester.h"
#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    ColorTester tester;
    tester.show();

    return app.exec();
}
