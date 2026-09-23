#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QRegularExpression>
#include <QTest>
#include <QTimer>

class McpHttpTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void authenticatedLifecycleAndExecutionGate();
    void persistentEvaluationAndRecovery();

private:
    struct HttpResult {
        int status = 0;
        QByteArray body;
        QByteArray sessionId;
    };

    bool startServer(bool allowExecution, const QString& interpreterPath);
    void stopServer();
    HttpResult sendRequest(const QByteArray& method,
                           const QJsonObject& body = QJsonObject(),
                           const QByteArray& sessionId = QByteArray(),
                           const QByteArray& protocolVersion = QByteArray(),
                           bool authenticate = true,
                           const QByteArray& origin = QByteArray());
    QJsonObject initializeMessage(int id = 1) const;
    QJsonObject requestMessage(int id,
                               const QString& method,
                               const QJsonObject& params = QJsonObject()) const;
    QJsonObject toolCall(int id, const QString& name, const QJsonObject& arguments) const;
    QJsonObject responseResult(const HttpResult& response) const;

    QString icppPath;
    QProcess serverProcess;
    QUrl endpoint;
    QNetworkAccessManager networkManager;
    const QByteArray token = QByteArrayLiteral("icpp-mcp-test-token-0123456789abcdef");
};

void McpHttpTest::initTestCase()
{
    icppPath = qEnvironmentVariable("ICPP_TEST_BINARY");
    QVERIFY2(!icppPath.isEmpty(), "ICPP_TEST_BINARY is required.");
    QVERIFY2(QFileInfo::exists(icppPath), qPrintable(icppPath));
}

void McpHttpTest::authenticatedLifecycleAndExecutionGate()
{
    QVERIFY(startServer(false, QStringLiteral("/usr/bin/false")));

    const HttpResult unauthorized = sendRequest(
        QByteArrayLiteral("POST"), initializeMessage(), {}, {}, false);
    QCOMPARE(unauthorized.status, 401);

    const HttpResult forbiddenOrigin = sendRequest(
        QByteArrayLiteral("POST"),
        initializeMessage(),
        {},
        {},
        true,
        QByteArrayLiteral("https://example.com"));
    QCOMPARE(forbiddenOrigin.status, 403);

    const HttpResult getResponse = sendRequest(QByteArrayLiteral("GET"));
    QCOMPARE(getResponse.status, 405);

    const HttpResult initialize = sendRequest(
        QByteArrayLiteral("POST"), initializeMessage());
    QCOMPARE(initialize.status, 200);
    QVERIFY(!initialize.sessionId.isEmpty());
    QCOMPARE(responseResult(initialize).value(QStringLiteral("protocolVersion")).toString(),
             QStringLiteral("2025-06-18"));

    const HttpResult secondInitialize = sendRequest(
        QByteArrayLiteral("POST"), initializeMessage(2));
    QCOMPARE(secondInitialize.status, 503);

    const QJsonObject initialized{
        {QStringLiteral("jsonrpc"), QStringLiteral("2.0")},
        {QStringLiteral("method"), QStringLiteral("notifications/initialized")},
        {QStringLiteral("params"), QJsonObject()},
    };
    const HttpResult notification = sendRequest(
        QByteArrayLiteral("POST"),
        initialized,
        initialize.sessionId,
        QByteArrayLiteral("2025-06-18"));
    QCOMPARE(notification.status, 202);

    const HttpResult toolsResponse = sendRequest(
        QByteArrayLiteral("POST"),
        requestMessage(3, QStringLiteral("tools/list")),
        initialize.sessionId,
        QByteArrayLiteral("2025-06-18"));
    QCOMPARE(toolsResponse.status, 200);
    const QJsonArray tools = responseResult(toolsResponse).value(QStringLiteral("tools")).toArray();
    QCOMPARE(tools.size(), 3);

    const HttpResult statusResponse = sendRequest(
        QByteArrayLiteral("POST"),
        toolCall(4, QStringLiteral("icpp/session/status"), QJsonObject()),
        initialize.sessionId,
        QByteArrayLiteral("2025-06-18"));
    QCOMPARE(statusResponse.status, 200);
    const QJsonObject statusResult = responseResult(statusResponse);
    QCOMPARE(statusResult.value(QStringLiteral("executionAllowed")).toBool(), false);
    QCOMPARE(statusResult.value(QStringLiteral("interpreterRunning")).toBool(), false);

    const HttpResult deniedEvaluation = sendRequest(
        QByteArrayLiteral("POST"),
        toolCall(5,
                 QStringLiteral("icpp/code/evaluate"),
                 QJsonObject{{QStringLiteral("source"), QStringLiteral("1 + 1")}}),
        initialize.sessionId,
        QByteArrayLiteral("2025-06-18"));
    QCOMPARE(deniedEvaluation.status, 200);
    const QJsonObject deniedBody = QJsonDocument::fromJson(deniedEvaluation.body).object();
    QCOMPARE(deniedBody.value(QStringLiteral("error")).toObject()
                 .value(QStringLiteral("code")).toInt(),
             -32001);

    const HttpResult deleteResponse = sendRequest(
        QByteArrayLiteral("DELETE"),
        {},
        initialize.sessionId,
        QByteArrayLiteral("2025-06-18"));
    QCOMPARE(deleteResponse.status, 204);

    stopServer();
}

void McpHttpTest::persistentEvaluationAndRecovery()
{
    const QString clingPath = qEnvironmentVariable(
        "ICPP_CLING", QStringLiteral("/usr/local/src/cling/build/bin/cling"));
    if (!QFileInfo(clingPath).isExecutable()) {
        QSKIP(qPrintable(QStringLiteral("cling is not executable: %1").arg(clingPath)));
    }
    QVERIFY(startServer(true, clingPath));

    const HttpResult initialize = sendRequest(
        QByteArrayLiteral("POST"), initializeMessage());
    QCOMPARE(initialize.status, 200);

    const QJsonObject initialized{
        {QStringLiteral("jsonrpc"), QStringLiteral("2.0")},
        {QStringLiteral("method"), QStringLiteral("notifications/initialized")},
        {QStringLiteral("params"), QJsonObject()},
    };
    QCOMPARE(sendRequest(QByteArrayLiteral("POST"),
                         initialized,
                         initialize.sessionId,
                         QByteArrayLiteral("2025-06-18")).status,
             202);

    const auto evaluate = [this, &initialize](int id,
                                               const QString& source,
                                               int timeout = 5000) {
        return sendRequest(
            QByteArrayLiteral("POST"),
            toolCall(id,
                     QStringLiteral("icpp/code/evaluate"),
                     QJsonObject{{QStringLiteral("source"), source},
                                 {QStringLiteral("timeoutMs"), timeout}}),
            initialize.sessionId,
            QByteArrayLiteral("2025-06-18"));
    };

    const HttpResult defineResult = evaluate(
        2, QStringLiteral("int mcpAdd(int a, int b) { return a + b; }"));
    QCOMPARE(defineResult.status, 200);
    QCOMPARE(responseResult(defineResult).value(QStringLiteral("completed")).toBool(), true);

    const HttpResult useResult = evaluate(
        3,
        QStringLiteral("#include <iostream>\nstd::cout << mcpAdd(2, 3) << std::endl;"));
    QCOMPARE(useResult.status, 200);
    const QJsonObject useEvaluation = responseResult(useResult);
    QCOMPARE(useEvaluation.value(QStringLiteral("completed")).toBool(), true);
    QCOMPARE(useEvaluation.value(QStringLiteral("stdout")).toString(), QStringLiteral("5\n"));
    QCOMPARE(useEvaluation.value(QStringLiteral("stderr")).toString(), QString());
    QCOMPARE(QString::fromUtf8(serverProcess.readAllStandardOutput()), QStringLiteral("5\n"));

    const HttpResult environmentResult = evaluate(
        4,
        QStringLiteral("#include <cstdlib>\n#include <iostream>\n"
                       "std::cout << (std::getenv(\"ICPP_MCP_TOKEN\") == nullptr) << std::endl;"));
    QCOMPARE(environmentResult.status, 200);
    QVERIFY(responseResult(environmentResult).value(QStringLiteral("stdout"))
                .toString().contains(QStringLiteral("1")));

    const HttpResult incompleteResult = evaluate(
        5, QStringLiteral("void incomplete() {"));
    QCOMPARE(incompleteResult.status, 200);
    QCOMPARE(QJsonDocument::fromJson(incompleteResult.body).object()
                 .value(QStringLiteral("error")).toObject()
                 .value(QStringLiteral("code")).toInt(),
             -32602);

    const HttpResult timeoutResult = evaluate(
        6, QStringLiteral("while (true) {}"), 100);
    QCOMPARE(timeoutResult.status, 200);
    const QJsonObject timeoutEvaluation = responseResult(timeoutResult);
    QCOMPARE(timeoutEvaluation.value(QStringLiteral("completed")).toBool(), false);
    QCOMPARE(timeoutEvaluation.value(QStringLiteral("timedOut")).toBool(), true);
    QCOMPARE(timeoutEvaluation.value(QStringLiteral("interpreterRunning")).toBool(), false);

    const HttpResult recoveryResult = evaluate(
        7,
        QStringLiteral("#include <iostream>\nstd::cout << 7 << std::endl;"));
    QCOMPARE(recoveryResult.status, 200);
    const QJsonObject recoveryEvaluation = responseResult(recoveryResult);
    QCOMPARE(recoveryEvaluation.value(QStringLiteral("completed")).toBool(), true);
    QVERIFY(recoveryEvaluation.value(QStringLiteral("stdout"))
                .toString().contains(QStringLiteral("7")));

    const HttpResult resetResult = sendRequest(
        QByteArrayLiteral("POST"),
        toolCall(8, QStringLiteral("icpp/session/reset"), QJsonObject()),
        initialize.sessionId,
        QByteArrayLiteral("2025-06-18"));
    QCOMPARE(resetResult.status, 200);
    QCOMPARE(responseResult(resetResult).value(QStringLiteral("reset")).toBool(), true);

    const HttpResult deleteResponse = sendRequest(
        QByteArrayLiteral("DELETE"),
        {},
        initialize.sessionId,
        QByteArrayLiteral("2025-06-18"));
    QCOMPARE(deleteResponse.status, 204);
    stopServer();
}

bool McpHttpTest::startServer(bool allowExecution, const QString& interpreterPath)
{
    stopServer();
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("ICPP_MCP_TOKEN"), QString::fromLatin1(token));
    environment.insert(QStringLiteral("ICPP_HELP_LANGUAGE"), QStringLiteral("en"));
    serverProcess.setProcessEnvironment(environment);

    QStringList arguments{
        QStringLiteral("--mcp-http"),
        QStringLiteral("--mcp-port"),
        QStringLiteral("0"),
        QStringLiteral("--engine"),
        QStringLiteral("cling"),
        QStringLiteral("--cling"),
        interpreterPath,
    };
    if (allowExecution) {
        arguments.append(QStringLiteral("--mcp-allow-execution"));
    }
    serverProcess.start(icppPath, arguments);
    if (!serverProcess.waitForStarted(5000)) {
        return false;
    }

    QByteArray errorOutput;
    QElapsedTimer elapsed;
    elapsed.start();
    const QRegularExpression endpointPattern(QStringLiteral("http://127\\.0\\.0\\.1:(\\d+)/mcp"));
    while (elapsed.elapsed() < 5000) {
        errorOutput.append(serverProcess.readAllStandardError());
        const QRegularExpressionMatch match = endpointPattern.match(QString::fromUtf8(errorOutput));
        if (match.hasMatch()) {
            endpoint = QUrl(match.captured(0));
            return endpoint.isValid();
        }
        if (serverProcess.state() == QProcess::NotRunning) {
            return false;
        }
        serverProcess.waitForReadyRead(100);
    }
    return false;
}

void McpHttpTest::stopServer()
{
    endpoint.clear();
    if (serverProcess.state() == QProcess::NotRunning) {
        return;
    }
    serverProcess.terminate();
    if (!serverProcess.waitForFinished(2000)) {
        serverProcess.kill();
        serverProcess.waitForFinished(2000);
    }
}

McpHttpTest::HttpResult McpHttpTest::sendRequest(const QByteArray& method,
                                                const QJsonObject& body,
                                                const QByteArray& sessionId,
                                                const QByteArray& protocolVersion,
                                                bool authenticate,
                                                const QByteArray& origin)
{
    QNetworkRequest request(endpoint);
    request.setRawHeader("Accept", "application/json, text/event-stream");
    if (authenticate) {
        request.setRawHeader("Authorization", QByteArrayLiteral("Bearer ") + token);
    }
    if (!sessionId.isEmpty()) {
        request.setRawHeader("Mcp-Session-Id", sessionId);
    }
    if (!protocolVersion.isEmpty()) {
        request.setRawHeader("MCP-Protocol-Version", protocolVersion);
    }
    if (!origin.isEmpty()) {
        request.setRawHeader("Origin", origin);
    }

    QNetworkReply* reply = nullptr;
    if (method == QByteArrayLiteral("POST")) {
        request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
        reply = networkManager.post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));
    } else if (method == QByteArrayLiteral("DELETE")) {
        reply = networkManager.sendCustomRequest(request, QByteArrayLiteral("DELETE"));
    } else {
        reply = networkManager.get(request);
    }

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(35000);
    loop.exec();

    HttpResult result;
    if (reply->isFinished()) {
        result.status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        result.body = reply->readAll();
        result.sessionId = reply->rawHeader("Mcp-Session-Id");
    }
    reply->deleteLater();
    return result;
}

QJsonObject McpHttpTest::initializeMessage(int id) const
{
    return requestMessage(
        id,
        QStringLiteral("initialize"),
        QJsonObject{
            {QStringLiteral("protocolVersion"), QStringLiteral("2025-06-18")},
            {QStringLiteral("capabilities"), QJsonObject()},
            {QStringLiteral("clientInfo"), QJsonObject{
                 {QStringLiteral("name"), QStringLiteral("icpp-test")},
                 {QStringLiteral("version"), QStringLiteral("1.0")},
             }},
        });
}

QJsonObject McpHttpTest::requestMessage(int id,
                                        const QString& method,
                                        const QJsonObject& params) const
{
    return {
        {QStringLiteral("jsonrpc"), QStringLiteral("2.0")},
        {QStringLiteral("id"), id},
        {QStringLiteral("method"), method},
        {QStringLiteral("params"), params},
    };
}

QJsonObject McpHttpTest::toolCall(int id,
                                  const QString& name,
                                  const QJsonObject& arguments) const
{
    return requestMessage(
        id,
        QStringLiteral("tools/call"),
        QJsonObject{
            {QStringLiteral("name"), name},
            {QStringLiteral("arguments"), arguments},
        });
}

QJsonObject McpHttpTest::responseResult(const HttpResult& response) const
{
    return QJsonDocument::fromJson(response.body).object()
        .value(QStringLiteral("result")).toObject();
}

QTEST_MAIN(McpHttpTest)
#include "tst_mcp_http.moc"
