#pragma once

#include <QJsonObject>

class FileRepository;

class McpToolHandler
{
public:
    explicit McpToolHandler(FileRepository *repository);

    QJsonObject handleRequest(const QJsonObject &request) const;

private:
    QJsonObject initialize(const QJsonValue &id) const;
    QJsonObject toolsList(const QJsonValue &id) const;
    QJsonObject toolsCall(const QJsonValue &id, const QJsonObject &params) const;
    QJsonObject listFiles(const QJsonValue &id) const;
    QJsonObject downloadFile(const QJsonValue &id, const QJsonObject &arguments) const;

    FileRepository *m_repository = nullptr;
};
