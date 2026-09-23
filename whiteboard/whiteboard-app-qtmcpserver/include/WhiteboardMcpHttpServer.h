#pragma once

#include "WhiteboardMcpController.h"

#include <QDateTime>
#include <QHash>
#include <QObject>
#include <QPointer>

class BoardModel;
class CanvasWidget;
class QHttpServer;
class QHttpServerRequest;
class QHttpServerResponse;
class QTcpServer;

class WhiteboardMcpHttpServer : public QObject {
public:
    static constexpr quint16 DefaultPort = 8765;

    WhiteboardMcpHttpServer(BoardModel *model,
                            CanvasWidget *canvas,
                            WhiteboardMcpController::ChangeCallback changeCallback,
                            WhiteboardMcpController::PageNavigationCallback pageNavigationCallback,
                            WhiteboardMcpController::ApprovalCallback approvalCallback,
                            QObject *parent = nullptr);
    ~WhiteboardMcpHttpServer() override;

    bool start(quint16 port = DefaultPort);
    void stop();
    quint16 port() const;
    QString endpointUrl() const;
    QString errorString() const;

private:
    struct Session {
        WhiteboardMcpController *controller = nullptr;
        QDateTime lastActivity;
    };

    QHttpServerResponse handleRequest(const QHttpServerRequest &request);
    QHttpServerResponse handlePost(const QHttpServerRequest &request);
    QHttpServerResponse handleDelete(const QHttpServerRequest &request);
    WhiteboardMcpController *createSessionController();
    void cleanupExpiredSessions();
    bool originAllowed(const QByteArray &origin) const;

    BoardModel *m_model = nullptr;
    CanvasWidget *m_canvas = nullptr;
    WhiteboardMcpController::ChangeCallback m_changeCallback;
    WhiteboardMcpController::PageNavigationCallback m_pageNavigationCallback;
    WhiteboardMcpController::ApprovalCallback m_approvalCallback;
    QPointer<QHttpServer> m_httpServer;
    QPointer<QTcpServer> m_tcpServer;
    QHash<QString, Session> m_sessions;
    QString m_errorString;
    quint16 m_port = 0;
};
