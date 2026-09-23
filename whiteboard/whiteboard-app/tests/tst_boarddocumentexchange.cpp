#include "BoardDocumentExchange.h"
#include "BoardModel.h"

#include <QBuffer>
#include <QFile>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSet>
#include <QTemporaryDir>
#include <QTest>

namespace {
BoardDocument sample()
{
    BoardModel model;
    model.addRectangle(QRectF(100, 100, 80, 50));
    model.addText(QPointF(300, 200), QStringLiteral("日本語\nDrawing data"));
    model.addLine(QPointF(140, 125), QPointF(310, 210));
    QImage image(8, 8, QImage::Format_ARGB32);
    image.fill(Qt::cyan);
    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    model.addImage(QRectF(400, 400, 60, 40), png, QStringLiteral("image/png"));
    auto document = model.document();
    auto pages = document.pages();
    auto elements = pages[0].elements();
    elements[0].setGroupId(12);
    elements[1].setGroupId(12);
    elements[0].setRotationDegrees(30);
    elements[2].setStartConnectionElementId(elements[0].id());
    elements[2].setEndConnectionElementId(elements[1].id());
    pages[0].setElements(elements);
    pages[0].setZoomPercent(150);
    pages[0].setScrollPosition(QPoint(20, 30));
    pages[0].setLocked(true);
    Page empty;
    pages.append(empty);
    document.setPages(pages);
    document.setCurrentPageIndex(1);
    return document;
}

QByteArray encode(const QJsonObject &object)
{
    return QJsonDocument(object).toJson();
}
}

class BoardDocumentExchangeTest : public QObject {
    Q_OBJECT
private slots:
    void roundTripAndScope();
    void importRemapsIdsAndIsOneUndo();
    void rejectionDoesNotChangeModel();
    void malformed_data();
    void malformed();
    void legacyWithoutIds();
    void exportFailurePreservesExistingFile();
    void allElementTypesRoundTrip();
};

void BoardDocumentExchangeTest::roundTripAndScope()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const auto document = sample();
    QString error;
    const auto path = directory.filePath("drawing.whiteboard");
    QVERIFY2(BoardDocumentExchange::write(path, document, BoardDocumentExchange::Scope::AllPages, &error), qPrintable(error));
    QVector<Page> pages;
    QVERIFY2(BoardDocumentExchange::read(path, &pages, &error), qPrintable(error));
    QCOMPARE(pages.size(), 2);
    for (qsizetype i = 0; i < pages.size(); ++i) QCOMPARE(pages[i].toJson(), document.pages()[i].toJson());
    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    const auto root = QJsonDocument::fromJson(file.readAll()).object();
    QCOMPARE(root.value("format").toString(), QStringLiteral("WhiteboardApp.DrawingData"));
    QCOMPARE(root.value("formatVersion").toInt(), 1);
    QVERIFY(!root.contains("settings"));
    QVERIFY(!root.contains("revision"));
    file.close();
    QVERIFY(BoardDocumentExchange::write(path, document, BoardDocumentExchange::Scope::CurrentPage, &error));
    QVERIFY(BoardDocumentExchange::read(path, &pages, &error));
    QCOMPARE(pages.size(), 1);
    QCOMPARE(pages.first().toJson(), document.currentPage().toJson());
}

void BoardDocumentExchangeTest::importRemapsIdsAndIsOneUndo()
{
    BoardModel model;
    model.addRectangle(QRectF(10, 10, 40, 40));
    model.selectTool(Tool::Select);
    const auto original = model.document();
    QString error;
    QVERIFY2(model.importPages(sample().pages(), &error), qPrintable(error));
    QCOMPARE(model.document().pages().size(), 3);
    QCOMPARE(model.document().currentPageIndex(), original.currentPageIndex());
    QCOMPARE(model.settings().toJson(), original.settings().toJson());
    QCOMPARE(model.currentPage().toJson(), original.currentPage().toJson());
    QCOMPARE(model.document().revision(), original.revision() + 1);
    const auto imported = model.document().pages()[1];
    QVERIFY(imported.locked());
    const auto elements = imported.elements();
    QCOMPARE(elements[2].startConnectionElementId(), elements[0].id());
    QCOMPARE(elements[2].endConnectionElementId(), elements[1].id());
    const auto sourceElements = sample().pages()[0].elements();
    for (qsizetype i = 0; i < elements.size(); ++i) {
        auto actual = elements[i].toJson();
        auto expected = sourceElements[i].toJson();
        for (const char *key : {"id", "startConnectionElementId", "endConnectionElementId"}) {
            actual.remove(key); expected.remove(key);
        }
        QCOMPARE(actual, expected);
    }
    const int nextId = model.document().nextElementId();
    QVERIFY(model.canUndo());
    QVERIFY(model.undo());
    QCOMPARE(model.document().pages().size(), 1);
    QCOMPARE(model.currentPage().toJson(), original.currentPage().toJson());
    QVERIFY(model.document().nextElementId() >= nextId);
    QVERIFY(model.redo());
    QCOMPARE(model.document().pages()[1].toJson(), imported.toJson());
    QVERIFY(model.importPages(sample().pages(), &error));
    QSet<QString> ids;
    for (const Page &page : model.document().pages()) {
        for (const auto &element : page.elements()) {
            QVERIFY(!ids.contains(element.id()));
            ids.insert(element.id());
        }
    }
    // A subsequent import after Undo must also allocate new IDs.
    const auto previousIds = ids;
    QVERIFY(model.undo());
    QVERIFY(model.importPages(sample().pages(), &error));
    for (const auto &element : model.document().pages()[1].elements()) QVERIFY(!previousIds.contains(element.id()));
}

void BoardDocumentExchangeTest::rejectionDoesNotChangeModel()
{
    BoardModel model;
    model.addRectangle(QRectF(1, 1, 30, 30));
    QString error;
    const auto before = model.document().toJson();
    QVERIFY(!model.importPages(QVector<Page>(BoardModel::MaxPages), &error));
    QCOMPARE(model.document().toJson(), before);
    QVERIFY(!model.importPages({}, &error));
    QCOMPARE(model.document().toJson(), before);
    auto pages = sample().pages();
    auto elements = pages[0].elements();
    elements[2].setStartConnectionElementId("missing");
    pages[0].setElements(elements);
    QVERIFY(!model.importPages(pages, &error));
    QCOMPARE(model.document().toJson(), before);
    QVERIFY(model.undo()); // Failed operations did not add undo entries.
    QVERIFY(model.currentPage().elements().isEmpty());
    QVERIFY(model.canRedo());
    QVERIFY(!model.importPages(pages, &error));
    QVERIFY(model.canRedo());
    model.setCurrentPageLocked(true);
    const auto locked = model.document().toJson();
    QVERIFY(!model.importPages(sample().pages(), &error));
    QCOMPARE(model.document().toJson(), locked);
    model.setCurrentPageLocked(false);
    QVERIFY(model.importPages(QVector<Page>(BoardModel::MaxPages - 1), &error));
    QCOMPARE(model.document().pages().size(), BoardModel::MaxPages);
}

void BoardDocumentExchangeTest::malformed_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::newRow("broken-json") << QByteArray("{");
    QTest::newRow("unrelated-object") << QByteArray("{}");
    QTest::newRow("empty-pages") << QByteArray("{\"pages\":[]}");
    QTest::newRow("non-page") << QByteArray("{\"pages\":[null]}");
    auto root = sample().toJson();
    auto bad = root;
    bad.insert("format", "WhiteboardApp.DrawingData");
    bad.insert("formatVersion", 2);
    QTest::newRow("future-version") << encode(bad);
    bad.insert("formatVersion", 1.5);
    QTest::newRow("fractional-version") << encode(bad);
    bad.insert("format", "other");
    bad.insert("formatVersion", 1);
    QTest::newRow("unknown-format") << encode(bad);
    QJsonArray tooMany;
    for (int i = 0; i <= BoardDocument::MaxPages; ++i) tooMany.append(Page().toJson());
    bad = root; bad.insert("pages", tooMany);
    QTest::newRow("too-many-pages") << encode(bad);
    const auto mutate = [&](const char *name, int index, const QString &key, const QJsonValue &value) {
        auto pages = root.value("pages").toArray();
        auto page = pages[0].toObject();
        auto elements = page.value("elements").toArray();
        auto element = elements[index].toObject();
        element.insert(key, value); elements[index] = element;
        page.insert("elements", elements); pages[0] = page;
        auto changed = root; changed.insert("pages", pages);
        QTest::newRow(name) << encode(changed);
    };
    mutate("unknown-type", 0, "type", "unknown");
    mutate("wrong-type", 0, "closed", "false");
    mutate("negative-width", 0, "strokeWidth", -1);
    mutate("invalid-color", 0, "color", "no-such-color");
    mutate("duplicate-id", 1, "id", "element-1");
    mutate("missing-target", 2, "endConnectionElementId", "missing");
    mutate("self-reference", 2, "endConnectionElementId", "element-3");
    mutate("corrupt-png", 3, "imageData", "AAAA");
    mutate("bad-base64", 3, "imageData", "%%%%");
    mutate("bad-points", 2, "points", QJsonArray{QJsonObject{{"x", "no"}, {"y", 0}}});
}

void BoardDocumentExchangeTest::malformed()
{
    QFETCH(QByteArray, data);
    auto pages = sample().pages();
    const auto before = pages[0].toJson();
    QString error;
    QVERIFY(!BoardDocumentExchange::decode(data, &pages, &error));
    QVERIFY(!error.isEmpty());
    QCOMPARE(pages.size(), 2);
    QCOMPARE(pages[0].toJson(), before);
}

void BoardDocumentExchangeTest::legacyWithoutIds()
{
    BoardModel source;
    source.addRectangle(QRectF(30, 40, 50, 60));
    auto root = source.document().toJson();
    auto page = root.value("pages").toArray()[0].toObject();
    auto element = page.value("elements").toArray()[0].toObject();
    for (const char *key : {"id", "rotationDegrees", "startConnectionElementId", "endConnectionElementId"}) element.remove(key);
    page.remove("zoomPercent");
    page.insert("elements", QJsonArray{element});
    root.insert("pages", QJsonArray{page});
    root.insert("settings", QJsonObject{{"zoomPercent", 150}});
    QVector<Page> pages;
    QString error;
    QVERIFY2(BoardDocumentExchange::decode(encode(root), &pages, &error), qPrintable(error));
    QCOMPARE(pages.first().zoomPercent(), 150);
    BoardModel target;
    QVERIFY(target.importPages(pages, &error));
    QVERIFY(!target.document().pages()[1].elements()[0].id().isEmpty());
    QCOMPARE(target.document().pages()[1].elements()[0].rect(), source.currentPage().elements()[0].rect());
}

void BoardDocumentExchangeTest::exportFailurePreservesExistingFile()
{
    QTemporaryDir directory;
    const auto path = directory.filePath("keep.whiteboard");
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QCOMPARE(file.write("original"), 8);
    file.close();
    auto invalid = sample();
    invalid.currentPage().addElement(DrawingElement()); // Empty freehand is invalid.
    QString error;
    QVERIFY(!BoardDocumentExchange::write(path, invalid, BoardDocumentExchange::Scope::AllPages, &error));
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.readAll(), QByteArray("original"));
    QVERIFY(!BoardDocumentExchange::write(directory.filePath("missing/file.whiteboard"), sample(), BoardDocumentExchange::Scope::AllPages, &error));
}

void BoardDocumentExchangeTest::allElementTypesRoundTrip()
{
    BoardModel model;
    model.addFreehand({QPointF(1, 1), QPointF(2, 3)});
    model.addLine(QPointF(20, 20), QPointF(40, 40));
    model.addRectangle(QRectF(100, 100, 40, 50));
    model.addRoundedRectangle(QRectF(200, 100, 40, 50));
    model.addEllipse(QRectF(300, 100, 40, 50));
    model.addCircle(QRectF(400, 100, 40, 40));
    model.addArc(QRectF(500, 100, 40, 40));
    model.addPolyline({QPointF(10, 200), QPointF(20, 220), QPointF(40, 200)}, true);
    model.addBezier({QPointF(10, 300), QPointF(20, 320), QPointF(40, 300)});
    model.addText(QPointF(100, 300), "Text");
    QString error;
    QVector<Page> pages;
    QVERIFY2(BoardDocumentExchange::decode(encode(model.document().toJson()), &pages, &error), qPrintable(error));
    QCOMPARE(pages[0].toJson(), model.currentPage().toJson());
}

QTEST_MAIN(BoardDocumentExchangeTest)
#include "tst_boarddocumentexchange.moc"
