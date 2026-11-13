#include "SseMcpServer.h"

#include "JsonRpcMessage.h"
#include "McpToolHandler.h"
#include "SseSession.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QRandomGenerator>
#include <QTcpSocket>
#include <QUrl>
#include <QUrlQuery>

SseMcpServer::SseMcpServer(McpToolHandler *handler, QObject *parent)
    : QObject(parent)
    , m_handler(handler)
{
    connect(&m_server, &QTcpServer::newConnection, this, &SseMcpServer::handleConnection);
}

bool SseMcpServer::listen(const QHostAddress &host, quint16 port, QString *error)
{
    if (!m_server.listen(host, port)) {
        if (error)
            *error = m_server.errorString();
        return false;
    }
    return true;
}

void SseMcpServer::handleConnection()
{
    while (QTcpSocket *socket = m_server.nextPendingConnection()) {
        connect(socket, &QTcpSocket::readyRead, this, [this, socket] { readHttpRequest(socket); });
        connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
    }
}

void SseMcpServer::readHttpRequest(QTcpSocket *socket)
{
    QByteArray buffer = socket->property("requestBuffer").toByteArray();
    buffer += socket->readAll();
    socket->setProperty("requestBuffer", buffer);

    const int headerEnd = buffer.indexOf("\r\n\r\n");
    if (headerEnd < 0)
        return;

    const QByteArray header = buffer.left(headerEnd);
    int contentLength = 0;
    const QList<QByteArray> lines = header.split('\n');
    for (QByteArray line : lines) {
        line = line.trimmed();
        if (line.toLower().startsWith("content-length:"))
            contentLength = line.mid(line.indexOf(':') + 1).trimmed().toInt();
    }

    const int totalLength = headerEnd + 4 + contentLength;
    if (buffer.size() < totalLength)
        return;

    const QByteArray request = buffer.left(totalLength);
    socket->setProperty("requestBuffer", buffer.mid(totalLength));
    dispatchRequest(socket, request);
}

void SseMcpServer::dispatchRequest(QTcpSocket *socket, const QByteArray &request)
{
    const int firstLineEnd = request.indexOf("\r\n");
    if (firstLineEnd < 0) {
        sendSimpleResponse(socket, 400, "Bad Request", "bad request\n");
        return;
    }

    const QList<QByteArray> parts = request.left(firstLineEnd).split(' ');
    if (parts.size() < 2) {
        sendSimpleResponse(socket, 400, "Bad Request", "bad request\n");
        return;
    }

    const QByteArray method = parts.at(0);
    const QString path = QString::fromUtf8(parts.at(1));
    const int headerEnd = request.indexOf("\r\n\r\n");
    const QByteArray body = headerEnd >= 0 ? request.mid(headerEnd + 4) : QByteArray {};

    if (method == "GET" && path == QStringLiteral("/sse")) {
        handleSse(socket);
    } else if (method == "POST" && path.startsWith(QStringLiteral("/message"))) {
        handleMessage(socket, path, body);
    } else if (method == "OPTIONS") {
        sendSimpleResponse(socket, 204, "No Content", {});
    } else {
        sendSimpleResponse(socket, 404, "Not Found", "not found\n");
    }
}

void SseMcpServer::handleSse(QTcpSocket *socket)
{
    socket->disconnect(this);
    const QString sessionId = QString::number(QRandomGenerator::global()->generate64(), 16);
    auto *session = new SseSession(sessionId, socket, this);
    m_sessions.insert(sessionId, session);
    connect(session, &SseSession::closed, this, [this, session](const QString &id) {
        m_sessions.remove(id);
        session->deleteLater();
    });
    session->start(QStringLiteral("/message?session=%1").arg(sessionId));
}

void SseMcpServer::handleMessage(QTcpSocket *socket, const QString &path, const QByteArray &body)
{
    const QString sessionId = sessionFromPath(path);
    SseSession *session = m_sessions.value(sessionId, nullptr);
    if (!session) {
        sendSimpleResponse(socket, 404, "Not Found", "unknown session\n");
        return;
    }

    QJsonParseError parseError {};
    const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        const QJsonObject response = JsonRpcMessage::error(QJsonValue(), -32700, QStringLiteral("parse error"));
        session->sendMessage(JsonRpcMessage::compactJson(response));
        sendSimpleResponse(socket, 202, "Accepted", "accepted\n");
        return;
    }

    const QJsonObject response = m_handler->handleRequest(document.object());
    if (!response.isEmpty())
        session->sendMessage(JsonRpcMessage::compactJson(response));
    sendSimpleResponse(socket, 202, "Accepted", "accepted\n");
}

void SseMcpServer::sendSimpleResponse(QTcpSocket *socket,
                                      int code,
                                      const QByteArray &reason,
                                      const QByteArray &body)
{
    QByteArray response = "HTTP/1.1 " + QByteArray::number(code) + ' ' + reason + "\r\n"
                          "Access-Control-Allow-Origin: *\r\n"
                          "Access-Control-Allow-Headers: content-type\r\n"
                          "Content-Length: " + QByteArray::number(body.size()) + "\r\n"
                          "Content-Type: text/plain; charset=utf-8\r\n"
                          "Connection: close\r\n"
                          "\r\n"
                          + body;
    socket->write(response);
    socket->disconnectFromHost();
}

QString SseMcpServer::sessionFromPath(const QString &path) const
{
    const QUrl url(QStringLiteral("http://localhost") + path);
    return QUrlQuery(url).queryItemValue(QStringLiteral("session"));
}
