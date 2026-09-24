#pragma once

#include "model.h"

#include <QString>
#include <QVector>

namespace QmlProfilerAnalyzer {

struct TypeSummary
{
    int eventIndex = -1;
    QString typeName;
    QString displayName;
    QString location;
    int count = 0;
    qint64 totalDuration = 0;
    qint64 maxDuration = 0;
};

struct Summary
{
    QString version;
    qint64 traceStart = -1;
    qint64 traceEnd = -1;
    qint64 traceDuration = -1;
    qint64 totalTime = -1;
    int typeCount = 0;
    int eventCount = 0;
    int rangeCount = 0;
    int pointEventCount = 0;
    int noteCount = 0;
    int longestEventIndex = -1;
    qint64 longestDuration = 0;
    QVector<TypeSummary> perType;
};

struct RangeNode
{
    int id = -1;
    int eventIndex = -1;
    qint64 startTime = 0;
    qint64 duration = 0;
    qint64 endTime = 0;
    int parentId = -1;
    QVector<int> childIds;
    int depth = 0;
    qint64 inclusiveTime = 0;
    qint64 selfTime = 0;
    qint64 childCoveredTime = 0;
    qint64 gapTime = 0;
    bool focusCandidate = false;
    bool gapWarning = false;
};

struct RangeAnalysis
{
    QVector<RangeNode> nodes;
};

struct HotspotSummary
{
    int eventIndex = -1;
    QString groupKey;
    QVector<int> eventIndexes;
    int calls = 0;
    qint64 inclusiveTotal = 0;
    qint64 selfTotal = 0;
    qint64 minInclusive = 0;
    qint64 maxInclusive = 0;
    qint64 medianInclusive = 0;
    qint64 minSelf = 0;
    qint64 maxSelf = 0;
    qint64 medianSelf = 0;
    bool outlierHint = false;
    bool burstHint = false;
    bool imageHint = false;
    bool compilingHint = false;
};

struct HotspotAnalysis
{
    QVector<HotspotSummary> hotspots;
};

struct HotspotComparisonEntry
{
    QString groupKey;
    bool hasBefore = false;
    bool hasAfter = false;
    HotspotSummary before;
    HotspotSummary after;
    qint64 selfTotalDelta = 0;
    qint64 inclusiveTotalDelta = 0;
    int callsDelta = 0;
    qint64 medianSelfDelta = 0;
    qint64 maxSelfDelta = 0;
};

struct HotspotComparison
{
    QVector<HotspotComparisonEntry> entries;
};

class Analyzer
{
public:
    static Summary summarize(const TraceData &trace);
    static RangeAnalysis analyzeRanges(const TraceData &trace);
    static HotspotAnalysis analyzeHotspots(const TraceData &trace);
    static HotspotComparison compareHotspots(const TraceData &beforeTrace,
                                             const TraceData &afterTrace);
};

} // namespace QmlProfilerAnalyzer
