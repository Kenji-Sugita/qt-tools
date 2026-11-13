#include "Output.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QTextStream>

void Output::printJson(const QJsonObject &object, QTextStream &out, bool compact)
{
    out << QString::fromUtf8(QJsonDocument(object).toJson(compact ? QJsonDocument::Compact
                                                                  : QJsonDocument::Indented))
        << Qt::endl;
}

void Output::printFiles(const QJsonObject &result, QTextStream &out, bool jsonOutput)
{
    if (jsonOutput) {
        printJson(result, out);
        return;
    }
    const QJsonArray files = result.value(QStringLiteral("files")).toArray();
    for (const QJsonValue &value : files) {
        const QJsonObject file = value.toObject();
        out << file.value(QStringLiteral("name")).toString()
            << "\t" << file.value(QStringLiteral("size")).toVariant().toLongLong()
            << "\t" << file.value(QStringLiteral("modified")).toString() << Qt::endl;
    }
}

void Output::printToolResult(const QJsonObject &result, QTextStream &out, bool jsonOutput)
{
    Q_UNUSED(jsonOutput);
    printJson(result, out);
}
