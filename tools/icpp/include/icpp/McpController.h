#pragma once

#include "icpp/McpInterpreterSession.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QObject>

#include <QByteArray>
#include <QString>
#include <QStringList>

namespace icpp {

class McpController : public QObject
{
public:
    McpController(Engine engine,
                  QString interpreterProgram,
                  QStringList interpreterArguments,
                  bool executionAllowed,
                  QObject* parent = nullptr);
    ~McpController() override;

    bool startMessageTransport();
    bool processMessage(const QByteArray& message, QJsonObject* response, bool* hasResponse);
    QString negotiatedProtocolVersion() const;

private:
    bool registerTools();
    QJsonObject statusTool(const QJsonObject& arguments, QString* errorMessage) const;
    QJsonObject evaluateTool(const QJsonObject& arguments, QString* errorMessage);
    QJsonObject resetTool(const QJsonObject& arguments, QString* errorMessage);
    QJsonObject initializeResult(const QJsonObject& params);
    QJsonObject toolsListResult() const;
    QJsonObject jsonRpcError(const QJsonValue& id, int code, const QString& message) const;
    QJsonObject jsonRpcResult(const QJsonValue& id, const QJsonObject& result) const;
    QJsonObject toolDescription(const QString& name,
                                const QString& description,
                                const QJsonObject& inputSchema) const;

    Engine currentEngine;
    bool allowExecution = false;
    McpInterpreterSession interpreter;
    bool toolsRegistered = false;
    bool initialized = false;
    QString protocolVersion;
};

} // namespace icpp
