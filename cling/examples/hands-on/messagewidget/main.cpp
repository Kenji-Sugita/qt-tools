#include <QApplication>
#include "messagewidget.h"

int main(int argc, char** argv)
{
    QApplication app{argc, argv};

    MessageWidget messageWidget{"Hello World!", nullptr};
    QObject::connect(&messageWidget, &MessageWidget::quit, &app, &QApplication::quit);
    messageWidget.show();

    return app.exec();
}
