#include "CommandRegistry.h"

#include "Output.h"
#include "SseMcpClient.h"

#include <QByteArray>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QTextStream>

CommandRegistry::CommandRegistry(SseMcpClient *client,
                                 const ClientOptions &options,
                                 QTextStream &out,
                                 QTextStream &err)
    : m_client(client)
    , m_out(out)
    , m_err(err)
    , m_jsonOutput(options.jsonOutput)
    , m_timeoutMs(options.timeoutMs)
{
    registerCommands();
}

bool CommandRegistry::contains(const QString &name) const
{
    return m_commands.contains(name);
}

QStringList CommandRegistry::commandNames() const
{
    return m_commands.keys();
}

int CommandRegistry::runOneShot(const QStringList &args)
{
    if (args.isEmpty())
        return 0;
    QString command = args.at(0);
    if (!command.startsWith(QLatin1Char('.')))
        command.prepend(QLatin1Char('.'));
    return runReplCommand(QStringList {command} + args.mid(1));
}

int CommandRegistry::runReplCommand(const QStringList &args)
{
    if (args.isEmpty())
        return 0;
    const QString command = args.at(0);
    if (!contains(command)) {
        m_err << "Unknown command: " << command << Qt::endl;
        return 1;
    }
    return m_commands.value(command).run(args);
}

void CommandRegistry::printHelp(const QString &command) const
{
    if (!command.isEmpty()) {
        const QString normalized = command.startsWith(QLatin1Char('.')) ? command : QStringLiteral(".") + command;
        if (m_commands.contains(normalized)) {
            const Command entry = m_commands.value(normalized);
            m_out << entry.usage << "\n\n" << entry.description << Qt::endl;
        } else {
            m_out << "Help not found: " << command << Qt::endl;
        }
        return;
    }

    m_out << "Commands:\n";
    for (auto it = m_commands.cbegin(); it != m_commands.cend(); ++it)
        m_out << "  " << it.value().usage << "\n      " << it.value().description << '\n';
    m_out.flush();
}

void CommandRegistry::registerCommands()
{
    m_commands.insert(QStringLiteral(".?"),
                      Command {QStringLiteral(".? [COMMAND]"),
                               QStringLiteral("Shows help."),
                               [this](const QStringList &args) {
                                   printHelp(args.value(1));
                                   return 0;
                               }});
    m_commands.insert(QStringLiteral(".help"), m_commands.value(QStringLiteral(".?")));
    m_commands.insert(QStringLiteral(".status"),
                      Command {QStringLiteral(".status"),
                               QStringLiteral("Checks the MCP connection."),
                               [this](const QStringList &) { return status(); }});
    m_commands.insert(QStringLiteral(".tools"),
                      Command {QStringLiteral(".tools"),
                               QStringLiteral("Lists MCP tools."),
                               [this](const QStringList &) { return tools(); }});
    m_commands.insert(QStringLiteral(".files"),
                      Command {QStringLiteral(".files"),
                               QStringLiteral("Lists downloadable files."),
                               [this](const QStringList &) { return files(); }});
    m_commands.insert(QStringLiteral(".download"),
                      Command {QStringLiteral(".download NAME [PATH]"),
                               QStringLiteral("Downloads NAME to PATH or the current directory."),
                               [this](const QStringList &args) { return download(args); }});
    m_commands.insert(QStringLiteral(".call"),
                      Command {QStringLiteral(".call TOOL [JSON]"),
                               QStringLiteral("Calls an MCP tool directly."),
                               [this](const QStringList &args) { return call(args); }});
}

bool CommandRegistry::ensureConnected()
{
    if (m_client->isInitialized())
        return true;
    QString error;
    if (!m_client->connectToServer(m_timeoutMs, &error)) {
        m_err << "Error: " << error << Qt::endl;
        return false;
    }
    return true;
}

int CommandRegistry::status()
{
    if (!ensureConnected())
        return 1;
    m_out << "Connected: " << m_client->url().toString() << Qt::endl;
    return 0;
}

int CommandRegistry::tools()
{
    if (!ensureConnected())
        return 1;
    QString error;
    const QJsonObject result = m_client->request(QStringLiteral("tools/list"), {}, m_timeoutMs, &error);
    if (!error.isEmpty()) {
        m_err << "Error: " << error << Qt::endl;
        return 1;
    }
    Output::printJson(result, m_out);
    return 0;
}

int CommandRegistry::files()
{
    if (!ensureConnected())
        return 1;
    QString error;
    const QJsonObject result = m_client->callTool(QStringLiteral("list_files"), {}, m_timeoutMs, &error);
    if (!error.isEmpty()) {
        m_err << "Error: " << error << Qt::endl;
        return 1;
    }
    Output::printFiles(result, m_out, m_jsonOutput);
    return 0;
}

int CommandRegistry::download(const QStringList &args)
{
    if (args.size() < 2) {
        printHelp(QStringLiteral(".download"));
        return 1;
    }
    if (!ensureConnected())
        return 1;

    const QString name = args.at(1);
    const QString outputPath = args.size() >= 3 ? args.at(2) : QFileInfo(name).fileName();
    if (QFileInfo::exists(outputPath)) {
        m_err << "Error: output file already exists: " << outputPath << Qt::endl;
        return 1;
    }

    QString error;
    const QJsonObject result = m_client->callTool(QStringLiteral("download_file"),
                                                  QJsonObject {{QStringLiteral("name"), name}},
                                                  m_timeoutMs,
                                                  &error);
    if (!error.isEmpty()) {
        m_err << "Error: " << error << Qt::endl;
        return 1;
    }

    const QByteArray bytes = QByteArray::fromBase64(result.value(QStringLiteral("dataBase64")).toString().toLatin1());
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly)) {
        m_err << "Error: " << file.errorString() << Qt::endl;
        return 1;
    }
    file.write(bytes);
    m_out << "Downloaded " << name << " -> " << outputPath << Qt::endl;
    return 0;
}

int CommandRegistry::call(const QStringList &args)
{
    if (args.size() < 2) {
        printHelp(QStringLiteral(".call"));
        return 1;
    }
    if (!ensureConnected())
        return 1;

    QJsonObject arguments;
    if (args.size() >= 3) {
        QJsonParseError parseError {};
        const QJsonDocument document = QJsonDocument::fromJson(args.mid(2).join(' ').toUtf8(), &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
            m_err << "Error: JSON arguments must be an object." << Qt::endl;
            return 1;
        }
        arguments = document.object();
    }

    QString error;
    const QJsonObject result = m_client->callTool(args.at(1), arguments, m_timeoutMs, &error);
    if (!error.isEmpty()) {
        m_err << "Error: " << error << Qt::endl;
        return 1;
    }
    Output::printToolResult(result, m_out, m_jsonOutput);
    return 0;
}
