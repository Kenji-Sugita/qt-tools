#include "SseMcpClient.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QNetworkRequest>
#include <QTimer>

SseMcpClient::SseMcpClient(QUrl url, QObject *parent)
    : QObject(parent)
    , m_sseUrl(std::move(url))
{
}

bool SseMcpClient::connectToServer(int timeoutMs, QString *error)
{
    if (m_initialized)
        return true;

    QNetworkRequest sseRequest(m_sseUrl);
    sseRequest.setRawHeader("Accept", "text/event-stream");
    m_sseReply = m_network.get(sseRequest);

    connect(m_sseReply, &QNetworkReply::readyRead, this, &SseMcpClient::readSse);
    connect(m_sseReply, &QNetworkReply::errorOccurred, this, [this](QNetworkReply::NetworkError) {
        m_lastError = m_sseReply ? m_sseReply->errorString() : QStringLiteral("SSE connection failed");
    });

    QEventLoop endpointLoop;
    QTimer endpointTimer;
    endpointTimer.setSingleShot(true);
    connect(this, &SseMcpClient::endpointReady, &endpointLoop, &QEventLoop::quit);
    connect(&endpointTimer, &QTimer::timeout, &endpointLoop, &QEventLoop::quit);
    endpointTimer.start(timeoutMs);
    endpointLoop.exec();

    if (m_postUrl.isEmpty()) {
        if (error)
            *error = m_lastError.isEmpty() ? QStringLiteral("SSE endpoint was not received") : m_lastError;
        return false;
    }

    QString initializeError;
    const QJsonObject result = request(QStringLiteral("initialize"),
                                       QJsonObject {
                                           {QStringLiteral("protocolVersion"),
                                            QStringLiteral("2024-11-05")},
                                           {QStringLiteral("capabilities"), QJsonObject {}},
                                           {QStringLiteral("clientInfo"),
                                            QJsonObject {
                                                {QStringLiteral("name"),
                                                 QStringLiteral("fileservermcp-client")},
                                                {QStringLiteral("version"), QStringLiteral("0.1.0")},
                                            }},
                                       },
                                       timeoutMs,
                                       &initializeError);
    if (!initializeError.isEmpty()) {
        if (error)
            *error = initializeError;
        return false;
    }

    Q_UNUSED(result);
    notify(QStringLiteral("notifications/initialized"));
    m_initialized = true;
    return true;
}

QJsonObject SseMcpClient::request(const QString &method,
                                  const QJsonObject &params,
                                  int timeoutMs,
                                  QString *error)
{
    const int id = ++m_nextId;
    QJsonObject payload {
        {QStringLiteral("jsonrpc"), QStringLiteral("2.0")},
        {QStringLiteral("id"), id},
        {QStringLiteral("method"), method},
    };
    if (!params.isEmpty())
        payload.insert(QStringLiteral("params"), params);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    m_waitingLoops[id] = &loop;
    postJson(payload);

    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(timeoutMs);
    loop.exec();

    m_waitingLoops.remove(id);
    if (!m_responses.contains(id)) {
        if (error)
            *error = QStringLiteral("Timed out waiting for response to %1").arg(method);
        return {};
    }

    const QJsonObject response = m_responses.take(id);
    if (response.contains(QStringLiteral("error"))) {
        if (error)
            *error = QString::fromUtf8(QJsonDocument(response.value(QStringLiteral("error")).toObject())
                                           .toJson(QJsonDocument::Compact));
        return {};
    }
    if (error)
        error->clear();
    return response.value(QStringLiteral("result")).toObject();
}

QJsonObject SseMcpClient::callTool(const QString &name,
                                   const QJsonObject &arguments,
                                   int timeoutMs,
                                   QString *error)
{
    return request(QStringLiteral("tools/call"),
                   QJsonObject {
                       {QStringLiteral("name"), name},
                       {QStringLiteral("arguments"), arguments},
                   },
                   timeoutMs,
                   error);
}

QUrl SseMcpClient::url() const
{
    return m_sseUrl;
}

bool SseMcpClient::isInitialized() const
{
    return m_initialized;
}

void SseMcpClient::readSse()
{
    m_sseBuffer += m_sseReply->readAll();
    while (true) {
        const qsizetype newline = m_sseBuffer.indexOf('\n');
        if (newline < 0)
            return;

        QByteArray line = m_sseBuffer.left(newline);
        m_sseBuffer.remove(0, newline + 1);
        if (line.endsWith('\r'))
            line.chop(1);

        if (line.isEmpty()) {
            dispatchSseEvent();
            continue;
        }
        if (line.startsWith("event:")) {
            m_eventName = QString::fromUtf8(line.mid(6)).trimmed();
        } else if (line.startsWith("data:")) {
            if (!m_eventData.isEmpty())
                m_eventData += '\n';
            m_eventData += line.mid(5).trimmed();
        }
    }
}

void SseMcpClient::dispatchSseEvent()
{
    if (m_eventName == QStringLiteral("endpoint")) {
        const QString endpoint = QString::fromUtf8(m_eventData).trimmed();
        m_postUrl = m_sseUrl.resolved(QUrl(endpoint));
        emit endpointReady();
    } else if (m_eventName == QStringLiteral("message") || m_eventName.isEmpty()) {
        QJsonParseError parseError {};
        const QJsonDocument document = QJsonDocument::fromJson(m_eventData, &parseError);
        if (parseError.error == QJsonParseError::NoError && document.isObject()) {
            const QJsonObject object = document.object();
            const int id = object.value(QStringLiteral("id")).toInt(-1);
            if (id >= 0) {
                m_responses[id] = object;
                if (m_waitingLoops.contains(id))
                    m_waitingLoops.value(id)->quit();
            }
        }
    }

    m_eventName.clear();
    m_eventData.clear();
}

void SseMcpClient::postJson(const QJsonObject &payload)
{
    QNetworkRequest request(m_postUrl.isEmpty() ? m_sseUrl : m_postUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    QNetworkReply *reply = m_network.post(request,
                                          QJsonDocument(payload).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, reply, &QObject::deleteLater);
}

void SseMcpClient::notify(const QString &method)
{
    postJson(QJsonObject {
        {QStringLiteral("jsonrpc"), QStringLiteral("2.0")},
        {QStringLiteral("method"), method},
    });
}
