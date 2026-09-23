#pragma once

#include "icpp/repl.h"

#include <QProcess>
#include <QString>
#include <QStringList>

class QTextStream;

namespace icpp {

class InterpreterProcess
{
public:
    InterpreterProcess(Engine engine,
                       QString program,
                       QStringList extraArguments,
                       HelpLanguage language,
                       QTextStream& output,
                       QTextStream& error);
    ~InterpreterProcess();

    bool start();
    bool restart();
    void stop();
    bool sendSource(const QString& sourceCode);
    void showArguments() const;
    void setHelpLanguage(HelpLanguage language);

private:
    QString trMessage(const QString& japanese, const QString& english) const;
    QStringList arguments() const;
    void waitForBatchInputToSettle();
    void waitForInterpreterStartupToSettle();

    Engine currentEngine;
    QString programPath;
    QStringList additionalArguments;
    HelpLanguage helpLanguage;
    QTextStream& standardOutput;
    QTextStream& standardError;
    QProcess interpreterProcess;
};

} // namespace icpp
