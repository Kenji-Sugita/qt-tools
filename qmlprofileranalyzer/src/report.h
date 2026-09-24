#pragma once

#include "analyzer.h"

namespace QmlProfilerAnalyzer {

class Report
{
public:
    static QString renderSummary(const TraceData &trace, const Summary &summary);
    static QString renderTypes(const TraceData &trace, const Summary &summary);
    static QString renderRanges(const TraceData &trace, const RangeAnalysis &analysis);
    static QString renderHotspots(const TraceData &trace, const HotspotAnalysis &analysis);
    static QString renderHotspotComparison(const TraceData &beforeTrace,
                                           const TraceData &afterTrace,
                                           const HotspotComparison &comparison);
    static QString renderEvents(const TraceData &trace);
    static QString renderAggregatedEvents(const TraceData &trace);
    static QString renderNotes(const TraceData &trace);
};

} // namespace QmlProfilerAnalyzer
