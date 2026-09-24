#include "qtdreader.h"

#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QBuffer>
#include <QDataStream>
#include <QXmlStreamAttributes>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <type_traits>

namespace QmlProfilerAnalyzer {

namespace {

enum Message {
    Event = 0,
    RangeStart = 1,
    RangeData = 2,
    RangeLocation = 3,
    RangeEnd = 4,
    Complete = 5,
    PixmapCacheEvent = 6,
    SceneGraphFrame = 7,
    MemoryAllocation = 8,
    DebugMessage = 9,
    MaximumMessage = 10
};

enum EventType {
    FramePaint = 0,
    Mouse = 1,
    Key = 2,
    AnimationFrame = 3
};

enum RangeType {
    Painting = 0,
    Compiling = 1,
    Creating = 2,
    Binding = 3,
    HandlingSignal = 4,
    Javascript = 5,
    MaximumRangeType = 6
};

enum PixmapEventType {
    PixmapSizeKnown = 0,
    PixmapReferenceCountChanged = 1,
    PixmapCacheCountChanged = 2
};

struct QztEventLocation
{
    QString filename;
    int line = -1;
    int column = -1;
};

struct QztEventType
{
    QString displayName;
    QString data;
    QztEventLocation location;
    quint8 message = MaximumMessage;
    quint8 rangeType = MaximumRangeType;
    int detailType = -1;
};

struct QztNote
{
    int typeIndex = -1;
    int collapsedRow = -1;
    qint64 startTime = -1;
    qint64 duration = 0;
    QString text;
};

struct QztEvent
{
    qint64 timestamp = -1;
    int typeIndex = -1;
    QVector<qint64> numbers;
    QByteArray byteData;

    int rangeStage() const
    {
        if (numbers.isEmpty())
            return -1;
        return static_cast<int>(numbers.first());
    }

    QString stringData() const
    {
        return QString::fromUtf8(byteData);
    }
};

QDataStream &operator>>(QDataStream &stream, QztEventLocation &location)
{
    return stream >> location.filename >> location.line >> location.column;
}

QDataStream &operator<<(QDataStream &stream, const QztEventLocation &location)
{
    return stream << location.filename << location.line << location.column;
}

QDataStream &operator>>(QDataStream &stream, QztEventType &type)
{
    quint8 message = MaximumMessage;
    quint8 rangeType = MaximumRangeType;
    stream >> type.displayName >> type.data >> type.location >> message >> rangeType
           >> type.detailType;
    type.message = message;
    type.rangeType = rangeType;
    return stream;
}

QDataStream &operator<<(QDataStream &stream, const QztEventType &type)
{
    return stream << type.displayName << type.data << type.location << type.message
                  << type.rangeType << type.detailType;
}

QDataStream &operator>>(QDataStream &stream, QztNote &note)
{
    return stream >> note.typeIndex >> note.collapsedRow >> note.startTime >> note.duration
                  >> note.text;
}

QDataStream &operator<<(QDataStream &stream, const QztNote &note)
{
    return stream << note.typeIndex << note.collapsedRow << note.startTime << note.duration
                  << note.text;
}

enum SerializationType {
    OneByte = 0,
    TwoByte = 1,
    FourByte = 2,
    EightByte = 3,
    TypeMask = 0x3
};

enum SerializationTypeOffset {
    TimestampOffset = 0,
    TypeIndexOffset = 2,
    DataLengthOffset = 4,
    DataOffset = 6
};

template <typename Number>
Number readSerializedNumber(QDataStream &stream, qint8 type)
{
    switch (type) {
    case OneByte: {
        qint8 value = 0;
        stream >> value;
        return static_cast<Number>(value);
    }
    case TwoByte: {
        qint16 value = 0;
        stream >> value;
        return static_cast<Number>(value);
    }
    case FourByte: {
        qint32 value = 0;
        stream >> value;
        return static_cast<Number>(value);
    }
    case EightByte: {
        qint64 value = 0;
        stream >> value;
        return static_cast<Number>(value);
    }
    default:
        return 0;
    }
}

QDataStream &operator>>(QDataStream &stream, QztEvent &event)
{
    qint8 type = 0;
    stream >> type;

    event.timestamp = readSerializedNumber<qint64>(stream, (type >> TimestampOffset) & TypeMask);
    event.typeIndex = readSerializedNumber<qint32>(stream, (type >> TypeIndexOffset) & TypeMask);
    const quint16 dataLength =
        readSerializedNumber<quint16>(stream, (type >> DataLengthOffset) & TypeMask);
    const uint bytesPerNumber = 1u << ((type >> DataOffset) & TypeMask);

    event.numbers.clear();
    event.byteData.clear();
    event.numbers.reserve(dataLength);
    if (bytesPerNumber == 1)
        event.byteData.reserve(dataLength);

    for (quint16 i = 0; i < dataLength; ++i) {
        qint64 value = 0;
        switch (bytesPerNumber) {
        case 1: {
            qint8 number = 0;
            stream >> number;
            value = number;
            event.byteData.append(static_cast<char>(number));
            break;
        }
        case 2: {
            qint16 number = 0;
            stream >> number;
            value = number;
            break;
        }
        case 4: {
            qint32 number = 0;
            stream >> number;
            value = number;
            break;
        }
        case 8: {
            qint64 number = 0;
            stream >> number;
            value = number;
            break;
        }
        default:
            stream.setStatus(QDataStream::ReadCorruptData);
            return stream;
        }
        event.numbers.append(value);
    }

    return stream;
}

void writeNumber(QDataStream &stream, qint64 value, SerializationType type)
{
    switch (type) {
    case OneByte:
        stream << static_cast<qint8>(value);
        break;
    case TwoByte:
        stream << static_cast<qint16>(value);
        break;
    case FourByte:
        stream << static_cast<qint32>(value);
        break;
    case EightByte:
        stream << value;
        break;
    }
}

QDataStream &operator<<(QDataStream &stream, const QztEvent &event)
{
    const SerializationType timestampType = EightByte;
    const SerializationType typeIndexType = FourByte;
    const SerializationType dataLengthType = TwoByte;
    const SerializationType dataType = event.byteData.isEmpty() ? EightByte : OneByte;
    const qsizetype dataLength = event.byteData.isEmpty() ? event.numbers.size()
                                                          : event.byteData.size();

    const qint8 packedType =
        static_cast<qint8>((timestampType << TimestampOffset)
                           | (typeIndexType << TypeIndexOffset)
                           | (dataLengthType << DataLengthOffset)
                           | (dataType << DataOffset));
    stream << packedType;
    writeNumber(stream, event.timestamp, timestampType);
    writeNumber(stream, event.typeIndex, typeIndexType);
    writeNumber(stream, dataLength, dataLengthType);

    if (!event.byteData.isEmpty()) {
        for (const char value : event.byteData)
            stream << static_cast<qint8>(value);
        return stream;
    }

    for (const qint64 value : event.numbers)
        writeNumber(stream, value, dataType);
    return stream;
}

QString messageToString(quint8 message)
{
    static const char *messageStrings[] = {
        "Event",
        "RangeStart",
        "RangeData",
        "RangeLocation",
        "RangeEnd",
        "Complete",
        "PixmapCache",
        "SceneGraph",
        "MemoryAllocation",
        "DebugMessage"
    };

    if (message < MaximumMessage)
        return QString::fromLatin1(messageStrings[message]);
    return QString();
}

QString rangeTypeToString(quint8 rangeType)
{
    static const char *rangeTypeStrings[] = {
        "Painting",
        "Compiling",
        "Creating",
        "Binding",
        "HandlingSignal",
        "Javascript"
    };

    if (rangeType < MaximumRangeType)
        return QString::fromLatin1(rangeTypeStrings[rangeType]);
    return QString();
}

EventTypeDef toEventTypeDef(const QztEventType &source, int index)
{
    EventTypeDef type;
    type.index = index;
    type.displayName = source.displayName;
    type.typeName = source.rangeType < MaximumRangeType ? rangeTypeToString(source.rangeType)
                                                        : messageToString(source.message);
    type.filename = source.location.filename;
    type.line = source.location.line;
    type.column = source.location.column;
    type.details = source.data;

    if (source.rangeType == Binding) {
        type.detailTag = QStringLiteral("bindingType");
        type.detailValue = source.detailType;
        type.hasDetailValue = true;
    } else if (source.message == Event) {
        if (source.detailType == AnimationFrame)
            type.detailTag = QStringLiteral("animationFrame");
        else if (source.detailType == Key)
            type.detailTag = QStringLiteral("keyEvent");
        else if (source.detailType == Mouse)
            type.detailTag = QStringLiteral("mouseEvent");
        if (!type.detailTag.isEmpty()) {
            type.detailValue = source.detailType;
            type.hasDetailValue = true;
        }
    } else if (source.message == PixmapCacheEvent) {
        type.detailTag = QStringLiteral("cacheEventType");
        type.detailValue = source.detailType;
        type.hasDetailValue = true;
    } else if (source.message == SceneGraphFrame) {
        type.detailTag = QStringLiteral("sgEventType");
        type.detailValue = source.detailType;
        type.hasDetailValue = true;
    } else if (source.message == MemoryAllocation) {
        type.detailTag = QStringLiteral("memoryEventType");
        type.detailValue = source.detailType;
        type.hasDetailValue = true;
    } else if (source.message == DebugMessage) {
        type.detailTag = QStringLiteral("level");
        type.detailValue = source.detailType;
        type.hasDetailValue = true;
    }

    return type;
}

void setCoreEventAttributes(TraceEvent &event)
{
    event.attributes.insert(QStringLiteral("startTime"), QString::number(event.startTime));
    event.attributes.insert(QStringLiteral("eventIndex"), QString::number(event.eventIndex));
    if (event.duration.has_value())
        event.attributes.insert(QStringLiteral("duration"), QString::number(event.duration.value()));
}

void addAttribute(TraceEvent &event, const char *name, qint64 value)
{
    event.attributes.insert(QString::fromLatin1(name), QString::number(value));
}

void ensureEventTypeCapacity(TraceData &trace, int index)
{
    if (index >= trace.eventTypes.size())
        trace.eventTypes.resize(index + 1);
}

void storeAttributes(const QXmlStreamAttributes &attributes, TraceEvent &event)
{
    for (const auto &attribute : attributes)
        event.attributes.insert(attribute.name().toString(), attribute.value().toString());
}

template <typename Integer>
std::optional<Integer> parseIntegerAttribute(const QXmlStreamAttributes &attributes,
                                             QLatin1String attributeName)
{
    const auto value = attributes.value(attributeName);
    if (value.isEmpty())
        return std::nullopt;

    bool ok = false;
    if constexpr (std::is_same_v<Integer, int>) {
        const int parsed = value.toInt(&ok);
        if (!ok)
            return std::nullopt;
        return parsed;
    } else {
        const qint64 parsed = value.toLongLong(&ok);
        if (!ok)
            return std::nullopt;
        return parsed;
    }
}

std::optional<int> parseIntegerText(const QString &value)
{
    bool ok = false;
    const int parsed = value.toInt(&ok);
    if (!ok)
        return std::nullopt;
    return parsed;
}

quint8 messageFromTypeName(const QString &typeName)
{
    if (typeName == QLatin1String("Event"))
        return Event;
    if (typeName == QLatin1String("PixmapCache"))
        return PixmapCacheEvent;
    if (typeName == QLatin1String("SceneGraph"))
        return SceneGraphFrame;
    if (typeName == QLatin1String("MemoryAllocation"))
        return MemoryAllocation;
    if (typeName == QLatin1String("DebugMessage"))
        return DebugMessage;
    return MaximumMessage;
}

quint8 rangeTypeFromTypeName(const QString &typeName)
{
    if (typeName == QLatin1String("Painting"))
        return Painting;
    if (typeName == QLatin1String("Compiling"))
        return Compiling;
    if (typeName == QLatin1String("Creating"))
        return Creating;
    if (typeName == QLatin1String("Binding"))
        return Binding;
    if (typeName == QLatin1String("HandlingSignal"))
        return HandlingSignal;
    if (typeName == QLatin1String("Javascript"))
        return Javascript;
    return MaximumRangeType;
}

bool isRangeTypeName(const QString &typeName)
{
    return rangeTypeFromTypeName(typeName) < MaximumRangeType;
}

QztEventType toQztEventType(const EventTypeDef &source)
{
    QztEventType type;
    type.displayName = source.displayName;
    type.data = source.details;
    type.location = {source.filename, source.line, source.column};
    type.rangeType = rangeTypeFromTypeName(source.typeName);
    type.message = type.rangeType < MaximumRangeType ? RangeData : messageFromTypeName(source.typeName);
    type.detailType = source.hasDetailValue ? source.detailValue : -1;
    return type;
}

qint64 attributeNumber(const TraceEvent &event, const QString &name)
{
    bool ok = false;
    const qint64 value = event.attributes.value(name).toLongLong(&ok);
    return ok ? value : 0;
}

QztEvent makeQztEvent(qint64 timestamp, int typeIndex, std::initializer_list<qint64> numbers)
{
    QztEvent event;
    event.timestamp = timestamp;
    event.typeIndex = typeIndex;
    event.numbers = QVector<qint64>(numbers.begin(), numbers.end());
    return event;
}

QztEvent toQztPointEvent(const TraceEvent &source, const EventTypeDef &type)
{
    QztEvent event;
    event.timestamp = source.startTime;
    event.typeIndex = source.eventIndex;

    if (type.typeName == QLatin1String("Event")) {
        if (type.detailTag == QLatin1String("animationFrame")) {
            event.numbers = {attributeNumber(source, QStringLiteral("framerate")),
                             attributeNumber(source, QStringLiteral("animationcount")),
                             attributeNumber(source, QStringLiteral("thread"))};
        } else if (type.detailTag == QLatin1String("keyEvent")
                   || type.detailTag == QLatin1String("mouseEvent")) {
            event.numbers = {attributeNumber(source, QStringLiteral("type")),
                             attributeNumber(source, QStringLiteral("data1")),
                             attributeNumber(source, QStringLiteral("data2"))};
        }
    } else if (type.typeName == QLatin1String("PixmapCache")) {
        if (type.detailValue == PixmapSizeKnown) {
            event.numbers = {attributeNumber(source, QStringLiteral("width")),
                             attributeNumber(source, QStringLiteral("height"))};
        } else if (type.detailValue == PixmapReferenceCountChanged
                   || type.detailValue == PixmapCacheCountChanged) {
            event.numbers = {0, 0, attributeNumber(source, QStringLiteral("refCount"))};
        }
    } else if (type.typeName == QLatin1String("SceneGraph")) {
        for (int i = 1; i <= 5; ++i)
            event.numbers.append(attributeNumber(source, QStringLiteral("timing%1").arg(i)));
    } else if (type.typeName == QLatin1String("MemoryAllocation")) {
        event.numbers = {attributeNumber(source, QStringLiteral("amount"))};
    } else if (type.typeName == QLatin1String("DebugMessage")) {
        event.byteData = source.attributes.value(QStringLiteral("text")).toUtf8();
    }

    return event;
}

bool writeCompressedBuffer(QDataStream &stream, QBuffer &buffer)
{
    if (buffer.data().isEmpty())
        return true;
    stream << qCompress(buffer.data());
    buffer.close();
    buffer.buffer().clear();
    return buffer.open(QIODevice::WriteOnly);
}

} // namespace

std::optional<TraceData> QtdReader::readFile(const QString &filePath)
{
    m_errorString.clear();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        setError(QStringLiteral("Could not open %1 for reading.").arg(filePath));
        return std::nullopt;
    }

    const QString suffix = QFileInfo(filePath).suffix().toLower();
    if (suffix == QLatin1String("qzt"))
        return readQztDevice(&file);
    if (suffix == QLatin1String("qtd"))
        return readXmlDevice(&file);

    return readDevice(&file);
}

QString QtdReader::errorString() const
{
    return m_errorString;
}

bool QtdReader::writeQtdFile(const TraceData &trace, const QString &filePath)
{
    m_errorString.clear();

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        setError(QStringLiteral("Could not open %1 for writing.").arg(filePath));
        return false;
    }

    return writeXmlDevice(trace, &file);
}

bool QtdReader::writeQztFile(const TraceData &trace, const QString &filePath)
{
    m_errorString.clear();

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        setError(QStringLiteral("Could not open %1 for writing.").arg(filePath));
        return false;
    }

    return writeQztDevice(trace, &file);
}

void QtdReader::setError(const QString &message)
{
    if (m_errorString.isEmpty())
        m_errorString = message;
}

std::optional<TraceData> QtdReader::readDevice(QIODevice *device)
{
    const QByteArray prefix = device->peek(64).trimmed();
    if (prefix.startsWith("<?xml") || prefix.startsWith("<trace"))
        return readXmlDevice(device);
    return readQztDevice(device);
}

std::optional<TraceData> QtdReader::readXmlDevice(QIODevice *device)
{
    QXmlStreamReader xml(device);
    TraceData trace;
    trace.fileFormat = QStringLiteral("qtd/xml");

    if (!readTrace(xml, trace)) {
        if (m_errorString.isEmpty()) {
            if (xml.hasError())
                setError(QStringLiteral("XML parse error: %1").arg(xml.errorString()));
            else
                setError(QStringLiteral("Failed to read trace data."));
        }
        return std::nullopt;
    }

    if (xml.hasError()) {
        setError(QStringLiteral("XML parse error: %1").arg(xml.errorString()));
        return std::nullopt;
    }

    return trace;
}

bool QtdReader::writeXmlDevice(const TraceData &trace, QIODevice *device)
{
    QXmlStreamWriter xml(device);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();

    xml.writeStartElement(QStringLiteral("trace"));
    xml.writeAttribute(QStringLiteral("version"),
                       trace.version.isEmpty() ? QStringLiteral("1.02") : trace.version);
    if (trace.traceStart >= 0)
        xml.writeAttribute(QStringLiteral("traceStart"), QString::number(trace.traceStart));
    if (trace.traceEnd >= 0)
        xml.writeAttribute(QStringLiteral("traceEnd"), QString::number(trace.traceEnd));

    xml.writeStartElement(QStringLiteral("eventData"));
    if (trace.totalTime >= 0)
        xml.writeAttribute(QStringLiteral("totalTime"), QString::number(trace.totalTime));
    for (int index = 0; index < trace.eventTypes.size(); ++index) {
        const EventTypeDef &type = trace.eventTypes.at(index);
        xml.writeStartElement(QStringLiteral("event"));
        xml.writeAttribute(QStringLiteral("index"),
                           QString::number(type.index >= 0 ? type.index : index));
        xml.writeTextElement(QStringLiteral("displayname"), type.displayName);
        xml.writeTextElement(QStringLiteral("type"), type.typeName);
        if (!type.filename.isEmpty()) {
            xml.writeTextElement(QStringLiteral("filename"), type.filename);
            xml.writeTextElement(QStringLiteral("line"), QString::number(type.line));
            xml.writeTextElement(QStringLiteral("column"), QString::number(type.column));
        }
        if (!type.details.isEmpty())
            xml.writeTextElement(QStringLiteral("details"), type.details);
        if (type.hasDetailValue && !type.detailTag.isEmpty())
            xml.writeTextElement(type.detailTag, QString::number(type.detailValue));
        xml.writeEndElement();
    }
    xml.writeEndElement();

    xml.writeStartElement(QStringLiteral("profilerDataModel"));
    for (const TraceEvent &event : trace.events) {
        xml.writeStartElement(QStringLiteral("range"));
        for (auto it = event.attributes.cbegin(); it != event.attributes.cend(); ++it)
            xml.writeAttribute(it.key(), it.value());
        xml.writeEndElement();
    }
    xml.writeEndElement();

    xml.writeStartElement(QStringLiteral("noteData"));
    for (const TraceNote &note : trace.notes) {
        xml.writeStartElement(QStringLiteral("note"));
        xml.writeAttribute(QStringLiteral("startTime"), QString::number(note.startTime));
        xml.writeAttribute(QStringLiteral("duration"), QString::number(note.duration));
        xml.writeAttribute(QStringLiteral("eventIndex"), QString::number(note.eventIndex));
        if (note.collapsedRow >= 0)
            xml.writeAttribute(QStringLiteral("collapsedRow"), QString::number(note.collapsedRow));
        xml.writeCharacters(note.text);
        xml.writeEndElement();
    }
    xml.writeEndElement();

    xml.writeEndElement();
    xml.writeEndDocument();

    if (xml.hasError()) {
        setError(QStringLiteral("Error while writing .qtd XML."));
        return false;
    }

    return true;
}

bool QtdReader::writeQztDevice(const TraceData &trace, QIODevice *device)
{
    QDataStream stream(device);
    stream.setVersion(QDataStream::Qt_5_5);
    stream << QByteArray("QMLPROFILER");
    stream << static_cast<qint32>(QDataStream::Qt_DefaultCompiledVersion);
    stream.setVersion(QDataStream::Qt_DefaultCompiledVersion);

    stream << trace.traceStart << trace.traceEnd;

    {
        QBuffer buffer;
        if (!buffer.open(QIODevice::WriteOnly)) {
            setError(QStringLiteral("Failed to open .qzt event types buffer for writing."));
            return false;
        }
        QDataStream bufferStream(&buffer);
        bufferStream.setVersion(stream.version());
        bufferStream << static_cast<quint32>(trace.eventTypes.size());
        for (const EventTypeDef &eventType : trace.eventTypes)
            bufferStream << toQztEventType(eventType);
        stream << qCompress(buffer.data());
    }

    {
        QBuffer buffer;
        if (!buffer.open(QIODevice::WriteOnly)) {
            setError(QStringLiteral("Failed to open .qzt notes buffer for writing."));
            return false;
        }
        QDataStream bufferStream(&buffer);
        bufferStream.setVersion(stream.version());
        QList<QztNote> notes;
        notes.reserve(trace.notes.size());
        for (const TraceNote &source : trace.notes)
            notes.append({source.eventIndex, source.collapsedRow, source.startTime, source.duration,
                          source.text});
        bufferStream << notes;
        stream << qCompress(buffer.data());
    }

    QBuffer buffer;
    if (!buffer.open(QIODevice::WriteOnly)) {
        setError(QStringLiteral("Failed to open .qzt event buffer for writing."));
        return false;
    }
    QDataStream bufferStream(&buffer);
    bufferStream.setVersion(stream.version());

    for (const TraceEvent &source : trace.events) {
        if (source.eventIndex < 0 || source.eventIndex >= trace.eventTypes.size()) {
            setError(QStringLiteral("Invalid event type index while writing .qzt: %1")
                         .arg(source.eventIndex));
            return false;
        }

        const EventTypeDef &type = trace.eventTypes.at(source.eventIndex);
        if (source.duration.has_value() && isRangeTypeName(type.typeName)) {
            bufferStream << makeQztEvent(source.startTime, source.eventIndex, {RangeStart});
            bufferStream << makeQztEvent(source.startTime + source.duration.value(),
                                         source.eventIndex,
                                         {RangeEnd});
        } else {
            bufferStream << toQztPointEvent(source, type);
        }

        if (buffer.data().size() > (1 << 25) && !writeCompressedBuffer(stream, buffer)) {
            setError(QStringLiteral("Failed to flush .qzt event buffer."));
            return false;
        }
    }

    if (!writeCompressedBuffer(stream, buffer)) {
        setError(QStringLiteral("Failed to flush .qzt event buffer."));
        return false;
    }

    if (stream.status() != QDataStream::Ok) {
        setError(QStringLiteral("Error while writing .qzt stream."));
        return false;
    }

    return true;
}

std::optional<TraceData> QtdReader::readQztDevice(QIODevice *device)
{
    TraceData trace;
    trace.fileFormat = QStringLiteral("qzt/binary");

    QDataStream stream(device);
    stream.setVersion(QDataStream::Qt_5_5);

    QByteArray magic;
    stream >> magic;
    if (magic != QByteArray("QMLPROFILER")) {
        setError(QStringLiteral("Invalid .qzt magic."));
        return std::nullopt;
    }

    qint32 dataStreamVersion = 0;
    stream >> dataStreamVersion;
    if (dataStreamVersion > QDataStream::Qt_DefaultCompiledVersion) {
        setError(QStringLiteral("Unsupported .qzt data stream version: %1").arg(dataStreamVersion));
        return std::nullopt;
    }
    stream.setVersion(static_cast<QDataStream::Version>(dataStreamVersion));

    stream >> trace.traceStart >> trace.traceEnd;
    if (trace.traceStart >= 0 && trace.traceEnd >= trace.traceStart)
        trace.totalTime = trace.traceEnd - trace.traceStart;

    if (!readQztEventTypes(stream, trace) || !readQztNotes(stream, trace) || !readQztEvents(stream, trace))
        return std::nullopt;

    if (stream.status() != QDataStream::Ok && stream.status() != QDataStream::ReadPastEnd) {
        setError(QStringLiteral("Error while reading .qzt stream."));
        return std::nullopt;
    }

    return trace;
}

bool QtdReader::readQztEventTypes(QDataStream &stream, TraceData &trace)
{
    QByteArray compressedData;
    stream >> compressedData;
    const QByteArray data = qUncompress(compressedData);
    if (compressedData.isEmpty() || data.isEmpty()) {
        setError(QStringLiteral("Failed to decompress .qzt event types block."));
        return false;
    }

    QBuffer buffer;
    buffer.setData(data);
    if (!buffer.open(QIODevice::ReadOnly)) {
        setError(QStringLiteral("Failed to open .qzt event types buffer."));
        return false;
    }

    QDataStream bufferStream(&buffer);
    bufferStream.setVersion(stream.version());
    quint32 numEventTypes = 0;
    bufferStream >> numEventTypes;
    trace.eventTypes.resize(static_cast<int>(numEventTypes));
    for (quint32 index = 0; index < numEventTypes; ++index) {
        QztEventType rawType;
        bufferStream >> rawType;
        if (bufferStream.status() != QDataStream::Ok) {
            setError(QStringLiteral("Corrupt .qzt event type block."));
            return false;
        }
        trace.eventTypes[static_cast<int>(index)] = toEventTypeDef(rawType, static_cast<int>(index));
    }

    return true;
}

bool QtdReader::readQztNotes(QDataStream &stream, TraceData &trace)
{
    QByteArray compressedData;
    stream >> compressedData;
    const QByteArray data = qUncompress(compressedData);
    if (compressedData.isEmpty() || data.isEmpty()) {
        setError(QStringLiteral("Failed to decompress .qzt notes block."));
        return false;
    }

    QBuffer buffer;
    buffer.setData(data);
    if (!buffer.open(QIODevice::ReadOnly)) {
        setError(QStringLiteral("Failed to open .qzt notes buffer."));
        return false;
    }

    QDataStream bufferStream(&buffer);
    bufferStream.setVersion(stream.version());
    QList<QztNote> notes;
    bufferStream >> notes;
    if (bufferStream.status() != QDataStream::Ok) {
        setError(QStringLiteral("Corrupt .qzt notes block."));
        return false;
    }

    trace.notes.reserve(notes.size());
    for (const auto &rawNote : notes) {
        TraceNote note;
        note.eventIndex = rawNote.typeIndex;
        note.collapsedRow = rawNote.collapsedRow;
        note.startTime = rawNote.startTime;
        note.duration = rawNote.duration;
        note.text = rawNote.text;
        trace.notes.append(std::move(note));
    }

    return true;
}

bool QtdReader::readQztEvents(QDataStream &stream, TraceData &trace)
{
    QVector<QztEvent> openRanges;

    while (!stream.atEnd()) {
        QByteArray compressedData;
        stream >> compressedData;
        if (stream.status() == QDataStream::ReadPastEnd)
            break;
        if (stream.status() != QDataStream::Ok) {
            setError(QStringLiteral("Failed while reading .qzt event chunk."));
            return false;
        }

        const QByteArray data = qUncompress(compressedData);
        if (compressedData.isEmpty() || data.isEmpty()) {
            setError(QStringLiteral("Failed to decompress .qzt event block."));
            return false;
        }

        QBuffer buffer;
        buffer.setData(data);
        if (!buffer.open(QIODevice::ReadOnly)) {
            setError(QStringLiteral("Failed to open .qzt event buffer."));
            return false;
        }

        QDataStream bufferStream(&buffer);
        bufferStream.setVersion(stream.version());
        while (!buffer.atEnd()) {
            QztEvent rawEvent;
            bufferStream >> rawEvent;
            if (bufferStream.status() == QDataStream::ReadPastEnd)
                break;
            if (bufferStream.status() != QDataStream::Ok) {
                setError(QStringLiteral("Corrupt .qzt event block."));
                return false;
            }

            if (rawEvent.typeIndex < 0 || rawEvent.typeIndex >= trace.eventTypes.size()) {
                setError(QStringLiteral("Invalid .qzt event type index: %1").arg(rawEvent.typeIndex));
                return false;
            }

            const EventTypeDef &type = trace.eventTypes.at(rawEvent.typeIndex);
            const bool isRangeType = !type.typeName.isEmpty()
                && (type.typeName == QLatin1String("Painting")
                    || type.typeName == QLatin1String("Compiling")
                    || type.typeName == QLatin1String("Creating")
                    || type.typeName == QLatin1String("Binding")
                    || type.typeName == QLatin1String("HandlingSignal")
                    || type.typeName == QLatin1String("Javascript"));

            if (isRangeType && rawEvent.rangeStage() == RangeStart) {
                openRanges.append(rawEvent);
                continue;
            }

            if (isRangeType && rawEvent.rangeStage() == RangeEnd) {
                int startIndex = -1;
                for (int i = openRanges.size() - 1; i >= 0; --i) {
                    if (openRanges.at(i).typeIndex == rawEvent.typeIndex) {
                        startIndex = i;
                        break;
                    }
                }
                if (startIndex < 0)
                    continue;

                const QztEvent startEvent = openRanges.takeAt(startIndex);
                TraceEvent event;
                event.startTime = std::max<qint64>(0, startEvent.timestamp);
                event.eventIndex = startEvent.typeIndex;
                event.duration = std::max<qint64>(0, rawEvent.timestamp - startEvent.timestamp);
                setCoreEventAttributes(event);
                trace.events.append(std::move(event));
                continue;
            }

            TraceEvent event;
            event.startTime = std::max<qint64>(0, rawEvent.timestamp);
            event.eventIndex = rawEvent.typeIndex;
            setCoreEventAttributes(event);

            if (type.typeName == QLatin1String("Event")) {
                if (type.detailTag == QLatin1String("animationFrame")) {
                    if (rawEvent.numbers.size() > 0)
                        addAttribute(event, "framerate", rawEvent.numbers.at(0));
                    if (rawEvent.numbers.size() > 1)
                        addAttribute(event, "animationcount", rawEvent.numbers.at(1));
                    if (rawEvent.numbers.size() > 2)
                        addAttribute(event, "thread", rawEvent.numbers.at(2));
                } else if (type.detailTag == QLatin1String("keyEvent")
                           || type.detailTag == QLatin1String("mouseEvent")) {
                    if (rawEvent.numbers.size() > 0)
                        addAttribute(event, "type", rawEvent.numbers.at(0));
                    if (rawEvent.numbers.size() > 1)
                        addAttribute(event, "data1", rawEvent.numbers.at(1));
                    if (rawEvent.numbers.size() > 2)
                        addAttribute(event, "data2", rawEvent.numbers.at(2));
                }
            } else if (type.typeName == QLatin1String("PixmapCache")) {
                if (type.detailValue == PixmapSizeKnown) {
                    if (rawEvent.numbers.size() > 0)
                        addAttribute(event, "width", rawEvent.numbers.at(0));
                    if (rawEvent.numbers.size() > 1)
                        addAttribute(event, "height", rawEvent.numbers.at(1));
                } else if (type.detailValue == PixmapReferenceCountChanged
                           || type.detailValue == PixmapCacheCountChanged) {
                    if (rawEvent.numbers.size() > 2)
                        addAttribute(event, "refCount", rawEvent.numbers.at(2));
                }
            } else if (type.typeName == QLatin1String("SceneGraph")) {
                for (int i = 0; i < rawEvent.numbers.size() && i < 5; ++i) {
                    if (rawEvent.numbers.at(i) > 0)
                        addAttribute(event, QString("timing%1").arg(i + 1).toLatin1().constData(),
                                     rawEvent.numbers.at(i));
                }
            } else if (type.typeName == QLatin1String("MemoryAllocation")) {
                if (!rawEvent.numbers.isEmpty())
                    addAttribute(event, "amount", rawEvent.numbers.at(0));
            } else if (type.typeName == QLatin1String("DebugMessage")) {
                event.attributes.insert(QStringLiteral("text"), rawEvent.stringData());
            }

            trace.events.append(std::move(event));
        }
    }

    return true;
}

bool QtdReader::readTrace(QXmlStreamReader &xml, TraceData &trace)
{
    while (!xml.atEnd()) {
        const auto token = xml.readNext();
        if (token != QXmlStreamReader::StartElement)
            continue;

        if (xml.name() != QLatin1String("trace")) {
            setError(QStringLiteral("Unexpected root element: %1").arg(xml.name().toString()));
            return false;
        }

        const auto attributes = xml.attributes();
        trace.version = attributes.value(QLatin1String("version")).toString();
        if (const auto traceStart = parseIntegerAttribute<qint64>(attributes,
                                                                  QLatin1String("traceStart"))) {
            trace.traceStart = traceStart.value();
        }
        if (const auto traceEnd = parseIntegerAttribute<qint64>(attributes,
                                                                QLatin1String("traceEnd"))) {
            trace.traceEnd = traceEnd.value();
        }

        while (!xml.atEnd()) {
            const auto childToken = xml.readNext();
            if (childToken == QXmlStreamReader::EndElement && xml.name() == QLatin1String("trace"))
                return true;

            if (childToken != QXmlStreamReader::StartElement)
                continue;

            if (xml.name() == QLatin1String("eventData")) {
                if (!readEventData(xml, trace))
                    return false;
            } else if (xml.name() == QLatin1String("profilerDataModel")) {
                if (!readProfilerDataModel(xml, trace))
                    return false;
            } else if (xml.name() == QLatin1String("noteData")) {
                if (!readNoteData(xml, trace))
                    return false;
            } else {
                xml.skipCurrentElement();
            }
        }
    }

    setError(QStringLiteral("Missing trace element."));
    return false;
}

bool QtdReader::readEventData(QXmlStreamReader &xml, TraceData &trace)
{
    if (const auto totalTime = parseIntegerAttribute<qint64>(xml.attributes(),
                                                             QLatin1String("totalTime"))) {
        trace.totalTime = totalTime.value();
    }

    while (!xml.atEnd()) {
        const auto token = xml.readNext();
        if (token == QXmlStreamReader::EndElement && xml.name() == QLatin1String("eventData"))
            return true;

        if (token != QXmlStreamReader::StartElement || xml.name() != QLatin1String("event"))
            continue;

        const auto eventAttributes = xml.attributes();
        const auto index = parseIntegerAttribute<int>(eventAttributes, QLatin1String("index"));
        if (!index.has_value()) {
            xml.skipCurrentElement();
            continue;
        }

        EventTypeDef type;
        type.index = index.value();

        while (!xml.atEnd()) {
            const auto eventToken = xml.readNext();
            if (eventToken == QXmlStreamReader::EndElement && xml.name() == QLatin1String("event"))
                break;

            if (eventToken != QXmlStreamReader::StartElement)
                continue;

            const QString name = xml.name().toString();
            const QString value = xml.readElementText();

            if (name == QLatin1String("displayname"))
                type.displayName = value;
            else if (name == QLatin1String("type"))
                type.typeName = value;
            else if (name == QLatin1String("filename"))
                type.filename = value;
            else if (name == QLatin1String("line")) {
                if (const auto parsed = parseIntegerText(value))
                    type.line = parsed.value();
            } else if (name == QLatin1String("column")) {
                if (const auto parsed = parseIntegerText(value))
                    type.column = parsed.value();
            }
            else if (name == QLatin1String("details"))
                type.details = value;
            else if (name == QLatin1String("bindingType")
                     || name == QLatin1String("animationFrame")
                     || name == QLatin1String("keyEvent")
                     || name == QLatin1String("mouseEvent")
                     || name == QLatin1String("cacheEventType")
                     || name == QLatin1String("sgEventType")
                     || name == QLatin1String("memoryEventType")
                     || name == QLatin1String("level")) {
                if (const auto parsed = parseIntegerText(value)) {
                    type.detailTag = name;
                    type.detailValue = parsed.value();
                    type.hasDetailValue = true;
                }
            }
        }

        if (type.index >= 0) {
            ensureEventTypeCapacity(trace, type.index);
            trace.eventTypes[type.index] = type;
        }
    }

    return !xml.hasError();
}

bool QtdReader::readProfilerDataModel(QXmlStreamReader &xml, TraceData &trace)
{
    while (!xml.atEnd()) {
        const auto token = xml.readNext();
        if (token == QXmlStreamReader::EndElement
            && xml.name() == QLatin1String("profilerDataModel")) {
            return true;
        }

        if (token != QXmlStreamReader::StartElement || xml.name() != QLatin1String("range"))
            continue;

        TraceEvent event;
        const auto attributes = xml.attributes();
        const auto startTime = parseIntegerAttribute<qint64>(attributes, QLatin1String("startTime"));
        const auto eventIndex = parseIntegerAttribute<int>(attributes, QLatin1String("eventIndex"));
        if (!startTime.has_value() || !eventIndex.has_value()) {
            xml.skipCurrentElement();
            continue;
        }

        event.startTime = startTime.value();
        event.eventIndex = eventIndex.value();
        if (attributes.hasAttribute(QLatin1String("duration"))) {
            const auto duration = parseIntegerAttribute<qint64>(attributes, QLatin1String("duration"));
            if (!duration.has_value()) {
                xml.skipCurrentElement();
                continue;
            }
            event.duration = duration.value();
        }
        storeAttributes(attributes, event);
        trace.events.append(std::move(event));
        xml.skipCurrentElement();
    }

    return !xml.hasError();
}

bool QtdReader::readNoteData(QXmlStreamReader &xml, TraceData &trace)
{
    while (!xml.atEnd()) {
        const auto token = xml.readNext();
        if (token == QXmlStreamReader::EndElement && xml.name() == QLatin1String("noteData"))
            return true;

        if (token != QXmlStreamReader::StartElement || xml.name() != QLatin1String("note"))
            continue;

        const auto attributes = xml.attributes();
        const auto startTime = parseIntegerAttribute<qint64>(attributes, QLatin1String("startTime"));
        const auto duration = parseIntegerAttribute<qint64>(attributes, QLatin1String("duration"));
        const auto eventIndex = parseIntegerAttribute<int>(attributes, QLatin1String("eventIndex"));
        if (!startTime.has_value() || !duration.has_value() || !eventIndex.has_value()) {
            xml.skipCurrentElement();
            continue;
        }

        TraceNote note;
        note.startTime = startTime.value();
        note.duration = duration.value();
        note.eventIndex = eventIndex.value();
        if (attributes.hasAttribute(QLatin1String("collapsedRow"))) {
            if (const auto collapsedRow = parseIntegerAttribute<int>(attributes,
                                                                     QLatin1String("collapsedRow"))) {
                note.collapsedRow = collapsedRow.value();
            }
        }
        note.text = xml.readElementText();
        trace.notes.append(std::move(note));
    }

    return !xml.hasError();
}

} // namespace QmlProfilerAnalyzer
