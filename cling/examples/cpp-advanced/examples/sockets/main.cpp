#include "listener.h"
#include <QApplication>
#include <QNetworkProxy>
#include <QNetworkProxyFactory>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QNetworkProxyFactory::setUseSystemConfiguration(true);

    Listener listener;
    listener.show();

    return app.exec();
}
