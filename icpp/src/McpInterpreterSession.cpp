#include "icpp/McpInterpreterSession.h"

#include <QElapsedTimer>
#include <QProcessEnvironment>
#include <QUuid>

#include <algorithm>
#include <cstdio>

namespace icpp {
namespace {

constexpr qsizetype MaxCapturedOutputBytes = 1024 * 1024;

class MarkerCollector
{
public:
    explicit MarkerCollector(QByteArray marker)
        : endMarker(std::move(marker))
    {
    }

    void append(const QByteArray& bytes)
    {
        if (foundMarker || bytes.isEmpty()) {
            return;
        }

        pending.append(bytes);
        const qsizetype markerIndex = pending.indexOf(endMarker);
        if (markerIndex >= 0) {
            capture(pending.first(markerIndex));
            pending.clear();
            foundMarker = true;
            return;
        }

        const qsizetype retainedSize = qMin(pending.size(), endMarker.size() - 1);
        const qsizetype flushSize = pending.size() - retainedSize;
        if (flushSize > 0) {
            capture(pending.first(flushSize));
            pending.remove(0, flushSize);
        }
    }

    void finishWithoutMarker()
    {
        capture(pending);
        pending.clear();
    }

    bool complete() const
    {
        return foundMarker;
    }

    bool truncated() const
    {
        return wasTruncated;
    }

    QString text() const
    {
        return QString::fromLocal8Bit(captured);
    }

    const QByteArray& bytes() const
    {
        return captured;
    }

private:
    void capture(const QByteArray& bytes)
    {
        const qsizetype available = MaxCapturedOutputBytes - captured.size();
        if (available > 0) {
            captured.append(bytes.first(qMin(available, bytes.size())));
        }
        if (bytes.size() > available) {
            wasTruncated = true;
        }
    }

    QByteArray endMarker;
    QByteArray pending;
    QByteArray captured;
    bool foundMarker = false;
    bool wasTruncated = false;
};

} // namespace

McpInterpreterSession::McpInterpreterSession(Engine engine,
                                             QString program,
                                             QStringList arguments)
    : currentEngine(engine)
    , programPath(std::move(program))
    , additionalArguments(std::move(arguments))
{
}

McpInterpreterSession::~McpInterpreterSession()
{
    stop();
}

bool McpInterpreterSession::isRunning() const
{
    return interpreterProcess.state() != QProcess::NotRunning;
}

QString McpInterpreterSession::lastError() const
{
    return currentError;
}

bool McpInterpreterSession::reset()
{
    stop();
    return start();
}

McpEvaluationResult McpInterpreterSession::evaluate(const QString& sourceCode,
                                                     int timeoutMilliseconds)
{
    McpEvaluationResult result;
    QElapsedTimer elapsed;
    elapsed.start();

    if (!isRunning() && !start()) {
        result.errorMessage = currentError;
        result.durationMilliseconds = elapsed.elapsed();
        return result;
    }

    discardPendingOutput();
    const QByteArray marker = QStringLiteral("__ICPP_MCP_END_%1__")
                                  .arg(QUuid::createUuid().toString(QUuid::WithoutBraces))
                                  .toUtf8();
    MarkerCollector outputCollector(marker);
    MarkerCollector errorCollector(marker);

    QByteArray input = sourceCode.toUtf8();
    if (!input.endsWith('\n')) {
        input.append('\n');
    }
    input.append("#include <cstdio>\n");
    input.append("std::fputs(\"");
    input.append(marker);
    input.append("\\n\", stdout); std::fflush(stdout);\n");
    input.append("std::fputs(\"");
    input.append(marker);
    input.append("\\n\", stderr); std::fflush(stderr);\n");

    if (interpreterProcess.write(input) < 0
        || !interpreterProcess.waitForBytesWritten(qMin(timeoutMilliseconds, 1000))) {
        result.errorMessage = QStringLiteral("Could not write source code to the interpreter: %1")
                                  .arg(interpreterProcess.errorString());
        stop();
        result.durationMilliseconds = elapsed.elapsed();
        return result;
    }

    while (elapsed.elapsed() < timeoutMilliseconds
           && !(outputCollector.complete() && errorCollector.complete())) {
        outputCollector.append(interpreterProcess.readAllStandardOutput());
        errorCollector.append(interpreterProcess.readAllStandardError());
        if (outputCollector.complete() && errorCollector.complete()) {
            break;
        }
        if (interpreterProcess.state() == QProcess::NotRunning) {
            break;
        }
        const int remaining = int(timeoutMilliseconds - elapsed.elapsed());
        interpreterProcess.waitForReadyRead(qMin(remaining, 50));
    }

    outputCollector.append(interpreterProcess.readAllStandardOutput());
    errorCollector.append(interpreterProcess.readAllStandardError());
    const bool markersFound = outputCollector.complete() && errorCollector.complete();
    if (!markersFound) {
        outputCollector.finishWithoutMarker();
        errorCollector.finishWithoutMarker();
    }

    result.standardOutput = outputCollector.text();
    result.standardError = errorCollector.text();
    if (!outputCollector.bytes().isEmpty()) {
        std::fwrite(outputCollector.bytes().constData(),
                    1,
                    size_t(outputCollector.bytes().size()),
                    stdout);
        std::fflush(stdout);
    }
    if (!errorCollector.bytes().isEmpty()) {
        std::fwrite(errorCollector.bytes().constData(),
                    1,
                    size_t(errorCollector.bytes().size()),
                    stderr);
        std::fflush(stderr);
    }
    result.outputTruncated = outputCollector.truncated() || errorCollector.truncated();
    result.durationMilliseconds = elapsed.elapsed();

    if (markersFound) {
        result.completed = true;
        result.interpreterRunning = isRunning();
        currentError.clear();
        return result;
    }

    if (interpreterProcess.state() == QProcess::NotRunning) {
        result.errorMessage = QStringLiteral("The interpreter exited before evaluation completed.");
    } else {
        result.timedOut = true;
        result.errorMessage = QStringLiteral("Evaluation exceeded the %1 ms timeout.")
                                  .arg(timeoutMilliseconds);
    }
    stop();
    result.interpreterRunning = false;
    return result;
}

bool McpInterpreterSession::start()
{
    if (programPath.trimmed().isEmpty()) {
        currentError = QStringLiteral("Interpreter program is not configured.");
        return false;
    }
    if (isRunning()) {
        return true;
    }

    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.remove(QStringLiteral("ICPP_MCP_TOKEN"));
    interpreterProcess.setProcessEnvironment(environment);
    interpreterProcess.setProgram(programPath);
    interpreterProcess.setArguments(processArguments());
    interpreterProcess.setProcessChannelMode(QProcess::SeparateChannels);
    interpreterProcess.start();
    if (!interpreterProcess.waitForStarted(5000)) {
        currentError = QStringLiteral("Could not start %1: %2")
                           .arg(programPath, interpreterProcess.errorString());
        return false;
    }
    interpreterProcess.waitForReadyRead(100);
    discardPendingOutput();
    currentError.clear();
    return true;
}

void McpInterpreterSession::stop()
{
    if (interpreterProcess.state() == QProcess::NotRunning) {
        return;
    }

    interpreterProcess.closeWriteChannel();
    if (!interpreterProcess.waitForFinished(250)) {
        interpreterProcess.terminate();
    }
    if (!interpreterProcess.waitForFinished(500)) {
        interpreterProcess.kill();
        interpreterProcess.waitForFinished(1000);
    }
}

void McpInterpreterSession::discardPendingOutput()
{
    interpreterProcess.readAllStandardOutput();
    interpreterProcess.readAllStandardError();
}

QStringList McpInterpreterSession::processArguments() const
{
    QStringList result{QStringLiteral("--nologo")};
    result.append(additionalArguments);
    return result;
}

} // namespace icpp
