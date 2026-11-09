#include "BoardDocumentValidator.h"

#include "BoardDocument.h"
#include "BoardEnums.h"

#include <QColor>
#include <QHash>
#include <QImage>
#include <QJsonArray>

#include <cmath>
#include <limits>

namespace {
bool fail(QString *error, const QString &path, const QString &reason)
{
    if (error)
        *error = path + QStringLiteral(": ") + reason;
    return false;
}

bool number(const QJsonValue &value)
{
    return value.isDouble() && std::isfinite(value.toDouble());
}

bool integer(const QJsonValue &value)
{
    const double n = value.toDouble();
    return number(value) && std::floor(n) == n
        && n >= std::numeric_limits<int>::min() && n <= std::numeric_limits<int>::max();
}

bool optionalIntegers(const QJsonObject &object, std::initializer_list<const char *> keys,
                      const QString &path, QString *error)
{
    for (const char *key : keys) {
        if (object.contains(key) && !integer(object.value(key)))
            return fail(error, path + QLatin1Char('.') + QLatin1String(key), QStringLiteral("Expected an integer."));
    }
    return true;
}

bool optionalStrings(const QJsonObject &object, std::initializer_list<const char *> keys,
                     const QString &path, QString *error)
{
    for (const char *key : keys) {
        if (object.contains(key) && !object.value(key).isString())
            return fail(error, path + QLatin1Char('.') + QLatin1String(key), QStringLiteral("Expected a string."));
    }
    return true;
}

bool optionalColors(const QJsonObject &object, std::initializer_list<const char *> keys,
                    const QString &path, QString *error)
{
    for (const char *key : keys) {
        if (object.contains(key) && (!object.value(key).isString() || !QColor(object.value(key).toString()).isValid()))
            return fail(error, path + QLatin1Char('.') + QLatin1String(key), QStringLiteral("Invalid color."));
    }
    return true;
}

bool optionalStyles(const QJsonObject &object, const QString &path, QString *error)
{
    if (object.contains("strokeStyle") && (!object.value("strokeStyle").isString()
        || strokeStyleToString(strokeStyleFromString(object.value("strokeStyle").toString())) != object.value("strokeStyle").toString()))
        return fail(error, path + QStringLiteral(".strokeStyle"), QStringLiteral("Unknown stroke style."));
    for (const char *key : {"startArrowHead", "endArrowHead"}) {
        if (object.contains(key) && (!object.value(key).isString()
            || arrowHeadToString(arrowHeadFromString(object.value(key).toString())) != object.value(key).toString()))
            return fail(error, path + QLatin1Char('.') + QLatin1String(key), QStringLiteral("Unknown arrow head."));
    }
    return true;
}

bool element(const QJsonObject &object, const QString &path, QString *error)
{
    const QString type = object.value("type").toString();
    if (type.isEmpty() || elementTypeToString(elementTypeFromString(type)) != type)
        return fail(error, path + QStringLiteral(".type"), QStringLiteral("Unknown or missing element type."));
    if (!optionalStrings(object, {"id", "text", "fontFamily", "startConnectionElementId", "endConnectionElementId"}, path, error)
        || !optionalColors(object, {"color", "fillColor"}, path, error)
        || !optionalStyles(object, path, error)
        || !optionalIntegers(object, {"strokeWidth", "cornerRadius", "fontPointSize", "arcStartAngle", "arcSpanAngle", "groupId"}, path, error))
        return false;
    if (object.contains("closed") && !object.value("closed").isBool())
        return fail(error, path + QStringLiteral(".closed"), QStringLiteral("Expected a boolean."));
    if (object.contains("rotationDegrees") && !number(object.value("rotationDegrees")))
        return fail(error, path + QStringLiteral(".rotationDegrees"), QStringLiteral("Expected a finite number."));
    const bool pointBased = type == "freehand" || type == "line" || type == "polyline" || type == "bezier";
    if (pointBased || object.contains("points")) {
        if (!object.value("points").isArray())
            return fail(error, path + QStringLiteral(".points"), QStringLiteral("Expected a points array."));
        const auto points = object.value("points").toArray();
        if ((pointBased && points.isEmpty()) || (type == "line" && points.size() != 2)
            || ((type == "polyline" || type == "bezier") && points.size() < 2))
            return fail(error, path + QStringLiteral(".points"), QStringLiteral("Invalid number of points."));
        for (qsizetype i = 0; i < points.size(); ++i) {
            if (!points[i].isObject() || !number(points[i].toObject().value("x")) || !number(points[i].toObject().value("y")))
                return fail(error, path + QStringLiteral(".points[%1]").arg(i), QStringLiteral("Expected finite x and y coordinates."));
        }
    }
    if (!pointBased || object.contains("rect")) {
        if (!object.value("rect").isObject())
            return fail(error, path + QStringLiteral(".rect"), QStringLiteral("Expected a rectangle."));
        const auto rect = object.value("rect").toObject();
        for (const char *key : {"x", "y", "width", "height"}) {
            if (!number(rect.value(key)))
                return fail(error, path + QStringLiteral(".rect.") + QLatin1String(key), QStringLiteral("Expected a finite number."));
        }
    }
    if (type == "image") {
        if (!object.value("imageData").isString() || object.value("imageMimeType").toString() != "image/png")
            return fail(error, path + QStringLiteral(".imageData"), QStringLiteral("Expected a PNG image."));
        const auto decoded = QByteArray::fromBase64Encoding(object.value("imageData").toString().toLatin1(), QByteArray::AbortOnBase64DecodingErrors);
        if (!decoded || QImage::fromData(decoded.decoded, "PNG").isNull())
            return fail(error, path + QStringLiteral(".imageData"), QStringLiteral("Invalid PNG image."));
    }
    return true;
}
}

bool BoardDocumentValidator::validate(const QJsonObject &object, QString *error)
{
    if (error)
        error->clear();
    if (!object.value("pages").isArray())
        return fail(error, QStringLiteral("pages"), QStringLiteral("Expected a pages array."));
    const QJsonArray pages = object.value("pages").toArray();
    if (pages.isEmpty() || pages.size() > BoardDocument::MaxPages)
        return fail(error, QStringLiteral("pages"), QStringLiteral("Expected between 1 and %1 pages.").arg(BoardDocument::MaxPages));
    if (!optionalIntegers(object, {"currentPageIndex", "nextElementId"}, QStringLiteral("document"), error))
        return false;
    if (object.contains("revision")) {
        const QJsonValue value = object.value("revision");
        // toInteger preserves JSON integers that cannot be represented exactly as double.
        constexpr qint64 sentinel = std::numeric_limits<qint64>::min();
        if (!value.isDouble() || (value.toInteger(sentinel) == sentinel && value != QJsonValue(sentinel)))
            return fail(error, QStringLiteral("revision"), QStringLiteral("Expected a 64-bit integer."));
    }
    if (object.contains("settings")) {
        if (!object.value("settings").isObject())
            return fail(error, QStringLiteral("settings"), QStringLiteral("Expected a settings object."));
        const QJsonObject settings = object.value("settings").toObject();
        if (!optionalIntegers(settings, {"zoomPercent", "strokeWidth", "cornerRadius", "fontPointSize", "windowWidth", "windowHeight", "floatingActionDockY"}, QStringLiteral("settings"), error)
            || !optionalStrings(settings, {"fontFamily"}, QStringLiteral("settings"), error)
            || !optionalColors(settings, {"selectedColor", "fillColor"}, QStringLiteral("settings"), error)
            || !optionalStyles(settings, QStringLiteral("settings"), error))
            return false;
        if (settings.contains("selectedTool") && (!settings.value("selectedTool").isString()
            || toolToString(toolFromString(settings.value("selectedTool").toString())) != settings.value("selectedTool").toString()))
            return fail(error, QStringLiteral("settings.selectedTool"), QStringLiteral("Unknown tool."));
    }
    qint64 elementCount = 0;
    qint64 maximumId = 0;
    for (qsizetype i = 0; i < pages.size(); ++i) {
        const QString path = QStringLiteral("pages[%1]").arg(i);
        if (!pages[i].isObject())
            return fail(error, path, QStringLiteral("Expected a page object."));
        const QJsonObject page = pages[i].toObject();
        if (!page.value("elements").isArray())
            return fail(error, path + QStringLiteral(".elements"), QStringLiteral("Expected an elements array."));
        if (!optionalIntegers(page, {"zoomPercent", "scrollX", "scrollY"}, path, error))
            return false;
        if (page.contains("locked") && !page.value("locked").isBool())
            return fail(error, path + QStringLiteral(".locked"), QStringLiteral("Expected a boolean."));
        const QJsonArray elements = page.value("elements").toArray();
        elementCount += elements.size();
        for (qsizetype j = 0; j < elements.size(); ++j) {
            const QString elementPath = path + QStringLiteral(".elements[%1]").arg(j);
            if (!elements[j].isObject())
                return fail(error, elementPath, QStringLiteral("Expected an element object."));
            const QJsonObject item = elements[j].toObject();
            if (!element(item, elementPath, error))
                return false;
            const QString id = item.value("id").toString().trimmed();
            if (id.startsWith(QStringLiteral("element-"))) {
                bool ok = false;
                const int numericId = id.mid(8).toInt(&ok);
                if (ok && numericId > 0)
                    maximumId = qMax(maximumId, qint64(numericId));
            }
        }
    }
    // fromJson repairs IDs using int counters. Reject data that would overflow those counters.
    const qint64 nextId = qMax<qint64>(1, object.value("nextElementId").toInt(1));
    if (qMax(nextId, maximumId + 1) + elementCount >= std::numeric_limits<int>::max())
        return fail(error, QStringLiteral("nextElementId"), QStringLiteral("Element ID space is exhausted."));
    return true;
}

bool BoardDocumentValidator::validateConnections(const BoardDocument &document, QString *error)
{
    const auto pages = document.pages();
    for (qsizetype i = 0; i < pages.size(); ++i) {
        const auto elements = pages[i].elements();
        QHash<QString, ElementType> ids;
        for (const auto &element : elements)
            ids.insert(element.id(), element.type());
        for (qsizetype j = 0; j < elements.size(); ++j) {
            const auto &element = elements[j];
            for (const QString &target : {element.startConnectionElementId(), element.endConnectionElementId()}) {
                if (target.isEmpty())
                    continue;
                const auto type = ids.value(target, ElementType::Freehand);
                const bool connectable = type == ElementType::Rectangle || type == ElementType::RoundedRectangle
                    || type == ElementType::Ellipse || type == ElementType::Circle || type == ElementType::Text || type == ElementType::Image;
                if (element.type() != ElementType::Line || !ids.contains(target) || !connectable)
                    return fail(error, QStringLiteral("pages[%1].elements[%2]").arg(i).arg(j),
                                QStringLiteral("Missing or ambiguous connector target: %1").arg(target));
            }
        }
    }
    return true;
}
