#include "FileRepository.h"
#include "McpToolHandler.h"
#include "ServerOptions.h"
#include "SseMcpServer.h"

#include <QCoreApplication>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QTextStream out(stdout);
    QTextStream err(stderr);

    const ServerOptions options = ServerOptions::parse(QCoreApplication::arguments(), err);
    if (options.helpRequested) {
        ServerOptions::printHelp(out);
        return 0;
    }
    if (!options.valid)
        return 1;

    FileRepository repository(options.rootPath, options.maxSize);
    QString repositoryError;
    if (!repository.isValid(&repositoryError)) {
        err << "Error: " << repositoryError << Qt::endl;
        return 1;
    }

    McpToolHandler tools(&repository);
    SseMcpServer server(&tools);
    QString listenError;
    if (!server.listen(options.host, options.port, &listenError)) {
        err << "Error: " << listenError << Qt::endl;
        return 1;
    }

    out << "fileservermcp-server listening at http://" << options.host.toString() << ':'
        << options.port << "/sse" << Qt::endl;
    return app.exec();
}
