#pragma once

#include "ClientOptions.h"
#include "Command.h"

#include <QMap>
#include <QStringList>

class SseMcpClient;
class QTextStream;

class CommandRegistry
{
public:
    CommandRegistry(SseMcpClient *client, const ClientOptions &options, QTextStream &out, QTextStream &err);

    bool contains(const QString &name) const;
    QStringList commandNames() const;
    int runOneShot(const QStringList &args);
    int runReplCommand(const QStringList &args);
    void printHelp(const QString &command = QString()) const;

private:
    void registerCommands();
    bool ensureConnected();
    int status();
    int tools();
    int files();
    int download(const QStringList &args);
    int call(const QStringList &args);

    SseMcpClient *m_client = nullptr;
    QTextStream &m_out;
    QTextStream &m_err;
    bool m_jsonOutput = false;
    int m_timeoutMs = 10000;
    QMap<QString, Command> m_commands;
};
