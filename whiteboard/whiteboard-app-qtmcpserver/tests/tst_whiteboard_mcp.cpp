#include "BoardModel.h"
#include "CanvasWidget.h"
#include "WhiteboardMcpController.h"

#include <QBuffer>
#include <QFile>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTest>
#include <QTemporaryDir>

#include <cstring>

class McpInputDevice : public QIODevice {
public:
    bool isSequential() const override
    {
        return true;
    }

    qint64 bytesAvailable() const override
    {
        return QIODevice::bytesAvailable() + m_buffer.size();
    }

    void appendMessage(const QJsonObject &message)
    {
        m_buffer.append(QJsonDocument(message).toJson(QJsonDocument::Compact));
        m_buffer.append('\n');
        emit readyRead();
    }

protected:
    qint64 readData(char *data, qint64 maxSize) override
    {
        if (maxSize <= 0 || m_buffer.isEmpty())
            return 0;
        const qint64 size = qMin(maxSize, static_cast<qint64>(m_buffer.size()));
        memcpy(data, m_buffer.constData(), static_cast<size_t>(size));
        m_buffer.remove(0, static_cast<int>(size));
        return size;
    }

    qint64 writeData(const char *, qint64) override
    {
        return -1;
    }

private:
    QByteArray m_buffer;
};

class WhiteboardMcpTest : public QObject {
    Q_OBJECT

private slots:
    void exposesOnlyWhiteboardTools();
    void listsAndNavigatesPages();
    void managesPagesAndHistory();
    void exportsAndSavesImages();
    void appliesRendersAndUndoesDiagram();
    void rejectsInvalidDiagramWithoutMutation();
    void rejectsChangesToLockedPage();
    void rejectsUnapprovedChange();
};

namespace {
void resetOutput(QBuffer *output)
{
    output->buffer().clear();
    output->seek(0);
}

QJsonObject sendMessage(McpInputDevice *input, QBuffer *output, const QJsonObject &message)
{
    resetOutput(output);
    input->appendMessage(message);
    const QByteArray line = output->data().split('\n').first();
    return QJsonDocument::fromJson(line).object();
}

void initializeServer(McpInputDevice *input, QBuffer *output)
{
    const QJsonObject response = sendMessage(
        input,
        output,
        QJsonObject{{QStringLiteral("jsonrpc"), QStringLiteral("2.0")},
                    {QStringLiteral("id"), 1},
                    {QStringLiteral("method"), QStringLiteral("initialize")},
                    {QStringLiteral("params"), QJsonObject()}});
    QVERIFY(response.contains(QStringLiteral("result")));

    resetOutput(output);
    input->appendMessage(
        QJsonObject{{QStringLiteral("jsonrpc"), QStringLiteral("2.0")},
                    {QStringLiteral("method"), QStringLiteral("notifications/initialized")},
                    {QStringLiteral("params"), QJsonObject()}});
    QVERIFY(output->data().isEmpty());
}

QJsonObject callTool(McpInputDevice *input,
                     QBuffer *output,
                     int id,
                     const QString &name,
                     const QJsonObject &arguments = QJsonObject())
{
    return sendMessage(
        input,
        output,
        QJsonObject{{QStringLiteral("jsonrpc"), QStringLiteral("2.0")},
                    {QStringLiteral("id"), id},
                    {QStringLiteral("method"), QStringLiteral("tools/call")},
                    {QStringLiteral("params"), QJsonObject{{QStringLiteral("name"), name},
                                                            {QStringLiteral("arguments"), arguments}}}});
}

QJsonObject rectangleElement(int x, int y, int width, int height)
{
    return {
        {QStringLiteral("type"), QStringLiteral("rectangle")},
        {QStringLiteral("rect"), QJsonObject{{QStringLiteral("x"), x},
                                              {QStringLiteral("y"), y},
                                              {QStringLiteral("width"), width},
                                              {QStringLiteral("height"), height}}},
        {QStringLiteral("color"), QStringLiteral("#344054")},
        {QStringLiteral("fillColor"), QStringLiteral("#d1fae5")},
    };
}

struct McpFixture {
    BoardModel model;
    CanvasWidget canvas;
    McpInputDevice input;
    QBuffer output;
    int changeCount = 0;
    int approvalCount = 0;
    bool approveChanges = true;
    WhiteboardMcpController controller;

    McpFixture()
        : controller(
            &model,
            &canvas,
            [this]() { ++changeCount; },
            [this](const QString &direction) {
                const int previousIndex = model.document().currentPageIndex();
                if (direction == QStringLiteral("next"))
                    model.nextPage();
                else if (direction == QStringLiteral("previous"))
                    model.previousPage();
                return model.document().currentPageIndex() != previousIndex;
            },
            [this](const QString &, const QJsonObject &, QString *denyReason) {
                ++approvalCount;
                if (!approveChanges && denyReason)
                    *denyReason = QStringLiteral("Declined for test.");
                return approveChanges;
            })
    {
        canvas.setModel(&model);
        input.open(QIODevice::ReadOnly);
        output.open(QIODevice::ReadWrite);
        controller.setInputDevice(&input);
        controller.setOutputDevice(&output);
    }

    void start()
    {
        QVERIFY(controller.start());
        initializeServer(&input, &output);
    }
};
}

void WhiteboardMcpTest::exposesOnlyWhiteboardTools()
{
    McpFixture fixture;
    fixture.start();

    const QJsonObject response = sendMessage(
        &fixture.input,
        &fixture.output,
        QJsonObject{{QStringLiteral("jsonrpc"), QStringLiteral("2.0")},
                    {QStringLiteral("id"), 2},
                    {QStringLiteral("method"), QStringLiteral("tools/list")}});
    const QJsonArray tools = response.value(QStringLiteral("result"))
                                 .toObject()
                                 .value(QStringLiteral("tools"))
                                 .toArray();
    QCOMPARE(tools.size(), 12);

    QStringList names;
    for (const QJsonValue &tool : tools)
        names.append(tool.toObject().value(QStringLiteral("name")).toString());
    QCOMPARE(names,
             QStringList({QStringLiteral("whiteboard/diagram/apply"),
                          QStringLiteral("whiteboard/history/redo"),
                          QStringLiteral("whiteboard/history/undo"),
                          QStringLiteral("whiteboard/image/export"),
                          QStringLiteral("whiteboard/image/save"),
                          QStringLiteral("whiteboard/page/add"),
                          QStringLiteral("whiteboard/page/delete"),
                          QStringLiteral("whiteboard/page/lock"),
                          QStringLiteral("whiteboard/page/navigate"),
                          QStringLiteral("whiteboard/page/render"),
                          QStringLiteral("whiteboard/pages/list"),
                          QStringLiteral("whiteboard/state")}));

    const QJsonObject stateResponse = callTool(
        &fixture.input, &fixture.output, 3, QStringLiteral("whiteboard/state"));
    const QJsonObject state = stateResponse.value(QStringLiteral("result")).toObject();
    QCOMPARE(state.value(QStringLiteral("pageCount")).toInt(), 1);
    QCOMPARE(state.value(QStringLiteral("currentPageIndex")).toInt(), 0);
    QCOMPARE(state.value(QStringLiteral("canvas")).toObject().value(QStringLiteral("width")).toInt(), 1920);
    QCOMPARE(state.value(QStringLiteral("canvas")).toObject().value(QStringLiteral("height")).toInt(), 1080);
    QVERIFY(!state.value(QStringLiteral("canPreviousPage")).toBool());
    QVERIFY(!state.value(QStringLiteral("canNextPage")).toBool());
    QVERIFY(!state.value(QStringLiteral("canRedo")).toBool());
    QVERIFY(state.value(QStringLiteral("selectedElementIndexes")).toArray().isEmpty());
}

void WhiteboardMcpTest::listsAndNavigatesPages()
{
    McpFixture fixture;
    QVERIFY(fixture.model.addPage());
    fixture.model.addRectangle(QRectF(10, 20, 80, 60));
    QVERIFY(fixture.model.addPage());
    QVERIFY(fixture.model.setCurrentPageLocked(true));
    fixture.start();

    const QJsonObject listResponse = callTool(
        &fixture.input, &fixture.output, 20, QStringLiteral("whiteboard/pages/list"));
    const QJsonObject listResult = listResponse.value(QStringLiteral("result")).toObject();
    QCOMPARE(listResult.value(QStringLiteral("pageCount")).toInt(), 3);
    QCOMPARE(listResult.value(QStringLiteral("currentPageIndex")).toInt(), 2);
    const QJsonArray pages = listResult.value(QStringLiteral("pages")).toArray();
    QCOMPARE(pages.size(), 3);
    QCOMPARE(pages.at(1).toObject().value(QStringLiteral("elementCount")).toInt(), 1);
    QVERIFY(pages.at(2).toObject().value(QStringLiteral("locked")).toBool());

    const QJsonObject previousResponse = callTool(
        &fixture.input,
        &fixture.output,
        21,
        QStringLiteral("whiteboard/page/navigate"),
        QJsonObject{{QStringLiteral("direction"), QStringLiteral("previous")}});
    QVERIFY(!previousResponse.contains(QStringLiteral("error")));
    QCOMPARE(fixture.model.document().currentPageIndex(), 1);

    const QJsonObject nextResponse = callTool(
        &fixture.input,
        &fixture.output,
        22,
        QStringLiteral("whiteboard/page/navigate"),
        QJsonObject{{QStringLiteral("direction"), QStringLiteral("next")}});
    QVERIFY(!nextResponse.contains(QStringLiteral("error")));
    QCOMPARE(fixture.model.document().currentPageIndex(), 2);

    const QJsonObject boundaryResponse = callTool(
        &fixture.input,
        &fixture.output,
        23,
        QStringLiteral("whiteboard/page/navigate"),
        QJsonObject{{QStringLiteral("direction"), QStringLiteral("next")}});
    QVERIFY(boundaryResponse.contains(QStringLiteral("error")));
    QCOMPARE(fixture.changeCount, 0);
    QCOMPARE(fixture.approvalCount, 0);
}

void WhiteboardMcpTest::managesPagesAndHistory()
{
    McpFixture fixture;
    fixture.start();

    const QJsonObject addResponse = callTool(
        &fixture.input, &fixture.output, 30, QStringLiteral("whiteboard/page/add"));
    QVERIFY(!addResponse.contains(QStringLiteral("error")));
    QCOMPARE(fixture.model.document().pages().size(), 2);
    QCOMPARE(fixture.model.document().currentPageIndex(), 1);

    const QJsonObject undoResponse = callTool(
        &fixture.input, &fixture.output, 31, QStringLiteral("whiteboard/history/undo"));
    QVERIFY(!undoResponse.contains(QStringLiteral("error")));
    QCOMPARE(fixture.model.document().pages().size(), 1);

    const QJsonObject redoResponse = callTool(
        &fixture.input, &fixture.output, 32, QStringLiteral("whiteboard/history/redo"));
    QVERIFY(!redoResponse.contains(QStringLiteral("error")));
    QCOMPARE(fixture.model.document().pages().size(), 2);
    QCOMPARE(fixture.model.document().currentPageIndex(), 1);

    const QJsonObject lockResponse = callTool(
        &fixture.input,
        &fixture.output,
        33,
        QStringLiteral("whiteboard/page/lock"),
        QJsonObject{{QStringLiteral("locked"), true}});
    QVERIFY(!lockResponse.contains(QStringLiteral("error")));
    QVERIFY(fixture.model.currentPageLocked());

    const QJsonObject lockedDeleteResponse = callTool(
        &fixture.input, &fixture.output, 34, QStringLiteral("whiteboard/page/delete"));
    QVERIFY(lockedDeleteResponse.contains(QStringLiteral("error")));
    QCOMPARE(fixture.model.document().pages().size(), 2);

    const QJsonObject unlockResponse = callTool(
        &fixture.input,
        &fixture.output,
        35,
        QStringLiteral("whiteboard/page/lock"),
        QJsonObject{{QStringLiteral("locked"), false}});
    QVERIFY(!unlockResponse.contains(QStringLiteral("error")));

    const QJsonObject deleteResponse = callTool(
        &fixture.input, &fixture.output, 36, QStringLiteral("whiteboard/page/delete"));
    QVERIFY(!deleteResponse.contains(QStringLiteral("error")));
    QCOMPARE(fixture.model.document().pages().size(), 1);
    QCOMPARE(fixture.changeCount, 6);
    QCOMPARE(fixture.approvalCount, 7);
}

void WhiteboardMcpTest::exportsAndSavesImages()
{
    McpFixture fixture;
    fixture.model.addRectangle(QRectF(10, 20, 80, 60));
    fixture.start();

    const QJsonObject pngArguments{
        {QStringLiteral("format"), QStringLiteral("png")},
        {QStringLiteral("background"), QStringLiteral("transparent")},
        {QStringLiteral("scope"), QStringLiteral("page")},
    };
    const QJsonObject pngResponse = callTool(
        &fixture.input,
        &fixture.output,
        40,
        QStringLiteral("whiteboard/image/export"),
        pngArguments);
    const QJsonObject pngResult = pngResponse.value(QStringLiteral("result")).toObject();
    const QJsonObject pngContent = pngResult.value(QStringLiteral("content")).toArray().first().toObject();
    QCOMPARE(pngContent.value(QStringLiteral("mimeType")).toString(), QStringLiteral("image/png"));
    const QImage pngImage = QImage::fromData(
        QByteArray::fromBase64(pngContent.value(QStringLiteral("data")).toString().toLatin1()), "PNG");
    QVERIFY(!pngImage.isNull());

    QJsonObject svgArguments = pngArguments;
    svgArguments.insert(QStringLiteral("format"), QStringLiteral("svg"));
    svgArguments.insert(QStringLiteral("background"), QStringLiteral("white"));
    const QJsonObject svgResponse = callTool(
        &fixture.input,
        &fixture.output,
        41,
        QStringLiteral("whiteboard/image/export"),
        svgArguments);
    const QJsonObject svgContent = svgResponse.value(QStringLiteral("result")).toObject()
                                       .value(QStringLiteral("content")).toArray().first().toObject();
    QCOMPARE(svgContent.value(QStringLiteral("mimeType")).toString(), QStringLiteral("image/svg+xml"));
    const QByteArray svg = QByteArray::fromBase64(
        svgContent.value(QStringLiteral("data")).toString().toLatin1());
    QVERIFY(svg.startsWith("<?xml"));

    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());
    const QString filePath = temporaryDir.filePath(QStringLiteral("mcp-export.svg"));
    svgArguments.insert(QStringLiteral("filePath"), filePath);
    svgArguments.insert(QStringLiteral("overwrite"), false);
    const QJsonObject saveResponse = callTool(
        &fixture.input,
        &fixture.output,
        42,
        QStringLiteral("whiteboard/image/save"),
        svgArguments);
    QVERIFY(!saveResponse.contains(QStringLiteral("error")));
    QFile savedFile(filePath);
    QVERIFY(savedFile.open(QIODevice::ReadOnly));
    QVERIFY(savedFile.readAll().startsWith("<?xml"));

    const QJsonObject overwriteResponse = callTool(
        &fixture.input,
        &fixture.output,
        43,
        QStringLiteral("whiteboard/image/save"),
        svgArguments);
    QVERIFY(overwriteResponse.contains(QStringLiteral("error")));
    QCOMPARE(fixture.changeCount, 0);
    QCOMPARE(fixture.approvalCount, 2);
}

void WhiteboardMcpTest::appliesRendersAndUndoesDiagram()
{
    McpFixture fixture;
    fixture.start();

    QJsonObject textElement{
        {QStringLiteral("type"), QStringLiteral("text")},
        {QStringLiteral("rect"), QJsonObject{{QStringLiteral("x"), 160},
                                              {QStringLiteral("y"), 160},
                                              {QStringLiteral("width"), 280},
                                              {QStringLiteral("height"), 80}}},
        {QStringLiteral("text"), QStringLiteral("Generated diagram")},
        {QStringLiteral("fontPointSize"), 24},
    };
    const QJsonObject applyResponse = callTool(
        &fixture.input,
        &fixture.output,
        4,
        QStringLiteral("whiteboard/diagram/apply"),
        QJsonObject{{QStringLiteral("mode"), QStringLiteral("new_page")},
                    {QStringLiteral("elements"), QJsonArray{rectangleElement(100, 100, 400, 220), textElement}}});
    QVERIFY(!applyResponse.contains(QStringLiteral("error")));
    QCOMPARE(fixture.model.document().pages().size(), 2);
    QCOMPARE(fixture.model.document().currentPageIndex(), 1);
    QCOMPARE(fixture.model.currentPage().elements().size(), 2);
    QCOMPARE(fixture.changeCount, 1);

    const QJsonObject renderResponse = callTool(
        &fixture.input, &fixture.output, 5, QStringLiteral("whiteboard/page/render"));
    const QJsonObject renderResult = renderResponse.value(QStringLiteral("result")).toObject();
    const QJsonArray content = renderResult.value(QStringLiteral("content")).toArray();
    QCOMPARE(content.size(), 1);
    QCOMPARE(content.first().toObject().value(QStringLiteral("type")).toString(), QStringLiteral("image"));
    const QByteArray png = QByteArray::fromBase64(
        content.first().toObject().value(QStringLiteral("data")).toString().toLatin1());
    const QImage image = QImage::fromData(png, "PNG");
    QCOMPARE(image.size(), QSize(1920, 1080));

    const QJsonObject undoResponse = callTool(
        &fixture.input, &fixture.output, 6, QStringLiteral("whiteboard/history/undo"));
    QVERIFY(!undoResponse.contains(QStringLiteral("error")));
    QCOMPARE(fixture.model.document().pages().size(), 1);
    QCOMPARE(fixture.changeCount, 2);
}

void WhiteboardMcpTest::rejectsInvalidDiagramWithoutMutation()
{
    McpFixture fixture;
    fixture.start();

    const QJsonObject response = callTool(
        &fixture.input,
        &fixture.output,
        7,
        QStringLiteral("whiteboard/diagram/apply"),
        QJsonObject{{QStringLiteral("mode"), QStringLiteral("append_current")},
                    {QStringLiteral("elements"), QJsonArray{rectangleElement(1900, 100, 100, 100)}}});
    QVERIFY(response.contains(QStringLiteral("error")));
    QVERIFY(fixture.model.currentPage().elements().isEmpty());
    QVERIFY(!fixture.model.canUndo());
    QCOMPARE(fixture.changeCount, 0);

    QJsonObject invalidStyle = rectangleElement(100, 100, 100, 100);
    invalidStyle.insert(QStringLiteral("strokeWidth"), QStringLiteral("3"));
    const QJsonObject typeResponse = callTool(
        &fixture.input,
        &fixture.output,
        10,
        QStringLiteral("whiteboard/diagram/apply"),
        QJsonObject{{QStringLiteral("mode"), QStringLiteral("append_current")},
                    {QStringLiteral("elements"), QJsonArray{invalidStyle}}});
    QVERIFY(typeResponse.contains(QStringLiteral("error")));
    QVERIFY(fixture.model.currentPage().elements().isEmpty());
    QVERIFY(!fixture.model.canUndo());
}

void WhiteboardMcpTest::rejectsChangesToLockedPage()
{
    McpFixture fixture;
    QVERIFY(fixture.model.setCurrentPageLocked(true));
    fixture.start();

    const QJsonObject response = callTool(
        &fixture.input,
        &fixture.output,
        8,
        QStringLiteral("whiteboard/diagram/apply"),
        QJsonObject{{QStringLiteral("mode"), QStringLiteral("append_current")},
                    {QStringLiteral("elements"), QJsonArray{rectangleElement(100, 100, 100, 100)}}});
    QVERIFY(response.contains(QStringLiteral("error")));
    QVERIFY(fixture.model.currentPage().elements().isEmpty());
    QCOMPARE(fixture.changeCount, 0);
}

void WhiteboardMcpTest::rejectsUnapprovedChange()
{
    McpFixture fixture;
    fixture.start();
    fixture.approveChanges = false;

    const QJsonObject response = callTool(
        &fixture.input,
        &fixture.output,
        9,
        QStringLiteral("whiteboard/diagram/apply"),
        QJsonObject{{QStringLiteral("mode"), QStringLiteral("append_current")},
                    {QStringLiteral("elements"), QJsonArray{rectangleElement(100, 100, 200, 120)}}});
    QVERIFY(response.contains(QStringLiteral("error")));
    QCOMPARE(fixture.approvalCount, 1);
    QCOMPARE(fixture.changeCount, 0);
    QVERIFY(fixture.model.currentPage().elements().isEmpty());
    QVERIFY(!fixture.model.canUndo());
}

QTEST_MAIN(WhiteboardMcpTest)
#include "tst_whiteboard_mcp.moc"
