#pragma once

#include <QJsonObject>
#include <QPointF>
#include <QRectF>

QJsonObject pointToJson(const QPointF &point);
QPointF pointFromJson(const QJsonObject &object);
QJsonObject rectToJson(const QRectF &rect);
QRectF rectFromJson(const QJsonObject &object);
