#include "BoardDocumentExchange.h"

#include <QFile>
#include <QHash>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>

#include <cmath>
#include <limits>

namespace {
bool fail(QString *error, const QString &message)
{
    if (error)
        *error = message;
    return false;
}

bool number(const QJsonValue &value)
{
    return value.isDouble() && std::isfinite(value.toDouble());
}

bool integer(const QJsonValue &value, int minimum, int maximum)
{
    const double n = value.toDouble();
    return number(value) && std::floor(n) == n && n >= minimum && n <= maximum;
}

bool validateElement(const QJsonObject &object, QString *error)
{
    const QString type = object.value("type").toString();
    if (type.isEmpty() || elementTypeToString(elementTypeFromString(type)) != type)
        return fail(error, QStringLiteral("Unknown or missing element type."));

    for (const char *key : {"id", "text", "fontFamily", "startConnectionElementId", "endConnectionElementId"}) {
        if (object.contains(key) && !object.value(key).isString())
            return fail(error, QStringLiteral("Invalid string field: %1").arg(QLatin1String(key)));
    }
    for (const char *key : {"color", "fillColor"}) {
        if (object.contains(key) && (!object.value(key).isString() || !QColor(object.value(key).toString()).isValid()))
            return fail(error, QStringLiteral("Invalid color field: %1").arg(QLatin1String(key)));
    }
    if (object.contains("closed") && !object.value("closed").isBool())
        return fail(error, QStringLiteral("Invalid closed flag."));
    if (object.contains("strokeStyle") && (!object.value("strokeStyle").isString()
        || strokeStyleToString(strokeStyleFromString(object.value("strokeStyle").toString())) != object.value("strokeStyle").toString()))
        return fail(error, QStringLiteral("Unknown stroke style."));
    for (const char *key : {"startArrowHead", "endArrowHead"}) {
        if (object.contains(key) && (!object.value(key).isString()
            || arrowHeadToString(arrowHeadFromString(object.value(key).toString())) != object.value(key).toString()))
            return fail(error, QStringLiteral("Unknown arrow head."));
    }
    for (const char *key : {"strokeWidth", "groupId", "cornerRadius", "fontPointSize", "arcStartAngle", "arcSpanAngle"}) {
        int minimum = 0;
        int maximum = std::numeric_limits<int>::max();
        if (QString::fromLatin1(key) == "cornerRadius") maximum = 200;
        if (QString::fromLatin1(key) == "fontPointSize") minimum = 1;
        if (QString::fromLatin1(key) == "arcStartAngle") minimum = std::numeric_limits<int>::min();
        if (QString::fromLatin1(key) == "arcSpanAngle") { minimum = -359; maximum = 359; }
        if (object.contains(key) && !integer(object.value(key), minimum, maximum))
            return fail(error, QStringLiteral("Invalid integer field: %1").arg(QLatin1String(key)));
    }
    if (object.contains("rotationDegrees") && !number(object.value("rotationDegrees")))
        return fail(error, QStringLiteral("Invalid rotation."));

    const bool pointBased = type == "freehand" || type == "line" || type == "polyline" || type == "bezier";
    if (pointBased || object.contains("points")) {
        if (!object.value("points").isArray())
            return fail(error, QStringLiteral("Invalid points array."));
        const auto points = object.value("points").toArray();
        if ((pointBased && points.isEmpty()) || (type == "line" && points.size() != 2)
            || ((type == "polyline" || type == "bezier") && points.size() < 2))
            return fail(error, QStringLiteral("Invalid number of points."));
        for (const auto &point : points) {
            if (!point.isObject() || !number(point.toObject().value("x")) || !number(point.toObject().value("y")))
                return fail(error, QStringLiteral("Invalid point coordinates."));
        }
    }
    if (!pointBased || object.contains("rect")) {
        if (!object.value("rect").isObject())
            return fail(error, QStringLiteral("Invalid rectangle."));
        const auto rect = object.value("rect").toObject();
        for (const char *key : {"x", "y", "width", "height"}) {
            if (!number(rect.value(key)))
                return fail(error, QStringLiteral("Invalid rectangle coordinates."));
        }
        if (rect.value("width").toDouble() < 0 || rect.value("height").toDouble() < 0)
            return fail(error, QStringLiteral("Negative rectangle size."));
    }
    if (type == "image") {
        const auto encoded = object.value("imageData");
        if (!encoded.isString() || object.value("imageMimeType").toString() != "image/png")
            return fail(error, QStringLiteral("The embedded image must be PNG."));
        const auto decoded = QByteArray::fromBase64Encoding(encoded.toString().toLatin1(), QByteArray::AbortOnBase64DecodingErrors);
        if (!decoded || QImage::fromData(decoded.decoded, "PNG").isNull())
            return fail(error, QStringLiteral("The embedded PNG image is damaged."));
    }
    return true;
}

bool decodePages(const QJsonArray &array, QVector<Page> *output, QString *error)
{
    if (array.isEmpty() || array.size() > BoardDocument::MaxPages)
        return fail(error, QStringLiteral("Drawing data must contain between 1 and %1 pages.").arg(BoardDocument::MaxPages));
    QVector<Page> pages;
    for (qsizetype i = 0; i < array.size(); ++i) {
        if (!array[i].isObject() || !array[i].toObject().value("elements").isArray())
            return fail(error, QStringLiteral("Page %1 has no valid elements array.").arg(i + 1));
        const auto page = array[i].toObject();
        if ((page.contains("locked") && !page.value("locked").isBool())
            || (page.contains("zoomPercent") && !integer(page.value("zoomPercent"), 10, 400))
            || (page.contains("scrollX") && !integer(page.value("scrollX"), 0, std::numeric_limits<int>::max()))
            || (page.contains("scrollY") && !integer(page.value("scrollY"), 0, std::numeric_limits<int>::max())))
            return fail(error, QStringLiteral("Invalid page settings on page %1.").arg(i + 1));
        const auto elements = page.value("elements").toArray();
        QHash<QString, ElementType> ids;
        for (qsizetype j = 0; j < elements.size(); ++j) {
            QString detail;
            if (!elements[j].isObject() || !validateElement(elements[j].toObject(), &detail))
                return fail(error, QStringLiteral("Page %1, element %2: %3").arg(i + 1).arg(j + 1).arg(detail));
            const auto object = elements[j].toObject();
            const QString id = object.value("id").toString();
            if (!id.isEmpty()) {
                if (ids.contains(id))
                    return fail(error, QStringLiteral("Duplicate element ID on page %1: %2").arg(i + 1).arg(id));
                ids.insert(id, elementTypeFromString(object.value("type").toString()));
            }
        }
        for (const auto &value : elements) {
            const auto object = value.toObject();
            for (const char *key : {"startConnectionElementId", "endConnectionElementId"}) {
                const QString target = object.value(key).toString();
                if (target.isEmpty())
                    continue;
                const ElementType targetType = ids.value(target, ElementType::Freehand);
                const bool connectable = targetType == ElementType::Rectangle || targetType == ElementType::RoundedRectangle
                    || targetType == ElementType::Ellipse || targetType == ElementType::Circle
                    || targetType == ElementType::Text || targetType == ElementType::Image;
                if (object.value("type").toString() != "line" || !ids.contains(target) || !connectable)
                    return fail(error, QStringLiteral("Invalid connector reference on page %1: %2").arg(i + 1).arg(target));
            }
        }
        pages.append(Page::fromJson(page));
    }
    *output = pages;
    return true;
}
}

bool BoardDocumentExchange::decode(const QByteArray &data, QVector<Page> *pages, QString *errorMessage)
{
    if (errorMessage) errorMessage->clear();
    if (!pages) return fail(errorMessage, QStringLiteral("Missing output destination."));
    QJsonParseError parseError;
    const auto json = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError)
        return fail(errorMessage, parseError.errorString());
    if (!json.isObject()) return fail(errorMessage, QStringLiteral("Drawing data must be a JSON object."));
    const auto root = json.object();
    // A root pages array without format/version is the legacy autosave format.
    if (root.contains("format") || root.contains("formatVersion")) {
        if (root.value("format").toString() != "WhiteboardApp.DrawingData"
            || !integer(root.value("formatVersion"), 1, 1))
            return fail(errorMessage, QStringLiteral("Unsupported drawing data format or version."));
    }
    if (!root.value("pages").isArray())
        return fail(errorMessage, QStringLiteral("Missing pages array."));
    QJsonArray array = root.value("pages").toArray();
    // Older autosaves stored the shared zoom in settings instead of each page.
    if (!root.contains("format")) {
        const auto zoom = root.value("settings").toObject().value("zoomPercent");
        for (qsizetype i = 0; i < array.size(); ++i) {
            if (!array[i].isObject()) continue;
            auto page = array[i].toObject();
            if (!page.contains("zoomPercent") && !zoom.isUndefined()) page.insert("zoomPercent", zoom);
            array[i] = page;
        }
    }
    return decodePages(array, pages, errorMessage);
}

bool BoardDocumentExchange::validatePages(const QVector<Page> &pages, QString *errorMessage)
{
    QJsonArray array;
    for (const auto &page : pages) array.append(page.toJson());
    QVector<Page> validated;
    return decodePages(array, &validated, errorMessage);
}

bool BoardDocumentExchange::read(const QString &filePath, QVector<Page> *pages, QString *errorMessage)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return fail(errorMessage, file.errorString());
    const auto data = file.readAll();
    if (file.error() != QFileDevice::NoError) return fail(errorMessage, file.errorString());
    return decode(data, pages, errorMessage);
}

bool BoardDocumentExchange::write(const QString &filePath, const BoardDocument &document,
                                  Scope scope, QString *errorMessage)
{
    if (errorMessage) errorMessage->clear();
    const QVector<Page> pages = scope == Scope::CurrentPage ? QVector<Page>{document.currentPage()} : document.pages();
    if (!validatePages(pages, errorMessage)) return false;
    QJsonArray array;
    for (const auto &page : pages) array.append(page.toJson());
    const QByteArray data = QJsonDocument(QJsonObject{
        {"format", "WhiteboardApp.DrawingData"}, {"formatVersion", 1}, {"pages", array}
    }).toJson(QJsonDocument::Indented);
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return fail(errorMessage, file.errorString());
    if (file.write(data) != data.size()) return fail(errorMessage, file.errorString());
    if (!file.commit()) return fail(errorMessage, file.errorString());
    return true;
}
