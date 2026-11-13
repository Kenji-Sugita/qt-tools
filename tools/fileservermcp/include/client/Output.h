#pragma once

#include <QJsonObject>
#include <QString>

class QTextStream;

class Output
{
public:
    static void printJson(const QJsonObject &object, QTextStream &out, bool compact = false);
    static void printFiles(const QJsonObject &result, QTextStream &out, bool jsonOutput);
    static void printToolResult(const QJsonObject &result, QTextStream &out, bool jsonOutput);
};
