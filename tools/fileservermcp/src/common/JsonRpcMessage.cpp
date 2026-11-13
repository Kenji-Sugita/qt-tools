#include "JsonRpcMessage.h"

#include <QJsonDocument>

namespace JsonRpcMessage {

QJsonObject success(const QJsonValue &id, const QJsonObject &result)
{
    return QJsonObject {
        {QStringLiteral("jsonrpc"), QStringLiteral("2.0")},
        {QStringLiteral("id"), id},
        {QStringLiteral("result"), result},
    };
}

QJsonObject error(const QJsonValue &id, int code, const QString &message)
{
    return QJsonObject {
        {QStringLiteral("jsonrpc"), QStringLiteral("2.0")},
        {QStringLiteral("id"), id},
        {QStringLiteral("error"),
         QJsonObject {
             {QStringLiteral("code"), code},
             {QStringLiteral("message"), message},
         }},
    };
}

QString compactJson(const QJsonObject &object)
{
    return QString::fromUtf8(QJsonDocument(object).toJson(QJsonDocument::Compact));
}

} // namespace JsonRpcMessage
