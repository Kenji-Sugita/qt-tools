#pragma once

#include <QHostAddress>
#include <QString>
#include <QStringList>

class QTextStream;

class ServerOptions
{
public:
    static ServerOptions parse(const QStringList &arguments, QTextStream &err);
    static void printHelp(QTextStream &out);

    bool valid = true;
    bool helpRequested = false;
    QString rootPath;
    QHostAddress host = QHostAddress::LocalHost;
    quint16 port = 3101;
    qint64 maxSize = 10 * 1024 * 1024;
};
