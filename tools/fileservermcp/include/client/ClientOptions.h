#pragma once

#include <QUrl>
#include <QStringList>

class QTextStream;

class ClientOptions
{
public:
    static ClientOptions parse(const QStringList &arguments, QTextStream &err);
    static void printHelp(QTextStream &out);

    bool valid = true;
    bool helpRequested = false;
    bool jsonOutput = false;
    QUrl url = QUrl(QStringLiteral("http://127.0.0.1:3101/sse"));
    int timeoutMs = 10000;
    QStringList positionalArgs;
};
