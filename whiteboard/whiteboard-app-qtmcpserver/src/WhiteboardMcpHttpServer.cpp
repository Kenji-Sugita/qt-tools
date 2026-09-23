#include "WhiteboardMcpHttpServer.h"

#include <QHostAddress>
#include <QHttpHeaders>
#include <QHttpServer>
#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpServer>
#include <QUrl>
#include <QUuid>

#include <utility>

namespace {
constexpr qsizetype MaxRequestBytes = 4 * 1024 * 1024;
constexpr qsizetype MaxSessions = 32;
constexpr qint64 SessionTimeoutSeconds = 24 * 60 * 60;

using StatusCode = QHttpServerResponse::StatusCode;

QHttpServerResponse errorResponse(StatusCode statusCode, const QString &message)
{
    return QHttpServerResponse(
        QJsonObject{{QStringLiteral("error"), message}}, statusCode);
}

QHttpServerResponse jsonResponse(const QJsonObject &body,
                                 StatusCode statusCode = StatusCode::Ok,
                                 const QString &sessionId = QString())
{
    QHttpServerResponse response(body, statusCode);
    if (!sessionId.isEmpty()) {
        QHttpHeaders headers = response.headers();
        headers.append("Mcp-Session-Id", sessionId);
        response.setHeaders(std::move(headers));
    }
    return response;
}

bool isHttpProtocolVersion(const QString &version)
{
    return version == QStringLiteral("2025-03-26")
        || version == QStringLiteral("2025-06-18");
}
}

WhiteboardMcpHttpServer::WhiteboardMcpHttpServer(
    BoardModel *model,
    CanvasWidget *canvas,
    WhiteboardMcpController::ChangeCallback changeCallback,
    WhiteboardMcpController::PageNavigationCallback pageNavigationCallback,
    WhiteboardMcpController::ApprovalCallback approvalCallback,
    QObject *parent)
    : QObject(parent)
    , m_model(model)
    , m_canvas(canvas)
    , m_changeCallback(std::move(changeCallback))
    , m_pageNavigationCallback(std::move(pageNavigationCallback))
    , m_approvalCallback(std::move(approvalCallback))
{
}

WhiteboardMcpHttpServer::~WhiteboardMcpHttpServer()
{
    stop();
}

bool WhiteboardMcpHttpServer::start(quint16 requestedPort)
{
    if (m_httpServer)
        return true;

    m_httpServer = new QHttpServer(this);
    m_httpServer->route(
        QStringLiteral("/mcp"),
        [this](const QHttpServerRequest &request) {
            return handleRequest(request);
        });

    m_tcpServer = new QTcpServer(this);
    if (!m_tcpServer->listen(QHostAddress::LocalHost, requestedPort)) {
        m_errorString = m_tcpServer->errorString();
        stop();
        return false;
    }
    if (!m_httpServer->bind(m_tcpServer)) {
        m_errorString = QStringLiteral("Qt HTTP server could not bind to the local TCP listener.");
        stop();
        return false;
    }

    m_port = m_tcpServer->serverPort();
    m_errorString.clear();
    return true;
}

void WhiteboardMcpHttpServer::stop()
{
    for (const Session &session : std::as_const(m_sessions))
        delete session.controller;
    m_sessions.clear();

    if (m_tcpServer)
        m_tcpServer->close();
    delete m_httpServer.data();
    delete m_tcpServer.data();
    m_httpServer = nullptr;
    m_tcpServer = nullptr;
    m_port = 0;
}

quint16 WhiteboardMcpHttpServer::port() const
{
    return m_port;
}

QString WhiteboardMcpHttpServer::endpointUrl() const
{
    return m_port == 0
        ? QString()
        : QStringLiteral("http://127.0.0.1:%1/mcp").arg(m_port);
}

QString WhiteboardMcpHttpServer::errorString() const
{
    return m_errorString;
}

QHttpServerResponse WhiteboardMcpHttpServer::handleRequest(const QHttpServerRequest &request)
{
    if (!request.remoteAddress().isLoopback())
        return errorResponse(StatusCode::Forbidden, QStringLiteral("Only loopback clients are allowed."));
    if (!originAllowed(request.value("Origin")))
        return errorResponse(StatusCode::Forbidden, QStringLiteral("The Origin header is not allowed."));

    cleanupExpiredSessions();
    switch (request.method()) {
    case QHttpServerRequest::Method::Post:
        return handlePost(request);
    case QHttpServerRequest::Method::Delete:
        return handleDelete(request);
    case QHttpServerRequest::Method::Get:
        return QHttpServerResponse(StatusCode::MethodNotAllowed);
    default:
        return QHttpServerResponse(StatusCode::MethodNotAllowed);
    }
}

QHttpServerResponse WhiteboardMcpHttpServer::handlePost(const QHttpServerRequest &request)
{
    const QByteArray contentType = request.value("Content-Type").toLower();
    if (!contentType.startsWith("application/json"))
        return errorResponse(StatusCode::UnsupportedMediaType, QStringLiteral("Content-Type must be application/json."));

    const QByteArray accept = request.value("Accept").toLower();
    if (!accept.contains("application/json") || !accept.contains("text/event-stream")) {
        return errorResponse(
            StatusCode::NotAcceptable,
            QStringLiteral("Accept must include application/json and text/event-stream."));
    }

    const QByteArray body = request.body();
    if (body.isEmpty() || body.size() > MaxRequestBytes)
        return errorResponse(StatusCode::PayloadTooLarge, QStringLiteral("The MCP request body size is invalid."));

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject())
        return errorResponse(StatusCode::BadRequest, QStringLiteral("The request body must be a JSON object."));

    const QJsonObject message = document.object();
    const QString method = message.value(QStringLiteral("method")).toString();
    const QString sessionId = QString::fromLatin1(request.value("Mcp-Session-Id"));

    if (method == QStringLiteral("initialize")) {
        if (!sessionId.isEmpty())
            return errorResponse(StatusCode::BadRequest, QStringLiteral("Initialize must not include Mcp-Session-Id."));
        if (m_sessions.size() >= MaxSessions)
            return errorResponse(StatusCode::ServiceUnavailable, QStringLiteral("Too many MCP sessions are active."));

        WhiteboardMcpController *controller = createSessionController();
        if (!controller || !controller->startMessageTransport()) {
            delete controller;
            return errorResponse(StatusCode::InternalServerError, QStringLiteral("The MCP session could not be started."));
        }

        QJsonObject responseBody;
        bool hasResponse = false;
        if (!controller->processMessage(body, &responseBody, &hasResponse) || !hasResponse
            || !isHttpProtocolVersion(controller->negotiatedProtocolVersion())) {
            delete controller;
            return errorResponse(
                StatusCode::BadRequest,
                QStringLiteral("Streamable HTTP requires MCP protocol version 2025-03-26 or newer."));
        }

        const QString newSessionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        m_sessions.insert(newSessionId, Session{controller, QDateTime::currentDateTimeUtc()});
        return jsonResponse(responseBody, StatusCode::Ok, newSessionId);
    }

    if (sessionId.isEmpty())
        return errorResponse(StatusCode::BadRequest, QStringLiteral("Mcp-Session-Id is required."));
    auto sessionIt = m_sessions.find(sessionId);
    if (sessionIt == m_sessions.end())
        return errorResponse(StatusCode::NotFound, QStringLiteral("The MCP session was not found."));

    const QString protocolVersion = QString::fromLatin1(request.value("MCP-Protocol-Version"));
    if (protocolVersion != sessionIt->controller->negotiatedProtocolVersion()) {
        return errorResponse(StatusCode::BadRequest, QStringLiteral("MCP-Protocol-Version does not match the session."));
    }

    QJsonObject responseBody;
    bool hasResponse = false;
    if (!sessionIt->controller->processMessage(body, &responseBody, &hasResponse))
        return errorResponse(StatusCode::BadRequest, QStringLiteral("The JSON-RPC message was rejected."));
    sessionIt->lastActivity = QDateTime::currentDateTimeUtc();

    if (!hasResponse)
        return QHttpServerResponse(StatusCode::Accepted);
    return jsonResponse(responseBody);
}

QHttpServerResponse WhiteboardMcpHttpServer::handleDelete(const QHttpServerRequest &request)
{
    const QString sessionId = QString::fromLatin1(request.value("Mcp-Session-Id"));
    if (sessionId.isEmpty())
        return errorResponse(StatusCode::BadRequest, QStringLiteral("Mcp-Session-Id is required."));

    auto sessionIt = m_sessions.find(sessionId);
    if (sessionIt == m_sessions.end())
        return errorResponse(StatusCode::NotFound, QStringLiteral("The MCP session was not found."));
    const QString protocolVersion = QString::fromLatin1(request.value("MCP-Protocol-Version"));
    if (protocolVersion != sessionIt->controller->negotiatedProtocolVersion()) {
        return errorResponse(StatusCode::BadRequest, QStringLiteral("MCP-Protocol-Version does not match the session."));
    }
    delete sessionIt->controller;
    m_sessions.erase(sessionIt);
    return QHttpServerResponse(StatusCode::NoContent);
}

WhiteboardMcpController *WhiteboardMcpHttpServer::createSessionController()
{
    return new WhiteboardMcpController(
        m_model, m_canvas, m_changeCallback, m_pageNavigationCallback, m_approvalCallback, this);
}

void WhiteboardMcpHttpServer::cleanupExpiredSessions()
{
    const QDateTime expiration = QDateTime::currentDateTimeUtc().addSecs(-SessionTimeoutSeconds);
    for (auto it = m_sessions.begin(); it != m_sessions.end();) {
        if (it->lastActivity >= expiration) {
            ++it;
            continue;
        }
        delete it->controller;
        it = m_sessions.erase(it);
    }
}

bool WhiteboardMcpHttpServer::originAllowed(const QByteArray &origin) const
{
    if (origin.isEmpty())
        return true;

    const QUrl originUrl = QUrl::fromEncoded(origin);
    const QString host = originUrl.host().toLower();
    return originUrl.isValid()
        && (originUrl.scheme() == QStringLiteral("http") || originUrl.scheme() == QStringLiteral("https"))
        && (host == QStringLiteral("localhost") || host == QStringLiteral("127.0.0.1")
            || host == QStringLiteral("::1"));
}
