#include "icpp/McpHttpServer.h"

#include "icpp/McpController.h"

#include <QHostAddress>
#include <QHttpHeaders>
#include <QHttpServer>
#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpServer>
#include <QTimer>
#include <QUrl>
#include <QUuid>

#include <utility>

namespace icpp {
namespace {

constexpr qsizetype MaxRequestBytes = 1024 * 1024;
constexpr qsizetype MaxSessions = 1;
constexpr qint64 SessionTimeoutSeconds = 30 * 60;
constexpr int CleanupIntervalMilliseconds = 60 * 1000;

using StatusCode = QHttpServerResponse::StatusCode;

QHttpServerResponse errorResponse(StatusCode statusCode, const QString& message)
{
    return QHttpServerResponse(
        QJsonObject{{QStringLiteral("error"), message}}, statusCode);
}

QHttpServerResponse unauthorizedResponse()
{
    QHttpServerResponse response(
        QJsonObject{{QStringLiteral("error"), QStringLiteral("Bearer authentication is required.")}},
        StatusCode::Unauthorized);
    QHttpHeaders headers = response.headers();
    headers.append("WWW-Authenticate", "Bearer realm=\"icpp-mcp\"");
    response.setHeaders(std::move(headers));
    return response;
}

QHttpServerResponse jsonResponse(const QJsonObject& body,
                                 StatusCode statusCode = StatusCode::Ok,
                                 const QString& sessionId = QString())
{
    QHttpServerResponse response(body, statusCode);
    if (!sessionId.isEmpty()) {
        QHttpHeaders headers = response.headers();
        headers.append("Mcp-Session-Id", sessionId);
        response.setHeaders(std::move(headers));
    }
    return response;
}

bool isStreamableHttpProtocolVersion(const QString& version)
{
    return version == QStringLiteral("2025-03-26")
        || version == QStringLiteral("2025-06-18");
}

bool constantTimeEquals(const QByteArray& left, const QByteArray& right)
{
    const qsizetype maximumSize = qMax(left.size(), right.size());
    quint64 difference = quint64(left.size()) ^ quint64(right.size());
    for (qsizetype index = 0; index < maximumSize; ++index) {
        const unsigned char leftByte = index < left.size()
            ? static_cast<unsigned char>(left.at(index))
            : 0;
        const unsigned char rightByte = index < right.size()
            ? static_cast<unsigned char>(right.at(index))
            : 0;
        difference |= quint64(leftByte ^ rightByte);
    }
    return difference == 0;
}

} // namespace

McpHttpServer::McpHttpServer(Engine engine,
                             QString interpreterProgram,
                             QStringList interpreterArguments,
                             QByteArray authenticationToken,
                             bool executionAllowed,
                             QObject* parent)
    : QObject(parent)
    , currentEngine(engine)
    , programPath(std::move(interpreterProgram))
    , additionalArguments(std::move(interpreterArguments))
    , expectedAuthorization(QByteArrayLiteral("Bearer ") + authenticationToken)
    , allowExecution(executionAllowed)
{
}

McpHttpServer::~McpHttpServer()
{
    stop();
}

bool McpHttpServer::start(quint16 requestedPort)
{
    if (httpServer) {
        return true;
    }

    httpServer = new QHttpServer(this);
    httpServer->route(
        QStringLiteral("/mcp"),
        [this](const QHttpServerRequest& request) {
            return handleRequest(request);
        });

    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen(QHostAddress::LocalHost, requestedPort)) {
        currentError = tcpServer->errorString();
        stop();
        return false;
    }
    if (!httpServer->bind(tcpServer)) {
        currentError = QStringLiteral("Qt HTTP server could not bind to the local TCP listener.");
        stop();
        return false;
    }

    cleanupTimer = new QTimer(this);
    cleanupTimer->setInterval(CleanupIntervalMilliseconds);
    connect(cleanupTimer, &QTimer::timeout, this, [this]() { cleanupExpiredSessions(); });
    cleanupTimer->start();

    listeningPort = tcpServer->serverPort();
    currentError.clear();
    return true;
}

void McpHttpServer::stop()
{
    if (cleanupTimer) {
        cleanupTimer->stop();
    }
    delete cleanupTimer.data();
    cleanupTimer = nullptr;

    for (const Session& session : std::as_const(sessions)) {
        delete session.controller;
    }
    sessions.clear();

    if (tcpServer) {
        tcpServer->close();
    }
    delete httpServer.data();
    delete tcpServer.data();
    httpServer = nullptr;
    tcpServer = nullptr;
    listeningPort = 0;
}

quint16 McpHttpServer::port() const
{
    return listeningPort;
}

QString McpHttpServer::endpointUrl() const
{
    return listeningPort == 0
        ? QString()
        : QStringLiteral("http://127.0.0.1:%1/mcp").arg(listeningPort);
}

QString McpHttpServer::errorString() const
{
    return currentError;
}

QHttpServerResponse McpHttpServer::handleRequest(const QHttpServerRequest& request)
{
    if (!request.remoteAddress().isLoopback()) {
        return errorResponse(StatusCode::Forbidden, QStringLiteral("Only loopback clients are allowed."));
    }
    if (!hostAllowed(request.value("Host"))) {
        return errorResponse(StatusCode::Forbidden, QStringLiteral("The Host header is not allowed."));
    }
    if (!originAllowed(request.value("Origin"))) {
        return errorResponse(StatusCode::Forbidden, QStringLiteral("The Origin header is not allowed."));
    }
    if (!authenticated(request.value("Authorization"))) {
        return unauthorizedResponse();
    }

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

QHttpServerResponse McpHttpServer::handlePost(const QHttpServerRequest& request)
{
    const QByteArray contentType = request.value("Content-Type").toLower();
    if (!contentType.startsWith("application/json")) {
        return errorResponse(
            StatusCode::UnsupportedMediaType,
            QStringLiteral("Content-Type must be application/json."));
    }

    const QByteArray accept = request.value("Accept").toLower();
    if (!accept.contains("application/json") || !accept.contains("text/event-stream")) {
        return errorResponse(
            StatusCode::NotAcceptable,
            QStringLiteral("Accept must include application/json and text/event-stream."));
    }

    const QByteArray body = request.body();
    if (body.isEmpty()) {
        return errorResponse(StatusCode::BadRequest, QStringLiteral("The MCP request body is empty."));
    }
    if (body.size() > MaxRequestBytes) {
        return errorResponse(StatusCode::PayloadTooLarge, QStringLiteral("The MCP request exceeds 1 MiB."));
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return errorResponse(
            StatusCode::BadRequest,
            QStringLiteral("The request body must be a JSON object."));
    }

    const QJsonObject message = document.object();
    const QString method = message.value(QStringLiteral("method")).toString();
    const QString sessionId = QString::fromLatin1(request.value("Mcp-Session-Id"));

    if (method == QStringLiteral("initialize")) {
        if (!sessionId.isEmpty()) {
            return errorResponse(
                StatusCode::BadRequest,
                QStringLiteral("Initialize must not include Mcp-Session-Id."));
        }
        if (sessions.size() >= MaxSessions) {
            return errorResponse(
                StatusCode::ServiceUnavailable,
                QStringLiteral("Another MCP session is already active."));
        }

        McpController* controller = createSessionController();
        if (!controller || !controller->startMessageTransport()) {
            delete controller;
            return errorResponse(
                StatusCode::InternalServerError,
                QStringLiteral("The MCP session could not be started."));
        }

        QJsonObject responseBody;
        bool hasResponse = false;
        if (!controller->processMessage(body, &responseBody, &hasResponse) || !hasResponse
            || !isStreamableHttpProtocolVersion(controller->negotiatedProtocolVersion())) {
            delete controller;
            return errorResponse(
                StatusCode::BadRequest,
                QStringLiteral("Streamable HTTP requires MCP protocol version 2025-03-26 or 2025-06-18."));
        }

        const QString newSessionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        sessions.insert(newSessionId, Session{controller, QDateTime::currentDateTimeUtc()});
        return jsonResponse(responseBody, StatusCode::Ok, newSessionId);
    }

    if (sessionId.isEmpty()) {
        return errorResponse(StatusCode::BadRequest, QStringLiteral("Mcp-Session-Id is required."));
    }
    auto sessionIt = sessions.find(sessionId);
    if (sessionIt == sessions.end()) {
        return errorResponse(StatusCode::NotFound, QStringLiteral("The MCP session was not found."));
    }

    const QString protocolVersion = QString::fromLatin1(request.value("MCP-Protocol-Version"));
    if (protocolVersion != sessionIt->controller->negotiatedProtocolVersion()) {
        return errorResponse(
            StatusCode::BadRequest,
            QStringLiteral("MCP-Protocol-Version does not match the session."));
    }

    QJsonObject responseBody;
    bool hasResponse = false;
    if (!sessionIt->controller->processMessage(body, &responseBody, &hasResponse)) {
        return errorResponse(StatusCode::BadRequest, QStringLiteral("The JSON-RPC message was rejected."));
    }
    sessionIt->lastActivity = QDateTime::currentDateTimeUtc();

    if (!hasResponse) {
        return QHttpServerResponse(StatusCode::Accepted);
    }
    return jsonResponse(responseBody);
}

QHttpServerResponse McpHttpServer::handleDelete(const QHttpServerRequest& request)
{
    const QString sessionId = QString::fromLatin1(request.value("Mcp-Session-Id"));
    if (sessionId.isEmpty()) {
        return errorResponse(StatusCode::BadRequest, QStringLiteral("Mcp-Session-Id is required."));
    }

    auto sessionIt = sessions.find(sessionId);
    if (sessionIt == sessions.end()) {
        return errorResponse(StatusCode::NotFound, QStringLiteral("The MCP session was not found."));
    }
    const QString protocolVersion = QString::fromLatin1(request.value("MCP-Protocol-Version"));
    if (protocolVersion != sessionIt->controller->negotiatedProtocolVersion()) {
        return errorResponse(
            StatusCode::BadRequest,
            QStringLiteral("MCP-Protocol-Version does not match the session."));
    }

    delete sessionIt->controller;
    sessions.erase(sessionIt);
    return QHttpServerResponse(StatusCode::NoContent);
}

McpController* McpHttpServer::createSessionController()
{
    return new McpController(
        currentEngine, programPath, additionalArguments, allowExecution, this);
}

void McpHttpServer::cleanupExpiredSessions()
{
    const QDateTime expiration = QDateTime::currentDateTimeUtc().addSecs(-SessionTimeoutSeconds);
    for (auto it = sessions.begin(); it != sessions.end();) {
        if (it->lastActivity >= expiration) {
            ++it;
            continue;
        }
        delete it->controller;
        it = sessions.erase(it);
    }
}

bool McpHttpServer::originAllowed(const QByteArray& origin) const
{
    if (origin.isEmpty()) {
        return true;
    }

    const QUrl originUrl = QUrl::fromEncoded(origin);
    const QString host = originUrl.host().toLower();
    return originUrl.isValid()
        && (originUrl.scheme() == QStringLiteral("http")
            || originUrl.scheme() == QStringLiteral("https"))
        && (host == QStringLiteral("localhost") || host == QStringLiteral("127.0.0.1")
            || host == QStringLiteral("::1"));
}

bool McpHttpServer::hostAllowed(const QByteArray& host) const
{
    if (host.isEmpty()) {
        return false;
    }
    const QUrl hostUrl = QUrl::fromEncoded(QByteArrayLiteral("http://") + host);
    const QString hostName = hostUrl.host().toLower();
    return hostUrl.isValid()
        && (hostName == QStringLiteral("localhost") || hostName == QStringLiteral("127.0.0.1")
            || hostName == QStringLiteral("::1"));
}

bool McpHttpServer::authenticated(const QByteArray& authorization) const
{
    return !expectedAuthorization.isEmpty()
        && constantTimeEquals(authorization, expectedAuthorization);
}

} // namespace icpp
