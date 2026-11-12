#include "JsonUtil.h"

QJsonObject pointToJson(const QPointF &point)
{
    return {
        {QStringLiteral("x"), point.x()},
        {QStringLiteral("y"), point.y()},
    };
}

QPointF pointFromJson(const QJsonObject &object)
{
    return {
        object.value(QStringLiteral("x")).toDouble(),
        object.value(QStringLiteral("y")).toDouble(),
    };
}

QJsonObject rectToJson(const QRectF &rect)
{
    return {
        {QStringLiteral("x"), rect.x()},
        {QStringLiteral("y"), rect.y()},
        {QStringLiteral("width"), rect.width()},
        {QStringLiteral("height"), rect.height()},
    };
}

QRectF rectFromJson(const QJsonObject &object)
{
    return {
        object.value(QStringLiteral("x")).toDouble(),
        object.value(QStringLiteral("y")).toDouble(),
        object.value(QStringLiteral("width")).toDouble(),
        object.value(QStringLiteral("height")).toDouble(),
    };
}
