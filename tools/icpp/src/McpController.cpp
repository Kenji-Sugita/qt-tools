#include "icpp/McpController.h"

#include "icpp/ReplHelpers.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include <cmath>
#include <utility>

namespace icpp {
namespace {

constexpr qsizetype MaxSourceBytes = 1024 * 1024;
constexpr int DefaultTimeoutMilliseconds = 5000;
constexpr int MinimumTimeoutMilliseconds = 100;
constexpr int MaximumTimeoutMilliseconds = 30000;

QJsonObject emptyObjectSchema()
{
    return {{QStringLiteral("type"), QStringLiteral("object")}};
}

bool sourceAppearsIncomplete(const QString& sourceCode)
{
    int braceDelta = 0;
    const QStringList lines = normalizeSourceCode(sourceCode).split(QLatin1Char('\n'));
    for (const QString& line : lines) {
        braceDelta += braceDeltaOutsideQuotedText(line);
    }
    return braceDelta != 0;
}

bool readTimeout(const QJsonObject& arguments, int* timeout, QString* errorMessage)
{
    const QJsonValue value = arguments.value(QStringLiteral("timeoutMs"));
    if (value.isUndefined()) {
        *timeout = DefaultTimeoutMilliseconds;
        return true;
    }
    if (!value.isDouble() || !std::isfinite(value.toDouble())
        || std::floor(value.toDouble()) != value.toDouble()) {
        *errorMessage = QStringLiteral("timeoutMs must be an integer.");
        return false;
    }

    const int parsed = value.toInt(-1);
    if (parsed < MinimumTimeoutMilliseconds || parsed > MaximumTimeoutMilliseconds) {
        *errorMessage = QStringLiteral("timeoutMs must be between %1 and %2.")
                            .arg(MinimumTimeoutMilliseconds)
                            .arg(MaximumTimeoutMilliseconds);
        return false;
    }
    *timeout = parsed;
    return true;
}

} // namespace

McpController::McpController(Engine engine,
                             QString interpreterProgram,
                             QStringList interpreterArguments,
                             bool executionAllowed,
                             QObject* parent)
    : QObject(parent)
    , currentEngine(engine)
    , allowExecution(executionAllowed)
    , interpreter(engine, std::move(interpreterProgram), std::move(interpreterArguments))
{
}

McpController::~McpController() = default;

bool McpController::startMessageTransport()
{
    return registerTools();
}

bool McpController::processMessage(const QByteArray& message,
                                   QJsonObject* response,
                                   bool* hasResponse)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(message, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (response) {
            *response = jsonRpcError(QJsonValue(), -32700, QStringLiteral("Parse error."));
        }
        if (hasResponse) {
            *hasResponse = true;
        }
        return false;
    }

    const QJsonObject request = document.object();
    const QJsonValue id = request.value(QStringLiteral("id"));
    const QString method = request.value(QStringLiteral("method")).toString();
    const bool notification = id.isUndefined();

    if (hasResponse) {
        *hasResponse = !notification;
    }

    if (request.value(QStringLiteral("jsonrpc")).toString() != QStringLiteral("2.0")
        || method.isEmpty()) {
        if (response) {
            *response = jsonRpcError(id, -32600, QStringLiteral("Invalid Request."));
        }
        if (hasResponse) {
            *hasResponse = true;
        }
        return false;
    }

    if (method == QStringLiteral("initialize")) {
        if (notification) {
            return false;
        }
        const QJsonObject params = request.value(QStringLiteral("params")).toObject();
        const QString requestedVersion = params.value(QStringLiteral("protocolVersion")).toString();
        if (requestedVersion != QStringLiteral("2025-03-26")
            && requestedVersion != QStringLiteral("2025-06-18")) {
            if (response) {
                *response = jsonRpcError(
                    id,
                    -32602,
                    QStringLiteral("Unsupported MCP protocol version: %1").arg(requestedVersion));
            }
            if (hasResponse) {
                *hasResponse = true;
            }
            return false;
        }
        if (response) {
            *response = jsonRpcResult(id, initializeResult(params));
        }
        initialized = true;
        return true;
    }

    if (method == QStringLiteral("notifications/initialized")) {
        return notification;
    }

    if (!initialized) {
        if (response) {
            *response = jsonRpcError(id, -32002, QStringLiteral("MCP session is not initialized."));
        }
        if (hasResponse) {
            *hasResponse = true;
        }
        return false;
    }

    if (method == QStringLiteral("tools/list")) {
        if (notification) {
            return false;
        }
        if (response) {
            *response = jsonRpcResult(id, toolsListResult());
        }
        return true;
    }

    if (method == QStringLiteral("tools/call")) {
        if (notification) {
            return false;
        }
        const QJsonObject params = request.value(QStringLiteral("params")).toObject();
        const QString name = params.value(QStringLiteral("name")).toString();
        const QJsonObject arguments = params.value(QStringLiteral("arguments")).toObject();

        QString errorMessage;
        QJsonObject result;
        int errorCode = -32602;
        if (name == QStringLiteral("icpp/session/status")) {
            result = statusTool(arguments, &errorMessage);
        } else if (name == QStringLiteral("icpp/code/evaluate")) {
            if (!allowExecution) {
                errorMessage = QStringLiteral(
                    "Execution is disabled. Restart icpp with --mcp-allow-execution to enable it.");
                errorCode = -32001;
            } else {
                result = evaluateTool(arguments, &errorMessage);
            }
        } else if (name == QStringLiteral("icpp/session/reset")) {
            if (!allowExecution) {
                errorMessage = QStringLiteral(
                    "Execution is disabled. Restart icpp with --mcp-allow-execution to enable it.");
                errorCode = -32001;
            } else {
                result = resetTool(arguments, &errorMessage);
            }
        } else {
            errorMessage = QStringLiteral("Unknown tool: %1").arg(name);
            errorCode = -32601;
        }

        if (!errorMessage.isEmpty()) {
            if (response) {
                *response = jsonRpcError(id, errorCode, errorMessage);
            }
            return true;
        }
        if (response) {
            *response = jsonRpcResult(id, result);
        }
        return true;
    }

    if (response) {
        *response = jsonRpcError(id, -32601, QStringLiteral("Method not found."));
    }
    if (hasResponse) {
        *hasResponse = true;
    }
    return false;
}

QString McpController::negotiatedProtocolVersion() const
{
    return protocolVersion;
}

bool McpController::registerTools()
{
    if (toolsRegistered) {
        return true;
    }

    toolsRegistered = true;
    return toolsRegistered;
}

QJsonObject McpController::initializeResult(const QJsonObject& params)
{
    const QString requestedVersion = params.value(QStringLiteral("protocolVersion")).toString();
    protocolVersion = requestedVersion;

    return {
        {QStringLiteral("protocolVersion"), protocolVersion},
        {QStringLiteral("capabilities"), QJsonObject{
             {QStringLiteral("tools"), QJsonObject{}},
         }},
        {QStringLiteral("serverInfo"), QJsonObject{
             {QStringLiteral("name"), QStringLiteral("icpp")},
             {QStringLiteral("version"), QStringLiteral(ICPP_VERSION)},
         }},
    };
}

QJsonObject McpController::toolsListResult() const
{
    const QJsonObject evaluateSchema{
        {QStringLiteral("type"), QStringLiteral("object")},
        {QStringLiteral("properties"), QJsonObject{
             {QStringLiteral("source"), QJsonObject{
                  {QStringLiteral("type"), QStringLiteral("string")},
                  {QStringLiteral("minLength"), 1},
                  {QStringLiteral("maxLength"), int(MaxSourceBytes)},
              }},
             {QStringLiteral("timeoutMs"), QJsonObject{
                  {QStringLiteral("type"), QStringLiteral("integer")},
                  {QStringLiteral("minimum"), MinimumTimeoutMilliseconds},
                  {QStringLiteral("maximum"), MaximumTimeoutMilliseconds},
              }},
         }},
        {QStringLiteral("required"), QJsonArray{QStringLiteral("source")}},
    };

    return {
        {QStringLiteral("tools"), QJsonArray{
             toolDescription(
                 QStringLiteral("icpp/session/status"),
                 QStringLiteral("Return the icpp engine, interpreter state, working directory, and execution permission."),
                 emptyObjectSchema()),
             toolDescription(
                 QStringLiteral("icpp/code/evaluate"),
                 QStringLiteral("Evaluate C++ or Qt code in the persistent interpreter for this MCP session."),
                 evaluateSchema),
             toolDescription(
                 QStringLiteral("icpp/session/reset"),
                 QStringLiteral("Restart the interpreter and clear definitions in this MCP session."),
                 emptyObjectSchema()),
         }},
    };
}

QJsonObject McpController::jsonRpcError(const QJsonValue& id, int code, const QString& message) const
{
    QJsonObject result{
        {QStringLiteral("jsonrpc"), QStringLiteral("2.0")},
        {QStringLiteral("error"), QJsonObject{
             {QStringLiteral("code"), code},
             {QStringLiteral("message"), message},
         }},
    };
    result.insert(QStringLiteral("id"), id.isUndefined() ? QJsonValue::Null : id);
    return result;
}

QJsonObject McpController::jsonRpcResult(const QJsonValue& id, const QJsonObject& resultObject) const
{
    QJsonObject result{
        {QStringLiteral("jsonrpc"), QStringLiteral("2.0")},
        {QStringLiteral("result"), resultObject},
    };
    result.insert(QStringLiteral("id"), id.isUndefined() ? QJsonValue::Null : id);
    return result;
}

QJsonObject McpController::toolDescription(const QString& name,
                                           const QString& description,
                                           const QJsonObject& inputSchema) const
{
    return {
        {QStringLiteral("name"), name},
        {QStringLiteral("description"), description},
        {QStringLiteral("inputSchema"), inputSchema},
    };
}

QJsonObject McpController::statusTool(const QJsonObject&, QString*) const
{
    QJsonObject result{
        {QStringLiteral("engine"), engineName(currentEngine)},
        {QStringLiteral("interpreterRunning"), interpreter.isRunning()},
        {QStringLiteral("executionAllowed"), allowExecution},
        {QStringLiteral("workingDirectory"), QDir::currentPath()},
    };
    if (!interpreter.lastError().isEmpty()) {
        result.insert(QStringLiteral("lastError"), interpreter.lastError());
    }
    return result;
}

QJsonObject McpController::evaluateTool(const QJsonObject& arguments, QString* errorMessage)
{
    const QJsonValue sourceValue = arguments.value(QStringLiteral("source"));
    if (!sourceValue.isString() || sourceValue.toString().trimmed().isEmpty()) {
        *errorMessage = QStringLiteral("source must be a non-empty string.");
        return {};
    }
    const QString sourceCode = sourceValue.toString();
    if (sourceCode.contains(QChar::Null)) {
        *errorMessage = QStringLiteral("source must not contain NUL characters.");
        return {};
    }
    if (sourceCode.toUtf8().size() > MaxSourceBytes) {
        *errorMessage = QStringLiteral("source exceeds the 1 MiB limit.");
        return {};
    }
    if (sourceAppearsIncomplete(sourceCode)) {
        *errorMessage = QStringLiteral(
            "Input appears incomplete: unmatched { }. Close the block and evaluate again.");
        return {};
    }

    int timeoutMilliseconds = DefaultTimeoutMilliseconds;
    if (!readTimeout(arguments, &timeoutMilliseconds, errorMessage)) {
        return {};
    }

    const McpEvaluationResult evaluation = interpreter.evaluate(sourceCode, timeoutMilliseconds);
    return {
        {QStringLiteral("completed"), evaluation.completed},
        {QStringLiteral("timedOut"), evaluation.timedOut},
        {QStringLiteral("interpreterRunning"), evaluation.interpreterRunning},
        {QStringLiteral("outputTruncated"), evaluation.outputTruncated},
        {QStringLiteral("stdout"), evaluation.standardOutput},
        {QStringLiteral("stderr"), evaluation.standardError},
        {QStringLiteral("durationMs"), evaluation.durationMilliseconds},
        {QStringLiteral("error"), evaluation.errorMessage},
    };
}

QJsonObject McpController::resetTool(const QJsonObject&, QString* errorMessage)
{
    if (!interpreter.reset()) {
        *errorMessage = interpreter.lastError().isEmpty()
            ? QStringLiteral("The interpreter could not be reset.")
            : interpreter.lastError();
        return {};
    }
    return {
        {QStringLiteral("reset"), true},
        {QStringLiteral("interpreterRunning"), true},
    };
}

} // namespace icpp
