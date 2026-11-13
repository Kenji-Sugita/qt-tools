#include "ClientOptions.h"

#include <QTextStream>

ClientOptions ClientOptions::parse(const QStringList &arguments, QTextStream &err)
{
    ClientOptions options;
    for (int i = 1; i < arguments.size(); ++i) {
        const QString arg = arguments.at(i);
        if (arg == QStringLiteral("--url") && i + 1 < arguments.size()) {
            options.url = QUrl(arguments.at(++i));
        } else if (arg == QStringLiteral("--json")) {
            options.jsonOutput = true;
        } else if (arg == QStringLiteral("--timeout") && i + 1 < arguments.size()) {
            bool ok = false;
            const int seconds = arguments.at(++i).toInt(&ok);
            if (!ok || seconds < 1) {
                err << "Invalid timeout." << Qt::endl;
                options.valid = false;
                return options;
            }
            options.timeoutMs = seconds * 1000;
        } else if (arg == QStringLiteral("-h") || arg == QStringLiteral("--help")) {
            if (options.positionalArgs.isEmpty())
                options.helpRequested = true;
            else
                options.positionalArgs << arg;
        } else {
            options.positionalArgs << arg;
        }
    }

    if (!options.url.isValid() || options.url.scheme().isEmpty()) {
        err << "Invalid URL." << Qt::endl;
        options.valid = false;
    }
    return options;
}

void ClientOptions::printHelp(QTextStream &out)
{
    out << "Usage: fileservermcp-client [options] [command]\n\n"
        << "Options:\n"
        << "  --url URL          SSE URL. Default: http://127.0.0.1:3101/sse\n"
        << "  --json             Print JSON output.\n"
        << "  --timeout SECONDS  Request timeout. Default: 10\n"
        << "  -h, --help         Show this help.\n\n"
        << "Commands:\n"
        << "  status\n"
        << "  tools\n"
        << "  files\n"
        << "  download NAME [PATH]\n"
        << "  call TOOL [JSON]\n\n"
        << "Without a command, starts the REPL.\n";
}
