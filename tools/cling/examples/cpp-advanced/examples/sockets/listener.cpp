#include "listener.h"
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#endif
#include <QDebug>

Listener::Listener(QWidget* parent) : QTextEdit(parent)
{
    _socket = new QTcpSocket(this);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(_socket, static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::errorOccurred), this, &Listener::slotError);
#else
    connect(_socket, static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this, &Listener::slotError);
#endif
    connect(_socket, &QTcpSocket::connected, this, &Listener::slotConnected);
    connect(_socket, &QTcpSocket::hostFound, this, &Listener::slotHostFound);

    qDebug() << "Connecting to host...";
    connect(_socket, &QTcpSocket::readyRead, this, &Listener::readData);
    _socket->connectToHost("www.sra.co.jp", 80);
}

void Listener::readData()
{
    QTextStream stream(_socket);
    const QString text = stream.readAll();
    setHtml(text);
}

void Listener::slotError(QAbstractSocket::SocketError err)
{
    qDebug() << "Error:" << err << _socket->errorString();
}

void Listener::slotConnected()
{
    qDebug() << "Connected, sending request...";
    QTextStream stream(_socket);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    stream.setCodec(QTextCodec::codecForName("latin1"));
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    stream << "GET https://www.sra.co.jp HTTP/1.0\r\n\r\n" << Qt::flush;
#else
    stream << "GET https://www.sra.co.jp HTTP/1.0\r\n\r\n" << flush;
#endif
}

void Listener::slotHostFound()
{
    qDebug() << "Host found";
}
