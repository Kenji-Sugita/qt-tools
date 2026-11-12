#include "icpp/InterpreterProcess.h"

#include <QTextStream>
#include <QThread>

namespace icpp {

InterpreterProcess::InterpreterProcess(Engine engine,
                                       QString program,
                                       QStringList extraArguments,
                                       HelpLanguage language,
                                       QTextStream& output,
                                       QTextStream& error)
    : currentEngine(engine)
    , programPath(std::move(program))
    , additionalArguments(std::move(extraArguments))
    , helpLanguage(language)
    , standardOutput(output)
    , standardError(error)
{
}

InterpreterProcess::~InterpreterProcess()
{
    stop();
}

bool InterpreterProcess::start()
{
    if (programPath.isEmpty()) {
        return false;
    }

    interpreterProcess.setProgram(programPath);
    interpreterProcess.setArguments(arguments());
    interpreterProcess.setProcessChannelMode(QProcess::ForwardedChannels);
    interpreterProcess.start();
    if (!interpreterProcess.waitForStarted()) {
        standardError << trMessage(QStringLiteral("%1 を起動できません: %2"),
                                   QStringLiteral("Cannot start %1: %2"))
                             .arg(programPath, interpreterProcess.errorString())
                      << Qt::endl;
        return false;
    }
    waitForInterpreterStartupToSettle();
    return true;
}

bool InterpreterProcess::restart()
{
    stop();
    return start();
}

void InterpreterProcess::stop()
{
    if (interpreterProcess.state() == QProcess::NotRunning) {
        return;
    }

    interpreterProcess.closeWriteChannel();
    if (!interpreterProcess.waitForFinished(500)) {
        interpreterProcess.terminate();
    }
    if (!interpreterProcess.waitForFinished(1000)) {
        interpreterProcess.kill();
        interpreterProcess.waitForFinished(1000);
    }
}

bool InterpreterProcess::sendSource(const QString& sourceCode)
{
    if (interpreterProcess.state() == QProcess::NotRunning && !start()) {
        return false;
    }

    QString source = sourceCode;
    if (!source.endsWith(QLatin1Char('\n'))) {
        source.append(QLatin1Char('\n'));
    }

    interpreterProcess.write(source.toLocal8Bit());
    if (!interpreterProcess.waitForBytesWritten(1000)) {
        standardError << trMessage(QStringLiteral("%1 に書き込めません: %2"),
                                   QStringLiteral("Cannot write to %1: %2"))
                             .arg(programPath, interpreterProcess.errorString())
                      << Qt::endl;
        return false;
    }

    waitForBatchInputToSettle();
    return true;
}

void InterpreterProcess::showArguments() const
{
    standardOutput << trMessage(QStringLiteral("エンジン: %1"), QStringLiteral("engine: %1"))
                          .arg(engineName(currentEngine))
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("プログラム: %1"), QStringLiteral("program: %1"))
                          .arg(programPath)
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("引数:"), QStringLiteral("arguments:"));
    const QStringList currentArguments = arguments();
    if (currentArguments.isEmpty()) {
        standardOutput << trMessage(QStringLiteral(" (なし)"), QStringLiteral(" (none)"))
                       << Qt::endl;
        return;
    }
    standardOutput << Qt::endl;
    for (const QString& argument : currentArguments) {
        standardOutput << QStringLiteral("  %1").arg(argument) << Qt::endl;
    }
}

void InterpreterProcess::setHelpLanguage(HelpLanguage language)
{
    helpLanguage = language;
}

QString InterpreterProcess::trMessage(const QString& japanese, const QString& english) const
{
    return helpLanguage == HelpLanguage::Japanese ? japanese : english;
}

QStringList InterpreterProcess::arguments() const
{
    QStringList result;
    result << QStringLiteral("--nologo");
    result << additionalArguments;
    return result;
}

void InterpreterProcess::waitForBatchInputToSettle()
{
    QThread::msleep(1000);
}

void InterpreterProcess::waitForInterpreterStartupToSettle()
{
    QThread::msleep(1000);
}

} // namespace icpp
