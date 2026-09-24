#pragma once

#include "model.h"

#include <QString>

#include <optional>

QT_FORWARD_DECLARE_CLASS(QIODevice)
QT_FORWARD_DECLARE_CLASS(QXmlStreamReader)
QT_FORWARD_DECLARE_CLASS(QDataStream)

namespace QmlProfilerAnalyzer {

class QtdReader
{
public:
    std::optional<TraceData> readFile(const QString &filePath);
    bool writeQtdFile(const TraceData &trace, const QString &filePath);
    bool writeQztFile(const TraceData &trace, const QString &filePath);
    QString errorString() const;

private:
    void setError(const QString &message);
    std::optional<TraceData> readDevice(QIODevice *device);
    std::optional<TraceData> readXmlDevice(QIODevice *device);
    std::optional<TraceData> readQztDevice(QIODevice *device);
    bool writeXmlDevice(const TraceData &trace, QIODevice *device);
    bool writeQztDevice(const TraceData &trace, QIODevice *device);
    bool readTrace(QXmlStreamReader &xml, TraceData &trace);
    bool readEventData(QXmlStreamReader &xml, TraceData &trace);
    bool readProfilerDataModel(QXmlStreamReader &xml, TraceData &trace);
    bool readNoteData(QXmlStreamReader &xml, TraceData &trace);
    bool readQztEventTypes(QDataStream &stream, TraceData &trace);
    bool readQztNotes(QDataStream &stream, TraceData &trace);
    bool readQztEvents(QDataStream &stream, TraceData &trace);

    QString m_errorString;
};

} // namespace QmlProfilerAnalyzer
