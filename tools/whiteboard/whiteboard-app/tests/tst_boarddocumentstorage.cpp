#include "BoardDocumentStorage.h"
#include "BoardModel.h"
#include "support/FailingReadDevice.h"
#include "support/FailingWriteDevice.h"

#include <QBuffer>
#include <QFile>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QTemporaryDir>
#include <QTest>

#include <filesystem>
#include <limits>

#ifdef Q_OS_UNIX
#include <csignal>
#include <sys/resource.h>
#include <unistd.h>
#endif

namespace {
QByteArray bytes(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return {};
    return file.readAll();
}

bool put(const QString &path, const QByteArray &data)
{
    QFile file(path);
    return file.open(QIODevice::WriteOnly) && file.write(data) == data.size();
}

BoardFileResult decode(const QJsonObject &json, BoardDocument *output)
{
    QByteArray data = QJsonDocument(json).toJson();
    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);
    return BoardDocumentStorage::readDevice(buffer, output);
}

QJsonObject sample()
{
    BoardModel model;
    model.addRectangle(QRectF(10, 20, 80, 60));
    model.addText(QPointF(20, 100), QStringLiteral("Saved text"));
    return model.document().toJson();
}
}

class BoardDocumentStorageTest : public QObject {
    Q_OBJECT

private slots:
    void roundTripAllElementTypesAndSettings();
    void distinguishesMissingFromInvalidPath();
    void permissionFailureIsNotMissing();
    void invalidInput_data();
    void invalidInput();
    void legacyNormalizationAndIds();
    void numericBoundaries();
    void ambiguousConnectionIsRejected();
    void validConnectionRoundTrips();
    void failedModelLoadPreservesSelectionAndHistory();
    void readErrorAfterValidPrefix();
    void writeFailures_data();
    void writeFailures();
    void completionFailures_data();
    void completionFailures();
    void cancelledCommitPreservesOriginal();
    void renameFailurePreservesOriginal();
    void realShortWritePreservesOriginal();
    void saveCreatesParentAndReportsErrors();
    void recognizesAliasesAndHardLinks();
};

void BoardDocumentStorageTest::roundTripAllElementTypesAndSettings()
{
    BoardModel model;
    model.addFreehand({QPointF(10, 10)});
    model.addLine(QPointF(10, 20), QPointF(30, 40));
    model.addPolyline({QPointF(10, 10), QPointF(20, 30)});
    model.addBezier({QPointF(10, 10), QPointF(30, 40)});
    model.addArc(QRectF(100, 100, 50, 50));
    model.addCircle(QRectF(200, 100, 50, 50));
    model.addRectangle(QRectF(300, 100, 50, 50));
    model.addRoundedRectangle(QRectF(400, 100, 50, 50));
    model.addEllipse(QRectF(500, 100, 50, 50));
    model.addText(QPointF(600, 100), QStringLiteral("round trip"));
    QImage image(2, 2, QImage::Format_ARGB32);
    image.fill(Qt::red);
    QByteArray png;
    QBuffer buffer(&png);
    QVERIFY(buffer.open(QIODevice::WriteOnly));
    QVERIFY(image.save(&buffer, "PNG"));
    model.addImage(QRectF(700, 100, 50, 50), png, QStringLiteral("image/png"));
    model.setWindowSize(QSize(1200, 800));
    model.setCurrentPageScrollPosition(QPoint(11, 22));
    model.setZoomPercent(125);
    model.setFloatingActionDockY(66);
    model.selectTool(Tool::Select);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("document.json"));
    const auto saved = BoardDocumentStorage::write(path, model.document());
    QVERIFY2(saved.ok(), qPrintable(saved.message));
    BoardDocument loaded;
    const auto read = BoardDocumentStorage::read(path, &loaded);
    QVERIFY2(read.ok(), qPrintable(read.message));
    QCOMPARE(loaded.toJson(), model.document().toJson());
    QCOMPARE(loaded.currentPage().elements().size(), 11);
}

void BoardDocumentStorageTest::distinguishesMissingFromInvalidPath()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    BoardDocument document;
    QVERIFY(BoardDocumentStorage::read(directory.filePath("new/whiteboard.json"), &document).missing());
    const auto directoryResult = BoardDocumentStorage::read(directory.path(), &document);
    QVERIFY(!directoryResult.ok());
    QVERIFY(!directoryResult.missing());
#ifdef Q_OS_UNIX
    const QString dangling = directory.filePath("dangling");
    QVERIFY(QFile::link(directory.filePath("absent"), dangling));
    const auto danglingResult = BoardDocumentStorage::read(dangling, &document);
    QVERIFY(!danglingResult.ok());
    QVERIFY(!danglingResult.missing());
    const auto descendant = BoardDocumentStorage::read(dangling + "/whiteboard.json", &document);
    QVERIFY(!descendant.ok());
    QVERIFY(!descendant.missing());
#endif
}

void BoardDocumentStorageTest::permissionFailureIsNotMissing()
{
#ifdef Q_OS_UNIX
    if (geteuid() == 0)
        QSKIP("Root bypasses file permission restrictions.");
    QTemporaryDir directory;
    const QString path = directory.filePath("document.json");
    QVERIFY(put(path, QJsonDocument(sample()).toJson()));
    QVERIFY(QFile::setPermissions(path, QFileDevice::WriteOwner));
    BoardDocument output;
    const auto result = BoardDocumentStorage::read(path, &output);
    const bool restored = QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    QVERIFY(restored);
    QVERIFY(!result.ok());
    QVERIFY(!result.missing());
    QCOMPARE(result.stage, QStringLiteral("open"));
    QVERIFY(QFile::setPermissions(directory.path(), QFileDevice::WriteOwner));
    const auto inaccessibleChild = BoardDocumentStorage::read(directory.filePath("missing.json"), &output);
    const bool restoredDirectory = QFile::setPermissions(directory.path(), QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    QVERIFY(restoredDirectory);
    QVERIFY(!inaccessibleChild.ok());
    QVERIFY(!inaccessibleChild.missing());
#else
    QSKIP("POSIX permission fixture; Windows ACL cases require Windows verification.");
#endif
}

void BoardDocumentStorageTest::invalidInput_data()
{
    QTest::addColumn<QByteArray>("input");
    QTest::newRow("empty") << QByteArray();
    QTest::newRow("truncated") << QByteArray("{\"pages\":[");
    QTest::newRow("array-root") << QByteArray("[]");
    QTest::newRow("empty-object") << QByteArray("{}");
    QTest::newRow("bad-pages") << QByteArray("{\"pages\":\"invalid\"}");
    QTest::newRow("bad-page") << QByteArray("{\"pages\":[42]}");
    QTest::newRow("bad-elements") << QByteArray("{\"pages\":[{\"elements\":null}]}");
    auto add = [](const char *name, const QJsonObject &json) {
        QTest::newRow(name) << QJsonDocument(json).toJson();
    };
    QJsonObject root = sample();
    auto pages = root.value("pages").toArray();
    while (pages.size() < 21)
        pages.append(pages.first());
    root["pages"] = pages;
    add("page-limit", root);
    for (const auto &entry : QList<QPair<QString, QJsonValue>>{
             {"type", "future-shape"}, {"rect", 4}, {"strokeWidth", "3"},
             {"strokeWidth", 1.5}, {"locked", 3}, {"closed", "false"},
             {"color", "invalid-color"}, {"strokeStyle", "unknown"},
             {"startArrowHead", "unknown"}, {"rotationDegrees", QJsonValue::Null}}) {
        root = sample();
        pages = root.value("pages").toArray();
        auto page = pages[0].toObject();
        auto elements = page.value("elements").toArray();
        auto element = elements[0].toObject();
        if (entry.first == "locked")
            page[entry.first] = entry.second;
        else
            element[entry.first] = entry.second;
        elements[0] = element;
        page["elements"] = elements;
        pages[0] = page;
        root["pages"] = pages;
        const QByteArray name = (entry.first + '-' + QString::number(entry.second.type())).toUtf8();
        add(name.constData(), root);
    }
    root = sample(); root["settings"] = "invalid"; add("settings-type", root);
    root = sample(); root["revision"] = 1.25; add("revision-fraction", root);
    root = sample(); root["nextElementId"] = std::numeric_limits<int>::max(); add("id-overflow", root);
    root = sample(); pages = root.value("pages").toArray();
    auto page = pages[0].toObject();
    auto elements = page.value("elements").toArray();
    auto element = elements[0].toObject();
    element["type"] = "image"; element["imageData"] = "!bad!"; element["imageMimeType"] = "image/png";
    elements[0] = element; page["elements"] = elements; pages[0] = page; root["pages"] = pages;
    add("bad-image", root);
}

void BoardDocumentStorageTest::invalidInput()
{
    QFETCH(QByteArray, input);
    QBuffer buffer(&input);
    QVERIFY(buffer.open(QIODevice::ReadOnly));
    BoardDocument output = BoardDocument::fromJson(sample());
    const auto before = output.toJson();
    const auto result = BoardDocumentStorage::readDevice(buffer, &output);
    QVERIFY(!result.ok());
    QVERIFY(!result.missing());
    QVERIFY(!result.message.isEmpty());
    QCOMPARE(output.toJson(), before);
}

void BoardDocumentStorageTest::legacyNormalizationAndIds()
{
    auto root = sample();
    root.remove("revision");
    root.remove("nextElementId");
    auto settings = root.value("settings").toObject();
    settings["zoomPercent"] = 800;
    settings["strokeWidth"] = -3;
    settings["cornerRadius"] = 300;
    root["settings"] = settings;
    root["currentPageIndex"] = 80;
    auto pages = root.value("pages").toArray();
    auto page = pages[0].toObject();
    page.remove("zoomPercent");
    page["scrollX"] = -20;
    auto elements = page.value("elements").toArray();
    for (qsizetype i = 0; i < elements.size(); ++i) {
        auto element = elements[i].toObject();
        element["id"] = "duplicate";
        element.remove("rotationDegrees");
        elements[i] = element;
    }
    auto first = elements[0].toObject();
    first["rect"] = QJsonObject{{"x", 50}, {"y", 50}, {"width", -30}, {"height", -40}};
    elements[0] = first;
    page["elements"] = elements;
    pages[0] = page;
    root["pages"] = pages;
    BoardDocument loaded;
    const auto result = decode(root, &loaded);
    QVERIFY2(result.ok(), qPrintable(result.message));
    QCOMPARE(loaded.currentPage().zoomPercent(), 400);
    QCOMPARE(loaded.currentPage().scrollPosition().x(), 0);
    QCOMPARE(loaded.currentPageIndex(), 0);
    QCOMPARE(loaded.settings().strokeWidth(), 0);
    QCOMPARE(loaded.settings().cornerRadius(), 200);
    const auto output = loaded.currentPage().elements();
    QVERIFY(output[0].id() != output[1].id());
    QCOMPARE(output[0].rotationDegrees(), 0.0);
    QCOMPARE(output[0].rect(), QRectF(20, 10, 30, 40));
}

void BoardDocumentStorageTest::ambiguousConnectionIsRejected()
{
    BoardModel model;
    model.addRectangle(QRectF(100, 100, 50, 50));
    model.addEllipse(QRectF(300, 100, 50, 50));
    model.addLine(QPointF(125, 125), QPointF(325, 125));
    auto root = model.document().toJson();
    auto pages = root.value("pages").toArray();
    auto page = pages[0].toObject();
    auto elements = page.value("elements").toArray();
    auto second = elements[1].toObject();
    second["id"] = elements[0].toObject().value("id");
    elements[1] = second;
    page["elements"] = elements; pages[0] = page; root["pages"] = pages;
    BoardDocument loaded;
    const auto before = loaded.toJson();
    const auto result = decode(root, &loaded);
    QVERIFY(!result.ok());
    QCOMPARE(result.stage, QStringLiteral("validate"));
    QCOMPARE(loaded.toJson(), before);
}

void BoardDocumentStorageTest::numericBoundaries()
{
    auto root = sample();
    root["revision"] = std::numeric_limits<qint64>::max();
    BoardDocument loaded;
    auto result = decode(root, &loaded);
    QVERIFY2(result.ok(), qPrintable(result.message));
    QCOMPARE(loaded.revision(), std::numeric_limits<qint64>::max());
    root["revision"] = std::numeric_limits<qint64>::min();
    result = decode(root, &loaded);
    QVERIFY2(result.ok(), qPrintable(result.message));
    QCOMPARE(loaded.revision(), qint64(0));
    root["revision"] = 1e30;
    QVERIFY(!decode(root, &loaded).ok());
    root = sample();
    auto pages = root.value("pages").toArray();
    auto page = pages[0].toObject();
    auto elements = page.value("elements").toArray();
    auto element = elements[0].toObject();
    element["id"] = QStringLiteral("element-%1").arg(std::numeric_limits<int>::max());
    elements[0] = element; page["elements"] = elements; pages[0] = page; root["pages"] = pages;
    QVERIFY(!decode(root, &loaded).ok());
}

void BoardDocumentStorageTest::validConnectionRoundTrips()
{
    BoardModel model;
    model.addRectangle(QRectF(100, 100, 50, 50));
    model.addEllipse(QRectF(300, 100, 50, 50));
    model.addLine(QPointF(125, 125), QPointF(325, 125));
    BoardDocument loaded;
    const auto result = decode(model.document().toJson(), &loaded);
    QVERIFY2(result.ok(), qPrintable(result.message));
    QCOMPARE(loaded.toJson(), model.document().toJson());
}

void BoardDocumentStorageTest::failedModelLoadPreservesSelectionAndHistory()
{
    BoardModel model;
    model.addRectangle(QRectF(10, 10, 50, 50));
    model.addEllipse(QRectF(100, 10, 50, 50));
    QVERIFY(model.undo());
    QVERIFY(model.selectAt(QPointF(20, 20)));
    const auto before = model.document().toJson();
    const auto selection = model.selectedElementIndexes();
    QTemporaryDir directory;
    const QString path = directory.filePath("bad.json");
    QVERIFY(put(path, "{}"));
    QVERIFY(!model.loadFromFile(path));
    QCOMPARE(model.document().toJson(), before);
    QCOMPARE(model.selectedElementIndexes(), selection);
    QVERIFY(model.canUndo());
    QVERIFY(model.canRedo());
    QCOMPARE(bytes(path), QByteArray("{}"));
}

void BoardDocumentStorageTest::readErrorAfterValidPrefix()
{
    FailingReadDevice device(QJsonDocument(sample()).toJson());
    BoardDocument loaded;
    const auto before = loaded.toJson();
    const auto result = BoardDocumentStorage::readDevice(device, &loaded);
    QCOMPARE(result.stage, QStringLiteral("read"));
    QVERIFY(!result.ok());
    QCOMPARE(loaded.toJson(), before);
}

void BoardDocumentStorageTest::writeFailures_data()
{
    QTest::addColumn<bool>("shortWrite");
    QTest::newRow("negative-write") << false;
    QTest::newRow("short-write") << true;
}

void BoardDocumentStorageTest::writeFailures()
{
    QFETCH(bool, shortWrite);
    FailingWriteDevice device(shortWrite ? FailingWriteDevice::Mode::Short : FailingWriteDevice::Mode::Error);
    bool committed = false;
    const auto result = BoardDocumentStorage::writeDevice(device, BoardDocument(), [&]() { committed = true; return true; });
    QVERIFY(!result.ok());
    QCOMPARE(result.stage, QStringLiteral("write"));
    QVERIFY(result.expectedBytes > 0);
    QCOMPARE(result.writtenBytes, shortWrite ? result.expectedBytes - 1 : -1);
    QVERIFY(result.errorJson().contains("writtenBytes"));
    QVERIFY(!committed);
}

void BoardDocumentStorageTest::completionFailures_data()
{
    QTest::addColumn<QString>("failure");
    QTest::newRow("buffered-flush") << QStringLiteral("Injected buffered flush failure");
    QTest::newRow("commit") << QStringLiteral("Injected commit failure");
}

void BoardDocumentStorageTest::completionFailures()
{
    QFETCH(QString, failure);
    FailingWriteDevice device(FailingWriteDevice::Mode::Accept);
    const auto result = BoardDocumentStorage::writeDevice(device, BoardDocument(), [&]() { return device.failCompletion(failure); });
    QVERIFY(!result.ok());
    QCOMPARE(result.stage, QStringLiteral("commit"));
    QCOMPARE(result.message, failure);
    QCOMPARE(result.expectedBytes, result.writtenBytes);
}

void BoardDocumentStorageTest::cancelledCommitPreservesOriginal()
{
    QTemporaryDir directory;
    const QString path = directory.filePath("document.json");
    const QByteArray original("original bytes");
    QVERIFY(put(path, original));
    {
        QSaveFile file(path);
        file.setDirectWriteFallback(false);
        QVERIFY(file.open(QIODevice::WriteOnly));
        const auto result = BoardDocumentStorage::writeDevice(file, BoardDocument(), [&]() {
            file.cancelWriting();
            return file.commit();
        });
        QVERIFY(!result.ok());
        QCOMPARE(result.stage, QStringLiteral("commit"));
    }
    QCOMPARE(bytes(path), original);
}

void BoardDocumentStorageTest::renameFailurePreservesOriginal()
{
#ifdef Q_OS_UNIX
    if (geteuid() == 0)
        QSKIP("Root bypasses directory permission restrictions.");
    QTemporaryDir directory;
    const QString path = directory.filePath("document.json");
    const QByteArray original("original bytes");
    QVERIFY(put(path, original));
    QSaveFile file(path);
    file.setDirectWriteFallback(false);
    QVERIFY(file.open(QIODevice::WriteOnly));
    bool permissionsChanged = false;
    const auto result = BoardDocumentStorage::writeDevice(file, BoardDocument(), [&]() {
        permissionsChanged = QFile::setPermissions(directory.path(), QFileDevice::ReadOwner | QFileDevice::ExeOwner);
        return file.commit();
    });
    const bool restored = QFile::setPermissions(directory.path(), QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    QVERIFY(restored);
    QVERIFY(permissionsChanged);
    QVERIFY(!result.ok());
    QCOMPARE(result.stage, QStringLiteral("commit"));
    QCOMPARE(bytes(path), original);
#else
    QSKIP("POSIX rename-failure fixture; Windows ACL cases require Windows verification.");
#endif
}

void BoardDocumentStorageTest::realShortWritePreservesOriginal()
{
#ifdef Q_OS_UNIX
    QTemporaryDir directory;
    const QString path = directory.filePath("document.json");
    BoardModel model;
    model.addText(QPointF(20, 20), QString(30000, QLatin1Char('S')));
    QVERIFY(model.saveToFile(path));
    const QByteArray original = bytes(path);
    struct rlimit previous {};
    QVERIFY(getrlimit(RLIMIT_FSIZE, &previous) == 0);
    auto limited = previous;
    limited.rlim_cur = 128;
    auto previousSignal = std::signal(SIGXFSZ, SIG_IGN);
    const int limitResult = setrlimit(RLIMIT_FSIZE, &limited);
    BoardFileResult result;
    if (limitResult == 0)
        result = BoardDocumentStorage::write(path, model.document());
    const int restored = setrlimit(RLIMIT_FSIZE, &previous);
    std::signal(SIGXFSZ, previousSignal);
    QVERIFY(limitResult == 0);
    QVERIFY(restored == 0);
    QVERIFY(!result.ok());
    QVERIFY(result.stage == "write" || result.stage == "commit");
    QCOMPARE(bytes(path), original);
#else
    QSKIP("RLIMIT_FSIZE is a POSIX failure fixture.");
#endif
}

void BoardDocumentStorageTest::saveCreatesParentAndReportsErrors()
{
    QTemporaryDir directory;
    const QString newPath = directory.filePath("new/whiteboard.json");
    QVERIFY(BoardDocumentStorage::write(newPath, BoardDocument()).ok());
    const auto invalidParent = BoardDocumentStorage::write(newPath + "/child.json", BoardDocument());
    QVERIFY(!invalidParent.ok());
    QCOMPARE(invalidParent.stage, QStringLiteral("create_directory"));
    const auto invalidTarget = BoardDocumentStorage::write(directory.path(), BoardDocument());
    QVERIFY(!invalidTarget.ok());
    QCOMPARE(invalidTarget.stage, QStringLiteral("open"));
}

void BoardDocumentStorageTest::recognizesAliasesAndHardLinks()
{
    QTemporaryDir directory;
    const QString original = directory.filePath("whiteboard.json");
    QVERIFY(put(original, "original"));
    QVERIFY(BoardDocumentStorage::sameFile(original, directory.filePath("./whiteboard.json")));
    QVERIFY(!BoardDocumentStorage::sameFile(original, directory.filePath("different.json")));
#ifdef Q_OS_UNIX
    const QString symbolic = directory.filePath("alias.png");
    QVERIFY(QFile::link(original, symbolic));
    QVERIFY(BoardDocumentStorage::sameFile(original, symbolic));
    const QString hard = directory.filePath("hard.svg");
    std::error_code error;
    std::filesystem::create_hard_link(QFile::encodeName(original).constData(), QFile::encodeName(hard).constData(), error);
    QVERIFY2(!error, error.message().c_str());
    QVERIFY(BoardDocumentStorage::sameFile(original, hard));
    const QString missing = directory.filePath("missing.json");
    const QString dangling = directory.filePath("dangling.whiteboard");
    QVERIFY(QFile::link(missing, dangling));
    QVERIFY(BoardDocumentStorage::sameFile(missing, dangling));
#endif
}

QTEST_MAIN(BoardDocumentStorageTest)
#include "tst_boarddocumentstorage.moc"
