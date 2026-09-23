#include "harness.h"
#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    Harness harness;
    harness.show();

    return app.exec();
}
