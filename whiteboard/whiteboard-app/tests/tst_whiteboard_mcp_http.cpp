#include "BoardModel.h"
#include "CanvasWidget.h"
#include "WhiteboardMcpHttpServer.h"

#include <QEventLoop>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTest>
#include <QTemporaryDir>
#include <QTimer>

#ifndef WHITEBOARD_APP_VERSION
#define WHITEBOARD_APP_VERSION "0.0.0"
#endif

class WhiteboardMcpHttpTest : public QObject {
    Q_OBJECT

private slots:
    void streamableHttpLifecycleAndDiagramApply();
    void propagatesPersistenceCallbacksToExistingAndNewSessions();
};

namespace {
struct HttpResult {
    int status = 0;
    QByteArray body;
    QByteArray sessionId;
};

HttpResult sendRequest(QNetworkAccessManager *manager,
                       const QUrl &url,
                       const QByteArray &method,
                       const QJsonObject &body = QJsonObject(),
                       const QByteArray &sessionId = QByteArray(),
                       const QByteArray &protocolVersion = QByteArray(),
                       const QByteArray &origin = QByteArray())
{
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json, text/event-stream");
    if (!sessionId.isEmpty())
        request.setRawHeader("Mcp-Session-Id", sessionId);
    if (!protocolVersion.isEmpty())
        request.setRawHeader("MCP-Protocol-Version", protocolVersion);
    if (!origin.isEmpty())
        request.setRawHeader("Origin", origin);

    QNetworkReply *reply = nullptr;
    if (method == QByteArrayLiteral("POST")) {
        request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
        reply = manager->post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));
    } else if (method == QByteArrayLiteral("DELETE")) {
        reply = manager->sendCustomRequest(request, QByteArrayLiteral("DELETE"));
    } else {
        reply = manager->get(request);
    }

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    timeout.start(3000);
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

QJsonObject requestMessage(int id, const QString &method, const QJsonObject &params = QJsonObject())
{
    return QJsonObject{{QStringLiteral("jsonrpc"), QStringLiteral("2.0")},
                       {QStringLiteral("id"), id},
                       {QStringLiteral("method"), method},
                       {QStringLiteral("params"), params}};
}
}

void WhiteboardMcpHttpTest::streamableHttpLifecycleAndDiagramApply()
{
    BoardModel model;
    CanvasWidget canvas;
    canvas.setModel(&model);
    int changeCount = 0;
    int approvalCount = 0;
    WhiteboardMcpHttpServer server(
        &model,
        &canvas,
        [&changeCount]() { ++changeCount; },
        [&model](const QString &direction) {
            const int previousIndex = model.document().currentPageIndex();
            if (direction == QStringLiteral("next"))
                model.nextPage();
            else if (direction == QStringLiteral("previous"))
                model.previousPage();
            return model.document().currentPageIndex() != previousIndex;
        },
        [&approvalCount](const QString &, const QJsonObject &, QString *) {
            ++approvalCount;
            return true;
        });
    QVERIFY2(server.start(0), qPrintable(server.errorString()));
    QVERIFY(server.port() != 0);

    QNetworkAccessManager manager;
    const QUrl endpoint(server.endpointUrl());

    const HttpResult getResult = sendRequest(&manager, endpoint, QByteArrayLiteral("GET"));
    QCOMPARE(getResult.status, 405);

    const QJsonObject initialize = requestMessage(
        1,
        QStringLiteral("initialize"),
        QJsonObject{{QStringLiteral("protocolVersion"), QStringLiteral("2025-06-18")},
                    {QStringLiteral("capabilities"), QJsonObject()},
                    {QStringLiteral("clientInfo"), QJsonObject{{QStringLiteral("name"), QStringLiteral("test")},
                                                                 {QStringLiteral("version"), QStringLiteral("1.0")}}}});
    const HttpResult forbiddenResult = sendRequest(
        &manager,
        endpoint,
        QByteArrayLiteral("POST"),
        initialize,
        QByteArray(),
        QByteArray(),
        QByteArrayLiteral("https://example.com"));
    QCOMPARE(forbiddenResult.status, 403);

    const HttpResult initializeResult = sendRequest(
        &manager, endpoint, QByteArrayLiteral("POST"), initialize);
    QCOMPARE(initializeResult.status, 200);
    QVERIFY(!initializeResult.sessionId.isEmpty());
    const QJsonObject initializeResponse = QJsonDocument::fromJson(initializeResult.body).object();
    QCOMPARE(initializeResponse.value(QStringLiteral("result"))
                 .toObject().value(QStringLiteral("protocolVersion")).toString(),
             QStringLiteral("2025-06-18"));
    QCOMPARE(initializeResponse.value(QStringLiteral("result"))
                 .toObject()
                 .value(QStringLiteral("serverInfo"))
                 .toObject()
                 .value(QStringLiteral("version"))
                 .toString(),
             QStringLiteral(WHITEBOARD_APP_VERSION));

    const QJsonObject initialized{{QStringLiteral("jsonrpc"), QStringLiteral("2.0")},
                                  {QStringLiteral("method"), QStringLiteral("notifications/initialized")},
                                  {QStringLiteral("params"), QJsonObject()}};
    const HttpResult notificationResult = sendRequest(
        &manager,
        endpoint,
        QByteArrayLiteral("POST"),
        initialized,
        initializeResult.sessionId,
        QByteArrayLiteral("2025-06-18"));
    QCOMPARE(notificationResult.status, 202);
    QVERIFY(notificationResult.body.isEmpty());

    const HttpResult missingSessionResult = sendRequest(
        &manager,
        endpoint,
        QByteArrayLiteral("POST"),
        requestMessage(2, QStringLiteral("tools/list")));
    QCOMPARE(missingSessionResult.status, 400);

    const HttpResult toolsResult = sendRequest(
        &manager,
        endpoint,
        QByteArrayLiteral("POST"),
        requestMessage(3, QStringLiteral("tools/list")),
        initializeResult.sessionId,
        QByteArrayLiteral("2025-06-18"));
    QCOMPARE(toolsResult.status, 200);
    const QJsonArray tools = QJsonDocument::fromJson(toolsResult.body).object()
                                 .value(QStringLiteral("result")).toObject()
                                 .value(QStringLiteral("tools")).toArray();
    QCOMPARE(tools.size(), 16);

    const QJsonObject rectangle{
        {QStringLiteral("type"), QStringLiteral("rectangle")},
        {QStringLiteral("rect"), QJsonObject{{QStringLiteral("x"), 100},
                                               {QStringLiteral("y"), 100},
                                               {QStringLiteral("width"), 300},
                                               {QStringLiteral("height"), 180}}},
    };
    const HttpResult applyResult = sendRequest(
        &manager,
        endpoint,
        QByteArrayLiteral("POST"),
        requestMessage(
            4,
            QStringLiteral("tools/call"),
            QJsonObject{{QStringLiteral("name"), QStringLiteral("whiteboard/diagram/apply")},
                        {QStringLiteral("arguments"), QJsonObject{
                             {QStringLiteral("mode"), QStringLiteral("append_current")},
                             {QStringLiteral("elements"), QJsonArray{rectangle}},
                         }}}),
        initializeResult.sessionId,
        QByteArrayLiteral("2025-06-18"));
    QCOMPARE(applyResult.status, 200);
    QCOMPARE(approvalCount, 1);
    QCOMPARE(changeCount, 1);
    QCOMPARE(model.currentPage().elements().size(), 1);

    const HttpResult deleteWithoutVersionResult = sendRequest(
        &manager,
        endpoint,
        QByteArrayLiteral("DELETE"),
        QJsonObject(),
        initializeResult.sessionId);
    QCOMPARE(deleteWithoutVersionResult.status, 400);

    const HttpResult deleteResult = sendRequest(
        &manager,
        endpoint,
        QByteArrayLiteral("DELETE"),
        QJsonObject(),
        initializeResult.sessionId,
        QByteArrayLiteral("2025-06-18"));
    QCOMPARE(deleteResult.status, 204);

    const HttpResult expiredResult = sendRequest(
        &manager,
        endpoint,
        QByteArrayLiteral("POST"),
        requestMessage(5, QStringLiteral("tools/list")),
        initializeResult.sessionId,
        QByteArrayLiteral("2025-06-18"));
    QCOMPARE(expiredResult.status, 404);
}

void WhiteboardMcpHttpTest::propagatesPersistenceCallbacksToExistingAndNewSessions()
{
    BoardModel model;
    CanvasWidget canvas;
    canvas.setModel(&model);
    WhiteboardMcpHttpServer server(&model, &canvas, {}, {},
                                  [](const QString &, const QJsonObject &, QString *) { return true; });
    server.setPersistenceCallbacks(
        []() { return QJsonObject{{QStringLiteral("status"), QStringLiteral("restore_blocked")},
                                 {QStringLiteral("persisted"), false}}; },
        [](const QString &) { return QStringLiteral("restore_blocked"); },
        {});
    QVERIFY2(server.start(0), qPrintable(server.errorString()));
    QNetworkAccessManager manager;
    const QUrl endpoint(server.endpointUrl());
    const QJsonObject initialize = requestMessage(1, QStringLiteral("initialize"),
        QJsonObject{{QStringLiteral("protocolVersion"), QStringLiteral("2025-06-18")}});
    const HttpResult firstSession = sendRequest(&manager, endpoint, QByteArrayLiteral("POST"), initialize);
    QCOMPARE(firstSession.status, 200);
    QVERIFY(!firstSession.sessionId.isEmpty());
    const auto queryState = [&manager, &endpoint](const QByteArray &session) {
        return sendRequest(&manager, endpoint, QByteArrayLiteral("POST"),
            requestMessage(2, QStringLiteral("tools/call"),
                QJsonObject{{QStringLiteral("name"), QStringLiteral("whiteboard/state")}}),
            session, QByteArrayLiteral("2025-06-18"));
    };
    const HttpResult blocked = queryState(firstSession.sessionId);
    QCOMPARE(blocked.status, 200);
    const QJsonObject blockedResult = QJsonDocument::fromJson(blocked.body).object()
                                         .value(QStringLiteral("result")).toObject();
    QVERIFY(blockedResult.value(QStringLiteral("isError")).toBool());
    QCOMPARE(blockedResult.value(QStringLiteral("structuredContent")).toObject()
                 .value(QStringLiteral("error")).toObject().value(QStringLiteral("code")).toString(),
             QStringLiteral("restore_blocked"));
    QVERIFY(!blockedResult.contains(QStringLiteral("pageCount")));

    int outputGuardCalls = 0;
    server.setPersistenceCallbacks(
        []() { return QJsonObject{{QStringLiteral("status"), QStringLiteral("normal")},
                                 {QStringLiteral("persisted"), false}}; },
        [](const QString &) { return QString(); },
        [&outputGuardCalls](const QString &, QString *error) {
            ++outputGuardCalls;
            *error = QStringLiteral("The destination is reserved for the document.");
            return false;
        });
    const HttpResult existing = queryState(firstSession.sessionId);
    QCOMPARE(existing.status, 200);
    const QJsonObject existingResult = QJsonDocument::fromJson(existing.body).object()
                                          .value(QStringLiteral("result")).toObject();
    QCOMPARE(existingResult.value(QStringLiteral("isError")).toBool(true), false);
    QCOMPARE(existingResult.value(QStringLiteral("pageCount")).toInt(), 1);
    QCOMPARE(existingResult.value(QStringLiteral("persistence")).toObject()
                 .value(QStringLiteral("status")).toString(), QStringLiteral("normal"));

    const HttpResult secondSession = sendRequest(&manager, endpoint, QByteArrayLiteral("POST"), initialize);
    QCOMPARE(secondSession.status, 200);
    QVERIFY(secondSession.sessionId != firstSession.sessionId);
    const HttpResult fresh = queryState(secondSession.sessionId);
    QCOMPARE(fresh.status, 200);
    const QJsonObject freshResult = QJsonDocument::fromJson(fresh.body).object()
                                       .value(QStringLiteral("result")).toObject();
    QCOMPARE(freshResult.value(QStringLiteral("isError")).toBool(true), false);
    QCOMPARE(freshResult.value(QStringLiteral("persistence")).toObject()
                 .value(QStringLiteral("persisted")).toBool(true), false);
    QCOMPARE(model.document().pages().size(), 1);
    QVERIFY(!model.canUndo());

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString outputPath = directory.filePath(QStringLiteral("protected.svg"));
    model.addRectangle(QRectF(10, 20, 80, 60));
    for (const QByteArray &session : {firstSession.sessionId, secondSession.sessionId}) {
        const HttpResult rejectedOutput = sendRequest(&manager, endpoint, QByteArrayLiteral("POST"),
            requestMessage(3, QStringLiteral("tools/call"),
                QJsonObject{{QStringLiteral("name"), QStringLiteral("whiteboard/image/save")},
                            {QStringLiteral("arguments"), QJsonObject{
                                 {QStringLiteral("format"), QStringLiteral("svg")},
                                 {QStringLiteral("background"), QStringLiteral("transparent")},
                                 {QStringLiteral("scope"), QStringLiteral("page")},
                                 {QStringLiteral("filePath"), outputPath},
                                 {QStringLiteral("overwrite"), true},
                             }}}),
            session, QByteArrayLiteral("2025-06-18"));
        QCOMPARE(rejectedOutput.status, 200);
        QVERIFY(QJsonDocument::fromJson(rejectedOutput.body).object().contains(QStringLiteral("error")));
        QVERIFY(!QFileInfo::exists(outputPath));
    }
    QCOMPARE(outputGuardCalls, 2);
}

QTEST_MAIN(WhiteboardMcpHttpTest)
#include "tst_whiteboard_mcp_http.moc"
