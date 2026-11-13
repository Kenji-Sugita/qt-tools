#pragma once

#include <QJsonObject>
#include <QString>

namespace JsonRpcMessage {

QJsonObject success(const QJsonValue &id, const QJsonObject &result);
QJsonObject error(const QJsonValue &id, int code, const QString &message);
QString compactJson(const QJsonObject &object);

} // namespace JsonRpcMessage
