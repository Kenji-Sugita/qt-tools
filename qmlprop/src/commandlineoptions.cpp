#include "commandlineoptions.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QStringList>

namespace
{

int countDebugOptionOccurrences(const QStringList& argumentList)
{
    int debugOptionCount = 0;
    bool positionalArgumentsOnly = false;

    for (int argumentIndex = 1; argumentIndex < argumentList.size(); ++argumentIndex) {
        const QString& argument = argumentList.at(argumentIndex);

        if (positionalArgumentsOnly) {
            continue;
        }

        if (argument == QStringLiteral("--")) {
            positionalArgumentsOnly = true;
            continue;
        }

        if (argument == QStringLiteral("-d") || argument == QStringLiteral("--debug")) {
            ++debugOptionCount;
            continue;
        }

        if (argument.startsWith(QStringLiteral("--"))) {
            continue;
        }

        if (!argument.startsWith(QLatin1Char('-')) || argument == QStringLiteral("-")) {
            continue;
        }

        for (int characterIndex = 1; characterIndex < argument.size(); ++characterIndex) {
            if (argument.at(characterIndex) == QLatin1Char('d')) {
                ++debugOptionCount;
            }
        }
    }

    return debugOptionCount;
}

} // namespace

CommandLineOptions parseCommandLineArguments(QCoreApplication& application)
{
    QCommandLineParser commandLineParser;
    commandLineParser.setApplicationDescription(QStringLiteral("Show information about a QML type."));

    const QCommandLineOption showAllOption({QStringLiteral("a"), QStringLiteral("all")},
                                           QStringLiteral("Show inherited members too."));
    const QCommandLineOption listTypesOption({QStringLiteral("l"), QStringLiteral("list-types")},
                                             QStringLiteral("List importable QML type names visible from the current runtime."));
    const QCommandLineOption listModulesOption({QStringLiteral("m"), QStringLiteral("list-modules")},
                                               QStringLiteral("List importable QML modules visible from the current runtime."));
    const QCommandLineOption moduleIndexOption(QStringLiteral("module-index"),
                                               QStringLiteral("Show a one-line index for all types in the specified QML module."),
                                               QStringLiteral("module"));
    const QCommandLineOption debugOption({QStringLiteral("d"), QStringLiteral("debug")},
                                         QStringLiteral("Show debug information. Repeat for more detail."));
    const QCommandLineOption showAttachedOption(QStringLiteral("show-attached"),
                                                QStringLiteral("Show attached properties."));
    const QCommandLineOption showDeclaredInOption(QStringLiteral("show-declared-in"),
                                                  QStringLiteral("Show the declaring C++ class for each member."));
    const QCommandLineOption showAllMatchesOption(QStringLiteral("all-matches"),
                                                  QStringLiteral("Show all same-name type matches across modules."));
    const QCommandLineOption noPagerOption(QStringLiteral("no-pager"),
                                           QStringLiteral("Write directly to standard output instead of using a pager."));
    const QCommandLineOption helpOption({QStringLiteral("h"), QStringLiteral("help")},
                                        QStringLiteral("Show help."));
    const QCommandLineOption versionOption({QStringLiteral("v"), QStringLiteral("version")},
                                           QStringLiteral("Show version."));

    commandLineParser.addOption(showAllOption);
    commandLineParser.addOption(listTypesOption);
    commandLineParser.addOption(listModulesOption);
    commandLineParser.addOption(moduleIndexOption);
    commandLineParser.addOption(debugOption);
    commandLineParser.addOption(showAttachedOption);
    commandLineParser.addOption(showDeclaredInOption);
    commandLineParser.addOption(showAllMatchesOption);
    commandLineParser.addOption(noPagerOption);
    commandLineParser.addOption(helpOption);
    commandLineParser.addOption(versionOption);
    commandLineParser.addPositionalArgument(QStringLiteral("type"),
                                            QStringLiteral("QML type name, for example Rectangle or FolderDialog."),
                                            QStringLiteral("type"));

    commandLineParser.process(application);

    if (commandLineParser.isSet(versionOption)) {
        if (!commandLineParser.positionalArguments().isEmpty() || commandLineParser.optionNames().size() > 1) {
            commandLineParser.showHelp(1);
        }
        commandLineParser.showVersion();
    }

    if (commandLineParser.isSet(helpOption)) {
        commandLineParser.showHelp(0);
    }

    const QStringList positionalArguments = commandLineParser.positionalArguments();
    const bool listTypes = commandLineParser.isSet(listTypesOption);
    const bool listModules = commandLineParser.isSet(listModulesOption);
    const bool moduleIndex = commandLineParser.isSet(moduleIndexOption);

    const int exclusiveModeCount = (listTypes ? 1 : 0) + (listModules ? 1 : 0) + (moduleIndex ? 1 : 0);
    if (exclusiveModeCount > 1
        || ((!listTypes && !listModules && !moduleIndex) && positionalArguments.size() != 1)
        || ((listTypes || listModules || moduleIndex) && !positionalArguments.isEmpty())) {
        commandLineParser.showHelp(1);
    }

    CommandLineOptions commandLineOptions;
    commandLineOptions.showAll = commandLineParser.isSet(showAllOption);
    commandLineOptions.listTypes = listTypes;
    commandLineOptions.listModules = listModules;
    commandLineOptions.moduleIndex = moduleIndex;
    commandLineOptions.showAttached = commandLineParser.isSet(showAttachedOption);
    commandLineOptions.showDeclaredIn = commandLineParser.isSet(showDeclaredInOption);
    commandLineOptions.showAllMatches = commandLineParser.isSet(showAllMatchesOption);
    commandLineOptions.usePager = !commandLineParser.isSet(noPagerOption);
    commandLineOptions.debugLevel = countDebugOptionOccurrences(application.arguments());
    if (moduleIndex) {
        commandLineOptions.requestedModuleUri = commandLineParser.value(moduleIndexOption);
    }
    if (!positionalArguments.isEmpty()) {
        commandLineOptions.requestedTypeName = positionalArguments.constFirst();
    }
    return commandLineOptions;
}
