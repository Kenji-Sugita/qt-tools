#include "WhiteboardMcpController.h"

#include "BoardModel.h"
#include "CanvasWidget.h"
#include "DrawingElement.h"
#include "ImageExporter.h"
#include "qmcpserver.h"

#include <QBuffer>
#include <QColor>
#include <QFileInfo>
#include <QImage>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include <cmath>
#include <limits>
#include <utility>

namespace {
constexpr int MaxMcpElements = 500;
constexpr int MaxMcpPointsPerElement = 1000;
constexpr int MaxMcpTextLength = 10000;

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
             {QStringLiteral("arcStartAngle"), QJsonObject{{QStringLiteral("type"), QStringLiteral("integer")}}},
             {QStringLiteral("arcSpanAngle"), QJsonObject{{QStringLiteral("type"), QStringLiteral("integer")}}},
             {QStringLiteral("groupId"), QJsonObject{{QStringLiteral("type"), QStringLiteral("integer")}}},
             {QStringLiteral("rotationDegrees"), QJsonObject{{QStringLiteral("type"), QStringLiteral("number")}}},
         }},
        {QStringLiteral("required"), QJsonArray{QStringLiteral("type")}},
    };
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
    , m_server(new QMcpServer(this))
{
    m_server->setServerName(QStringLiteral("whiteboard-app"));
    m_server->setBuiltinToolPackMode(QMcpServer::BuiltinToolPackMode::CustomOnly);
    m_server->setApprovalPolicy([this](const QString &toolName, const QJsonObject &arguments, QString *denyReason) {
        if (!m_approvalCallback) {
            if (denyReason)
                *denyReason = QStringLiteral("No approval handler is configured.");
            return false;
        }
        return m_approvalCallback(toolName, arguments, denyReason);
    });
}

WhiteboardMcpController::~WhiteboardMcpController() = default;

void WhiteboardMcpController::setInputDevice(QIODevice *device)
{
    m_server->setInputDevice(device);
}

void WhiteboardMcpController::setOutputDevice(QIODevice *device)
{
    m_server->setOutputDevice(device);
}

bool WhiteboardMcpController::start()
{
    return registerTools() && m_server->start();
}

bool WhiteboardMcpController::startMessageTransport()
{
    return registerTools() && m_server->startMessageTransport();
}

bool WhiteboardMcpController::processMessage(const QByteArray &message,
                                             QJsonObject *response,
                                             bool *hasResponse)
{
    const QMcpServer::MessageResult result = m_server->processMessage(message);
    if (response)
        *response = result.response;
    if (hasResponse)
        *hasResponse = result.hasResponse;
    return result.accepted;
}

QString WhiteboardMcpController::negotiatedProtocolVersion() const
{
    return m_server->negotiatedProtocolVersion();
}

void WhiteboardMcpController::stop()
{
    m_server->stop();
}

bool WhiteboardMcpController::registerTools()
{
    if (m_toolsRegistered)
        return true;

    const bool stateRegistered = m_server->registerTool(
        QStringLiteral("whiteboard/state"),
        QStringLiteral("Return the 1920 x 1080 canvas, current page, lock, history, and element JSON state."),
        [this](const QJsonObject &arguments, QString *errorMessage) {
            return stateTool(arguments, errorMessage);
        },
        emptyObjectSchema());

    const bool pagesRegistered = m_server->registerTool(
        QStringLiteral("whiteboard/pages/list"),
        QStringLiteral("List page indexes, element counts, lock states, and the current page without changing the display."),
        [this](const QJsonObject &arguments, QString *errorMessage) {
            return listPagesTool(arguments, errorMessage);
        },
        emptyObjectSchema());

    const bool navigateRegistered = m_server->registerTool(
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

    const bool addPageRegistered = m_server->registerTool(
        QStringLiteral("whiteboard/page/add"),
        QStringLiteral("Add an empty page immediately after the current page and display it."),
        [this](const QJsonObject &arguments, QString *errorMessage) {
            return addPageTool(arguments, errorMessage);
        },
        emptyObjectSchema(),
        true);

    const bool deletePageRegistered = m_server->registerTool(
        QStringLiteral("whiteboard/page/delete"),
        QStringLiteral("Delete the current page when it is unlocked and at least one other page exists."),
        [this](const QJsonObject &arguments, QString *errorMessage) {
            return deletePageTool(arguments, errorMessage);
        },
        emptyObjectSchema(),
        true);

    const bool lockPageRegistered = m_server->registerTool(
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

    const bool applyRegistered = m_server->registerTool(
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

    const bool renderRegistered = m_server->registerTool(
        QStringLiteral("whiteboard/page/render"),
        QStringLiteral("Render the current page as a full 1920 x 1080 PNG image for visual inspection."),
        [this](const QJsonObject &arguments, QString *errorMessage) {
            return renderPageTool(arguments, errorMessage);
        },
        emptyObjectSchema());

    const bool exportImageRegistered = m_server->registerTool(
        QStringLiteral("whiteboard/image/export"),
        QStringLiteral("Return current-page or selected elements as cropped PNG or SVG image data."),
        [this](const QJsonObject &arguments, QString *errorMessage) {
            return exportImageTool(arguments, errorMessage);
        },
        imageOptionsSchema(false));

    const bool saveImageRegistered = m_server->registerTool(
        QStringLiteral("whiteboard/image/save"),
        QStringLiteral("Save current-page or selected elements to an absolute PNG or SVG file path."),
        [this](const QJsonObject &arguments, QString *errorMessage) {
            return saveImageTool(arguments, errorMessage);
        },
        imageOptionsSchema(true),
        true);

    const bool undoRegistered = m_server->registerTool(
        QStringLiteral("whiteboard/history/undo"),
        QStringLiteral("Undo the most recent whiteboard change."),
        [this](const QJsonObject &arguments, QString *errorMessage) {
            return undoTool(arguments, errorMessage);
        },
        emptyObjectSchema(),
        true);

    const bool redoRegistered = m_server->registerTool(
        QStringLiteral("whiteboard/history/redo"),
        QStringLiteral("Redo the most recently undone whiteboard change."),
        [this](const QJsonObject &arguments, QString *errorMessage) {
            return redoTool(arguments, errorMessage);
        },
        emptyObjectSchema(),
        true);

    m_toolsRegistered = stateRegistered && pagesRegistered && navigateRegistered
        && addPageRegistered && deletePageRegistered && lockPageRegistered
        && applyRegistered && renderRegistered && exportImageRegistered
        && saveImageRegistered && undoRegistered && redoRegistered;
    return m_toolsRegistered;
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
        });
    }
    return {
        {QStringLiteral("pageCount"), pages.size()},
        {QStringLiteral("currentPageIndex"), currentPageIndex},
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
        elements.append(element);
    }

    if (!m_model->applyElementsToPage(elements, mode)) {
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

    return {
        {QStringLiteral("applied"), true},
        {QStringLiteral("mode"), modeName},
        {QStringLiteral("elementCount"), elements.size()},
        {QStringLiteral("currentPageIndex"), m_model->document().currentPageIndex()},
        {QStringLiteral("pageCount"), m_model->document().pages().size()},
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
