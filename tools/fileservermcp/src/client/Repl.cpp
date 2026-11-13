#include "Repl.h"

#include "CommandRegistry.h"

#include <QTextStream>

Repl::Repl(CommandRegistry *registry, QTextStream &out, QTextStream &err)
    : m_registry(registry)
    , m_out(out)
    , m_err(err)
{
}

int Repl::run()
{
    QTextStream in(stdin);
    m_out << "File Server MCP Client\nFSMCP> .? for help\n";
    while (true) {
        m_out << "FSMCP> ";
        m_out.flush();
        const QString line = in.readLine().trimmed();
        if (line.isNull() || line == QStringLiteral(".q") || line == QStringLiteral(".quit"))
            return 0;
        if (line.isEmpty())
            continue;
        QStringList args = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (!args.first().startsWith(QLatin1Char('.')))
            args.first().prepend(QLatin1Char('.'));
        Q_UNUSED(m_registry->runReplCommand(args));
    }
}
