#include "report.h"

#include <QFileInfo>
#include <QHash>
#include <QList>
#include <QStringList>

#include <algorithm>
#include <limits>

namespace QmlProfilerAnalyzer {

namespace {

constexpr int pixmapSizeKnown = 0;
constexpr int pixmapLoadingStarted = 3;
constexpr int pixmapLoadingFinished = 4;
constexpr int memoryHeapPage = 0;
constexpr int memoryLargeItem = 1;
constexpr int memorySmallItem = 2;

QString detailLabel(const EventTypeDef &type)
{
    if (!type.hasDetailValue || type.detailTag.isEmpty())
        return QString();
    return QStringLiteral("%1=%2").arg(type.detailTag).arg(type.detailValue);
}

const EventTypeDef *resolvedType(const TraceData &trace, int eventIndex)
{
    return trace.eventType(eventIndex);
}

QString resolvedTypeName(const TraceData &trace, int eventIndex)
{
    const auto *type = resolvedType(trace, eventIndex);
    if (!type)
        return QStringLiteral("<unknown>");
    if (!type->displayName.isEmpty())
        return type->displayName;
    if (!type->typeName.isEmpty())
        return type->typeName;
    return QStringLiteral("<unnamed>");
}

QString comparisonTypeName(const TraceData &beforeTrace,
                           const TraceData &afterTrace,
                           const HotspotComparisonEntry &entry)
{
    if (entry.hasBefore)
        return resolvedTypeName(beforeTrace, entry.before.eventIndex);
    if (entry.hasAfter)
        return resolvedTypeName(afterTrace, entry.after.eventIndex);
    return QStringLiteral("<unknown>");
}

QString comparisonStatus(const HotspotComparisonEntry &entry)
{
    if (entry.hasBefore && entry.hasAfter) {
        if (entry.selfTotalDelta == 0
            && entry.inclusiveTotalDelta == 0
            && entry.callsDelta == 0
            && entry.medianSelfDelta == 0
            && entry.maxSelfDelta == 0) {
            return QStringLiteral("unchanged");
        }
        return QStringLiteral("changed");
    }
    if (entry.hasAfter)
        return QStringLiteral("added");
    return QStringLiteral("removed");
}

QString formatSigned(qint64 value)
{
    if (value > 0)
        return QStringLiteral("+%1").arg(value);
    return QString::number(value);
}

QString formatSigned(int value)
{
    if (value > 0)
        return QStringLiteral("+%1").arg(value);
    return QString::number(value);
}

QString formatPercent(qint64 beforeValue, qint64 delta)
{
    if (beforeValue == 0)
        return QStringLiteral("n/a");
    const double percent = (static_cast<double>(delta) * 100.0)
        / static_cast<double>(beforeValue);
    const QString sign = percent > 0.0 ? QStringLiteral("+") : QString();
    return QStringLiteral("%1%2%").arg(sign, QString::number(percent, 'f', 1));
}

QString quoteValue(QString value)
{
    value.replace(QLatin1Char('\\'), QStringLiteral("\\\\"));
    value.replace(QLatin1Char('"'), QStringLiteral("\\\""));
    value.replace(QLatin1Char('\n'), QStringLiteral("\\n"));
    return QStringLiteral("\"%1\"").arg(value);
}

std::optional<qint64> parseInt64(const QString &value)
{
    bool ok = false;
    const qint64 parsed = value.toLongLong(&ok);
    if (!ok)
        return std::nullopt;
    return parsed;
}

QString formatEventLine(const TraceData &trace, const TraceEvent &event)
{
    QString line = QStringLiteral("%1  idx=%2  type=%3")
                       .arg(event.startTime)
                       .arg(event.eventIndex)
                       .arg(resolvedTypeName(trace, event.eventIndex));

    if (event.duration.has_value())
        line += QStringLiteral("  duration=%1").arg(event.duration.value());

    QStringList extras;
    for (auto it = event.attributes.cbegin(); it != event.attributes.cend(); ++it) {
        if (it.key() == QLatin1String("startTime")
            || it.key() == QLatin1String("duration")
            || it.key() == QLatin1String("eventIndex")) {
            continue;
        }
        extras.append(QStringLiteral("%1=%2").arg(it.key(), it.value()));
    }
    if (!extras.isEmpty())
        line += QStringLiteral("  ") + extras.join(QStringLiteral(" "));

    return line;
}

QString sceneGraphFrameTypeName(int value)
{
    switch (value) {
    case 0:
        return QStringLiteral("SceneGraphRendererFrame");
    case 1:
        return QStringLiteral("SceneGraphAdaptationLayerFrame");
    case 2:
        return QStringLiteral("SceneGraphContextFrame");
    case 3:
        return QStringLiteral("SceneGraphRenderLoopFrame");
    case 4:
        return QStringLiteral("SceneGraphTexturePrepare");
    case 5:
        return QStringLiteral("SceneGraphTextureDeletion");
    case 6:
        return QStringLiteral("SceneGraphPolishAndSync");
    case 7:
        return QStringLiteral("SceneGraphWindowsRenderShow");
    case 8:
        return QStringLiteral("SceneGraphWindowsAnimations");
    case 9:
        return QStringLiteral("SceneGraphPolishFrame");
    default:
        return QStringLiteral("SceneGraph:%1").arg(value);
    }
}

QString memoryAllocationTypeName(int value)
{
    switch (value) {
    case memoryHeapPage:
        return QStringLiteral("HeapPage");
    case memoryLargeItem:
        return QStringLiteral("LargeItem");
    case memorySmallItem:
        return QStringLiteral("SmallItem");
    default:
        return QStringLiteral("MemoryAllocation:%1").arg(value);
    }
}

QString debugMessageLevelName(int value)
{
    switch (value) {
    case 0:
        return QStringLiteral("Debug");
    case 1:
        return QStringLiteral("Warning");
    case 2:
        return QStringLiteral("Critical");
    case 3:
        return QStringLiteral("Fatal");
    case 4:
        return QStringLiteral("Info");
    default:
        return QStringLiteral("Level%1").arg(value);
    }
}

QString qtCreatorPixmapCategory()
{
    return QStringLiteral("Pixmap Cache");
}

QString qtCreatorPixmapLabel()
{
    return QStringLiteral("Image Loaded");
}

QString qtCreatorMemoryCategory()
{
    return QStringLiteral("Memory Usage");
}

QString qtCreatorMemoryTypeName(int value)
{
    switch (value) {
    case memoryHeapPage:
        return QStringLiteral("Heap Allocation");
    case memoryLargeItem:
        return QStringLiteral("Large Item Allocation");
    case memorySmallItem:
        return QStringLiteral("Heap Usage");
    default:
        return QStringLiteral("Memory Usage");
    }
}

QString qtCreatorDebugCategory()
{
    return QStringLiteral("Debug Messages");
}

QString qtCreatorDebugLabel(int value)
{
    switch (value) {
    case 0:
        return QStringLiteral("Debug Message");
    case 1:
        return QStringLiteral("Warning Message");
    case 2:
        return QStringLiteral("Critical Message");
    case 3:
        return QStringLiteral("Fatal Message");
    case 4:
        return QStringLiteral("Info Message");
    default:
        return QStringLiteral("Unknown Message");
    }
}

QString qtCreatorSceneGraphThreadLabel(int value)
{
    switch (value) {
    case 3:
        return QStringLiteral("Render Thread");
    case 6:
    case 8:
    case 9:
        return QStringLiteral("GUI Thread");
    case 0:
    case 1:
    case 2:
    case 4:
    case 5:
        return QStringLiteral("Render Thread Details");
    default:
        return QStringLiteral("Scene Graph");
    }
}

QStringList qtCreatorSceneGraphStageLabels(int value)
{
    switch (value) {
    case 0:
        return {QStringLiteral("Render Preprocess"), QStringLiteral("Render Update"),
                QStringLiteral("Render Bind"), QStringLiteral("Render Render")};
    case 1:
        return {QStringLiteral("Glyph Render"), QStringLiteral("Glyph Upload")};
    case 2:
        return {QStringLiteral("Material Compile")};
    case 3:
        return {QStringLiteral("Render Thread Sync"), QStringLiteral("Render"),
                QStringLiteral("Swap")};
    case 4:
        return {QStringLiteral("Texture Bind"), QStringLiteral("Texture Convert"),
                QStringLiteral("Texture Swizzle"), QStringLiteral("Texture Upload"),
                QStringLiteral("Texture Mipmap")};
    case 5:
        return {QStringLiteral("Texture Delete")};
    case 6:
        return {QStringLiteral("Polish"), QStringLiteral("Wait"),
                QStringLiteral("GUI Thread Sync"), QStringLiteral("Animations")};
    case 8:
        return {QStringLiteral("Animations")};
    case 9:
        return {QStringLiteral("Polish")};
    default:
        return {};
    }
}

bool isPixmapCacheEvent(const EventTypeDef *type)
{
    return type && type->typeName == QLatin1String("PixmapCache") && type->hasDetailValue
        && type->detailTag == QLatin1String("cacheEventType");
}

bool isSceneGraphEvent(const EventTypeDef *type)
{
    return type && type->typeName == QLatin1String("SceneGraph") && type->hasDetailValue
        && type->detailTag == QLatin1String("sgEventType");
}

bool isMemoryAllocationEvent(const EventTypeDef *type)
{
    return type && type->typeName == QLatin1String("MemoryAllocation") && type->hasDetailValue
        && type->detailTag == QLatin1String("memoryEventType");
}

bool isDebugMessageEvent(const EventTypeDef *type)
{
    return type && type->typeName == QLatin1String("DebugMessage");
}

QString pixmapAggregateFileName(const EventTypeDef &type)
{
    if (!type.filename.isEmpty()) {
        const QString fileName = QFileInfo(type.filename).fileName();
        if (!fileName.isEmpty())
            return fileName;
    }

    const QString displayName = !type.displayName.isEmpty() ? type.displayName : type.details;
    const int suffixPos = displayName.indexOf(QLatin1Char(':'));
    return suffixPos >= 0 ? displayName.left(suffixPos) : displayName;
}

QString formatPixmapAggregateLine(const TraceData &trace,
                                  const TraceEvent &startEvent,
                                  const TraceEvent &finishEvent,
                                  const TraceEvent *sizeEvent)
{
    const auto *type = resolvedType(trace, startEvent.eventIndex);
    if (!type)
        return QString();

    QString line = QStringLiteral("%1  aggregate=PixmapCacheLoad  type=%2  duration=%3")
                       .arg(startEvent.startTime)
                       .arg(resolvedTypeName(trace, startEvent.eventIndex))
                       .arg(finishEvent.startTime - startEvent.startTime);
    line += QStringLiteral("  qtCreatorCategory=%1  qtCreatorLabel=%2")
                .arg(quoteValue(qtCreatorPixmapCategory()))
                .arg(quoteValue(qtCreatorPixmapLabel()));

    const QString fileName = pixmapAggregateFileName(*type);
    if (!fileName.isEmpty())
        line += QStringLiteral("  file=%1").arg(fileName);

    if (sizeEvent) {
        const QString width = sizeEvent->attributes.value(QStringLiteral("width"));
        const QString height = sizeEvent->attributes.value(QStringLiteral("height"));
        if (!width.isEmpty())
            line += QStringLiteral("  width=%1").arg(width);
        if (!height.isEmpty())
            line += QStringLiteral(" height=%1").arg(height);
    }

    return line;
}

QStringList renderPixmapAggregateLines(const TraceData &trace)
{
    struct PixmapLoadState
    {
        TraceEvent startEvent;
        std::optional<TraceEvent> sizeEvent;
    };

    QStringList lines;
    QHash<QString, QList<PixmapLoadState>> openLoads;

    for (const auto &event : trace.events) {
        const auto *type = resolvedType(trace, event.eventIndex);
        if (!isPixmapCacheEvent(type) || event.duration.has_value())
            continue;

        const QString key = !type->displayName.isEmpty() ? type->displayName : type->filename;
        if (key.isEmpty())
            continue;

        if (type->detailValue == pixmapLoadingStarted) {
            openLoads[key].append(PixmapLoadState{event, std::nullopt});
            continue;
        }

        auto it = openLoads.find(key);
        if (it == openLoads.end() || it->isEmpty())
            continue;

        if (type->detailValue == pixmapSizeKnown) {
            auto pendingIt = std::find_if(it->begin(), it->end(), [](const PixmapLoadState &state) {
                return !state.sizeEvent.has_value();
            });
            if (pendingIt == it->end())
                pendingIt = std::prev(it->end());
            pendingIt->sizeEvent = event;
            continue;
        }

        if (type->detailValue == pixmapLoadingFinished) {
            const PixmapLoadState state = it->takeFirst();
            lines << formatPixmapAggregateLine(trace, state.startEvent, event,
                                               state.sizeEvent ? &state.sizeEvent.value()
                                                               : nullptr);
            if (it->isEmpty())
                openLoads.erase(it);
        }
    }

    return lines;
}

QStringList renderSceneGraphAggregateLines(const TraceData &trace)
{
    QStringList lines;

    for (const auto &event : trace.events) {
        const auto *type = resolvedType(trace, event.eventIndex);
        if (!isSceneGraphEvent(type) || event.duration.has_value())
            continue;

        qint64 totalTiming = 0;
        QStringList timingParts;
        for (int i = 1; i <= 5; ++i) {
            const QString key = QStringLiteral("timing%1").arg(i);
            const auto timing = parseInt64(event.attributes.value(key));
            if (!timing.has_value())
                continue;
            totalTiming += timing.value();
            timingParts << QStringLiteral("%1=%2").arg(key).arg(timing.value());
        }

        QString line = QStringLiteral("%1  aggregate=SceneGraphFrame  type=%2  totalTiming=%3")
                           .arg(event.startTime)
                           .arg(sceneGraphFrameTypeName(type->detailValue))
                           .arg(totalTiming);
        line += QStringLiteral("  qtCreatorCategory=%1  qtCreatorThread=%2")
                    .arg(quoteValue(QStringLiteral("Scene Graph")))
                    .arg(quoteValue(qtCreatorSceneGraphThreadLabel(type->detailValue)));
        const QStringList stageLabels = qtCreatorSceneGraphStageLabels(type->detailValue);
        if (!stageLabels.isEmpty())
            line += QStringLiteral("  qtCreatorStages=%1")
                        .arg(quoteValue(stageLabels.join(QStringLiteral(","))));
        if (!timingParts.isEmpty())
            line += QStringLiteral("  ") + timingParts.join(QStringLiteral(" "));
        lines << line;
    }

    return lines;
}

QStringList renderMemoryAggregateLines(const TraceData &trace)
{
    struct MemorySummary
    {
        qint64 firstTimestamp = -1;
        int count = 0;
        qint64 totalAmount = 0;
        qint64 maxAmount = std::numeric_limits<qint64>::min();
        qint64 minAmount = std::numeric_limits<qint64>::max();
    };

    QHash<int, MemorySummary> summaries;

    for (const auto &event : trace.events) {
        const auto *type = resolvedType(trace, event.eventIndex);
        if (!isMemoryAllocationEvent(type) || event.duration.has_value())
            continue;

        const auto amount = parseInt64(event.attributes.value(QStringLiteral("amount")));
        if (!amount.has_value())
            continue;

        auto &summary = summaries[type->detailValue];
        if (summary.firstTimestamp < 0)
            summary.firstTimestamp = event.startTime;
        ++summary.count;
        summary.totalAmount += amount.value();
        summary.maxAmount = std::max(summary.maxAmount, amount.value());
        summary.minAmount = std::min(summary.minAmount, amount.value());
    }

    QList<int> detailValues = summaries.keys();
    std::sort(detailValues.begin(), detailValues.end());

    QStringList lines;
    for (int detailValue : detailValues) {
        const auto &summary = summaries[detailValue];
        lines << QStringLiteral(
                     "%1  aggregate=MemoryAllocationSummary  type=%2  count=%3  totalAmount=%4  minAmount=%5  maxAmount=%6  qtCreatorCategory=%7  qtCreatorType=%8")
                     .arg(summary.firstTimestamp)
                     .arg(memoryAllocationTypeName(detailValue))
                     .arg(summary.count)
                     .arg(summary.totalAmount)
                     .arg(summary.minAmount)
                     .arg(summary.maxAmount)
                     .arg(quoteValue(qtCreatorMemoryCategory()))
                     .arg(quoteValue(qtCreatorMemoryTypeName(detailValue)));
    }

    return lines;
}

QStringList renderDebugMessageAggregateLines(const TraceData &trace)
{
    struct DebugSummary
    {
        qint64 firstTimestamp = -1;
        int count = 0;
        QString firstText;
    };

    QHash<int, DebugSummary> summaries;

    for (const auto &event : trace.events) {
        const auto *type = resolvedType(trace, event.eventIndex);
        if (!isDebugMessageEvent(type) || event.duration.has_value())
            continue;

        const int level = type->hasDetailValue ? type->detailValue : -1;
        auto &summary = summaries[level];
        if (summary.firstTimestamp < 0) {
            summary.firstTimestamp = event.startTime;
            summary.firstText = event.attributes.value(QStringLiteral("text"));
        }
        ++summary.count;
    }

    QList<int> levels = summaries.keys();
    std::sort(levels.begin(), levels.end());

    QStringList lines;
    for (int level : levels) {
        const auto &summary = summaries[level];
        QString line = QStringLiteral("%1  aggregate=DebugMessageSummary  level=%2  count=%3")
                           .arg(summary.firstTimestamp)
                           .arg(debugMessageLevelName(level))
                           .arg(summary.count);
        line += QStringLiteral("  qtCreatorCategory=%1  qtCreatorLabel=%2")
                    .arg(quoteValue(qtCreatorDebugCategory()))
                    .arg(quoteValue(qtCreatorDebugLabel(level)));
        if (!summary.firstText.isEmpty())
            line += QStringLiteral("  firstText=%1").arg(quoteValue(summary.firstText));
        lines << line;
    }

    return lines;
}

QStringList renderAggregateLines(const TraceData &trace)
{
    struct AggregateLine
    {
        qint64 timestamp = 0;
        QString text;
    };

    QStringList sourceLines;
    sourceLines << renderPixmapAggregateLines(trace);
    sourceLines << renderSceneGraphAggregateLines(trace);
    sourceLines << renderMemoryAggregateLines(trace);
    sourceLines << renderDebugMessageAggregateLines(trace);

    QVector<AggregateLine> lines;
    lines.reserve(sourceLines.size());
    for (const auto &line : sourceLines) {
        const auto timestamp = parseInt64(line.section(QLatin1Char(' '), 0, 0));
        lines.append({timestamp.value_or(0), line});
    }

    std::sort(lines.begin(), lines.end(), [](const AggregateLine &lhs, const AggregateLine &rhs) {
        if (lhs.timestamp != rhs.timestamp)
            return lhs.timestamp < rhs.timestamp;
        return lhs.text < rhs.text;
    });

    QStringList rendered;
    for (const auto &line : lines)
        rendered << line.text;
    return rendered;
}

} // namespace

QString Report::renderSummary(const TraceData &trace, const Summary &summary)
{
    QStringList lines;
    lines << QStringLiteral("file format: %1").arg(trace.fileFormat);
    lines << QStringLiteral("version: %1").arg(summary.version);
    lines << QStringLiteral("traceStart: %1").arg(summary.traceStart);
    lines << QStringLiteral("traceEnd: %1").arg(summary.traceEnd);
    lines << QStringLiteral("traceDuration: %1").arg(summary.traceDuration);
    lines << QStringLiteral("totalTime: %1").arg(summary.totalTime);
    lines << QStringLiteral("eventTypes: %1").arg(summary.typeCount);
    lines << QStringLiteral("events: %1").arg(summary.eventCount);
    lines << QStringLiteral("ranges: %1").arg(summary.rangeCount);
    lines << QStringLiteral("pointEvents: %1").arg(summary.pointEventCount);
    lines << QStringLiteral("notes: %1").arg(summary.noteCount);
    if (summary.longestEventIndex >= 0) {
        lines << QStringLiteral("longestRange: idx=%1 type=%2 duration=%3")
                     .arg(summary.longestEventIndex)
                     .arg(resolvedTypeName(trace, summary.longestEventIndex))
                     .arg(summary.longestDuration);
    }
    if (summary.rangeCount > 0) {
        lines << QStringLiteral("note: per-type totalDuration includes parent-child overlap for nested ranges.");
        lines << QStringLiteral("note: use the ranges command to inspect selfTime and focus candidates.");
    }
    bool hasCompiling = false;
    for (const auto &typeSummary : summary.perType) {
        if (typeSummary.typeName == QLatin1String("Compiling") && typeSummary.maxDuration > 0) {
            hasCompiling = true;
            break;
        }
    }
    if (hasCompiling) {
        lines << QStringLiteral("note: large Compiling ranges can indicate large QML, but import/plugin load may also contribute.");
        lines << QStringLiteral("note: Compiling cost should be treated as a hint, not a complete root-cause diagnosis.");
    }
    lines << QString();
    lines << QStringLiteral("per-type:");
    for (const auto &typeSummary : summary.perType) {
        if (typeSummary.eventIndex < 0)
            continue;
        lines << QStringLiteral("  [%1] %2 count=%3 totalDuration=%4 maxDuration=%5")
                     .arg(typeSummary.eventIndex)
                     .arg(typeSummary.displayName.isEmpty()
                              ? typeSummary.typeName
                              : typeSummary.displayName)
                     .arg(typeSummary.count)
                     .arg(typeSummary.totalDuration)
                     .arg(typeSummary.maxDuration);
    }
    return lines.join(QLatin1Char('\n'));
}

QString Report::renderTypes(const TraceData &trace, const Summary &summary)
{
    Q_UNUSED(summary)

    QStringList lines;
    for (int index = 0; index < trace.eventTypes.size(); ++index) {
        const auto &type = trace.eventTypes.at(index);
        QString line = QStringLiteral("[%1] type=%2").arg(index).arg(type.typeName);
        if (!type.displayName.isEmpty())
            line += QStringLiteral(" display=%1").arg(type.displayName);
        if (!type.filename.isEmpty()) {
            line += QStringLiteral(" file=%1").arg(type.filename);
            if (type.line >= 0)
                line += QStringLiteral(":%1").arg(type.line);
            if (type.column >= 0)
                line += QStringLiteral(":%1").arg(type.column);
        }
        const QString detail = detailLabel(type);
        if (!detail.isEmpty())
            line += QStringLiteral(" %1").arg(detail);
        if (!type.details.isEmpty())
            line += QStringLiteral(" details=%1").arg(type.details);
        lines << line;
    }
    return lines.join(QLatin1Char('\n'));
}

QString Report::renderRanges(const TraceData &trace, const RangeAnalysis &analysis)
{
    QVector<const RangeNode *> ordered;
    ordered.reserve(analysis.nodes.size());
    for (const auto &node : analysis.nodes)
        ordered.append(&node);

    std::sort(ordered.begin(), ordered.end(), [](const RangeNode *lhs, const RangeNode *rhs) {
        if (lhs->startTime != rhs->startTime)
            return lhs->startTime < rhs->startTime;
        if (lhs->endTime != rhs->endTime)
            return lhs->endTime > rhs->endTime;
        return lhs->id < rhs->id;
    });

    QStringList lines;
    for (const auto *node : ordered) {
        QString line = QStringLiteral(
                           "%1  id=%2  duration=%3  selfTime=%4  childTime=%5  gapTime=%6  gapWarning=%7  focus=%8  depth=%9  idx=%10  type=%11")
                           .arg(node->startTime)
                           .arg(node->id)
                           .arg(node->duration)
                           .arg(node->selfTime)
                           .arg(node->childCoveredTime)
                           .arg(node->gapTime)
                           .arg(node->gapWarning ? QStringLiteral("yes")
                                                 : QStringLiteral("no"))
                           .arg(node->focusCandidate ? QStringLiteral("yes")
                                                     : QStringLiteral("no"))
                           .arg(node->depth)
                           .arg(node->eventIndex)
                           .arg(resolvedTypeName(trace, node->eventIndex));
        if (node->parentId >= 0)
            line += QStringLiteral("  parent=%1").arg(node->parentId);

        const auto *type = resolvedType(trace, node->eventIndex);
        if (type) {
            if (!type->displayName.isEmpty())
                line += QStringLiteral("  display=%1").arg(type->displayName);
            if (!type->filename.isEmpty()) {
                line += QStringLiteral("  file=%1").arg(type->filename);
                if (type->line >= 0)
                    line += QStringLiteral(":%1").arg(type->line);
            }
        }
        if (node->gapWarning)
            line += QStringLiteral("  note=possible_non_qml_work");

        lines << line;
    }

    return lines.join(QLatin1Char('\n'));
}

QString Report::renderHotspots(const TraceData &trace, const HotspotAnalysis &analysis)
{
    QStringList lines;
    for (const auto &hotspot : analysis.hotspots) {
        QStringList indexStrings;
        indexStrings.reserve(hotspot.eventIndexes.size());
        for (int eventIndex : hotspot.eventIndexes)
            indexStrings << QString::number(eventIndex);

        QString line = QStringLiteral(
                           "idx=%1  groupKey=%2  indexes=%3  type=%4  calls=%5  selfTotal=%6  inclusiveTotal=%7  minSelf=%8  medianSelf=%9  maxSelf=%10  minInclusive=%11  medianInclusive=%12  maxInclusive=%13  outlier=%14  burst=%15  image=%16  compilingNote=%17")
                           .arg(hotspot.eventIndex)
                           .arg(hotspot.groupKey)
                           .arg(indexStrings.join(QLatin1Char(',')))
                           .arg(resolvedTypeName(trace, hotspot.eventIndex))
                           .arg(hotspot.calls)
                           .arg(hotspot.selfTotal)
                           .arg(hotspot.inclusiveTotal)
                           .arg(hotspot.minSelf)
                           .arg(hotspot.medianSelf)
                           .arg(hotspot.maxSelf)
                           .arg(hotspot.minInclusive)
                           .arg(hotspot.medianInclusive)
                           .arg(hotspot.maxInclusive)
                           .arg(hotspot.outlierHint ? QStringLiteral("yes")
                                                    : QStringLiteral("no"))
                           .arg(hotspot.burstHint ? QStringLiteral("yes")
                                                  : QStringLiteral("no"))
                           .arg(hotspot.imageHint ? QStringLiteral("yes")
                                                  : QStringLiteral("no"))
                           .arg(hotspot.compilingHint ? QStringLiteral("yes")
                                                      : QStringLiteral("no"));
        const auto *type = resolvedType(trace, hotspot.eventIndex);
        if (type) {
            if (!type->displayName.isEmpty())
                line += QStringLiteral("  display=%1").arg(type->displayName);
            if (!type->filename.isEmpty()) {
                line += QStringLiteral("  file=%1").arg(type->filename);
                if (type->line >= 0)
                    line += QStringLiteral(":%1").arg(type->line);
            }
        }
        if (hotspot.imageHint)
            line += QStringLiteral("  note=possible_image_related_cost");
        if (hotspot.compilingHint)
            line += QStringLiteral("  note=compiling_cost_may_include_import_or_plugin_load");
        lines << line;
    }

    return lines.join(QLatin1Char('\n'));
}

QString Report::renderHotspotComparison(const TraceData &beforeTrace,
                                        const TraceData &afterTrace,
                                        const HotspotComparison &comparison)
{
    QStringList lines;
    for (const auto &entry : comparison.entries) {
        const QString line = QStringLiteral(
                                 "status=%1  groupKey=%2  type=%3  beforeIdx=%4  afterIdx=%5  beforeSelfTotal=%6  afterSelfTotal=%7  selfDelta=%8  selfDeltaPct=%9  beforeInclusiveTotal=%10  afterInclusiveTotal=%11  inclusiveDelta=%12  beforeCalls=%13  afterCalls=%14  callsDelta=%15  beforeMedianSelf=%16  afterMedianSelf=%17  medianSelfDelta=%18  beforeMaxSelf=%19  afterMaxSelf=%20  maxSelfDelta=%21")
                                 .arg(comparisonStatus(entry))
                                 .arg(entry.groupKey)
                                 .arg(comparisonTypeName(beforeTrace, afterTrace, entry))
                                 .arg(entry.hasBefore ? QString::number(entry.before.eventIndex)
                                                      : QStringLiteral("-"))
                                 .arg(entry.hasAfter ? QString::number(entry.after.eventIndex)
                                                     : QStringLiteral("-"))
                                 .arg(entry.hasBefore ? QString::number(entry.before.selfTotal)
                                                      : QStringLiteral("-"))
                                 .arg(entry.hasAfter ? QString::number(entry.after.selfTotal)
                                                     : QStringLiteral("-"))
                                 .arg(formatSigned(entry.selfTotalDelta))
                                 .arg(formatPercent(entry.before.selfTotal, entry.selfTotalDelta))
                                 .arg(entry.hasBefore ? QString::number(entry.before.inclusiveTotal)
                                                      : QStringLiteral("-"))
                                 .arg(entry.hasAfter ? QString::number(entry.after.inclusiveTotal)
                                                     : QStringLiteral("-"))
                                 .arg(formatSigned(entry.inclusiveTotalDelta))
                                 .arg(entry.hasBefore ? QString::number(entry.before.calls)
                                                      : QStringLiteral("-"))
                                 .arg(entry.hasAfter ? QString::number(entry.after.calls)
                                                     : QStringLiteral("-"))
                                 .arg(formatSigned(entry.callsDelta))
                                 .arg(entry.hasBefore ? QString::number(entry.before.medianSelf)
                                                      : QStringLiteral("-"))
                                 .arg(entry.hasAfter ? QString::number(entry.after.medianSelf)
                                                     : QStringLiteral("-"))
                                 .arg(formatSigned(entry.medianSelfDelta))
                                 .arg(entry.hasBefore ? QString::number(entry.before.maxSelf)
                                                      : QStringLiteral("-"))
                                 .arg(entry.hasAfter ? QString::number(entry.after.maxSelf)
                                                     : QStringLiteral("-"))
                                 .arg(formatSigned(entry.maxSelfDelta));
        lines << line;
    }

    return lines.join(QLatin1Char('\n'));
}

QString Report::renderEvents(const TraceData &trace)
{
    QStringList lines;
    for (const auto &event : trace.events)
        lines << formatEventLine(trace, event);
    return lines.join(QLatin1Char('\n'));
}

QString Report::renderAggregatedEvents(const TraceData &trace)
{
    QStringList lines;
    const QStringList aggregateLines = renderAggregateLines(trace);
    if (!aggregateLines.isEmpty()) {
        lines << QStringLiteral("# aggregates");
        lines << aggregateLines;
        lines << QString();
        lines << QStringLiteral("# raw events");
    }
    lines << renderEvents(trace);
    return lines.join(QLatin1Char('\n'));
}

QString Report::renderNotes(const TraceData &trace)
{
    QStringList lines;
    for (const auto &note : trace.notes) {
        lines << QStringLiteral("%1  duration=%2  idx=%3  row=%4  text=%5")
                     .arg(note.startTime)
                     .arg(note.duration)
                     .arg(note.eventIndex)
                     .arg(note.collapsedRow)
                     .arg(note.text);
    }
    return lines.join(QLatin1Char('\n'));
}

} // namespace QmlProfilerAnalyzer
