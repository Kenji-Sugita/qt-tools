#include <QCoreApplication>
#include <QTextStream>
#include <QHostAddress>
#include <QtNetwork>

class Server :public QUdpSocket
{
    Q_OBJECT

public:
    explicit Server(QObject* parent = nullptr) : QUdpSocket(parent) {
        if (!bind(QHostAddress::LocalHost, 4242)) {
            qDebug() << "Cannot bind.";
        }
        connect(this, &Server::readyRead, this, &Server::readPendingDatagrams);
    }

private slots:
    void readPendingDatagrams() {
        while (hasPendingDatagrams()) {
            QByteArray datagram;
            datagram.resize(pendingDatagramSize());
            QHostAddress sender;
            quint16 senderPort;
            readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
            QTextStream stream(datagram);
            const QString str = stream.readAll();
            qDebug("Data from %s (port %d): %s", sender.toString().toLatin1().data(), senderPort, str.toLatin1().data());
        }
    }
};

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Server server;

    return app.exec();
}

#include "main.moc"
