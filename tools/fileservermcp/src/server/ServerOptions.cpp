#include "ServerOptions.h"

#include <QCoreApplication>
#include <QTextStream>

ServerOptions ServerOptions::parse(const QStringList &arguments, QTextStream &err)
{
    ServerOptions options;
    for (int i = 1; i < arguments.size(); ++i) {
        const QString arg = arguments.at(i);
        if (arg == QStringLiteral("--root") && i + 1 < arguments.size()) {
            options.rootPath = arguments.at(++i);
        } else if (arg == QStringLiteral("--host") && i + 1 < arguments.size()) {
            const QString value = arguments.at(++i);
            if (!options.host.setAddress(value)) {
                err << "Invalid host: " << value << Qt::endl;
                options.valid = false;
                return options;
            }
        } else if (arg == QStringLiteral("--port") && i + 1 < arguments.size()) {
            bool ok = false;
            const int port = arguments.at(++i).toInt(&ok);
            if (!ok || port < 1 || port > 65535) {
                err << "Invalid port." << Qt::endl;
                options.valid = false;
                return options;
            }
            options.port = static_cast<quint16>(port);
        } else if (arg == QStringLiteral("--max-size") && i + 1 < arguments.size()) {
            bool ok = false;
            const qint64 maxSize = arguments.at(++i).toLongLong(&ok);
            if (!ok || maxSize < 1) {
                err << "Invalid max size." << Qt::endl;
                options.valid = false;
                return options;
            }
            options.maxSize = maxSize;
        } else if (arg == QStringLiteral("-h") || arg == QStringLiteral("--help")) {
            options.helpRequested = true;
        } else {
            err << "Unknown argument: " << arg << Qt::endl;
            options.valid = false;
            return options;
        }
    }

    if (!options.helpRequested && options.rootPath.isEmpty()) {
        err << "--root is required." << Qt::endl;
        options.valid = false;
    }
    return options;
}

void ServerOptions::printHelp(QTextStream &out)
{
    out << "Usage: fileservermcp-server --root PATH [options]\n\n"
        << "Options:\n"
        << "  --root PATH        Directory to expose. Required.\n"
        << "  --host ADDRESS     Listen address. Default: 127.0.0.1\n"
        << "  --port PORT        Listen port. Default: 3101\n"
        << "  --max-size BYTES   Maximum downloadable file size. Default: 10485760\n"
        << "  -h, --help         Show this help.\n";
}
