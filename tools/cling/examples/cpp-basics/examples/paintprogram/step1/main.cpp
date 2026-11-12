#include "scribblearea.h"
#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    ScribbleArea scribbleArea;
    scribbleArea.show();

    return app.exec();
}
