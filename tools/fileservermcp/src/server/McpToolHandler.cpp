#include "McpToolHandler.h"

#include "FileRepository.h"
#include "JsonRpcMessage.h"

#include <QJsonArray>
#include <QJsonObject>

McpToolHandler::McpToolHandler(FileRepository *repository)
    : m_repository(repository)
{
}

QJsonObject McpToolHandler::handleRequest(const QJsonObject &request) const
{
    const QJsonValue id = request.value(QStringLiteral("id"));
    const QString method = request.value(QStringLiteral("method")).toString();
    const QJsonObject params = request.value(QStringLiteral("params")).toObject();

    if (method == QStringLiteral("initialize"))
        return initialize(id);
    if (method == QStringLiteral("tools/list"))
        return toolsList(id);
    if (method == QStringLiteral("tools/call"))
        return toolsCall(id, params);
    if (method == QStringLiteral("notifications/initialized"))
        return {};
    return JsonRpcMessage::error(id, -32601, QStringLiteral("method not found"));
}

QJsonObject McpToolHandler::initialize(const QJsonValue &id) const
{
    return JsonRpcMessage::success(id,
                                   QJsonObject {
                                       {QStringLiteral("protocolVersion"),
                                        QStringLiteral("2024-11-05")},
                                       {QStringLiteral("serverInfo"),
                                        QJsonObject {
                                            {QStringLiteral("name"), QStringLiteral("fileservermcp")},
                                            {QStringLiteral("version"), QStringLiteral("0.1.0")},
                                        }},
                                       {QStringLiteral("capabilities"),
                                        QJsonObject {{QStringLiteral("tools"), QJsonObject {}}}},
                                   });
}

QJsonObject McpToolHandler::toolsList(const QJsonValue &id) const
{
    QJsonArray tools;
    tools.append(QJsonObject {
        {QStringLiteral("name"), QStringLiteral("list_files")},
        {QStringLiteral("description"), QStringLiteral("List regular files in the server root directory.")},
        {QStringLiteral("inputSchema"),
         QJsonObject {
             {QStringLiteral("type"), QStringLiteral("object")},
             {QStringLiteral("properties"), QJsonObject {}},
         }},
    });
    tools.append(QJsonObject {
        {QStringLiteral("name"), QStringLiteral("download_file")},
        {QStringLiteral("description"), QStringLiteral("Read one file from the server root as base64.")},
        {QStringLiteral("inputSchema"),
         QJsonObject {
             {QStringLiteral("type"), QStringLiteral("object")},
             {QStringLiteral("properties"),
              QJsonObject {
                  {QStringLiteral("name"),
                   QJsonObject {{QStringLiteral("type"), QStringLiteral("string")}}},
              }},
             {QStringLiteral("required"), QJsonArray {QStringLiteral("name")}},
         }},
    });
    return JsonRpcMessage::success(id, QJsonObject {{QStringLiteral("tools"), tools}});
}

QJsonObject McpToolHandler::toolsCall(const QJsonValue &id, const QJsonObject &params) const
{
    const QString name = params.value(QStringLiteral("name")).toString();
    const QJsonObject arguments = params.value(QStringLiteral("arguments")).toObject();
    if (name == QStringLiteral("list_files"))
        return listFiles(id);
    if (name == QStringLiteral("download_file"))
        return downloadFile(id, arguments);
    return JsonRpcMessage::error(id, -32601, QStringLiteral("tool not found"));
}

QJsonObject McpToolHandler::listFiles(const QJsonValue &id) const
{
    QString error;
    const QJsonArray files = m_repository->listFiles(&error);
    if (!error.isEmpty())
        return JsonRpcMessage::error(id, -32000, error);
    return JsonRpcMessage::success(id, QJsonObject {{QStringLiteral("files"), files}});
}

QJsonObject McpToolHandler::downloadFile(const QJsonValue &id, const QJsonObject &arguments) const
{
    const QString name = arguments.value(QStringLiteral("name")).toString();
    QString mimeType;
    QString error;
    const QByteArray bytes = m_repository->readFile(name, &mimeType, &error);
    if (!error.isEmpty())
        return JsonRpcMessage::error(id, error == QStringLiteral("invalid file name") ? -32602 : -32004, error);

    return JsonRpcMessage::success(id,
                                   QJsonObject {
                                       {QStringLiteral("name"), name},
                                       {QStringLiteral("size"), bytes.size()},
                                       {QStringLiteral("mimeType"), mimeType},
                                       {QStringLiteral("dataBase64"),
                                        QString::fromLatin1(bytes.toBase64())},
                                   });
}
