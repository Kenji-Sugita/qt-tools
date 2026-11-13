#pragma once

#include <QHash>
#include <QObject>
#include <QTcpServer>

class McpToolHandler;
class SseSession;
class QTcpSocket;

class SseMcpServer : public QObject
{
    Q_OBJECT

public:
    explicit SseMcpServer(McpToolHandler *handler, QObject *parent = nullptr);

    bool listen(const QHostAddress &host, quint16 port, QString *error);

private:
    void handleConnection();
    void readHttpRequest(QTcpSocket *socket);
    void dispatchRequest(QTcpSocket *socket, const QByteArray &request);
    void handleSse(QTcpSocket *socket);
    void handleMessage(QTcpSocket *socket, const QString &path, const QByteArray &body);
    void sendSimpleResponse(QTcpSocket *socket, int code, const QByteArray &reason, const QByteArray &body);
    QString sessionFromPath(const QString &path) const;

    QTcpServer m_server;
    McpToolHandler *m_handler = nullptr;
    QHash<QString, SseSession *> m_sessions;
};
