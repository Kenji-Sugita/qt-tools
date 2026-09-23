#include "WhiteboardMcpController.h"

#include "BoardModel.h"
#include "CanvasWidget.h"
#include "DrawingElement.h"
#include "ImageExporter.h"

#include <QBuffer>
#include <QColor>
#include <QFileInfo>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QSet>

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

#ifndef WHITEBOARD_APP_VERSION
#define WHITEBOARD_APP_VERSION "0.0.0"
#endif

namespace {
constexpr int MaxMcpElements = 500;
constexpr int MaxMcpPointsPerElement = 1000;
constexpr int MaxMcpTextLength = 10000;
constexpr int JsonRpcParseError = -32700;
constexpr int JsonRpcInvalidRequest = -32600;
constexpr int JsonRpcMethodNotFound = -32601;
constexpr int JsonRpcInvalidParams = -32602;
constexpr int JsonRpcInternalError = -32603;

bool isSupportedProtocolVersion(const QString &version)
{
    return version == QStringLiteral("2024-11-05")
        || version == QStringLiteral("2025-03-26")
        || version == QStringLiteral("2025-06-18");
}

bool isFiniteNumber(const QJsonValue &value)
{
    return value.isDouble() && std::isfinite(value.toDouble());
}

bool readFiniteNumber(const QJsonObject &object, const QString &key, qreal *result)
{
    const QJsonValue value = object.value(key);
    if (!isFiniteNumber(value))
        return false;
    *result = value.toDouble();
    return true;
}

bool readOptionalInteger(const QJsonObject &object, const QString &key, int defaultValue, int *result)
{
    if (!object.contains(key)) {
        *result = defaultValue;
        return true;
    }

    const QJsonValue value = object.value(key);
    if (!value.isDouble() || !std::isfinite(value.toDouble())
        || std::floor(value.toDouble()) != value.toDouble()
        || value.toDouble() < std::numeric_limits<int>::min()
        || value.toDouble() > std::numeric_limits<int>::max()) {
        return false;
    }

    *result = value.toInt();
    return true;
}

bool readRequiredRevision(const QJsonObject &object, qint64 *revision)
{
    const QJsonValue value = object.value(QStringLiteral("expectedRevision"));
    if (!value.isDouble() || !std::isfinite(value.toDouble())
        || std::floor(value.toDouble()) != value.toDouble() || value.toDouble() < 0.0
        || value.toDouble() > 9007199254740991.0) {
        return false;
    }
    *revision = value.toInteger();
    return true;
}

bool readPoint(const QJsonValue &value, QPointF *point)
{
    if (!value.isObject())
        return false;
    const QJsonObject object = value.toObject();
    qreal x = 0.0;
    qreal y = 0.0;
    if (!readFiniteNumber(object, QStringLiteral("x"), &x)
        || !readFiniteNumber(object, QStringLiteral("y"), &y)) {
        return false;
    }
    *point = QPointF(x, y);
    return true;
}

bool readRect(const QJsonValue &value, QRectF *rect)
{
    if (!value.isObject())
        return false;
    const QJsonObject object = value.toObject();
    qreal x = 0.0;
    qreal y = 0.0;
    qreal width = 0.0;
    qreal height = 0.0;
    if (!readFiniteNumber(object, QStringLiteral("x"), &x)
        || !readFiniteNumber(object, QStringLiteral("y"), &y)
        || !readFiniteNumber(object, QStringLiteral("width"), &width)
        || !readFiniteNumber(object, QStringLiteral("height"), &height)
        || width <= 0.0 || height <= 0.0) {
        return false;
    }
    *rect = QRectF(x, y, width, height);
    return true;
}

bool parseElementType(const QString &value, ElementType *type)
{
    if (value == QStringLiteral("line"))
        *type = ElementType::Line;
    else if (value == QStringLiteral("polyline"))
        *type = ElementType::Polyline;
    else if (value == QStringLiteral("bezier"))
        *type = ElementType::Bezier;
    else if (value == QStringLiteral("rectangle"))
        *type = ElementType::Rectangle;
    else if (value == QStringLiteral("roundedRectangle"))
        *type = ElementType::RoundedRectangle;
    else if (value == QStringLiteral("ellipse"))
        *type = ElementType::Ellipse;
    else if (value == QStringLiteral("circle"))
        *type = ElementType::Circle;
    else if (value == QStringLiteral("arc"))
        *type = ElementType::Arc;
    else if (value == QStringLiteral("text"))
        *type = ElementType::Text;
    else
        return false;
    return true;
}

bool isSupportedConnectorTarget(const DrawingElement &element)
{
    return element.type() == ElementType::Rectangle
        || element.type() == ElementType::RoundedRectangle
        || element.type() == ElementType::Ellipse
        || element.type() == ElementType::Circle
        || element.type() == ElementType::Text
        || element.type() == ElementType::Image;
}

bool parseStrokeStyle(const QString &value, StrokeStyle *style)
{
    if (value == QStringLiteral("solid"))
        *style = StrokeStyle::Solid;
    else if (value == QStringLiteral("dotted"))
        *style = StrokeStyle::Dotted;
    else
        return false;
    return true;
}

bool parseArrowHead(const QString &value, ArrowHead *arrowHead)
{
    if (value == QStringLiteral("none"))
        *arrowHead = ArrowHead::None;
    else if (value == QStringLiteral("triangle"))
        *arrowHead = ArrowHead::Triangle;
    else if (value == QStringLiteral("open"))
        *arrowHead = ArrowHead::Open;
    else if (value == QStringLiteral("diamond"))
        *arrowHead = ArrowHead::Diamond;
    else
        return false;
    return true;
}

bool readColor(const QJsonObject &object, const QString &key, const QColor &defaultColor, QColor *color)
{
    if (!object.contains(key)) {
        *color = defaultColor;
        return true;
    }
    if (!object.value(key).isString())
        return false;
    const QColor parsed(object.value(key).toString());
    if (!parsed.isValid())
        return false;
    *color = parsed;
    return true;
}

bool parseDrawingElement(const QJsonObject &object, DrawingElement *element, QString *errorMessage)
{
    ElementType type = ElementType::Rectangle;
    if (!parseElementType(object.value(QStringLiteral("type")).toString(), &type)) {
        *errorMessage = QStringLiteral("type must be one of line, polyline, bezier, rectangle, roundedRectangle, ellipse, circle, arc, or text.");
        return false;
    }

    DrawingElement parsed;
    parsed.setType(type);

    const QJsonValue idValue = object.value(QStringLiteral("id"));
    if (!idValue.isUndefined()) {
        if (!idValue.isString()) {
            *errorMessage = QStringLiteral("id must be a string.");
            return false;
        }
        const QString id = idValue.toString().trimmed();
        if (id.isEmpty() || id.size() > 128) {
            *errorMessage = QStringLiteral("id must be between 1 and 128 characters.");
            return false;
        }
        parsed.setId(id);
    }

    const bool usesPoints = type == ElementType::Line || type == ElementType::Polyline || type == ElementType::Bezier;
    if (usesPoints) {
        const QJsonValue pointsValue = object.value(QStringLiteral("points"));
        if (!pointsValue.isArray()) {
            *errorMessage = QStringLiteral("points must be an array for point-based elements.");
            return false;
        }
        const QJsonArray pointArray = pointsValue.toArray();
        const QJsonValue closedValue = object.value(QStringLiteral("closed"));
        if (!closedValue.isUndefined() && !closedValue.isBool()) {
            *errorMessage = QStringLiteral("closed must be a boolean.");
            return false;
        }
        const bool closed = closedValue.toBool(false);
        const int minimumPoints = closed ? 3 : 2;
        if (pointArray.size() < minimumPoints || pointArray.size() > MaxMcpPointsPerElement) {
            *errorMessage = QStringLiteral("points has an invalid number of entries.");
            return false;
        }
        if (type == ElementType::Line && pointArray.size() != 2) {
            *errorMessage = QStringLiteral("line requires exactly two points.");
            return false;
        }
        QVector<QPointF> points;
        points.reserve(pointArray.size());
        for (const QJsonValue &pointValue : pointArray) {
            QPointF point;
            if (!readPoint(pointValue, &point)) {
                *errorMessage = QStringLiteral("each point requires finite x and y values.");
                return false;
            }
            points.append(point);
        }
        parsed.setPoints(points);
        parsed.setClosed(closed);
    } else {
        QRectF rect;
        if (!readRect(object.value(QStringLiteral("rect")), &rect)) {
            *errorMessage = QStringLiteral("rect requires finite x, y, width, and height values with positive size.");
            return false;
        }
        if (type == ElementType::Circle && !qFuzzyCompare(rect.width(), rect.height())) {
            *errorMessage = QStringLiteral("circle requires equal width and height.");
            return false;
        }
        parsed.setRect(rect);
    }

    if (type == ElementType::Text) {
        const QString text = object.value(QStringLiteral("text")).toString();
        if (text.trimmed().isEmpty() || text.size() > MaxMcpTextLength) {
            *errorMessage = QStringLiteral("text must be non-empty and no longer than 10000 characters.");
            return false;
        }
        parsed.setText(text);
    }

    QColor color;
    QColor fillColor;
    if (!readColor(object, QStringLiteral("color"), BoardModel::DefaultColor, &color)
        || !readColor(object, QStringLiteral("fillColor"), BoardModel::DefaultFillColor, &fillColor)) {
        *errorMessage = QStringLiteral("color and fillColor must be valid color strings.");
        return false;
    }
    parsed.setColor(color);
    parsed.setFillColor(fillColor);

    int strokeWidth = 0;
    int cornerRadius = 0;
    int fontPointSize = 0;
    int groupId = 0;
    if (!readOptionalInteger(object, QStringLiteral("strokeWidth"), BoardModel::DefaultStrokeWidth, &strokeWidth)
        || !readOptionalInteger(object, QStringLiteral("cornerRadius"), BoardModel::DefaultCornerRadius, &cornerRadius)
        || !readOptionalInteger(object, QStringLiteral("fontPointSize"), BoardModel::DefaultFont.pointSize(), &fontPointSize)
        || !readOptionalInteger(object, QStringLiteral("groupId"), 0, &groupId)
        || strokeWidth < 0 || strokeWidth > 20 || cornerRadius < 0 || cornerRadius > 500
        || fontPointSize < 6 || fontPointSize > 144 || groupId < 0 || groupId > 1000000) {
        *errorMessage = QStringLiteral("style numeric values must be integers within the supported range.");
        return false;
    }
    parsed.setStrokeWidth(strokeWidth);
    parsed.setCornerRadius(cornerRadius);
    parsed.setGroupId(groupId);

    StrokeStyle strokeStyle = BoardModel::DefaultStrokeStyle;
    if (object.contains(QStringLiteral("strokeStyle"))
        && !parseStrokeStyle(object.value(QStringLiteral("strokeStyle")).toString(), &strokeStyle)) {
        *errorMessage = QStringLiteral("strokeStyle must be solid or dotted.");
        return false;
    }
    parsed.setStrokeStyle(strokeStyle);

    ArrowHead startArrowHead = BoardModel::DefaultStartArrowHead;
    ArrowHead endArrowHead = BoardModel::DefaultEndArrowHead;
    if ((object.contains(QStringLiteral("startArrowHead"))
         && !parseArrowHead(object.value(QStringLiteral("startArrowHead")).toString(), &startArrowHead))
        || (object.contains(QStringLiteral("endArrowHead"))
            && !parseArrowHead(object.value(QStringLiteral("endArrowHead")).toString(), &endArrowHead))) {
        *errorMessage = QStringLiteral("arrow head must be none, triangle, open, or diamond.");
        return false;
    }
    parsed.setStartArrowHead(startArrowHead);
    parsed.setEndArrowHead(endArrowHead);

    const QJsonValue startConnectionValue = object.value(QStringLiteral("startConnectionElementId"));
    const QJsonValue endConnectionValue = object.value(QStringLiteral("endConnectionElementId"));
    if ((!startConnectionValue.isUndefined() || !endConnectionValue.isUndefined()) && type != ElementType::Line) {
        *errorMessage = QStringLiteral("connection element ids are supported only for line elements.");
        return false;
    }
    if (!startConnectionValue.isUndefined()) {
        if (!startConnectionValue.isString()) {
            *errorMessage = QStringLiteral("startConnectionElementId must be a string.");
            return false;
        }
        const QString id = startConnectionValue.toString().trimmed();
        if (id.size() > 128) {
            *errorMessage = QStringLiteral("startConnectionElementId must be no longer than 128 characters.");
            return false;
        }
        parsed.setStartConnectionElementId(id);
    }
    if (!endConnectionValue.isUndefined()) {
        if (!endConnectionValue.isString()) {
            *errorMessage = QStringLiteral("endConnectionElementId must be a string.");
            return false;
        }
        const QString id = endConnectionValue.toString().trimmed();
        if (id.size() > 128) {
            *errorMessage = QStringLiteral("endConnectionElementId must be no longer than 128 characters.");
            return false;
        }
        parsed.setEndConnectionElementId(id);
    }

    const QJsonValue fontFamilyValue = object.value(QStringLiteral("fontFamily"));
    if (!fontFamilyValue.isUndefined() && !fontFamilyValue.isString()) {
        *errorMessage = QStringLiteral("fontFamily must be a string.");
        return false;
    }
    const QString fontFamily = fontFamilyValue.toString(BoardModel::DefaultFont.family()).trimmed();
    if (fontFamily.isEmpty() || fontFamily.size() > 100) {
        *errorMessage = QStringLiteral("fontFamily must be between 1 and 100 characters.");
        return false;
    }
    parsed.setFont(QFont(fontFamily, fontPointSize));

    const QJsonValue rotationValue = object.value(QStringLiteral("rotationDegrees"));
    const qreal rotationDegrees = rotationValue.isUndefined() ? 0.0 : rotationValue.toDouble(std::numeric_limits<qreal>::quiet_NaN());
    if (!std::isfinite(rotationDegrees)) {
        *errorMessage = QStringLiteral("rotationDegrees must be finite.");
        return false;
    }
    parsed.setRotationDegrees(rotationDegrees);

    if (type == ElementType::Arc) {
        int startAngle = 0;
        int spanAngle = 0;
        if (!readOptionalInteger(object, QStringLiteral("arcStartAngle"), 0, &startAngle)
            || !readOptionalInteger(object, QStringLiteral("arcSpanAngle"), 120, &spanAngle)
            || startAngle < -3600 || startAngle > 3600 || spanAngle < -360
            || spanAngle > 360 || spanAngle == 0) {
            *errorMessage = QStringLiteral("arc angles are outside the supported range.");
            return false;
        }
        parsed.setArcStartAngle(startAngle);
        parsed.setArcSpanAngle(spanAngle);
    }

    const QRectF canvasRect(0.0, 0.0, BoardModel::CanvasWidth, BoardModel::CanvasHeight);
    if (!canvasRect.contains(parsed.visualBounds())) {
        *errorMessage = QStringLiteral("element must fit entirely inside the 1920 x 1080 canvas.");
        return false;
    }

    *element = parsed;
    return true;
}

QJsonObject pointSchema()
{
    return {
        {QStringLiteral("type"), QStringLiteral("object")},
        {QStringLiteral("properties"), QJsonObject{
             {QStringLiteral("x"), QJsonObject{{QStringLiteral("type"), QStringLiteral("number")}}},
             {QStringLiteral("y"), QJsonObject{{QStringLiteral("type"), QStringLiteral("number")}}},
         }},
        {QStringLiteral("required"), QJsonArray{QStringLiteral("x"), QStringLiteral("y")}},
    };
}

QJsonObject rectSchema()
{
    return {
        {QStringLiteral("type"), QStringLiteral("object")},
        {QStringLiteral("properties"), QJsonObject{
             {QStringLiteral("x"), QJsonObject{{QStringLiteral("type"), QStringLiteral("number")}}},
             {QStringLiteral("y"), QJsonObject{{QStringLiteral("type"), QStringLiteral("number")}}},
             {QStringLiteral("width"), QJsonObject{{QStringLiteral("type"), QStringLiteral("number")}, {QStringLiteral("exclusiveMinimum"), 0}}},
             {QStringLiteral("height"), QJsonObject{{QStringLiteral("type"), QStringLiteral("number")}, {QStringLiteral("exclusiveMinimum"), 0}}},
         }},
        {QStringLiteral("required"), QJsonArray{QStringLiteral("x"), QStringLiteral("y"), QStringLiteral("width"), QStringLiteral("height")}},
    };
}

QJsonObject elementSchema()
{
    return {
        {QStringLiteral("type"), QStringLiteral("object")},
        {QStringLiteral("properties"), QJsonObject{
             {QStringLiteral("id"), QJsonObject{{QStringLiteral("type"), QStringLiteral("string")}}},
             {QStringLiteral("type"), QJsonObject{{QStringLiteral("type"), QStringLiteral("string")},
                                                   {QStringLiteral("enum"), QJsonArray{QStringLiteral("line"), QStringLiteral("polyline"), QStringLiteral("bezier"), QStringLiteral("rectangle"), QStringLiteral("roundedRectangle"), QStringLiteral("ellipse"), QStringLiteral("circle"), QStringLiteral("arc"), QStringLiteral("text")}}}},
             {QStringLiteral("rect"), rectSchema()},
             {QStringLiteral("points"), QJsonObject{{QStringLiteral("type"), QStringLiteral("array")}, {QStringLiteral("items"), pointSchema()}}},
             {QStringLiteral("text"), QJsonObject{{QStringLiteral("type"), QStringLiteral("string")}}},
             {QStringLiteral("color"), QJsonObject{{QStringLiteral("type"), QStringLiteral("string")}}},
             {QStringLiteral("fillColor"), QJsonObject{{QStringLiteral("type"), QStringLiteral("string")}}},
             {QStringLiteral("strokeWidth"), QJsonObject{{QStringLiteral("type"), QStringLiteral("integer")}, {QStringLiteral("minimum"), 0}, {QStringLiteral("maximum"), 20}}},
             {QStringLiteral("strokeStyle"), QJsonObject{{QStringLiteral("type"), QStringLiteral("string")}, {QStringLiteral("enum"), QJsonArray{QStringLiteral("solid"), QStringLiteral("dotted")}}}},
             {QStringLiteral("cornerRadius"), QJsonObject{{QStringLiteral("type"), QStringLiteral("integer")}}},
             {QStringLiteral("fontFamily"), QJsonObject{{QStringLiteral("type"), QStringLiteral("string")}}},
             {QStringLiteral("fontPointSize"), QJsonObject{{QStringLiteral("type"), QStringLiteral("integer")}}},
             {QStringLiteral("closed"), QJsonObject{{QStringLiteral("type"), QStringLiteral("boolean")}}},
             {QStringLiteral("startArrowHead"), QJsonObject{{QStringLiteral("type"), QStringLiteral("string")}, {QStringLiteral("enum"), QJsonArray{QStringLiteral("none"), QStringLiteral("triangle"), QStringLiteral("open"), QStringLiteral("diamond")}}}},
             {QStringLiteral("endArrowHead"), QJsonObject{{QStringLiteral("type"), QStringLiteral("string")}, {QStringLiteral("enum"), QJsonArray{QStringLiteral("none"), QStringLiteral("triangle"), QStringLiteral("open"), QStringLiteral("diamond")}}}},
             {QStringLiteral("startConnectionElementId"), QJsonObject{{QStringLiteral("type"), QStringLiteral("string")}}},
             {QStringLiteral("endConnectionElementId"), QJsonObject{{QStringLiteral("type"), QStringLiteral("string")}}},
             {QStringLiteral("arcStartAngle"), QJsonObject{{QStringLiteral("type"), QStringLiteral("integer")}}},
             {QStringLiteral("arcSpanAngle"), QJsonObject{{QStringLiteral("type"), QStringLiteral("integer")}}},
             {QStringLiteral("groupId"), QJsonObject{{QStringLiteral("type"), QStringLiteral("integer")}}},
             {QStringLiteral("rotationDegrees"), QJsonObject{{QStringLiteral("type"), QStringLiteral("number")}}},
         }},
        {QStringLiteral("required"), QJsonArray{QStringLiteral("type")}},
    };
}

QJsonObject elementUpdateSchema()
{
    QJsonObject properties = elementSchema().value(QStringLiteral("properties")).toObject();
    properties.remove(QStringLiteral("type"));
    return {
        {QStringLiteral("type"), QStringLiteral("object")},
        {QStringLiteral("properties"), properties},
        {QStringLiteral("required"), QJsonArray{QStringLiteral("id")}},
    };
}

QJsonObject elementAdditionSchema()
{
    QJsonObject properties = elementSchema().value(QStringLiteral("properties")).toObject();
    properties.remove(QStringLiteral("id"));
    properties.insert(QStringLiteral("clientId"), QJsonObject{{QStringLiteral("type"), QStringLiteral("string")}});
    return {
        {QStringLiteral("type"), QStringLiteral("object")},
        {QStringLiteral("properties"), properties},
        {QStringLiteral("required"), QJsonArray{QStringLiteral("clientId"), QStringLiteral("type")}},
    };
}

bool parseElementUpdate(const QJsonObject &update,
                        const DrawingElement &existing,
                        DrawingElement *result,
                        QString *errorMessage)
{
    static const QSet<QString> allowedKeys{
        QStringLiteral("id"), QStringLiteral("rect"), QStringLiteral("points"),
        QStringLiteral("text"), QStringLiteral("color"), QStringLiteral("fillColor"),
        QStringLiteral("strokeWidth"), QStringLiteral("strokeStyle"), QStringLiteral("cornerRadius"),
        QStringLiteral("fontFamily"), QStringLiteral("fontPointSize"), QStringLiteral("closed"),
        QStringLiteral("startArrowHead"), QStringLiteral("endArrowHead"),
        QStringLiteral("startConnectionElementId"), QStringLiteral("endConnectionElementId"),
        QStringLiteral("arcStartAngle"), QStringLiteral("arcSpanAngle"),
        QStringLiteral("groupId"), QStringLiteral("rotationDegrees")};
    for (auto it = update.constBegin(); it != update.constEnd(); ++it) {
        if (!allowedKeys.contains(it.key())) {
            *errorMessage = QStringLiteral("unsupported property '%1'.").arg(it.key());
            return false;
        }
    }
    if (update.size() <= 1) {
        *errorMessage = QStringLiteral("at least one editable property is required.");
        return false;
    }

    const bool pointBased = existing.type() == ElementType::Line
        || existing.type() == ElementType::Polyline || existing.type() == ElementType::Bezier;
    if ((update.contains(QStringLiteral("points")) && !pointBased)
        || (update.contains(QStringLiteral("rect")) && pointBased)) {
        *errorMessage = pointBased
            ? QStringLiteral("rect cannot update a point-based element.")
            : QStringLiteral("points cannot update a rectangle-based element.");
        return false;
    }
    if (update.contains(QStringLiteral("text")) && existing.type() != ElementType::Text) {
        *errorMessage = QStringLiteral("text can update only a text element.");
        return false;
    }
    if ((update.contains(QStringLiteral("startConnectionElementId"))
         || update.contains(QStringLiteral("endConnectionElementId")))
        && existing.type() != ElementType::Line) {
        *errorMessage = QStringLiteral("connection element ids can update only a line element.");
        return false;
    }

    if (existing.type() == ElementType::Image) {
        static const QSet<QString> imageKeys{
            QStringLiteral("id"), QStringLiteral("rect"), QStringLiteral("rotationDegrees"),
            QStringLiteral("groupId")};
        for (auto it = update.constBegin(); it != update.constEnd(); ++it) {
            if (!imageKeys.contains(it.key())) {
                *errorMessage = QStringLiteral("image elements support only rect, rotationDegrees, and groupId updates.");
                return false;
            }
        }
        DrawingElement parsed = existing;
        if (update.contains(QStringLiteral("rect"))) {
            QRectF rect;
            if (!readRect(update.value(QStringLiteral("rect")), &rect)) {
                *errorMessage = QStringLiteral("rect requires finite x, y, width, and height values with positive size.");
                return false;
            }
            parsed.setRect(rect);
        }
        if (update.contains(QStringLiteral("rotationDegrees"))) {
            const QJsonValue value = update.value(QStringLiteral("rotationDegrees"));
            if (!isFiniteNumber(value)) {
                *errorMessage = QStringLiteral("rotationDegrees must be finite.");
                return false;
            }
            parsed.setRotationDegrees(value.toDouble());
        }
        if (update.contains(QStringLiteral("groupId"))) {
            int groupId = 0;
            if (!readOptionalInteger(update, QStringLiteral("groupId"), 0, &groupId)
                || groupId < 0 || groupId > 1000000) {
                *errorMessage = QStringLiteral("groupId must be an integer between 0 and 1000000.");
                return false;
            }
            parsed.setGroupId(groupId);
        }
        const QRectF canvasRect(0.0, 0.0, BoardModel::CanvasWidth, BoardModel::CanvasHeight);
        if (!canvasRect.contains(parsed.visualBounds())) {
            *errorMessage = QStringLiteral("element must fit entirely inside the 1920 x 1080 canvas.");
            return false;
        }
        *result = parsed;
        return true;
    }

    QJsonObject merged = existing.toJson();
    for (auto it = update.constBegin(); it != update.constEnd(); ++it)
        merged.insert(it.key(), it.value());
    if (existing.type() != ElementType::Line) {
        merged.remove(QStringLiteral("startConnectionElementId"));
        merged.remove(QStringLiteral("endConnectionElementId"));
    }
    DrawingElement parsed;
    if (!parseDrawingElement(merged, &parsed, errorMessage))
        return false;
    parsed.setId(existing.id());
    *result = parsed;
    return true;
}

QJsonObject emptyObjectSchema()
{
    return {{QStringLiteral("type"), QStringLiteral("object")}};
}

QJsonObject imageOptionsSchema(bool includeFilePath)
{
    QJsonObject properties{
        {QStringLiteral("format"), QJsonObject{{QStringLiteral("type"), QStringLiteral("string")}, {QStringLiteral("enum"), QJsonArray{QStringLiteral("png"), QStringLiteral("svg")}}}},
        {QStringLiteral("background"), QJsonObject{{QStringLiteral("type"), QStringLiteral("string")}, {QStringLiteral("enum"), QJsonArray{QStringLiteral("transparent"), QStringLiteral("white")}}}},
        {QStringLiteral("scope"), QJsonObject{{QStringLiteral("type"), QStringLiteral("string")}, {QStringLiteral("enum"), QJsonArray{QStringLiteral("page"), QStringLiteral("selection")}}}},
    };
    QJsonArray required{QStringLiteral("format"), QStringLiteral("background"), QStringLiteral("scope")};
    if (includeFilePath) {
        properties.insert(QStringLiteral("filePath"), QJsonObject{{QStringLiteral("type"), QStringLiteral("string")}, {QStringLiteral("minLength"), 1}});
        properties.insert(QStringLiteral("overwrite"), QJsonObject{{QStringLiteral("type"), QStringLiteral("boolean")}});
        required.append(QStringLiteral("filePath"));
        required.append(QStringLiteral("overwrite"));
    }
    return {
        {QStringLiteral("type"), QStringLiteral("object")},
        {QStringLiteral("properties"), properties},
        {QStringLiteral("required"), required},
    };
}

bool readImageOptions(const QJsonObject &arguments,
                      const BoardModel *model,
                      ImageExporter::Options *options,
                      QVector<int> *selectedIndexes,
                      QString *errorMessage)
{
    const QString format = arguments.value(QStringLiteral("format")).toString();
    if (format == QStringLiteral("png"))
        options->format = ImageExporter::Format::Png;
    else if (format == QStringLiteral("svg"))
        options->format = ImageExporter::Format::Svg;
    else {
        *errorMessage = QStringLiteral("format must be png or svg.");
        return false;
    }

    const QString background = arguments.value(QStringLiteral("background")).toString();
    if (background == QStringLiteral("transparent"))
        options->background = ImageExporter::Background::Transparent;
    else if (background == QStringLiteral("white"))
        options->background = ImageExporter::Background::White;
    else {
        *errorMessage = QStringLiteral("background must be transparent or white.");
        return false;
    }

    const QString scope = arguments.value(QStringLiteral("scope")).toString();
    if (scope == QStringLiteral("page")) {
        selectedIndexes->clear();
    } else if (scope == QStringLiteral("selection")) {
        if (!model->hasSelection()) {
            *errorMessage = QStringLiteral("There is no current selection to export.");
            return false;
        }
        *selectedIndexes = model->selectedElementIndexes();
        const int focusedIndex = model->selectedElementIndex();
        if (focusedIndex >= 0 && !selectedIndexes->contains(focusedIndex))
            selectedIndexes->append(focusedIndex);
    } else {
        *errorMessage = QStringLiteral("scope must be page or selection.");
        return false;
    }

    if (!ImageExporter::hasExportableElements(model->currentPage())) {
        *errorMessage = QStringLiteral("The current page has no exportable elements.");
        return false;
    }
    return true;
}
}

WhiteboardMcpController::WhiteboardMcpController(BoardModel *model,
                                                 CanvasWidget *canvas,
                                                 ChangeCallback changeCallback,
                                                 PageNavigationCallback pageNavigationCallback,
                                                 ApprovalCallback approvalCallback,
                                                 QObject *parent)
    : QObject(parent)
    , m_model(model)
    , m_canvas(canvas)
    , m_changeCallback(std::move(changeCallback))
    , m_pageNavigationCallback(std::move(pageNavigationCallback))
    , m_approvalCallback(std::move(approvalCallback))
{
}

WhiteboardMcpController::~WhiteboardMcpController() = default;

void WhiteboardMcpController::setInputDevice(QIODevice *device)
{
    m_inputDevice = device;
}

void WhiteboardMcpController::setOutputDevice(QIODevice *device)
{
    m_outputDevice = device;
}

bool WhiteboardMcpController::start()
{
    if (!startMessageTransport())
        return false;
    if (!m_inputDevice || !m_outputDevice || !m_inputDevice->isReadable()
        || !m_outputDevice->isWritable()) {
        return false;
    }
    connect(m_inputDevice, &QIODevice::readyRead, this, [this]() {
        processInputDeviceMessages();
    });
    processInputDeviceMessages();
    return true;
}

bool WhiteboardMcpController::startMessageTransport()
{
    if (!registerTools())
        return false;
    m_started = true;
    return true;
}

bool WhiteboardMcpController::processMessage(const QByteArray &message,
                                             QJsonObject *response,
                                             bool *hasResponse)
{
    if (response)
        *response = {};
    if (hasResponse)
        *hasResponse = false;
    if (!m_started && !startMessageTransport())
        return false;

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(message, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (response)
            *response = makeErrorResponse(QJsonValue(), JsonRpcParseError, QStringLiteral("Parse error."));
        if (hasResponse)
            *hasResponse = true;
        return true;
    }

    QJsonObject request = document.object();
    return processRequest(request, response, hasResponse);
}

QString WhiteboardMcpController::negotiatedProtocolVersion() const
{
    return m_negotiatedProtocolVersion;
}

void WhiteboardMcpController::stop()
{
    m_started = false;
    m_inputBuffer.clear();
}

bool WhiteboardMcpController::registerTool(const QString &name,
                                           const QString &description,
                                           ToolCallback callback,
                                           const QJsonObject &inputSchema,
                                           bool requiresApproval)
{
    if (name.isEmpty() || !callback || m_toolIndexes.contains(name))
        return false;
    m_toolIndexes.insert(name, m_tools.size());
    m_tools.append(ToolDefinition{name, description, inputSchema, requiresApproval, std::move(callback)});
    return true;
}

bool WhiteboardMcpController::registerTools()
{
    if (m_toolsRegistered)
        return true;

    const bool stateRegistered = registerTool(
        QStringLiteral("whiteboard/state"),
        QStringLiteral("Return the 1920 x 1080 canvas, current page, lock, history, and element JSON state."),
        [this](const QJsonObject &arguments, QString *errorMessage) {
            return stateTool(arguments, errorMessage);
        },
        emptyObjectSchema());

    const bool elementsListRegistered = registerTool(
        QStringLiteral("whiteboard/elements/list"),
        QStringLiteral("List drawing elements and persistent IDs on the current or specified page without changing the display."),
        [this](const QJsonObject &arguments, QString *errorMessage) {
            return listElementsTool(arguments, errorMessage);
        },
        QJsonObject{
            {QStringLiteral("type"), QStringLiteral("object")},
            {QStringLiteral("properties"), QJsonObject{
                 {QStringLiteral("pageIndex"), QJsonObject{{QStringLiteral("type"), QStringLiteral("integer")}, {QStringLiteral("minimum"), 0}}},
             }},
        });

    const bool elementsUpdateRegistered = registerTool(
        QStringLiteral("whiteboard/elements/update"),
        QStringLiteral("Atomically update existing current-page elements by persistent ID. One call is one undo step."),
        [this](const QJsonObject &arguments, QString *errorMessage) {
            return updateElementsTool(arguments, errorMessage);
        },
        QJsonObject{
            {QStringLiteral("type"), QStringLiteral("object")},
            {QStringLiteral("properties"), QJsonObject{
                 {QStringLiteral("expectedRevision"), QJsonObject{{QStringLiteral("type"), QStringLiteral("integer")}, {QStringLiteral("minimum"), 0}}},
                 {QStringLiteral("updates"), QJsonObject{{QStringLiteral("type"), QStringLiteral("array")}, {QStringLiteral("minItems"), 1}, {QStringLiteral("maxItems"), MaxMcpElements}, {QStringLiteral("items"), elementUpdateSchema()}}},
             }},
            {QStringLiteral("required"), QJsonArray{QStringLiteral("expectedRevision"), QStringLiteral("updates")}},
        },
        true);

    const bool elementsDeleteRegistered = registerTool(
        QStringLiteral("whiteboard/elements/delete"),
        QStringLiteral("Atomically delete existing current-page elements by persistent ID. One call is one undo step."),
        [this](const QJsonObject &arguments, QString *errorMessage) {
            return deleteElementsTool(arguments, errorMessage);
        },
        QJsonObject{
            {QStringLiteral("type"), QStringLiteral("object")},
            {QStringLiteral("properties"), QJsonObject{
                 {QStringLiteral("expectedRevision"), QJsonObject{{QStringLiteral("type"), QStringLiteral("integer")}, {QStringLiteral("minimum"), 0}}},
                 {QStringLiteral("ids"), QJsonObject{{QStringLiteral("type"), QStringLiteral("array")}, {QStringLiteral("minItems"), 1}, {QStringLiteral("maxItems"), MaxMcpElements}, {QStringLiteral("uniqueItems"), true}, {QStringLiteral("items"), QJsonObject{{QStringLiteral("type"), QStringLiteral("string")}}}}},
             }},
            {QStringLiteral("required"), QJsonArray{QStringLiteral("expectedRevision"), QStringLiteral("ids")}},
        },
        true);

    const bool elementsApplyRegistered = registerTool(
        QStringLiteral("whiteboard/elements/apply"),
        QStringLiteral("Atomically add, update, and delete current-page elements after checking the document revision. One call is one undo step."),
        [this](const QJsonObject &arguments, QString *errorMessage) {
            return applyElementChangesTool(arguments, errorMessage);
        },
        QJsonObject{
            {QStringLiteral("type"), QStringLiteral("object")},
            {QStringLiteral("properties"), QJsonObject{
                 {QStringLiteral("expectedRevision"), QJsonObject{{QStringLiteral("type"), QStringLiteral("integer")}, {QStringLiteral("minimum"), 0}}},
                 {QStringLiteral("add"), QJsonObject{{QStringLiteral("type"), QStringLiteral("array")}, {QStringLiteral("maxItems"), MaxMcpElements}, {QStringLiteral("items"), elementAdditionSchema()}}},
                 {QStringLiteral("update"), QJsonObject{{QStringLiteral("type"), QStringLiteral("array")}, {QStringLiteral("maxItems"), MaxMcpElements}, {QStringLiteral("items"), elementUpdateSchema()}}},
                 {QStringLiteral("delete"), QJsonObject{{QStringLiteral("type"), QStringLiteral("array")}, {QStringLiteral("maxItems"), MaxMcpElements}, {QStringLiteral("uniqueItems"), true}, {QStringLiteral("items"), QJsonObject{{QStringLiteral("type"), QStringLiteral("string")}}}}},
             }},
            {QStringLiteral("required"), QJsonArray{QStringLiteral("expectedRevision")}},
        },
        true);

    const bool pagesRegistered = registerTool(
        QStringLiteral("whiteboard/pages/list"),
        QStringLiteral("List page indexes, element counts, lock states, and the current page without changing the display."),
        [this](const QJsonObject &arguments, QString *errorMessage) {
            return listPagesTool(arguments, errorMessage);
        },
        emptyObjectSchema());

    const bool navigateRegistered = registerTool(
        QStringLiteral("whiteboard/page/navigate"),
        QStringLiteral("Display the previous or next existing page without changing page contents."),
        [this](const QJsonObject &arguments, QString *errorMessage) {
            return navigatePageTool(arguments, errorMessage);
        },
        QJsonObject{
            {QStringLiteral("type"), QStringLiteral("object")},
            {QStringLiteral("properties"), QJsonObject{
                 {QStringLiteral("direction"), QJsonObject{{QStringLiteral("type"), QStringLiteral("string")}, {QStringLiteral("enum"), QJsonArray{QStringLiteral("previous"), QStringLiteral("next")}}}},
             }},
            {QStringLiteral("required"), QJsonArray{QStringLiteral("direction")}},
        });

    const bool addPageRegistered = registerTool(
        QStringLiteral("whiteboard/page/add"),
        QStringLiteral("Add an empty page immediately after the current page and display it."),
        [this](const QJsonObject &arguments, QString *errorMessage) {
            return addPageTool(arguments, errorMessage);
        },
        emptyObjectSchema(),
        true);

    const bool deletePageRegistered = registerTool(
        QStringLiteral("whiteboard/page/delete"),
        QStringLiteral("Delete the current page when it is unlocked and at least one other page exists."),
        [this](const QJsonObject &arguments, QString *errorMessage) {
            return deletePageTool(arguments, errorMessage);
        },
        emptyObjectSchema(),
        true);

    const bool lockPageRegistered = registerTool(
        QStringLiteral("whiteboard/page/lock"),
        QStringLiteral("Set the current page lock state."),
        [this](const QJsonObject &arguments, QString *errorMessage) {
            return lockPageTool(arguments, errorMessage);
        },
        QJsonObject{
            {QStringLiteral("type"), QStringLiteral("object")},
            {QStringLiteral("properties"), QJsonObject{
                 {QStringLiteral("locked"), QJsonObject{{QStringLiteral("type"), QStringLiteral("boolean")}}},
             }},
            {QStringLiteral("required"), QJsonArray{QStringLiteral("locked")}},
        },
        true);

    const bool applyRegistered = registerTool(
        QStringLiteral("whiteboard/diagram/apply"),
        QStringLiteral("Atomically add, replace, or create a page with validated whiteboard elements. One call is one undo step."),
        [this](const QJsonObject &arguments, QString *errorMessage) {
            return applyDiagramTool(arguments, errorMessage);
        },
        QJsonObject{
            {QStringLiteral("type"), QStringLiteral("object")},
            {QStringLiteral("properties"), QJsonObject{
                 {QStringLiteral("mode"), QJsonObject{{QStringLiteral("type"), QStringLiteral("string")}, {QStringLiteral("enum"), QJsonArray{QStringLiteral("new_page"), QStringLiteral("append_current"), QStringLiteral("replace_current")}}}},
                 {QStringLiteral("elements"), QJsonObject{{QStringLiteral("type"), QStringLiteral("array")}, {QStringLiteral("minItems"), 1}, {QStringLiteral("maxItems"), MaxMcpElements}, {QStringLiteral("items"), elementSchema()}}},
             }},
            {QStringLiteral("required"), QJsonArray{QStringLiteral("mode"), QStringLiteral("elements")}},
        },
        true);

    const bool renderRegistered = registerTool(
        QStringLiteral("whiteboard/page/render"),
        QStringLiteral("Render the current page as a full 1920 x 1080 PNG image for visual inspection."),
        [this](const QJsonObject &arguments, QString *errorMessage) {
            return renderPageTool(arguments, errorMessage);
        },
        emptyObjectSchema());

    const bool exportImageRegistered = registerTool(
        QStringLiteral("whiteboard/image/export"),
        QStringLiteral("Return current-page or selected elements as cropped PNG or SVG image data."),
        [this](const QJsonObject &arguments, QString *errorMessage) {
            return exportImageTool(arguments, errorMessage);
        },
        imageOptionsSchema(false));

    const bool saveImageRegistered = registerTool(
        QStringLiteral("whiteboard/image/save"),
        QStringLiteral("Save current-page or selected elements to an absolute PNG or SVG file path."),
        [this](const QJsonObject &arguments, QString *errorMessage) {
            return saveImageTool(arguments, errorMessage);
        },
        imageOptionsSchema(true),
        true);

    const bool undoRegistered = registerTool(
        QStringLiteral("whiteboard/history/undo"),
        QStringLiteral("Undo the most recent whiteboard change."),
        [this](const QJsonObject &arguments, QString *errorMessage) {
            return undoTool(arguments, errorMessage);
        },
        emptyObjectSchema(),
        true);

    const bool redoRegistered = registerTool(
        QStringLiteral("whiteboard/history/redo"),
        QStringLiteral("Redo the most recently undone whiteboard change."),
        [this](const QJsonObject &arguments, QString *errorMessage) {
            return redoTool(arguments, errorMessage);
        },
        emptyObjectSchema(),
        true);

    m_toolsRegistered = stateRegistered && elementsListRegistered && elementsUpdateRegistered
        && elementsDeleteRegistered && elementsApplyRegistered && pagesRegistered && navigateRegistered
        && addPageRegistered && deletePageRegistered && lockPageRegistered
        && applyRegistered && renderRegistered && exportImageRegistered
        && saveImageRegistered && undoRegistered && redoRegistered;
    std::sort(m_tools.begin(), m_tools.end(), [](const ToolDefinition &left, const ToolDefinition &right) {
        return left.name < right.name;
    });
    m_toolIndexes.clear();
    for (int index = 0; index < m_tools.size(); ++index)
        m_toolIndexes.insert(m_tools.at(index).name, index);
    return m_toolsRegistered;
}

bool WhiteboardMcpController::processRequest(const QJsonObject &request,
                                             QJsonObject *response,
                                             bool *hasResponse)
{
    const QJsonValue id = request.value(QStringLiteral("id"));
    if (response)
        *response = {};
    if (hasResponse)
        *hasResponse = false;

    const QString jsonrpc = request.value(QStringLiteral("jsonrpc")).toString();
    const QString method = request.value(QStringLiteral("method")).toString();
    if (jsonrpc != QStringLiteral("2.0") || method.isEmpty()) {
        if (id.isUndefined())
            return false;
        if (response)
            *response = makeErrorResponse(id, JsonRpcInvalidRequest, QStringLiteral("Invalid Request."));
        if (hasResponse)
            *hasResponse = true;
        return true;
    }

    const bool notification = id.isUndefined();
    if (notification) {
        if (method == QStringLiteral("notifications/initialized"))
            return true;
        return true;
    }

    const QJsonValue paramsValue = request.value(QStringLiteral("params"));
    const QJsonObject params = paramsValue.isObject() ? paramsValue.toObject() : QJsonObject();

    QString errorMessage;
    QJsonObject result;
    if (method == QStringLiteral("initialize")) {
        result = initializeRequest(params, &errorMessage);
    } else if (method == QStringLiteral("tools/list")) {
        result = toolsListRequest();
    } else if (method == QStringLiteral("tools/call")) {
        result = toolsCallRequest(params, &errorMessage);
    } else {
        if (response)
            *response = makeErrorResponse(id, JsonRpcMethodNotFound, QStringLiteral("Method not found."));
        if (hasResponse)
            *hasResponse = true;
        return true;
    }

    if (response) {
        *response = errorMessage.isEmpty()
            ? makeResultResponse(id, result)
            : makeErrorResponse(id, JsonRpcInvalidParams, errorMessage);
    }
    if (hasResponse)
        *hasResponse = true;
    return true;
}

QJsonObject WhiteboardMcpController::makeResultResponse(const QJsonValue &id, const QJsonObject &result) const
{
    return {
        {QStringLiteral("jsonrpc"), QStringLiteral("2.0")},
        {QStringLiteral("id"), id},
        {QStringLiteral("result"), result},
    };
}

QJsonObject WhiteboardMcpController::makeErrorResponse(const QJsonValue &id,
                                                       int code,
                                                       const QString &message) const
{
    QJsonObject response{
        {QStringLiteral("jsonrpc"), QStringLiteral("2.0")},
        {QStringLiteral("error"), QJsonObject{{QStringLiteral("code"), code},
                                               {QStringLiteral("message"), message}}},
    };
    if (!id.isUndefined())
        response.insert(QStringLiteral("id"), id);
    return response;
}

QJsonObject WhiteboardMcpController::initializeRequest(const QJsonObject &params, QString *errorMessage)
{
    QString requestedVersion = params.value(QStringLiteral("protocolVersion")).toString();
    if (requestedVersion.isEmpty())
        requestedVersion = QStringLiteral("2025-06-18");
    if (!isSupportedProtocolVersion(requestedVersion)) {
        *errorMessage = QStringLiteral("Unsupported MCP protocol version.");
        return {};
    }
    m_negotiatedProtocolVersion = requestedVersion;
    return {
        {QStringLiteral("protocolVersion"), m_negotiatedProtocolVersion},
        {QStringLiteral("capabilities"), QJsonObject{{QStringLiteral("tools"), QJsonObject{}}}},
        {QStringLiteral("serverInfo"), QJsonObject{{QStringLiteral("name"), QStringLiteral("whiteboard-app")},
                                                   {QStringLiteral("version"), QStringLiteral(WHITEBOARD_APP_VERSION)}}},
    };
}

QJsonObject WhiteboardMcpController::toolsListRequest() const
{
    QJsonArray tools;
    for (const ToolDefinition &tool : m_tools) {
        tools.append(QJsonObject{
            {QStringLiteral("name"), tool.name},
            {QStringLiteral("description"), tool.description},
            {QStringLiteral("inputSchema"), tool.inputSchema},
        });
    }
    return {{QStringLiteral("tools"), tools}};
}

QJsonObject WhiteboardMcpController::toolsCallRequest(const QJsonObject &params, QString *errorMessage)
{
    const QString name = params.value(QStringLiteral("name")).toString();
    const QJsonValue argumentsValue = params.value(QStringLiteral("arguments"));
    const QJsonObject arguments = argumentsValue.isObject() ? argumentsValue.toObject() : QJsonObject();
    const auto toolIt = m_toolIndexes.constFind(name);
    if (name.isEmpty() || toolIt == m_toolIndexes.cend()) {
        *errorMessage = QStringLiteral("Unknown whiteboard tool.");
        return {};
    }

    const ToolDefinition &tool = m_tools.at(*toolIt);
    if (tool.requiresApproval) {
        if (!m_approvalCallback) {
            *errorMessage = QStringLiteral("No approval handler is configured.");
            return {};
        }
        QString denyReason;
        if (!m_approvalCallback(tool.name, arguments, &denyReason)) {
            *errorMessage = denyReason.isEmpty()
                ? QStringLiteral("The MCP request was not approved.")
                : denyReason;
            return {};
        }
    }

    return tool.callback(arguments, errorMessage);
}

void WhiteboardMcpController::processInputDeviceMessages()
{
    if (!m_inputDevice || !m_outputDevice)
        return;

    m_inputBuffer.append(m_inputDevice->readAll());
    while (true) {
        const qsizetype newlineIndex = m_inputBuffer.indexOf('\n');
        if (newlineIndex < 0)
            break;
        QByteArray line = m_inputBuffer.left(newlineIndex).trimmed();
        m_inputBuffer.remove(0, newlineIndex + 1);
        if (line.isEmpty())
            continue;

        QJsonObject response;
        bool hasResponse = false;
        processMessage(line, &response, &hasResponse);
        if (!hasResponse)
            continue;
        m_outputDevice->write(QJsonDocument(response).toJson(QJsonDocument::Compact));
        m_outputDevice->write("\n");
    }
}

QJsonObject WhiteboardMcpController::stateTool(const QJsonObject &, QString *errorMessage) const
{
    if (!m_model) {
        *errorMessage = QStringLiteral("Whiteboard model is unavailable.");
        return {};
    }

    QJsonArray elements;
    for (const DrawingElement &element : m_model->currentPage().elements())
        elements.append(element.toJson());

    QVector<int> selectedIndexes = m_model->selectedElementIndexes();
    const int focusedIndex = m_model->selectedElementIndex();
    if (focusedIndex >= 0 && !selectedIndexes.contains(focusedIndex))
        selectedIndexes.append(focusedIndex);
    QJsonArray selection;
    for (int index : std::as_const(selectedIndexes))
        selection.append(index);

    const int pageCount = m_model->document().pages().size();
    const int currentPageIndex = m_model->document().currentPageIndex();

    return {
        {QStringLiteral("canvas"), QJsonObject{{QStringLiteral("width"), BoardModel::CanvasWidth}, {QStringLiteral("height"), BoardModel::CanvasHeight}}},
        {QStringLiteral("pageCount"), pageCount},
        {QStringLiteral("revision"), m_model->document().revision()},
        {QStringLiteral("currentPageIndex"), currentPageIndex},
        {QStringLiteral("currentPageLocked"), m_model->currentPageLocked()},
        {QStringLiteral("canAddPage"), m_model->canAddPage()},
        {QStringLiteral("canDeletePage"), m_model->canDeletePage()},
        {QStringLiteral("canPreviousPage"), currentPageIndex > 0},
        {QStringLiteral("canNextPage"), currentPageIndex + 1 < pageCount},
        {QStringLiteral("canUndo"), m_model->canUndo()},
        {QStringLiteral("canRedo"), m_model->canRedo()},
        {QStringLiteral("selectedElementIndexes"), selection},
        {QStringLiteral("elements"), elements},
    };
}

QJsonObject WhiteboardMcpController::listElementsTool(const QJsonObject &arguments, QString *errorMessage) const
{
    if (!m_model) {
        *errorMessage = QStringLiteral("Whiteboard model is unavailable.");
        return {};
    }
    int pageIndex = m_model->document().currentPageIndex();
    if (arguments.contains(QStringLiteral("pageIndex"))) {
        if (!readOptionalInteger(arguments, QStringLiteral("pageIndex"), pageIndex, &pageIndex)) {
            *errorMessage = QStringLiteral("pageIndex must be an integer.");
            return {};
        }
    }
    const QVector<Page> pages = m_model->document().pages();
    if (pageIndex < 0 || pageIndex >= pages.size()) {
        *errorMessage = QStringLiteral("pageIndex does not identify an existing page.");
        return {};
    }

    QJsonArray elements;
    for (const DrawingElement &element : pages.at(pageIndex).elements()) {
        QJsonObject object = element.toJson();
        if (element.type() == ElementType::Image) {
            object.remove(QStringLiteral("imageData"));
            object.insert(QStringLiteral("hasImageData"), !element.imageData().isEmpty());
        }
        elements.append(object);
    }
    return {
        {QStringLiteral("pageIndex"), pageIndex},
        {QStringLiteral("revision"), m_model->document().revision()},
        {QStringLiteral("current"), pageIndex == m_model->document().currentPageIndex()},
        {QStringLiteral("locked"), pages.at(pageIndex).locked()},
        {QStringLiteral("elementCount"), elements.size()},
        {QStringLiteral("elements"), elements},
    };
}

QJsonObject WhiteboardMcpController::updateElementsTool(const QJsonObject &arguments, QString *errorMessage)
{
    if (!m_model) {
        *errorMessage = QStringLiteral("Whiteboard model is unavailable.");
        return {};
    }
    if (m_model->currentPageLocked()) {
        *errorMessage = QStringLiteral("The current page is locked.");
        return {};
    }
    qint64 expectedRevision = 0;
    if (!readRequiredRevision(arguments, &expectedRevision)) {
        *errorMessage = QStringLiteral("expectedRevision must be a non-negative integer.");
        return {};
    }
    if (expectedRevision != m_model->document().revision()) {
        *errorMessage = QStringLiteral("The whiteboard changed after the elements were read.");
        return {};
    }
    const QJsonValue updatesValue = arguments.value(QStringLiteral("updates"));
    if (!updatesValue.isArray() || updatesValue.toArray().isEmpty()
        || updatesValue.toArray().size() > MaxMcpElements) {
        *errorMessage = QStringLiteral("updates must contain between 1 and 500 entries.");
        return {};
    }

    const QVector<DrawingElement> current = m_model->currentPage().elements();
    QHash<QString, DrawingElement> currentById;
    for (const DrawingElement &element : current)
        currentById.insert(element.id(), element);

    QVector<DrawingElement> replacements;
    QStringList updatedIds;
    QSet<QString> requestedIds;
    const QJsonArray updates = updatesValue.toArray();
    replacements.reserve(updates.size());
    for (qsizetype index = 0; index < updates.size(); ++index) {
        if (!updates.at(index).isObject()) {
            *errorMessage = QStringLiteral("updates[%1] must be an object.").arg(index);
            return {};
        }
        const QJsonObject update = updates.at(index).toObject();
        const QJsonValue idValue = update.value(QStringLiteral("id"));
        if (!idValue.isString() || idValue.toString().trimmed().isEmpty()) {
            *errorMessage = QStringLiteral("updates[%1].id must be a non-empty string.").arg(index);
            return {};
        }
        const QString id = idValue.toString().trimmed();
        if (id.size() > 128 || requestedIds.contains(id)) {
            *errorMessage = requestedIds.contains(id)
                ? QStringLiteral("updates contains duplicate id '%1'.").arg(id)
                : QStringLiteral("updates[%1].id must be no longer than 128 characters.").arg(index);
            return {};
        }
        if (!currentById.contains(id)) {
            *errorMessage = QStringLiteral("No current-page element has id '%1'.").arg(id);
            return {};
        }
        requestedIds.insert(id);
        DrawingElement replacement;
        QString updateError;
        if (!parseElementUpdate(update, currentById.value(id), &replacement, &updateError)) {
            *errorMessage = QStringLiteral("updates[%1]: %2").arg(index).arg(updateError);
            return {};
        }
        replacements.append(replacement);
        updatedIds.append(id);
    }

    for (const DrawingElement &replacement : std::as_const(replacements)) {
        if (replacement.type() != ElementType::Line)
            continue;
        for (const QString &targetId : {replacement.startConnectionElementId(), replacement.endConnectionElementId()}) {
            if (targetId.isEmpty())
                continue;
            if (!currentById.contains(targetId) || !isSupportedConnectorTarget(currentById.value(targetId))) {
                *errorMessage = QStringLiteral("Connector target '%1' is not an existing supported current-page element.").arg(targetId);
                return {};
            }
        }
    }

    if (!m_model->updateElementsById(replacements)) {
        *errorMessage = QStringLiteral("The requested updates do not change the current page.");
        return {};
    }
    if (m_changeCallback)
        m_changeCallback();
    QJsonArray ids;
    for (const QString &id : std::as_const(updatedIds))
        ids.append(id);
    return {
        {QStringLiteral("updatedIds"), ids},
        {QStringLiteral("updatedCount"), ids.size()},
        {QStringLiteral("currentPageIndex"), m_model->document().currentPageIndex()},
        {QStringLiteral("revision"), m_model->document().revision()},
    };
}

QJsonObject WhiteboardMcpController::deleteElementsTool(const QJsonObject &arguments, QString *errorMessage)
{
    if (!m_model) {
        *errorMessage = QStringLiteral("Whiteboard model is unavailable.");
        return {};
    }
    if (m_model->currentPageLocked()) {
        *errorMessage = QStringLiteral("The current page is locked.");
        return {};
    }
    qint64 expectedRevision = 0;
    if (!readRequiredRevision(arguments, &expectedRevision)) {
        *errorMessage = QStringLiteral("expectedRevision must be a non-negative integer.");
        return {};
    }
    if (expectedRevision != m_model->document().revision()) {
        *errorMessage = QStringLiteral("The whiteboard changed after the elements were read.");
        return {};
    }
    const QJsonValue idsValue = arguments.value(QStringLiteral("ids"));
    if (!idsValue.isArray() || idsValue.toArray().isEmpty()
        || idsValue.toArray().size() > MaxMcpElements) {
        *errorMessage = QStringLiteral("ids must contain between 1 and 500 entries.");
        return {};
    }

    QStringList ids;
    QSet<QString> uniqueIds;
    const QJsonArray idArray = idsValue.toArray();
    for (qsizetype index = 0; index < idArray.size(); ++index) {
        if (!idArray.at(index).isString()) {
            *errorMessage = QStringLiteral("ids[%1] must be a string.").arg(index);
            return {};
        }
        const QString id = idArray.at(index).toString().trimmed();
        if (id.isEmpty() || id.size() > 128 || uniqueIds.contains(id)) {
            *errorMessage = uniqueIds.contains(id)
                ? QStringLiteral("ids contains duplicate id '%1'.").arg(id)
                : QStringLiteral("ids[%1] must contain between 1 and 128 characters.").arg(index);
            return {};
        }
        uniqueIds.insert(id);
        ids.append(id);
    }

    QStringList detachedConnectorIds;
    if (!m_model->deleteElementsById(ids, &detachedConnectorIds)) {
        *errorMessage = QStringLiteral("Every id must identify an existing current-page element.");
        return {};
    }
    if (m_changeCallback)
        m_changeCallback();
    QJsonArray deleted;
    for (const QString &id : std::as_const(ids))
        deleted.append(id);
    QJsonArray detached;
    for (const QString &id : std::as_const(detachedConnectorIds))
        detached.append(id);
    return {
        {QStringLiteral("deletedIds"), deleted},
        {QStringLiteral("deletedCount"), deleted.size()},
        {QStringLiteral("detachedConnectorIds"), detached},
        {QStringLiteral("currentPageIndex"), m_model->document().currentPageIndex()},
        {QStringLiteral("revision"), m_model->document().revision()},
    };
}

QJsonObject WhiteboardMcpController::applyElementChangesTool(const QJsonObject &arguments, QString *errorMessage)
{
    if (!m_model) {
        *errorMessage = QStringLiteral("Whiteboard model is unavailable.");
        return {};
    }
    if (m_model->currentPageLocked()) {
        *errorMessage = QStringLiteral("The current page is locked.");
        return {};
    }
    qint64 expectedRevision = 0;
    if (!readRequiredRevision(arguments, &expectedRevision)) {
        *errorMessage = QStringLiteral("expectedRevision must be a non-negative integer.");
        return {};
    }
    if (expectedRevision != m_model->document().revision()) {
        *errorMessage = QStringLiteral("The whiteboard changed after the elements were read.");
        return {};
    }

    const QJsonValue addValue = arguments.value(QStringLiteral("add"));
    const QJsonValue updateValue = arguments.value(QStringLiteral("update"));
    const QJsonValue deleteValue = arguments.value(QStringLiteral("delete"));
    if ((!addValue.isUndefined() && !addValue.isArray())
        || (!updateValue.isUndefined() && !updateValue.isArray())
        || (!deleteValue.isUndefined() && !deleteValue.isArray())) {
        *errorMessage = QStringLiteral("add, update, and delete must be arrays when provided.");
        return {};
    }
    const QJsonArray addArray = addValue.toArray();
    const QJsonArray updateArray = updateValue.toArray();
    const QJsonArray deleteArray = deleteValue.toArray();
    const qsizetype operationCount = addArray.size() + updateArray.size() + deleteArray.size();
    if (operationCount < 1 || operationCount > MaxMcpElements) {
        *errorMessage = QStringLiteral("add, update, and delete must contain between 1 and 500 total entries.");
        return {};
    }

    const QVector<DrawingElement> current = m_model->currentPage().elements();
    QHash<QString, DrawingElement> currentById;
    for (const DrawingElement &element : current)
        currentById.insert(element.id(), element);

    QVector<DrawingElement> additions;
    QHash<QString, DrawingElement> additionsByClientId;
    QStringList clientIds;
    for (qsizetype index = 0; index < addArray.size(); ++index) {
        if (!addArray.at(index).isObject()) {
            *errorMessage = QStringLiteral("add[%1] must be an object.").arg(index);
            return {};
        }
        QJsonObject object = addArray.at(index).toObject();
        const QJsonValue clientIdValue = object.take(QStringLiteral("clientId"));
        if (!clientIdValue.isString()) {
            *errorMessage = QStringLiteral("add[%1].clientId must be a string.").arg(index);
            return {};
        }
        const QString clientId = clientIdValue.toString().trimmed();
        if (clientId.isEmpty() || clientId.size() > 128 || additionsByClientId.contains(clientId)
            || currentById.contains(clientId)) {
            *errorMessage = QStringLiteral("add[%1].clientId must be unique, 1 to 128 characters, and distinct from existing IDs.").arg(index);
            return {};
        }
        object.insert(QStringLiteral("id"), clientId);
        DrawingElement addition;
        QString additionError;
        if (!parseDrawingElement(object, &addition, &additionError)) {
            *errorMessage = QStringLiteral("add[%1]: %2").arg(index).arg(additionError);
            return {};
        }
        additions.append(addition);
        additionsByClientId.insert(clientId, addition);
        clientIds.append(clientId);
    }

    QVector<DrawingElement> replacements;
    QSet<QString> updateIds;
    QStringList updatedIds;
    for (qsizetype index = 0; index < updateArray.size(); ++index) {
        if (!updateArray.at(index).isObject()) {
            *errorMessage = QStringLiteral("update[%1] must be an object.").arg(index);
            return {};
        }
        const QJsonObject update = updateArray.at(index).toObject();
        const QString id = update.value(QStringLiteral("id")).toString().trimmed();
        if (id.isEmpty() || id.size() > 128 || updateIds.contains(id) || !currentById.contains(id)) {
            *errorMessage = QStringLiteral("update[%1].id must identify one unique current-page element.").arg(index);
            return {};
        }
        updateIds.insert(id);
        updatedIds.append(id);
        DrawingElement replacement;
        QString updateError;
        if (!parseElementUpdate(update, currentById.value(id), &replacement, &updateError)) {
            *errorMessage = QStringLiteral("update[%1]: %2").arg(index).arg(updateError);
            return {};
        }
        replacements.append(replacement);
    }

    QStringList deletionIds;
    QSet<QString> deleteIds;
    for (qsizetype index = 0; index < deleteArray.size(); ++index) {
        if (!deleteArray.at(index).isString()) {
            *errorMessage = QStringLiteral("delete[%1] must be a string.").arg(index);
            return {};
        }
        const QString id = deleteArray.at(index).toString().trimmed();
        if (id.isEmpty() || id.size() > 128 || deleteIds.contains(id)
            || updateIds.contains(id) || !currentById.contains(id)) {
            *errorMessage = QStringLiteral("delete[%1] must identify one unique, non-updated current-page element.").arg(index);
            return {};
        }
        deleteIds.insert(id);
        deletionIds.append(id);
    }

    auto connectorTarget = [&](const QString &id) -> const DrawingElement * {
        if (additionsByClientId.contains(id))
            return &additionsByClientId[id];
        if (!deleteIds.contains(id) && currentById.contains(id))
            return &currentById[id];
        return nullptr;
    };
    for (const DrawingElement &element : additions + replacements) {
        if (element.type() != ElementType::Line)
            continue;
        for (const QString &targetId : {element.startConnectionElementId(), element.endConnectionElementId()}) {
            if (targetId.isEmpty())
                continue;
            const DrawingElement *target = connectorTarget(targetId);
            if (!target || !isSupportedConnectorTarget(*target)) {
                *errorMessage = QStringLiteral("Connector target '%1' is not available after the combined change.").arg(targetId);
                return {};
            }
        }
    }

    QHash<QString, QString> assignedIds;
    QStringList detachedConnectorIds;
    if (!m_model->applyElementChanges(additions, replacements, deletionIds,
                                      &assignedIds, &detachedConnectorIds)) {
        *errorMessage = QStringLiteral("The combined element change could not be applied.");
        return {};
    }
    if (m_changeCallback)
        m_changeCallback();

    QJsonArray added;
    for (const QString &clientId : std::as_const(clientIds)) {
        added.append(QJsonObject{{QStringLiteral("clientId"), clientId},
                                 {QStringLiteral("id"), assignedIds.value(clientId)}});
    }
    QJsonArray updated;
    for (const QString &id : std::as_const(updatedIds))
        updated.append(id);
    QJsonArray deleted;
    for (const QString &id : std::as_const(deletionIds))
        deleted.append(id);
    QJsonArray detached;
    for (const QString &id : std::as_const(detachedConnectorIds))
        detached.append(id);
    return {
        {QStringLiteral("added"), added},
        {QStringLiteral("updatedIds"), updated},
        {QStringLiteral("deletedIds"), deleted},
        {QStringLiteral("detachedConnectorIds"), detached},
        {QStringLiteral("currentPageIndex"), m_model->document().currentPageIndex()},
        {QStringLiteral("revision"), m_model->document().revision()},
    };
}

QJsonObject WhiteboardMcpController::listPagesTool(const QJsonObject &, QString *errorMessage) const
{
    if (!m_model) {
        *errorMessage = QStringLiteral("Whiteboard model is unavailable.");
        return {};
    }

    const QVector<Page> pages = m_model->document().pages();
    const int currentPageIndex = m_model->document().currentPageIndex();
    QJsonArray pageStates;
    for (qsizetype index = 0; index < pages.size(); ++index) {
        const Page &page = pages.at(index);
        pageStates.append(QJsonObject{
            {QStringLiteral("index"), index},
            {QStringLiteral("current"), index == currentPageIndex},
            {QStringLiteral("locked"), page.locked()},
            {QStringLiteral("elementCount"), page.elements().size()},
            {QStringLiteral("revision"), m_model->document().revision()},
        });
    }
    return {
        {QStringLiteral("pageCount"), pages.size()},
        {QStringLiteral("currentPageIndex"), currentPageIndex},
        {QStringLiteral("revision"), m_model->document().revision()},
        {QStringLiteral("pages"), pageStates},
    };
}

QJsonObject WhiteboardMcpController::navigatePageTool(const QJsonObject &arguments, QString *errorMessage)
{
    if (!m_model || !m_pageNavigationCallback) {
        *errorMessage = QStringLiteral("Whiteboard page navigation is unavailable.");
        return {};
    }

    const QString direction = arguments.value(QStringLiteral("direction")).toString();
    const int currentPageIndex = m_model->document().currentPageIndex();
    const int pageCount = m_model->document().pages().size();
    if (direction == QStringLiteral("previous")) {
        if (currentPageIndex == 0) {
            *errorMessage = QStringLiteral("The first page is already displayed.");
            return {};
        }
    } else if (direction == QStringLiteral("next")) {
        if (currentPageIndex + 1 >= pageCount) {
            *errorMessage = QStringLiteral("The last page is already displayed.");
            return {};
        }
    } else {
        *errorMessage = QStringLiteral("direction must be previous or next.");
        return {};
    }

    if (!m_pageNavigationCallback(direction)) {
        *errorMessage = QStringLiteral("The requested page could not be displayed.");
        return {};
    }
    return {
        {QStringLiteral("navigated"), true},
        {QStringLiteral("direction"), direction},
        {QStringLiteral("currentPageIndex"), m_model->document().currentPageIndex()},
        {QStringLiteral("pageCount"), pageCount},
    };
}

QJsonObject WhiteboardMcpController::addPageTool(const QJsonObject &, QString *errorMessage)
{
    if (!m_model) {
        *errorMessage = QStringLiteral("Whiteboard model is unavailable.");
        return {};
    }
    if (!m_model->addPage()) {
        *errorMessage = QStringLiteral("The document already has the maximum number of pages.");
        return {};
    }
    if (m_changeCallback)
        m_changeCallback();
    return {
        {QStringLiteral("added"), true},
        {QStringLiteral("currentPageIndex"), m_model->document().currentPageIndex()},
        {QStringLiteral("pageCount"), m_model->document().pages().size()},
    };
}

QJsonObject WhiteboardMcpController::deletePageTool(const QJsonObject &, QString *errorMessage)
{
    if (!m_model) {
        *errorMessage = QStringLiteral("Whiteboard model is unavailable.");
        return {};
    }
    if (!m_model->deleteCurrentPage()) {
        if (m_model->currentPageLocked())
            *errorMessage = QStringLiteral("The current page is locked.");
        else
            *errorMessage = QStringLiteral("The last remaining page cannot be deleted.");
        return {};
    }
    if (m_changeCallback)
        m_changeCallback();
    return {
        {QStringLiteral("deleted"), true},
        {QStringLiteral("currentPageIndex"), m_model->document().currentPageIndex()},
        {QStringLiteral("pageCount"), m_model->document().pages().size()},
    };
}

QJsonObject WhiteboardMcpController::lockPageTool(const QJsonObject &arguments, QString *errorMessage)
{
    if (!m_model) {
        *errorMessage = QStringLiteral("Whiteboard model is unavailable.");
        return {};
    }
    const QJsonValue lockedValue = arguments.value(QStringLiteral("locked"));
    if (!lockedValue.isBool()) {
        *errorMessage = QStringLiteral("locked must be a boolean.");
        return {};
    }
    const bool locked = lockedValue.toBool();
    if (m_model->currentPageLocked() == locked) {
        *errorMessage = locked
            ? QStringLiteral("The current page is already locked.")
            : QStringLiteral("The current page is already unlocked.");
        return {};
    }
    if (!m_model->setCurrentPageLocked(locked)) {
        *errorMessage = QStringLiteral("The current page lock state could not be changed.");
        return {};
    }
    if (m_changeCallback)
        m_changeCallback();
    return {
        {QStringLiteral("locked"), locked},
        {QStringLiteral("currentPageIndex"), m_model->document().currentPageIndex()},
    };
}

QJsonObject WhiteboardMcpController::applyDiagramTool(const QJsonObject &arguments, QString *errorMessage)
{
    if (!m_model) {
        *errorMessage = QStringLiteral("Whiteboard model is unavailable.");
        return {};
    }

    BoardModel::PageElementApplyMode mode;
    const QString modeName = arguments.value(QStringLiteral("mode")).toString();
    if (modeName == QStringLiteral("new_page"))
        mode = BoardModel::PageElementApplyMode::NewPage;
    else if (modeName == QStringLiteral("append_current"))
        mode = BoardModel::PageElementApplyMode::AppendCurrent;
    else if (modeName == QStringLiteral("replace_current"))
        mode = BoardModel::PageElementApplyMode::ReplaceCurrent;
    else {
        *errorMessage = QStringLiteral("mode must be new_page, append_current, or replace_current.");
        return {};
    }

    const QJsonValue elementsValue = arguments.value(QStringLiteral("elements"));
    if (!elementsValue.isArray() || elementsValue.toArray().isEmpty()
        || elementsValue.toArray().size() > MaxMcpElements) {
        *errorMessage = QStringLiteral("elements must contain between 1 and 500 entries.");
        return {};
    }

    QVector<DrawingElement> elements;
    QSet<QString> inputIds;
    const QJsonArray elementArray = elementsValue.toArray();
    elements.reserve(elementArray.size());
    for (qsizetype index = 0; index < elementArray.size(); ++index) {
        if (!elementArray.at(index).isObject()) {
            *errorMessage = QStringLiteral("elements[%1] must be an object.").arg(index);
            return {};
        }
        DrawingElement element;
        QString elementError;
        if (!parseDrawingElement(elementArray.at(index).toObject(), &element, &elementError)) {
            *errorMessage = QStringLiteral("elements[%1]: %2").arg(index).arg(elementError);
            return {};
        }
        if (!element.id().isEmpty() && inputIds.contains(element.id())) {
            *errorMessage = QStringLiteral("elements contains duplicate id '%1'.").arg(element.id());
            return {};
        }
        if (!element.id().isEmpty())
            inputIds.insert(element.id());
        elements.append(element);
    }

    QHash<QString, QString> assignedIds;
    if (!m_model->applyElementsToPage(elements, mode, &assignedIds)) {
        if (mode != BoardModel::PageElementApplyMode::NewPage && m_model->currentPageLocked())
            *errorMessage = QStringLiteral("The current page is locked.");
        else if (mode == BoardModel::PageElementApplyMode::NewPage && !m_model->canAddPage())
            *errorMessage = QStringLiteral("The document already has the maximum number of pages.");
        else
            *errorMessage = QStringLiteral("The diagram could not be applied.");
        return {};
    }

    if (m_changeCallback)
        m_changeCallback();

    QJsonObject idMap;
    for (auto it = assignedIds.constBegin(); it != assignedIds.constEnd(); ++it)
        idMap.insert(it.key(), it.value());
    return {
        {QStringLiteral("applied"), true},
        {QStringLiteral("mode"), modeName},
        {QStringLiteral("elementCount"), elements.size()},
        {QStringLiteral("currentPageIndex"), m_model->document().currentPageIndex()},
        {QStringLiteral("pageCount"), m_model->document().pages().size()},
        {QStringLiteral("revision"), m_model->document().revision()},
        {QStringLiteral("idMap"), idMap},
    };
}

QJsonObject WhiteboardMcpController::renderPageTool(const QJsonObject &, QString *errorMessage)
{
    if (!m_canvas) {
        *errorMessage = QStringLiteral("Whiteboard canvas is unavailable.");
        return {};
    }

    const QImage image = m_canvas->renderPageImage();
    QByteArray png;
    QBuffer buffer(&png);
    if (!buffer.open(QIODevice::WriteOnly) || !image.save(&buffer, "PNG")) {
        *errorMessage = QStringLiteral("The current page could not be rendered.");
        return {};
    }

    return {
        {QStringLiteral("content"), QJsonArray{QJsonObject{
             {QStringLiteral("type"), QStringLiteral("image")},
             {QStringLiteral("data"), QString::fromLatin1(png.toBase64())},
             {QStringLiteral("mimeType"), QStringLiteral("image/png")},
         }}},
        {QStringLiteral("width"), image.width()},
        {QStringLiteral("height"), image.height()},
    };
}

QJsonObject WhiteboardMcpController::exportImageTool(const QJsonObject &arguments, QString *errorMessage)
{
    if (!m_model) {
        *errorMessage = QStringLiteral("Whiteboard model is unavailable.");
        return {};
    }

    ImageExporter::Options options;
    QVector<int> selectedIndexes;
    if (!readImageOptions(arguments, m_model, &options, &selectedIndexes, errorMessage))
        return {};

    QRect bounds;
    QByteArray data;
    QString mimeType;
    if (options.format == ImageExporter::Format::Png) {
        const QImage image = ImageExporter::renderPng(
            m_model->currentPage(), selectedIndexes, options.background, &bounds);
        QBuffer buffer(&data);
        if (image.isNull() || !buffer.open(QIODevice::WriteOnly) || !image.save(&buffer, "PNG")) {
            *errorMessage = QStringLiteral("The PNG image could not be encoded.");
            return {};
        }
        mimeType = QStringLiteral("image/png");
    } else {
        data = ImageExporter::renderSvg(
            m_model->currentPage(), selectedIndexes, options.background, &bounds);
        if (data.isEmpty()) {
            *errorMessage = QStringLiteral("The SVG image could not be encoded.");
            return {};
        }
        mimeType = QStringLiteral("image/svg+xml");
    }

    return {
        {QStringLiteral("content"), QJsonArray{QJsonObject{
             {QStringLiteral("type"), QStringLiteral("image")},
             {QStringLiteral("data"), QString::fromLatin1(data.toBase64())},
             {QStringLiteral("mimeType"), mimeType},
         }}},
        {QStringLiteral("format"), arguments.value(QStringLiteral("format")).toString()},
        {QStringLiteral("width"), bounds.width()},
        {QStringLiteral("height"), bounds.height()},
    };
}

QJsonObject WhiteboardMcpController::saveImageTool(const QJsonObject &arguments, QString *errorMessage)
{
    if (!m_model) {
        *errorMessage = QStringLiteral("Whiteboard model is unavailable.");
        return {};
    }

    ImageExporter::Options options;
    QVector<int> selectedIndexes;
    if (!readImageOptions(arguments, m_model, &options, &selectedIndexes, errorMessage))
        return {};

    const QString filePath = arguments.value(QStringLiteral("filePath")).toString().trimmed();
    const QFileInfo fileInfo(filePath);
    if (filePath.isEmpty() || !fileInfo.isAbsolute()) {
        *errorMessage = QStringLiteral("filePath must be an absolute path.");
        return {};
    }
    const QString expectedSuffix = ImageExporter::extension(options.format);
    if (fileInfo.suffix().toLower() != expectedSuffix) {
        *errorMessage = QStringLiteral("filePath extension must match format '.%1'.").arg(expectedSuffix);
        return {};
    }
    const QJsonValue overwriteValue = arguments.value(QStringLiteral("overwrite"));
    if (!overwriteValue.isBool()) {
        *errorMessage = QStringLiteral("overwrite must be a boolean.");
        return {};
    }
    if (fileInfo.exists() && !overwriteValue.toBool()) {
        *errorMessage = QStringLiteral("The destination file already exists and overwrite is false.");
        return {};
    }

    QString saveError;
    if (!ImageExporter::save(
            m_model->currentPage(), selectedIndexes, options, filePath, &saveError)) {
        *errorMessage = QStringLiteral("The image could not be saved: %1").arg(saveError);
        return {};
    }
    const QRect bounds = ImageExporter::exportBounds(m_model->currentPage(), selectedIndexes);
    return {
        {QStringLiteral("saved"), true},
        {QStringLiteral("filePath"), filePath},
        {QStringLiteral("format"), arguments.value(QStringLiteral("format")).toString()},
        {QStringLiteral("width"), bounds.width()},
        {QStringLiteral("height"), bounds.height()},
    };
}

QJsonObject WhiteboardMcpController::undoTool(const QJsonObject &, QString *errorMessage)
{
    if (!m_model || !m_model->undo()) {
        *errorMessage = QStringLiteral("There is no whiteboard change to undo.");
        return {};
    }
    if (m_changeCallback)
        m_changeCallback();
    return {
        {QStringLiteral("undone"), true},
        {QStringLiteral("currentPageIndex"), m_model->document().currentPageIndex()},
        {QStringLiteral("pageCount"), m_model->document().pages().size()},
    };
}

QJsonObject WhiteboardMcpController::redoTool(const QJsonObject &, QString *errorMessage)
{
    if (!m_model || !m_model->redo()) {
        *errorMessage = QStringLiteral("There is no whiteboard change to redo.");
        return {};
    }
    if (m_changeCallback)
        m_changeCallback();
    return {
        {QStringLiteral("redone"), true},
        {QStringLiteral("currentPageIndex"), m_model->document().currentPageIndex()},
        {QStringLiteral("pageCount"), m_model->document().pages().size()},
    };
}
