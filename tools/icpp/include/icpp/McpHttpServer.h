#pragma once

#include "icpp/repl.h"

#include <QDateTime>
#include <QHash>
#include <QObject>
#include <QPointer>

class QHttpServer;
class QHttpServerRequest;
class QHttpServerResponse;
class QTcpServer;
class QTimer;

namespace icpp {

class McpController;

class McpHttpServer : public QObject
{
public:
    static constexpr quint16 DefaultPort = 8766;

    McpHttpServer(Engine engine,
                  QString interpreterProgram,
                  QStringList interpreterArguments,
                  QByteArray authenticationToken,
                  bool executionAllowed,
                  QObject* parent = nullptr);
    ~McpHttpServer() override;

    bool start(quint16 port = DefaultPort);
    void stop();
    quint16 port() const;
    QString endpointUrl() const;
    QString errorString() const;

private:
    struct Session {
        McpController* controller = nullptr;
        QDateTime lastActivity;
    };

    QHttpServerResponse handleRequest(const QHttpServerRequest& request);
    QHttpServerResponse handlePost(const QHttpServerRequest& request);
    QHttpServerResponse handleDelete(const QHttpServerRequest& request);
    McpController* createSessionController();
    void cleanupExpiredSessions();
    bool originAllowed(const QByteArray& origin) const;
    bool hostAllowed(const QByteArray& host) const;
    bool authenticated(const QByteArray& authorization) const;

    Engine currentEngine;
    QString programPath;
    QStringList additionalArguments;
    QByteArray expectedAuthorization;
    bool allowExecution = false;
    QPointer<QHttpServer> httpServer;
    QPointer<QTcpServer> tcpServer;
    QPointer<QTimer> cleanupTimer;
    QHash<QString, Session> sessions;
    QString currentError;
    quint16 listeningPort = 0;
};

} // namespace icpp
