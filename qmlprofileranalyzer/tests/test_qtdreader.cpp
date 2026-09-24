#include "analyzer.h"
#include "qtdreader.h"
#include "report.h"

#include <QtTest>
#include <QBuffer>
#include <QDataStream>
#include <QFile>
#include <QTemporaryDir>

using namespace QmlProfilerAnalyzer;

namespace {

QString dataFilePath(const QString &fileName)
{
    return QStringLiteral(TEST_DATA_DIR) + QLatin1Char('/') + fileName;
}

constexpr qint32 fixtureDataStreamVersion = 20; // QDataStream::Qt_6_0

enum FixtureMessage {
    FixtureEventMessage = 0,
    FixtureRangeStart = 1,
    FixtureRangeEnd = 4,
    FixtureDebugMessage = 9
};

enum FixtureRangeType {
    FixtureBinding = 3,
    FixtureMaximumRangeType = 6
};

struct FixtureEventLocation
{
    QString filename;
    int line = -1;
    int column = -1;
};

struct FixtureEventType
{
    QString displayName;
    QString data;
    FixtureEventLocation location;
    quint8 message = FixtureDebugMessage;
    quint8 rangeType = FixtureMaximumRangeType;
    int detailType = -1;
};

struct FixtureNote
{
    int typeIndex = -1;
    int collapsedRow = -1;
    qint64 startTime = -1;
    qint64 duration = 0;
    QString text;
};

struct FixtureEvent
{
    qint64 timestamp = 0;
    qint32 typeIndex = -1;
    QByteArray byteData;
};

QDataStream &operator<<(QDataStream &stream, const FixtureEventLocation &location)
{
    return stream << location.filename << location.line << location.column;
}

QDataStream &operator<<(QDataStream &stream, const FixtureEventType &type)
{
    return stream << type.displayName << type.data << type.location << type.message
                  << type.rangeType << type.detailType;
}

QDataStream &operator<<(QDataStream &stream, const FixtureNote &note)
{
    return stream << note.typeIndex << note.collapsedRow << note.startTime << note.duration
                  << note.text;
}

QDataStream &operator<<(QDataStream &stream, const FixtureEvent &event)
{
    const qint8 packedType = 2 | (1 << 2) | (0 << 4) | (0 << 6);
    stream << packedType;
    stream << static_cast<qint32>(event.timestamp);
    stream << static_cast<qint16>(event.typeIndex);
    stream << static_cast<qint8>(event.byteData.size());
    for (const char value : event.byteData)
        stream << static_cast<qint8>(value);
    return stream;
}

QString writeQztFixture()
{
    QTemporaryDir dir;
    dir.setAutoRemove(false);
    const QString path = dir.path() + QStringLiteral("/fixture.qzt");

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return QString();

    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_5_5);
    stream << QByteArray("QMLPROFILER");
    stream << fixtureDataStreamVersion;
    stream.setVersion(static_cast<QDataStream::Version>(fixtureDataStreamVersion));
    stream << qint64(1000) << qint64(1400);

    {
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        QDataStream bufferStream(&buffer);
        bufferStream.setVersion(static_cast<QDataStream::Version>(fixtureDataStreamVersion));

        QList<FixtureEventType> types;
        types.append({QStringLiteral("root width binding"),
                      QStringLiteral("root.width"),
                      {QStringLiteral("samples/qml/AutoTrace.qml"), 17, 13},
                      FixtureDebugMessage,
                      FixtureBinding,
                      0});
        types.append({QStringLiteral("demo warning"),
                      QString(),
                      {},
                      FixtureDebugMessage,
                      FixtureMaximumRangeType,
                      1});

        bufferStream << quint32(types.size());
        for (const auto &type : types)
            bufferStream << type;
        stream << qCompress(buffer.data());
    }

    {
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        QDataStream bufferStream(&buffer);
        bufferStream.setVersion(static_cast<QDataStream::Version>(fixtureDataStreamVersion));
        QList<FixtureNote> notes;
        notes.append({0, 0, 1100, 40, QStringLiteral("Investigate qzt binding first.")});
        bufferStream << notes;
        stream << qCompress(buffer.data());
    }

    {
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        QDataStream bufferStream(&buffer);
        bufferStream.setVersion(static_cast<QDataStream::Version>(fixtureDataStreamVersion));
        bufferStream << FixtureEvent{1100, 0, QByteArray(1, char(FixtureRangeStart))};
        bufferStream << FixtureEvent{1140, 0, QByteArray(1, char(FixtureRangeEnd))};
        bufferStream << FixtureEvent{1300, 1, QByteArray("demo warning from qzt")};
        stream << qCompress(buffer.data());
    }

    file.close();
    return path;
}

TraceData syntheticPixmapTrace()
{
    TraceData trace;
    trace.fileFormat = QStringLiteral("qzt/binary");

    EventTypeDef loadStart;
    loadStart.index = 0;
    loadStart.typeName = QStringLiteral("PixmapCache");
    loadStart.displayName = QStringLiteral("shared.png:0");
    loadStart.filename = QStringLiteral("qrc:/shared.png");
    loadStart.detailTag = QStringLiteral("cacheEventType");
    loadStart.detailValue = 3;
    loadStart.hasDetailValue = true;

    EventTypeDef sizeKnown;
    sizeKnown.index = 1;
    sizeKnown.typeName = QStringLiteral("PixmapCache");
    sizeKnown.displayName = QStringLiteral("shared.png:0");
    sizeKnown.filename = QStringLiteral("qrc:/shared.png");
    sizeKnown.detailTag = QStringLiteral("cacheEventType");
    sizeKnown.detailValue = 0;
    sizeKnown.hasDetailValue = true;

    EventTypeDef loadFinished;
    loadFinished.index = 2;
    loadFinished.typeName = QStringLiteral("PixmapCache");
    loadFinished.displayName = QStringLiteral("shared.png:0");
    loadFinished.filename = QStringLiteral("qrc:/shared.png");
    loadFinished.detailTag = QStringLiteral("cacheEventType");
    loadFinished.detailValue = 4;
    loadFinished.hasDetailValue = true;

    trace.eventTypes = {loadStart, sizeKnown, loadFinished};

    TraceEvent start1;
    start1.startTime = 100;
    start1.eventIndex = 0;
    start1.attributes.insert(QStringLiteral("startTime"), QStringLiteral("100"));
    start1.attributes.insert(QStringLiteral("eventIndex"), QStringLiteral("0"));

    TraceEvent start2 = start1;
    start2.startTime = 110;
    start2.attributes.insert(QStringLiteral("startTime"), QStringLiteral("110"));

    TraceEvent size1;
    size1.startTime = 120;
    size1.eventIndex = 1;
    size1.attributes.insert(QStringLiteral("startTime"), QStringLiteral("120"));
    size1.attributes.insert(QStringLiteral("eventIndex"), QStringLiteral("1"));
    size1.attributes.insert(QStringLiteral("width"), QStringLiteral("10"));
    size1.attributes.insert(QStringLiteral("height"), QStringLiteral("20"));

    TraceEvent finish1;
    finish1.startTime = 130;
    finish1.eventIndex = 2;
    finish1.attributes.insert(QStringLiteral("startTime"), QStringLiteral("130"));
    finish1.attributes.insert(QStringLiteral("eventIndex"), QStringLiteral("2"));

    TraceEvent size2 = size1;
    size2.startTime = 140;
    size2.attributes.insert(QStringLiteral("startTime"), QStringLiteral("140"));
    size2.attributes.insert(QStringLiteral("width"), QStringLiteral("30"));
    size2.attributes.insert(QStringLiteral("height"), QStringLiteral("40"));

    TraceEvent finish2 = finish1;
    finish2.startTime = 145;
    finish2.attributes.insert(QStringLiteral("startTime"), QStringLiteral("145"));

    trace.events = {start1, start2, size1, finish1, size2, finish2};
    return trace;
}

TraceData syntheticAggregateTrace()
{
    TraceData trace;
    trace.fileFormat = QStringLiteral("qzt/binary");

    EventTypeDef debugWarning;
    debugWarning.index = 0;
    debugWarning.typeName = QStringLiteral("DebugMessage");
    debugWarning.displayName = QStringLiteral("DebugMessage:1");
    debugWarning.detailTag = QStringLiteral("level");
    debugWarning.detailValue = 1;
    debugWarning.hasDetailValue = true;

    EventTypeDef memoryHeap;
    memoryHeap.index = 1;
    memoryHeap.typeName = QStringLiteral("MemoryAllocation");
    memoryHeap.displayName = QStringLiteral("MemoryAllocation:0");
    memoryHeap.detailTag = QStringLiteral("memoryEventType");
    memoryHeap.detailValue = 0;
    memoryHeap.hasDetailValue = true;

    EventTypeDef memorySmall;
    memorySmall.index = 2;
    memorySmall.typeName = QStringLiteral("MemoryAllocation");
    memorySmall.displayName = QStringLiteral("MemoryAllocation:2");
    memorySmall.detailTag = QStringLiteral("memoryEventType");
    memorySmall.detailValue = 2;
    memorySmall.hasDetailValue = true;

    EventTypeDef sceneGraphRenderLoop;
    sceneGraphRenderLoop.index = 3;
    sceneGraphRenderLoop.typeName = QStringLiteral("SceneGraph");
    sceneGraphRenderLoop.displayName = QStringLiteral("SceneGraph:3");
    sceneGraphRenderLoop.detailTag = QStringLiteral("sgEventType");
    sceneGraphRenderLoop.detailValue = 3;
    sceneGraphRenderLoop.hasDetailValue = true;

    trace.eventTypes = {debugWarning, memoryHeap, memorySmall, sceneGraphRenderLoop};

    TraceEvent debug1;
    debug1.startTime = 100;
    debug1.eventIndex = 0;
    debug1.attributes.insert(QStringLiteral("text"), QStringLiteral("first warning"));

    TraceEvent debug2 = debug1;
    debug2.startTime = 120;
    debug2.attributes.insert(QStringLiteral("text"), QStringLiteral("second warning"));

    TraceEvent heap;
    heap.startTime = 200;
    heap.eventIndex = 1;
    heap.attributes.insert(QStringLiteral("amount"), QStringLiteral("1024"));

    TraceEvent small1;
    small1.startTime = 220;
    small1.eventIndex = 2;
    small1.attributes.insert(QStringLiteral("amount"), QStringLiteral("64"));

    TraceEvent small2 = small1;
    small2.startTime = 240;
    small2.attributes.insert(QStringLiteral("amount"), QStringLiteral("-16"));

    TraceEvent frame;
    frame.startTime = 300;
    frame.eventIndex = 3;
    frame.attributes.insert(QStringLiteral("timing1"), QStringLiteral("10"));
    frame.attributes.insert(QStringLiteral("timing2"), QStringLiteral("20"));
    frame.attributes.insert(QStringLiteral("timing4"), QStringLiteral("5"));

    trace.events = {debug1, debug2, heap, small1, small2, frame};
    return trace;
}

TraceData syntheticNestedRangeTrace()
{
    TraceData trace;
    trace.fileFormat = QStringLiteral("qtd/xml");

    EventTypeDef rootType;
    rootType.index = 0;
    rootType.typeName = QStringLiteral("Creating");
    rootType.displayName = QStringLiteral("Root.qml:1");
    rootType.filename = QStringLiteral("Root.qml");
    rootType.line = 1;

    EventTypeDef childType;
    childType.index = 1;
    childType.typeName = QStringLiteral("Creating");
    childType.displayName = QStringLiteral("Child.qml:10");
    childType.filename = QStringLiteral("Child.qml");
    childType.line = 10;

    EventTypeDef leafType;
    leafType.index = 2;
    leafType.typeName = QStringLiteral("Binding");
    leafType.displayName = QStringLiteral("Child.qml:11");
    leafType.filename = QStringLiteral("Child.qml");
    leafType.line = 11;

    trace.eventTypes = {rootType, childType, leafType};

    TraceEvent root;
    root.startTime = 100;
    root.duration = 50;
    root.eventIndex = 0;

    TraceEvent child;
    child.startTime = 110;
    child.duration = 30;
    child.eventIndex = 1;

    TraceEvent leaf;
    leaf.startTime = 120;
    leaf.duration = 10;
    leaf.eventIndex = 2;

    trace.events = {root, child, leaf};
    return trace;
}

TraceData syntheticHotspotTrace()
{
    TraceData trace;
    trace.fileFormat = QStringLiteral("qtd/xml");

    EventTypeDef outlierType;
    outlierType.index = 0;
    outlierType.typeName = QStringLiteral("Creating");
    outlierType.displayName = QStringLiteral("Outlier.qml:1");
    outlierType.filename = QStringLiteral("Outlier.qml");
    outlierType.line = 1;

    EventTypeDef steadyType;
    steadyType.index = 1;
    steadyType.typeName = QStringLiteral("Creating");
    steadyType.displayName = QStringLiteral("Steady.qml:2");
    steadyType.filename = QStringLiteral("Steady.qml");
    steadyType.line = 2;

    trace.eventTypes = {outlierType, steadyType};

    TraceEvent outlier1;
    outlier1.startTime = 100;
    outlier1.duration = 5;
    outlier1.eventIndex = 0;

    TraceEvent outlier2;
    outlier2.startTime = 120;
    outlier2.duration = 50;
    outlier2.eventIndex = 0;

    TraceEvent steady1;
    steady1.startTime = 200;
    steady1.duration = 20;
    steady1.eventIndex = 1;

    TraceEvent steady2;
    steady2.startTime = 230;
    steady2.duration = 22;
    steady2.eventIndex = 1;

    trace.events = {outlier1, outlier2, steady1, steady2};
    return trace;
}

TraceData syntheticHotspotGroupingTrace()
{
    TraceData trace;
    trace.fileFormat = QStringLiteral("qtd/xml");

    EventTypeDef firstType;
    firstType.index = 0;
    firstType.typeName = QStringLiteral("Binding");
    firstType.displayName = QStringLiteral("Same.qml:10");
    firstType.filename = QStringLiteral("Same.qml");
    firstType.line = 10;

    EventTypeDef secondType = firstType;
    secondType.index = 1;
    secondType.displayName = QStringLiteral("Different display for same location");

    trace.eventTypes = {firstType, secondType};

    TraceEvent first;
    first.startTime = 100;
    first.duration = 10;
    first.eventIndex = 0;

    TraceEvent second;
    second.startTime = 120;
    second.duration = 15;
    second.eventIndex = 1;

    trace.events = {first, second};
    return trace;
}

TraceData syntheticGapWarningTrace()
{
    TraceData trace;
    trace.fileFormat = QStringLiteral("qtd/xml");

    EventTypeDef creatingType;
    creatingType.index = 0;
    creatingType.typeName = QStringLiteral("Creating");
    creatingType.displayName = QStringLiteral("Warn.qml:1");
    creatingType.filename = QStringLiteral("Warn.qml");
    creatingType.line = 1;

    EventTypeDef childType;
    childType.index = 1;
    childType.typeName = QStringLiteral("Creating");
    childType.displayName = QStringLiteral("Warn.qml:2");
    childType.filename = QStringLiteral("Warn.qml");
    childType.line = 2;

    EventTypeDef compilingType;
    compilingType.index = 2;
    compilingType.typeName = QStringLiteral("Compiling");
    compilingType.displayName = QStringLiteral("Warn.qml:3");
    compilingType.filename = QStringLiteral("Warn.qml");
    compilingType.line = 3;

    trace.eventTypes = {creatingType, childType, compilingType};

    TraceEvent parent;
    parent.startTime = 100;
    parent.duration = 10 * 1000 * 1000;
    parent.eventIndex = 0;

    TraceEvent child;
    child.startTime = 101;
    child.duration = 500 * 1000;
    child.eventIndex = 1;

    TraceEvent compiling;
    compiling.startTime = 200;
    compiling.duration = 10 * 1000 * 1000;
    compiling.eventIndex = 2;

    trace.events = {parent, child, compiling};
    return trace;
}

TraceData syntheticPriority4Trace()
{
    TraceData trace;
    trace.fileFormat = QStringLiteral("qtd/xml");

    EventTypeDef burstType;
    burstType.index = 0;
    burstType.typeName = QStringLiteral("Creating");
    burstType.displayName = QStringLiteral("Burst.qml:1");
    burstType.filename = QStringLiteral("Burst.qml");
    burstType.line = 1;

    EventTypeDef imageType;
    imageType.index = 1;
    imageType.typeName = QStringLiteral("Creating");
    imageType.displayName = QStringLiteral("Image.qml:2");
    imageType.filename = QStringLiteral("Image.qml");
    imageType.line = 2;

    EventTypeDef compilingType;
    compilingType.index = 2;
    compilingType.typeName = QStringLiteral("Compiling");
    compilingType.displayName = QStringLiteral("Compile.qml:0");
    compilingType.filename = QStringLiteral("Compile.qml");
    compilingType.line = 0;

    EventTypeDef pixmapStart;
    pixmapStart.index = 3;
    pixmapStart.typeName = QStringLiteral("PixmapCache");
    pixmapStart.displayName = QStringLiteral("hero.png:0");
    pixmapStart.filename = QStringLiteral("hero.png");
    pixmapStart.detailTag = QStringLiteral("cacheEventType");
    pixmapStart.detailValue = 3;
    pixmapStart.hasDetailValue = true;

    EventTypeDef pixmapFinish = pixmapStart;
    pixmapFinish.index = 4;
    pixmapFinish.detailValue = 4;

    trace.eventTypes = {burstType, imageType, compilingType, pixmapStart, pixmapFinish};

    for (int i = 0; i < 5; ++i) {
        TraceEvent burst;
        burst.startTime = 1000 + (i * 1000);
        burst.duration = 2000;
        burst.eventIndex = 0;
        trace.events.append(burst);
    }

    TraceEvent image;
    image.startTime = 20000;
    image.duration = 10000;
    image.eventIndex = 1;
    trace.events.append(image);

    TraceEvent compiling;
    compiling.startTime = 40000;
    compiling.duration = 12000;
    compiling.eventIndex = 2;
    trace.events.append(compiling);

    TraceEvent pixmapLoadStart;
    pixmapLoadStart.startTime = 21000;
    pixmapLoadStart.eventIndex = 3;
    trace.events.append(pixmapLoadStart);

    TraceEvent pixmapLoadFinish;
    pixmapLoadFinish.startTime = 26000;
    pixmapLoadFinish.eventIndex = 4;
    trace.events.append(pixmapLoadFinish);

    return trace;
}

} // namespace

class QtdReaderTest : public QObject
{
    Q_OBJECT

private slots:
    void readsMinimalTrace();
    void skipsEventWithoutIndex();
    void skipsRangeMissingRequiredAttributes();
    void skipsNoteMissingRequiredAttributes();
    void failsOnInvalidXml();
    void readsQztTrace();
    void convertsQztTraceToQtdXml();
    void convertsQtdTraceToQztBinary();
    void analyzesNestedRanges();
    void rendersSamegameRanges();
    void analyzesHotspots();
    void groupsHotspotsByStableLocation();
    void comparesHotspotsByStableLocation();
    void rendersSamegameHotspots();
    void marksGapWarnings();
    void addsPriority4Hints();
    void rendersPixmapCacheAggregatesForSamegame();
    void keepsEventsOutputRaw();
    void aggregatesOverlappingPixmapLoads();
    void aggregatesSceneGraphMemoryAndDebugEvents();
};

void QtdReaderTest::readsMinimalTrace()
{
    QtdReader reader;
    const auto trace = reader.readFile(dataFilePath(QStringLiteral("minimal.qtd")));

    QVERIFY2(trace.has_value(), qPrintable(reader.errorString()));

    const Summary summary = Analyzer::summarize(trace.value());
    QCOMPARE(trace->version, QStringLiteral("1.02"));
    QCOMPARE(trace->traceStart, 1000);
    QCOMPARE(trace->traceEnd, 1800);
    QCOMPARE(summary.typeCount, 3);
    QCOMPARE(summary.eventCount, 3);
    QCOMPARE(summary.rangeCount, 1);
    QCOMPARE(summary.pointEventCount, 2);
    QCOMPARE(summary.noteCount, 1);
    QCOMPARE(summary.longestEventIndex, 0);
    QCOMPARE(summary.longestDuration, 40);
    QCOMPARE(trace->notes.at(0).text, QStringLiteral("Investigate this binding first."));

    const QString rendered = Report::renderSummary(trace.value(), summary);
    QVERIFY(rendered.contains(
        QStringLiteral("note: per-type totalDuration includes parent-child overlap for nested ranges.")));
    QVERIFY(rendered.contains(
        QStringLiteral("note: use the ranges command to inspect selfTime and focus candidates.")));
}

void QtdReaderTest::skipsEventWithoutIndex()
{
    QtdReader reader;
    const auto trace = reader.readFile(dataFilePath(QStringLiteral("missing_event_index.qtd")));

    QVERIFY2(trace.has_value(), qPrintable(reader.errorString()));
    QCOMPARE(trace->eventTypes.size(), 2);
    QCOMPARE(trace->eventTypes.at(0).index, -1);
    QCOMPARE(trace->eventTypes.at(1).index, 1);
    QCOMPARE(trace->eventTypes.at(1).displayName, QStringLiteral("valid event"));

    const Summary summary = Analyzer::summarize(trace.value());
    QCOMPARE(summary.typeCount, 2);
    QCOMPARE(summary.eventCount, 1);
    QCOMPARE(summary.rangeCount, 1);
}

void QtdReaderTest::skipsRangeMissingRequiredAttributes()
{
    QtdReader reader;
    const auto trace = reader.readFile(dataFilePath(QStringLiteral("missing_range_attrs.qtd")));

    QVERIFY2(trace.has_value(), qPrintable(reader.errorString()));
    QCOMPARE(trace->events.size(), 1);
    QCOMPARE(trace->events.at(0).startTime, 200);
    QCOMPARE(trace->events.at(0).eventIndex, 0);
    QVERIFY(trace->events.at(0).duration.has_value());
    QCOMPARE(trace->events.at(0).duration.value(), 10);

    const Summary summary = Analyzer::summarize(trace.value());
    QCOMPARE(summary.eventCount, 1);
    QCOMPARE(summary.rangeCount, 1);
    QCOMPARE(summary.pointEventCount, 0);
}

void QtdReaderTest::skipsNoteMissingRequiredAttributes()
{
    QtdReader reader;
    const auto trace = reader.readFile(dataFilePath(QStringLiteral("missing_note_attrs.qtd")));

    QVERIFY2(trace.has_value(), qPrintable(reader.errorString()));
    QCOMPARE(trace->notes.size(), 1);
    QCOMPARE(trace->notes.at(0).startTime, 100);
    QCOMPARE(trace->notes.at(0).duration, 10);
    QCOMPARE(trace->notes.at(0).eventIndex, 0);
    QCOMPARE(trace->notes.at(0).text, QStringLiteral("valid note"));

    const Summary summary = Analyzer::summarize(trace.value());
    QCOMPARE(summary.noteCount, 1);
}

void QtdReaderTest::failsOnInvalidXml()
{
    QtdReader reader;
    const auto trace = reader.readFile(dataFilePath(QStringLiteral("invalid_xml.qtd")));

    QVERIFY(!trace.has_value());
    QVERIFY(reader.errorString().contains(QStringLiteral("XML parse error")));
}

void QtdReaderTest::readsQztTrace()
{
    const QString path = writeQztFixture();
    QVERIFY(!path.isEmpty());

    QtdReader reader;
    const auto trace = reader.readFile(path);

    QVERIFY2(trace.has_value(), qPrintable(reader.errorString()));
    QCOMPARE(trace->fileFormat, QStringLiteral("qzt/binary"));
    QCOMPARE(trace->traceStart, 1000);
    QCOMPARE(trace->traceEnd, 1400);
    QCOMPARE(trace->totalTime, 400);
    QCOMPARE(trace->eventTypes.size(), 2);
    QCOMPARE(trace->events.size(), 2);
    QCOMPARE(trace->notes.size(), 1);

    const Summary summary = Analyzer::summarize(trace.value());
    QCOMPARE(summary.typeCount, 2);
    QCOMPARE(summary.eventCount, 2);
    QCOMPARE(summary.rangeCount, 1);
    QCOMPARE(summary.pointEventCount, 1);
    QCOMPARE(summary.noteCount, 1);
    QCOMPARE(summary.longestEventIndex, 0);
    QCOMPARE(summary.longestDuration, 40);

    QCOMPARE(trace->events.at(0).startTime, 1100);
    QVERIFY(trace->events.at(0).duration.has_value());
    QCOMPARE(trace->events.at(0).duration.value(), 40);
    QCOMPARE(trace->events.at(1).eventIndex, 1);
    QCOMPARE(trace->events.at(1).attributes.value(QStringLiteral("text")),
             QStringLiteral("demo warning from qzt"));
    QCOMPARE(trace->notes.at(0).text, QStringLiteral("Investigate qzt binding first."));
}

void QtdReaderTest::convertsQztTraceToQtdXml()
{
    const QString inputPath = writeQztFixture();
    QVERIFY(!inputPath.isEmpty());

    QTemporaryDir outputDir;
    QVERIFY(outputDir.isValid());
    const QString outputPath = outputDir.path() + QStringLiteral("/converted.qtd");

    QtdReader reader;
    const auto inputTrace = reader.readFile(inputPath);
    QVERIFY2(inputTrace.has_value(), qPrintable(reader.errorString()));
    QVERIFY2(reader.writeQtdFile(inputTrace.value(), outputPath), qPrintable(reader.errorString()));

    QFile outputFile(outputPath);
    QVERIFY(outputFile.open(QIODevice::ReadOnly));
    const QByteArray xml = outputFile.readAll();
    QVERIFY(xml.contains("<trace"));
    QVERIFY(xml.contains("<eventData"));
    QVERIFY(xml.contains("<profilerDataModel"));
    QVERIFY(xml.contains("<noteData"));
    outputFile.close();

    const auto convertedTrace = reader.readFile(outputPath);
    QVERIFY2(convertedTrace.has_value(), qPrintable(reader.errorString()));
    QCOMPARE(convertedTrace->fileFormat, QStringLiteral("qtd/xml"));
    QCOMPARE(convertedTrace->traceStart, inputTrace->traceStart);
    QCOMPARE(convertedTrace->traceEnd, inputTrace->traceEnd);
    QCOMPARE(convertedTrace->totalTime, inputTrace->totalTime);
    QCOMPARE(convertedTrace->eventTypes.size(), inputTrace->eventTypes.size());
    QCOMPARE(convertedTrace->events.size(), inputTrace->events.size());
    QCOMPARE(convertedTrace->notes.size(), inputTrace->notes.size());
    QCOMPARE(convertedTrace->events.at(0).duration, inputTrace->events.at(0).duration);
    QCOMPARE(convertedTrace->events.at(1).attributes.value(QStringLiteral("text")),
             QStringLiteral("demo warning from qzt"));
    QCOMPARE(convertedTrace->notes.at(0).text, QStringLiteral("Investigate qzt binding first."));
}

void QtdReaderTest::convertsQtdTraceToQztBinary()
{
    QtdReader reader;
    const auto inputTrace = reader.readFile(dataFilePath(QStringLiteral("minimal.qtd")));
    QVERIFY2(inputTrace.has_value(), qPrintable(reader.errorString()));

    QTemporaryDir outputDir;
    QVERIFY(outputDir.isValid());
    const QString outputPath = outputDir.path() + QStringLiteral("/converted.qzt");
    QVERIFY2(reader.writeQztFile(inputTrace.value(), outputPath), qPrintable(reader.errorString()));

    QFile outputFile(outputPath);
    QVERIFY(outputFile.open(QIODevice::ReadOnly));
    QDataStream outputStream(&outputFile);
    outputStream.setVersion(QDataStream::Qt_5_5);
    QByteArray magic;
    outputStream >> magic;
    QCOMPARE(magic, QByteArray("QMLPROFILER"));
    outputFile.close();

    const auto convertedTrace = reader.readFile(outputPath);
    QVERIFY2(convertedTrace.has_value(), qPrintable(reader.errorString()));
    QCOMPARE(convertedTrace->fileFormat, QStringLiteral("qzt/binary"));
    QCOMPARE(convertedTrace->traceStart, inputTrace->traceStart);
    QCOMPARE(convertedTrace->traceEnd, inputTrace->traceEnd);
    QCOMPARE(convertedTrace->eventTypes.size(), inputTrace->eventTypes.size());
    QCOMPARE(convertedTrace->events.size(), inputTrace->events.size());
    QCOMPARE(convertedTrace->notes.size(), inputTrace->notes.size());
    QCOMPARE(convertedTrace->events.at(0).duration, inputTrace->events.at(0).duration);
    QCOMPARE(convertedTrace->notes.at(0).text, inputTrace->notes.at(0).text);
}

void QtdReaderTest::analyzesNestedRanges()
{
    const TraceData trace = syntheticNestedRangeTrace();
    const RangeAnalysis analysis = Analyzer::analyzeRanges(trace);

    QCOMPARE(analysis.nodes.size(), 3);

    const auto &root = analysis.nodes.at(0);
    QCOMPARE(root.parentId, -1);
    QCOMPARE(root.depth, 0);
    QCOMPARE(root.childIds.size(), 1);
    QCOMPARE(root.childCoveredTime, 30);
    QCOMPARE(root.selfTime, 20);
    QCOMPARE(root.gapTime, 20);
    QVERIFY(!root.focusCandidate);

    const auto &child = analysis.nodes.at(1);
    QCOMPARE(child.parentId, 0);
    QCOMPARE(child.depth, 1);
    QCOMPARE(child.childIds.size(), 1);
    QCOMPARE(child.childCoveredTime, 10);
    QCOMPARE(child.selfTime, 20);
    QVERIFY(child.focusCandidate);

    const auto &leaf = analysis.nodes.at(2);
    QCOMPARE(leaf.parentId, 1);
    QCOMPARE(leaf.depth, 2);
    QCOMPARE(leaf.childIds.size(), 0);
    QCOMPARE(leaf.selfTime, 10);
    QVERIFY(leaf.focusCandidate);

    const QString rendered = Report::renderRanges(trace, analysis);
    QVERIFY(rendered.contains(
        QStringLiteral("100  id=0  duration=50  selfTime=20  childTime=30  gapTime=20  gapWarning=no  focus=no  depth=0  idx=0  type=Root.qml:1")));
    QVERIFY(rendered.contains(
        QStringLiteral("110  id=1  duration=30  selfTime=20  childTime=10  gapTime=20  gapWarning=no  focus=yes  depth=1  idx=1  type=Child.qml:10  parent=0")));
}

void QtdReaderTest::rendersSamegameRanges()
{
    QtdReader reader;
    const auto trace = reader.readFile(dataFilePath(QStringLiteral("samegame.qzt")));

    QVERIFY2(trace.has_value(), qPrintable(reader.errorString()));

    const RangeAnalysis analysis = Analyzer::analyzeRanges(trace.value());
    QVERIFY(!analysis.nodes.isEmpty());

    bool hasNestedRange = false;
    for (const auto &node : analysis.nodes) {
        if (node.parentId >= 0) {
            hasNestedRange = true;
            break;
        }
    }
    QVERIFY(hasNestedRange);

    const QString rendered = Report::renderRanges(trace.value(), analysis);
    QVERIFY(rendered.contains(QStringLiteral("selfTime=")));
    QVERIFY(rendered.contains(QStringLiteral("childTime=")));
    QVERIFY(rendered.contains(QStringLiteral("focus=")));
    QVERIFY(rendered.contains(QStringLiteral("depth=")));
    QVERIFY(rendered.contains(QStringLiteral("samegame.qml")));
    QVERIFY(rendered.contains(QStringLiteral("Dialog.qml:23")));
}

void QtdReaderTest::analyzesHotspots()
{
    const TraceData trace = syntheticHotspotTrace();
    const HotspotAnalysis analysis = Analyzer::analyzeHotspots(trace);

    QCOMPARE(analysis.hotspots.size(), 2);

    const auto &outlier = analysis.hotspots.at(0);
    QCOMPARE(outlier.eventIndex, 0);
    QCOMPARE(outlier.calls, 2);
    QCOMPARE(outlier.selfTotal, 55);
    QCOMPARE(outlier.minSelf, 5);
    QCOMPARE(outlier.medianSelf, 50);
    QCOMPARE(outlier.maxSelf, 50);
    QVERIFY(!outlier.outlierHint);

    const auto &steady = analysis.hotspots.at(1);
    QCOMPARE(steady.eventIndex, 1);
    QCOMPARE(steady.calls, 2);
    QCOMPARE(steady.minSelf, 20);
    QCOMPARE(steady.medianSelf, 22);
    QCOMPARE(steady.maxSelf, 22);

    const QString rendered = Report::renderHotspots(trace, analysis);
    QVERIFY(rendered.contains(
        QStringLiteral("idx=0  groupKey=type=Creating|file=Outlier.qml|line=1|column=-1  indexes=0  type=Outlier.qml:1  calls=2  selfTotal=55")));
}

void QtdReaderTest::groupsHotspotsByStableLocation()
{
    const TraceData trace = syntheticHotspotGroupingTrace();
    const HotspotAnalysis analysis = Analyzer::analyzeHotspots(trace);

    QCOMPARE(analysis.hotspots.size(), 1);

    const auto &hotspot = analysis.hotspots.at(0);
    QCOMPARE(hotspot.eventIndex, 0);
    QCOMPARE(hotspot.groupKey, QStringLiteral("type=Binding|file=Same.qml|line=10|column=-1"));
    QCOMPARE(hotspot.eventIndexes, QVector<int>({0, 1}));
    QCOMPARE(hotspot.calls, 2);
    QCOMPARE(hotspot.selfTotal, 25);

    const QString rendered = Report::renderHotspots(trace, analysis);
    QVERIFY(rendered.contains(QStringLiteral(
        "idx=0  groupKey=type=Binding|file=Same.qml|line=10|column=-1  indexes=0,1  type=Same.qml:10  calls=2  selfTotal=25")));
}

void QtdReaderTest::comparesHotspotsByStableLocation()
{
    const TraceData beforeTrace = syntheticHotspotGroupingTrace();
    TraceData afterTrace = syntheticHotspotGroupingTrace();
    afterTrace.events[0].duration = 20;
    afterTrace.events[1].duration = 30;

    const HotspotComparison comparison = Analyzer::compareHotspots(beforeTrace, afterTrace);
    QCOMPARE(comparison.entries.size(), 1);

    const auto &entry = comparison.entries.at(0);
    QCOMPARE(entry.groupKey, QStringLiteral("type=Binding|file=Same.qml|line=10|column=-1"));
    QVERIFY(entry.hasBefore);
    QVERIFY(entry.hasAfter);
    QCOMPARE(entry.before.selfTotal, 25);
    QCOMPARE(entry.after.selfTotal, 50);
    QCOMPARE(entry.selfTotalDelta, 25);
    QCOMPARE(entry.callsDelta, 0);

    const QString rendered = Report::renderHotspotComparison(beforeTrace, afterTrace, comparison);
    QVERIFY(rendered.contains(QStringLiteral(
        "status=changed  groupKey=type=Binding|file=Same.qml|line=10|column=-1  type=Same.qml:10")));
    QVERIFY(rendered.contains(QStringLiteral(
        "beforeSelfTotal=25  afterSelfTotal=50  selfDelta=+25  selfDeltaPct=+100.0%")));
}

void QtdReaderTest::rendersSamegameHotspots()
{
    QtdReader reader;
    const auto trace = reader.readFile(dataFilePath(QStringLiteral("samegame.qzt")));

    QVERIFY2(trace.has_value(), qPrintable(reader.errorString()));

    const HotspotAnalysis analysis = Analyzer::analyzeHotspots(trace.value());
    QVERIFY(!analysis.hotspots.isEmpty());

    const QString rendered = Report::renderHotspots(trace.value(), analysis);
    QVERIFY(rendered.contains(QStringLiteral("selfTotal=")));
    QVERIFY(rendered.contains(QStringLiteral("medianSelf=")));
    QVERIFY(rendered.contains(QStringLiteral("outlier=")));
    QVERIFY(rendered.contains(QStringLiteral("Dialog.qml:23")));
}

void QtdReaderTest::marksGapWarnings()
{
    const TraceData trace = syntheticGapWarningTrace();
    const RangeAnalysis analysis = Analyzer::analyzeRanges(trace);

    QCOMPARE(analysis.nodes.size(), 3);
    QVERIFY(analysis.nodes.at(0).gapWarning);
    QVERIFY(!analysis.nodes.at(1).gapWarning);
    QVERIFY(!analysis.nodes.at(2).gapWarning);

    const QString rendered = Report::renderRanges(trace, analysis);
    QVERIFY(rendered.contains(QStringLiteral("gapWarning=yes")));
    QVERIFY(rendered.contains(QStringLiteral("note=possible_non_qml_work")));
    QVERIFY(rendered.contains(QStringLiteral("type=Warn.qml:1")));
}

void QtdReaderTest::addsPriority4Hints()
{
    const TraceData trace = syntheticPriority4Trace();
    const Summary summary = Analyzer::summarize(trace);
    const HotspotAnalysis analysis = Analyzer::analyzeHotspots(trace);

    QCOMPARE(analysis.hotspots.size(), 3);

    bool foundBurst = false;
    bool foundImage = false;
    bool foundCompiling = false;
    for (const auto &hotspot : analysis.hotspots) {
        if (hotspot.eventIndex == 0) {
            QVERIFY(hotspot.burstHint);
            foundBurst = true;
        } else if (hotspot.eventIndex == 1) {
            QVERIFY(hotspot.imageHint);
            foundImage = true;
        } else if (hotspot.eventIndex == 2) {
            QVERIFY(hotspot.compilingHint);
            foundCompiling = true;
        }
    }
    QVERIFY(foundBurst);
    QVERIFY(foundImage);
    QVERIFY(foundCompiling);

    const QString summaryRendered = Report::renderSummary(trace, summary);
    QVERIFY(summaryRendered.contains(
        QStringLiteral("note: large Compiling ranges can indicate large QML, but import/plugin load may also contribute.")));

    const QString hotspotsRendered = Report::renderHotspots(trace, analysis);
    QVERIFY(hotspotsRendered.contains(QStringLiteral("burst=yes")));
    QVERIFY(hotspotsRendered.contains(QStringLiteral("image=yes")));
    QVERIFY(hotspotsRendered.contains(QStringLiteral("compilingNote=yes")));
    QVERIFY(hotspotsRendered.contains(QStringLiteral("note=possible_image_related_cost")));
    QVERIFY(hotspotsRendered.contains(
        QStringLiteral("note=compiling_cost_may_include_import_or_plugin_load")));
}

void QtdReaderTest::rendersPixmapCacheAggregatesForSamegame()
{
    QtdReader reader;
    const auto trace = reader.readFile(dataFilePath(QStringLiteral("samegame.qzt")));

    QVERIFY2(trace.has_value(), qPrintable(reader.errorString()));

    const QString rendered = Report::renderAggregatedEvents(trace.value());
    QVERIFY(rendered.contains(QStringLiteral("# aggregates")));
    QVERIFY(rendered.contains(
        QStringLiteral("aggregate=PixmapCacheLoad  type=background.jpg:0  duration=18490692")));
    QVERIFY(rendered.contains(
        QStringLiteral("qtCreatorCategory=\"Pixmap Cache\"  qtCreatorLabel=\"Image Loaded\"")));
    QVERIFY(rendered.contains(QStringLiteral("file=background.jpg  width=909 height=600")));
    QVERIFY(rendered.contains(
        QStringLiteral("4670477111  idx=79  type=background.jpg:0")));
}

void QtdReaderTest::keepsEventsOutputRaw()
{
    QtdReader reader;
    const auto trace = reader.readFile(dataFilePath(QStringLiteral("samegame.qzt")));

    QVERIFY2(trace.has_value(), qPrintable(reader.errorString()));

    const QString rawRendered = Report::renderEvents(trace.value());
    QVERIFY(!rawRendered.contains(QStringLiteral("# aggregates")));
    QVERIFY(rawRendered.startsWith(QStringLiteral("4546017179  idx=0")));

    const QString aggregatedRendered = Report::renderAggregatedEvents(trace.value());
    QVERIFY(aggregatedRendered.contains(QStringLiteral("# aggregates")));
    QVERIFY(aggregatedRendered.contains(QStringLiteral("# raw events")));
}

void QtdReaderTest::aggregatesOverlappingPixmapLoads()
{
    const TraceData trace = syntheticPixmapTrace();
    const QString rendered = Report::renderAggregatedEvents(trace);

    QVERIFY(rendered.contains(
        QStringLiteral("100  aggregate=PixmapCacheLoad  type=shared.png:0  duration=30  qtCreatorCategory=\"Pixmap Cache\"  qtCreatorLabel=\"Image Loaded\"  file=shared.png  width=10 height=20")));
    QVERIFY(rendered.contains(
        QStringLiteral("110  aggregate=PixmapCacheLoad  type=shared.png:0  duration=35  qtCreatorCategory=\"Pixmap Cache\"  qtCreatorLabel=\"Image Loaded\"  file=shared.png  width=30 height=40")));
}

void QtdReaderTest::aggregatesSceneGraphMemoryAndDebugEvents()
{
    const TraceData trace = syntheticAggregateTrace();
    const QString rendered = Report::renderAggregatedEvents(trace);

    QVERIFY(rendered.contains(
        QStringLiteral("100  aggregate=DebugMessageSummary  level=Warning  count=2  qtCreatorCategory=\"Debug Messages\"  qtCreatorLabel=\"Warning Message\"  firstText=\"first warning\"")));
    QVERIFY(rendered.contains(
        QStringLiteral("200  aggregate=MemoryAllocationSummary  type=HeapPage  count=1  totalAmount=1024  minAmount=1024  maxAmount=1024  qtCreatorCategory=\"Memory Usage\"  qtCreatorType=\"Heap Allocation\"")));
    QVERIFY(rendered.contains(
        QStringLiteral("220  aggregate=MemoryAllocationSummary  type=SmallItem  count=2  totalAmount=48  minAmount=-16  maxAmount=64  qtCreatorCategory=\"Memory Usage\"  qtCreatorType=\"Heap Usage\"")));
    QVERIFY(rendered.contains(
        QStringLiteral("300  aggregate=SceneGraphFrame  type=SceneGraphRenderLoopFrame  totalTiming=35  qtCreatorCategory=\"Scene Graph\"  qtCreatorThread=\"Render Thread\"  qtCreatorStages=\"Render Thread Sync,Render,Swap\"  timing1=10 timing2=20 timing4=5")));
}

QTEST_APPLESS_MAIN(QtdReaderTest)

#include "test_qtdreader.moc"
