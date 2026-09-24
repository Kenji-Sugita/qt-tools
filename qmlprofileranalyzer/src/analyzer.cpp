#include "analyzer.h"

#include <QHash>
#include <QSet>
#include <QStringList>

#include <algorithm>
#include <limits>
#include <utility>

namespace QmlProfilerAnalyzer {

namespace {

constexpr qint64 gapWarningThreshold = 1000 * 1000; // 1 ms in ns
constexpr qint64 burstWindow = 16 * 1000 * 1000; // 16 ms in ns
constexpr int burstCallThreshold = 5;

qint64 coveredDuration(QVector<std::pair<qint64, qint64>> intervals)
{
    if (intervals.isEmpty())
        return 0;

    std::sort(intervals.begin(), intervals.end(),
              [](const auto &lhs, const auto &rhs) { return lhs.first < rhs.first
                                                            || (lhs.first == rhs.first
                                                                && lhs.second < rhs.second); });

    qint64 covered = 0;
    qint64 currentStart = intervals.first().first;
    qint64 currentEnd = intervals.first().second;

    for (int index = 1; index < intervals.size(); ++index) {
        const auto &[start, end] = intervals.at(index);
        if (start > currentEnd) {
            covered += currentEnd - currentStart;
            currentStart = start;
            currentEnd = end;
            continue;
        }

        currentEnd = std::max(currentEnd, end);
    }

    covered += currentEnd - currentStart;
    return covered;
}

qint64 markFocusCandidates(QVector<RangeNode> &nodes, int nodeId)
{
    auto &node = nodes[nodeId];

    qint64 maxDescendantSelfTime = 0;
    for (int childId : node.childIds)
        maxDescendantSelfTime = std::max(maxDescendantSelfTime, markFocusCandidates(nodes, childId));

    node.focusCandidate = node.selfTime > maxDescendantSelfTime;
    return std::max(node.selfTime, maxDescendantSelfTime);
}

bool shouldWarnForGap(const TraceData &trace, const RangeNode &node)
{
    if (node.duration <= 0)
        return false;
    if (node.gapTime < gapWarningThreshold)
        return false;
    if (node.gapTime * 5 < node.duration)
        return false;

    const auto *type = trace.eventType(node.eventIndex);
    if (type && type->typeName == QLatin1String("Compiling"))
        return false;

    return true;
}

qint64 medianOf(QVector<qint64> values)
{
    if (values.isEmpty())
        return 0;

    std::sort(values.begin(), values.end());
    return values.at(values.size() / 2);
}

bool overlaps(qint64 lhsStart, qint64 lhsEnd, qint64 rhsStart, qint64 rhsEnd)
{
    return lhsStart < rhsEnd && rhsStart < lhsEnd;
}

bool hasBurstWindow(QVector<qint64> timestamps)
{
    if (timestamps.size() < burstCallThreshold)
        return false;

    std::sort(timestamps.begin(), timestamps.end());
    int windowStart = 0;
    for (int index = 0; index < timestamps.size(); ++index) {
        while (timestamps.at(index) - timestamps.at(windowStart) > burstWindow)
            ++windowStart;
        if (index - windowStart + 1 >= burstCallThreshold)
            return true;
    }

    return false;
}

qint64 absoluteValue(qint64 value)
{
    return value < 0 ? -value : value;
}

QString hotspotGroupKey(const TraceData &trace, int eventIndex)
{
    const auto *type = trace.eventType(eventIndex);
    if (!type)
        return QStringLiteral("idx:%1").arg(eventIndex);

    QStringList parts;
    parts << QStringLiteral("type=%1").arg(type->typeName);
    parts << QStringLiteral("file=%1").arg(type->filename);
    parts << QStringLiteral("line=%1").arg(type->line);
    parts << QStringLiteral("column=%1").arg(type->column);
    if (type->filename.isEmpty() && type->line < 0 && !type->displayName.isEmpty())
        parts << QStringLiteral("display=%1").arg(type->displayName);
    return parts.join(QLatin1Char('|'));
}

} // namespace

Summary Analyzer::summarize(const TraceData &trace)
{
    Summary summary;
    summary.version = trace.version;
    summary.traceStart = trace.traceStart;
    summary.traceEnd = trace.traceEnd;
    if (trace.traceStart >= 0 && trace.traceEnd >= trace.traceStart)
        summary.traceDuration = trace.traceEnd - trace.traceStart;
    summary.totalTime = trace.totalTime;
    summary.typeCount = trace.eventTypes.size();
    summary.eventCount = trace.events.size();
    summary.noteCount = trace.notes.size();
    summary.perType.resize(trace.eventTypes.size());

    for (int index = 0; index < trace.eventTypes.size(); ++index) {
        const auto &type = trace.eventTypes.at(index);
        auto &typeSummary = summary.perType[index];
        typeSummary.eventIndex = index;
        typeSummary.typeName = type.typeName;
        typeSummary.displayName = type.displayName;
        if (!type.filename.isEmpty()) {
            typeSummary.location = type.filename;
            if (type.line >= 0)
                typeSummary.location += QStringLiteral(":%1").arg(type.line);
        }
    }

    for (const auto &event : trace.events) {
        if (event.duration.has_value()) {
            ++summary.rangeCount;
            if (event.duration.value() > summary.longestDuration) {
                summary.longestDuration = event.duration.value();
                summary.longestEventIndex = event.eventIndex;
            }
        } else {
            ++summary.pointEventCount;
        }

        if (event.eventIndex < 0 || event.eventIndex >= summary.perType.size())
            continue;

        auto &typeSummary = summary.perType[event.eventIndex];
        ++typeSummary.count;
        if (event.duration.has_value()) {
            typeSummary.totalDuration += event.duration.value();
            typeSummary.maxDuration = std::max(typeSummary.maxDuration, event.duration.value());
        }
    }

    return summary;
}

RangeAnalysis Analyzer::analyzeRanges(const TraceData &trace)
{
    struct IndexedRange
    {
        int id = -1;
        qint64 startTime = 0;
        qint64 endTime = 0;
    };

    RangeAnalysis analysis;

    for (const auto &event : trace.events) {
        if (!event.duration.has_value())
            continue;

        RangeNode node;
        node.id = analysis.nodes.size();
        node.eventIndex = event.eventIndex;
        node.startTime = event.startTime;
        node.duration = std::max<qint64>(0, event.duration.value());
        node.endTime = node.startTime + node.duration;
        node.inclusiveTime = node.duration;
        node.selfTime = node.duration;
        node.gapTime = node.duration;
        analysis.nodes.append(std::move(node));
    }

    QVector<IndexedRange> ordered;
    ordered.reserve(analysis.nodes.size());
    for (const auto &node : analysis.nodes)
        ordered.append({node.id, node.startTime, node.endTime});

    std::sort(ordered.begin(), ordered.end(), [](const IndexedRange &lhs, const IndexedRange &rhs) {
        if (lhs.startTime != rhs.startTime)
            return lhs.startTime < rhs.startTime;
        if (lhs.endTime != rhs.endTime)
            return lhs.endTime > rhs.endTime;
        return lhs.id < rhs.id;
    });

    QVector<int> stack;
    for (const auto &orderedRange : ordered) {
        auto &node = analysis.nodes[orderedRange.id];

        while (!stack.isEmpty()) {
            const auto &candidateParent = analysis.nodes.at(stack.back());
            if (candidateParent.endTime >= node.endTime)
                break;
            stack.removeLast();
        }

        if (!stack.isEmpty()) {
            node.parentId = stack.back();
            node.depth = analysis.nodes.at(node.parentId).depth + 1;
            analysis.nodes[node.parentId].childIds.append(node.id);
        }

        stack.append(node.id);
    }

    for (auto &node : analysis.nodes) {
        QVector<std::pair<qint64, qint64>> childIntervals;
        childIntervals.reserve(node.childIds.size());
        for (int childId : node.childIds) {
            const auto &child = analysis.nodes.at(childId);
            childIntervals.append({child.startTime, child.endTime});
        }

        node.childCoveredTime = coveredDuration(std::move(childIntervals));
        node.selfTime = std::max<qint64>(0, node.inclusiveTime - node.childCoveredTime);
        node.gapTime = node.selfTime;
        node.gapWarning = shouldWarnForGap(trace, node);
    }

    for (const auto &node : analysis.nodes) {
        if (node.parentId < 0)
            markFocusCandidates(analysis.nodes, node.id);
    }

    return analysis;
}

HotspotAnalysis Analyzer::analyzeHotspots(const TraceData &trace)
{
    struct MutableHotspot
    {
        HotspotSummary summary;
        QVector<qint64> inclusiveSamples;
        QVector<qint64> selfSamples;
        QVector<qint64> startTimes;
    };

    const RangeAnalysis rangeAnalysis = analyzeRanges(trace);
    QHash<QString, MutableHotspot> grouped;
    QVector<std::pair<qint64, qint64>> pixmapLoadIntervals;

    struct PixmapLoadState
    {
        qint64 startTime = 0;
        QString key;
    };
    QHash<QString, QVector<PixmapLoadState>> openPixmapLoads;

    for (const auto &event : trace.events) {
        if (event.duration.has_value())
            continue;

        const auto *type = trace.eventType(event.eventIndex);
        if (!type || type->typeName != QLatin1String("PixmapCache") || !type->hasDetailValue
            || type->detailTag != QLatin1String("cacheEventType")) {
            continue;
        }

        const QString key = !type->displayName.isEmpty() ? type->displayName : type->filename;
        if (key.isEmpty())
            continue;

        if (type->detailValue == 3) {
            openPixmapLoads[key].append({event.startTime, key});
            continue;
        }

        if (type->detailValue != 4)
            continue;

        auto it = openPixmapLoads.find(key);
        if (it == openPixmapLoads.end() || it->isEmpty())
            continue;

        const auto state = it->takeFirst();
        pixmapLoadIntervals.append({state.startTime, event.startTime});
        if (it->isEmpty())
            openPixmapLoads.erase(it);
    }

    for (const auto &node : rangeAnalysis.nodes) {
        if (node.eventIndex < 0)
            continue;

        const QString groupKey = hotspotGroupKey(trace, node.eventIndex);
        auto &entry = grouped[groupKey];
        if (entry.summary.eventIndex < 0) {
            entry.summary.eventIndex = node.eventIndex;
            entry.summary.groupKey = groupKey;
            entry.summary.minInclusive = std::numeric_limits<qint64>::max();
            entry.summary.minSelf = std::numeric_limits<qint64>::max();
        }
        if (!entry.summary.eventIndexes.contains(node.eventIndex))
            entry.summary.eventIndexes.append(node.eventIndex);

        ++entry.summary.calls;
        entry.summary.inclusiveTotal += node.inclusiveTime;
        entry.summary.selfTotal += node.selfTime;
        entry.summary.minInclusive = std::min(entry.summary.minInclusive, node.inclusiveTime);
        entry.summary.maxInclusive = std::max(entry.summary.maxInclusive, node.inclusiveTime);
        entry.summary.minSelf = std::min(entry.summary.minSelf, node.selfTime);
        entry.summary.maxSelf = std::max(entry.summary.maxSelf, node.selfTime);
        entry.inclusiveSamples.append(node.inclusiveTime);
        entry.selfSamples.append(node.selfTime);
        entry.startTimes.append(node.startTime);

        if (!entry.summary.imageHint) {
            for (const auto &[loadStart, loadEnd] : pixmapLoadIntervals) {
                if (overlaps(node.startTime, node.endTime, loadStart, loadEnd)) {
                    entry.summary.imageHint = true;
                    break;
                }
            }
        }
    }

    HotspotAnalysis analysis;
    analysis.hotspots.reserve(grouped.size());

    for (auto it = grouped.begin(); it != grouped.end(); ++it) {
        auto summary = it->summary;
        std::sort(summary.eventIndexes.begin(), summary.eventIndexes.end());
        const auto *type = trace.eventType(summary.eventIndex);
        summary.medianInclusive = medianOf(it->inclusiveSamples);
        summary.medianSelf = medianOf(it->selfSamples);
        summary.outlierHint = summary.calls >= 2 && summary.maxSelf > summary.medianSelf * 2;
        summary.burstHint = hasBurstWindow(it->startTimes);
        summary.compilingHint =
            type && type->typeName == QLatin1String("Compiling") && summary.maxInclusive > 0;
        analysis.hotspots.append(std::move(summary));
    }

    std::sort(analysis.hotspots.begin(), analysis.hotspots.end(),
              [](const HotspotSummary &lhs, const HotspotSummary &rhs) {
                  if (lhs.selfTotal != rhs.selfTotal)
                      return lhs.selfTotal > rhs.selfTotal;
                  if (lhs.maxSelf != rhs.maxSelf)
                      return lhs.maxSelf > rhs.maxSelf;
                  return lhs.eventIndex < rhs.eventIndex;
              });

    return analysis;
}

HotspotComparison Analyzer::compareHotspots(const TraceData &beforeTrace,
                                            const TraceData &afterTrace)
{
    const HotspotAnalysis beforeAnalysis = analyzeHotspots(beforeTrace);
    const HotspotAnalysis afterAnalysis = analyzeHotspots(afterTrace);

    QHash<QString, HotspotSummary> beforeByKey;
    QHash<QString, HotspotSummary> afterByKey;
    QSet<QString> keys;

    for (const auto &hotspot : beforeAnalysis.hotspots) {
        beforeByKey.insert(hotspot.groupKey, hotspot);
        keys.insert(hotspot.groupKey);
    }
    for (const auto &hotspot : afterAnalysis.hotspots) {
        afterByKey.insert(hotspot.groupKey, hotspot);
        keys.insert(hotspot.groupKey);
    }

    HotspotComparison comparison;
    comparison.entries.reserve(keys.size());

    for (const auto &groupKey : keys) {
        HotspotComparisonEntry entry;
        entry.groupKey = groupKey;
        entry.hasBefore = beforeByKey.contains(groupKey);
        entry.hasAfter = afterByKey.contains(groupKey);
        if (entry.hasBefore)
            entry.before = beforeByKey.value(groupKey);
        if (entry.hasAfter)
            entry.after = afterByKey.value(groupKey);

        entry.selfTotalDelta = entry.after.selfTotal - entry.before.selfTotal;
        entry.inclusiveTotalDelta = entry.after.inclusiveTotal - entry.before.inclusiveTotal;
        entry.callsDelta = entry.after.calls - entry.before.calls;
        entry.medianSelfDelta = entry.after.medianSelf - entry.before.medianSelf;
        entry.maxSelfDelta = entry.after.maxSelf - entry.before.maxSelf;
        comparison.entries.append(std::move(entry));
    }

    std::sort(comparison.entries.begin(), comparison.entries.end(),
              [](const HotspotComparisonEntry &lhs, const HotspotComparisonEntry &rhs) {
                  const qint64 lhsWeight = std::max(absoluteValue(lhs.selfTotalDelta),
                                                    std::max(lhs.before.selfTotal,
                                                             lhs.after.selfTotal));
                  const qint64 rhsWeight = std::max(absoluteValue(rhs.selfTotalDelta),
                                                    std::max(rhs.before.selfTotal,
                                                             rhs.after.selfTotal));
                  if (lhsWeight != rhsWeight)
                      return lhsWeight > rhsWeight;
                  if (lhs.selfTotalDelta != rhs.selfTotalDelta)
                      return lhs.selfTotalDelta > rhs.selfTotalDelta;
                  return lhs.groupKey < rhs.groupKey;
              });

    return comparison;
}

} // namespace QmlProfilerAnalyzer
