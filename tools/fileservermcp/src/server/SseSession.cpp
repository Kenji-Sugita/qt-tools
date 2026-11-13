#include "SseSession.h"

#include <QTcpSocket>

SseSession::SseSession(QString id, QTcpSocket *socket, QObject *parent)
    : QObject(parent)
    , m_id(std::move(id))
    , m_socket(socket)
{
    m_socket->setParent(this);
    connect(m_socket, &QTcpSocket::disconnected, this, [this] { emit closed(m_id); });
}

QString SseSession::id() const
{
    return m_id;
}

void SseSession::start(const QString &messageEndpoint)
{
    const QByteArray header = "HTTP/1.1 200 OK\r\n"
                              "Content-Type: text/event-stream\r\n"
                              "Cache-Control: no-cache\r\n"
                              "Connection: keep-alive\r\n"
                              "Access-Control-Allow-Origin: *\r\n"
                              "\r\n";
    m_socket->write(header);
    m_socket->write("event: endpoint\n");
    m_socket->write("data: " + messageEndpoint.toUtf8() + "\n\n");
    m_socket->flush();
}

void SseSession::sendMessage(const QString &json)
{
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState)
        return;
    m_socket->write("event: message\n");
    m_socket->write("data: " + json.toUtf8() + "\n\n");
    m_socket->flush();
}
