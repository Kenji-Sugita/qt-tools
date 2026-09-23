#ifndef COMMANDLINEOPTIONS_H
#define COMMANDLINEOPTIONS_H

#include <QString>

class QCoreApplication;

struct CommandLineOptions
{
    bool showAll = false;
    bool listTypes = false;
    bool listModules = false;
    bool moduleIndex = false;
    bool showAttached = false;
    bool showDeclaredIn = false;
    bool showAllMatches = false;
    bool usePager = true;
    int debugLevel = 0;
    QString requestedModuleUri;
    QString requestedTypeName;
};

CommandLineOptions parseCommandLineArguments(QCoreApplication& application);

#endif // COMMANDLINEOPTIONS_H
