#include "ClientOptions.h"
#include "CommandRegistry.h"
#include "Repl.h"
#include "SseMcpClient.h"

#include <QCoreApplication>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QTextStream out(stdout);
    QTextStream err(stderr);

    const ClientOptions options = ClientOptions::parse(QCoreApplication::arguments(), err);
    if (options.helpRequested) {
        ClientOptions::printHelp(out);
        return 0;
    }
    if (!options.valid)
        return 1;

    SseMcpClient client(options.url);
    CommandRegistry registry(&client, options, out, err);

    if (!options.positionalArgs.isEmpty())
        return registry.runOneShot(options.positionalArgs);

    Repl repl(&registry, out, err);
    return repl.run();
}
