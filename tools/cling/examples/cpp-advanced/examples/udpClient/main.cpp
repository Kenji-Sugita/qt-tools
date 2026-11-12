#include <QCoreApplication>
#include <QtNetwork>
#include <QByteArray>
#include <QTextStream>
#include <QDebug>

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    {
        QUdpSocket socket;
        QByteArray data;
        QTextStream stream(&data);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        stream << "Hello world, here comes a UDP package" << Qt::flush;
#else
        stream << "Hello world, here comes a UDP package" << flush;
#endif
        if (socket.writeDatagram(data, QHostAddress::LocalHost, 4242) == -1) {
            qDebug() << "Cannot send data";
        }
    }

    {
        QUdpSocket socket;
        socket.connectToHost(QHostAddress::LocalHost, 4242);
        QTextStream stream(&socket);

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        stream << "Hello World, here comes a second UDP package" << Qt::flush;
#else
        stream << "Hello World, here comes a second UDP package" << flush;
#endif
    }
}
