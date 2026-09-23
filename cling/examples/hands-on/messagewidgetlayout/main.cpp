#include <QApplication>
#include "messagewidget.h"

int main(int argc, char** argv)
{
    QApplication app{argc, argv};

    MessageWidget messageWidget{"Hello World!", nullptr};
    messageWidget.show();

    return app.exec();
}
