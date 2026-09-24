#pragma once

#include <QMap>
#include <QString>
#include <QVector>

#include <optional>

namespace QmlProfilerAnalyzer {

struct EventTypeDef
{
    int index = -1;
    QString displayName;
    QString typeName;
    QString filename;
    int line = -1;
    int column = -1;
    QString details;
    QString detailTag;
    int detailValue = -1;
    bool hasDetailValue = false;
};

struct TraceEvent
{
    qint64 startTime = 0;
    std::optional<qint64> duration;
    int eventIndex = -1;
    QMap<QString, QString> attributes;
};

struct TraceNote
{
    qint64 startTime = 0;
    qint64 duration = 0;
    int eventIndex = -1;
    int collapsedRow = -1;
    QString text;
};

struct TraceData
{
    QString fileFormat = QStringLiteral("qtd/xml");
    QString version;
    qint64 traceStart = -1;
    qint64 traceEnd = -1;
    qint64 totalTime = -1;
    QVector<EventTypeDef> eventTypes;
    QVector<TraceEvent> events;
    QVector<TraceNote> notes;

    const EventTypeDef *eventType(int index) const
    {
        if (index < 0 || index >= eventTypes.size())
            return nullptr;
        return &eventTypes.at(index);
    }
};

} // namespace QmlProfilerAnalyzer
