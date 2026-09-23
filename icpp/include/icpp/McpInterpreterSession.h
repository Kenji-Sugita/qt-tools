#pragma once

#include "icpp/repl.h"

#include <QProcess>
#include <QString>
#include <QStringList>

namespace icpp {

struct McpEvaluationResult {
    bool completed = false;
    bool timedOut = false;
    bool outputTruncated = false;
    bool interpreterRunning = false;
    QString standardOutput;
    QString standardError;
    QString errorMessage;
    qint64 durationMilliseconds = 0;
};

class McpInterpreterSession
{
public:
    McpInterpreterSession(Engine engine, QString program, QStringList arguments);
    ~McpInterpreterSession();

    bool isRunning() const;
    QString lastError() const;
    bool reset();
    McpEvaluationResult evaluate(const QString& sourceCode, int timeoutMilliseconds);

private:
    bool start();
    void stop();
    void discardPendingOutput();
    QStringList processArguments() const;

    Engine currentEngine;
    QString programPath;
    QStringList additionalArguments;
    QProcess interpreterProcess;
    QString currentError;
};

} // namespace icpp
