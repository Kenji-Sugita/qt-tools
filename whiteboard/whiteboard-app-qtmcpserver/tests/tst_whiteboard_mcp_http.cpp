#include "BoardModel.h"
#include "CanvasWidget.h"
#include "WhiteboardMcpHttpServer.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTest>
#include <QTimer>

class WhiteboardMcpHttpTest : public QObject {
    Q_OBJECT

private slots:
    void streamableHttpLifecycleAndDiagramApply();
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
    QCOMPARE(tools.size(), 12);

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

QTEST_MAIN(WhiteboardMcpHttpTest)
#include "tst_whiteboard_mcp_http.moc"
