#include "DrawingElement.h"

#include "JsonUtil.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QLineF>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QTransform>
#include <QRectF>
#include <QSizeF>
#include <QtMath>
#include <QtGlobal>

#include <cmath>

namespace {
bool usesPointGeometry(ElementType type)
{
    return type == ElementType::Freehand || type == ElementType::Line
        || type == ElementType::Polyline || type == ElementType::Bezier;
}

qreal normalizedRotation(qreal degrees)
{
    qreal normalized = std::fmod(degrees, 360.0);
    if (normalized < 0.0)
        normalized += 360.0;
    return qFuzzyCompare(normalized, 360.0) || qFuzzyIsNull(normalized) ? 0.0 : normalized;
}

QPointF rotatePoint(const QPointF &point, const QPointF &pivot, qreal degrees)
{
    QTransform transform;
    transform.translate(pivot.x(), pivot.y());
    transform.rotate(degrees);
    transform.translate(-pivot.x(), -pivot.y());
    return transform.map(point);
}

QRectF squareRect(const QRectF &rect)
{
    const QRectF normalized = rect.normalized();
    const qreal side = qMin(normalized.width(), normalized.height());
    return QRectF(normalized.topLeft(), QSizeF(side, side)).normalized();
}

QPointF pointOnEllipse(const QRectF &rect, int angleDegrees)
{
    const qreal radians = qDegreesToRadians(static_cast<qreal>(angleDegrees));
    const QPointF center = rect.center();
    return QPointF(
        center.x() + std::cos(radians) * rect.width() / 2.0,
        center.y() - std::sin(radians) * rect.height() / 2.0);
}

qreal distanceToSegment(const QPointF &point, const QPointF &start, const QPointF &end)
{
    const QPointF segment = end - start;
    const qreal lengthSquared = segment.x() * segment.x() + segment.y() * segment.y();
    if (qFuzzyIsNull(lengthSquared))
        return QLineF(point, start).length();
    const QPointF pointVector = point - start;
    const qreal projection = qBound<qreal>(0.0, (pointVector.x() * segment.x() + pointVector.y() * segment.y()) / lengthSquared, 1.0);
    const QPointF closest = start + segment * projection;
    return QLineF(point, closest).length();
}

QPainterPath bezierPath(const QVector<QPointF> &points, bool closed = false)
{
    QPainterPath path;
    if (points.size() < 2)
        return path;
    path.moveTo(points.at(0));
    if (!closed && points.size() == 3) {
        path.quadTo(points.at(1), points.at(2));
        return path;
    }
    if (points.size() >= 4 && (points.size() - 1) % 3 == 0) {
        for (int i = 1; i + 2 < points.size(); i += 3)
            path.cubicTo(points.at(i), points.at(i + 1), points.at(i + 2));
        if (closed)
            path.closeSubpath();
        return path;
    }
    if (closed && points.size() >= 3) {
        for (int i = 0; i < points.size(); ++i) {
            const QPointF p0 = points.at((i + points.size() - 1) % points.size());
            const QPointF p1 = points.at(i);
            const QPointF p2 = points.at((i + 1) % points.size());
            const QPointF p3 = points.at((i + 2) % points.size());
            const QPointF c1 = p1 + (p2 - p0) / 6.0;
            const QPointF c2 = p2 - (p3 - p1) / 6.0;
            path.cubicTo(c1, c2, p2);
        }
        path.closeSubpath();
        return path;
    }
    if (points.size() == 2) {
        path.lineTo(points.at(1));
        return path;
    }
    for (int i = 0; i < points.size() - 1; ++i) {
        const QPointF p0 = (i == 0) ? points.at(i) : points.at(i - 1);
        const QPointF p1 = points.at(i);
        const QPointF p2 = points.at(i + 1);
        const QPointF p3 = (i + 2 < points.size()) ? points.at(i + 2) : p2;
        const QPointF c1 = p1 + (p2 - p0) / 6.0;
        const QPointF c2 = p2 - (p3 - p1) / 6.0;
        path.cubicTo(c1, c2, p2);
    }
    return path;
}

QPainterPath arcPath(const QRectF &rect, int startAngle, int spanAngle)
{
    QPainterPath path;
    if (rect.isEmpty())
        return path;
    path.moveTo(pointOnEllipse(rect, startAngle));
    path.arcTo(rect, startAngle, spanAngle);
    return path;
}

bool pathIntersectsPoint(const QPainterPath &path, const QPointF &point, qreal radius)
{
    if (path.isEmpty())
        return false;
    QPainterPathStroker stroker;
    stroker.setWidth(radius * 2.0);
    stroker.setCapStyle(Qt::RoundCap);
    stroker.setJoinStyle(Qt::RoundJoin);
    return stroker.createStroke(path).contains(point);
}

QPainterPath roundedRectanglePath(const QRectF &rect, int radius)
{
    QPainterPath path;
    const QRectF normalized = rect.normalized();
    const qreal clampedRadius = qMin<qreal>(qMax(0, radius), qMin(normalized.width(), normalized.height()) / 2.0);
    path.addRoundedRect(normalized, clampedRadius, clampedRadius);
    return path;
}
}

QString DrawingElement::id() const
{
    return m_id;
}

void DrawingElement::setId(const QString &id)
{
    m_id = id;
}

ElementType DrawingElement::type() const
{
    return m_type;
}

void DrawingElement::setType(ElementType type)
{
    m_type = type;
}

QVector<QPointF> DrawingElement::points() const
{
    return m_points;
}

void DrawingElement::setPoints(const QVector<QPointF> &points)
{
    m_points = points;
}

QRectF DrawingElement::rect() const
{
    return m_rect;
}

void DrawingElement::setRect(const QRectF &rect)
{
    m_rect = rect.normalized();
}

QString DrawingElement::text() const
{
    return m_text;
}

void DrawingElement::setText(const QString &text)
{
    m_text = text;
}

QByteArray DrawingElement::imageData() const
{
    return m_imageData;
}

void DrawingElement::setImageData(const QByteArray &data)
{
    m_imageData = data;
}

QString DrawingElement::imageMimeType() const
{
    return m_imageMimeType;
}

void DrawingElement::setImageMimeType(const QString &mimeType)
{
    m_imageMimeType = mimeType;
}

QColor DrawingElement::color() const
{
    return m_color;
}

void DrawingElement::setColor(const QColor &color)
{
    if (color.isValid())
        m_color = color;
}

QColor DrawingElement::fillColor() const
{
    return m_fillColor;
}

void DrawingElement::setFillColor(const QColor &color)
{
    if (color.isValid())
        m_fillColor = color;
}

int DrawingElement::strokeWidth() const
{
    return m_strokeWidth;
}

void DrawingElement::setStrokeWidth(int strokeWidth)
{
    m_strokeWidth = qMax(0, strokeWidth);
}

int DrawingElement::cornerRadius() const
{
    return m_cornerRadius;
}

void DrawingElement::setCornerRadius(int cornerRadius)
{
    m_cornerRadius = qBound(0, cornerRadius, 200);
}

StrokeStyle DrawingElement::strokeStyle() const
{
    return m_strokeStyle;
}

void DrawingElement::setStrokeStyle(StrokeStyle strokeStyle)
{
    m_strokeStyle = strokeStyle;
}

QFont DrawingElement::font() const
{
    return m_font;
}

void DrawingElement::setFont(const QFont &font)
{
    m_font = font;
}

bool DrawingElement::closed() const
{
    return m_closed;
}

void DrawingElement::setClosed(bool closed)
{
    m_closed = closed;
}

ArrowHead DrawingElement::startArrowHead() const
{
    return m_startArrowHead;
}

void DrawingElement::setStartArrowHead(ArrowHead arrowHead)
{
    m_startArrowHead = arrowHead;
}

ArrowHead DrawingElement::endArrowHead() const
{
    return m_endArrowHead;
}

void DrawingElement::setEndArrowHead(ArrowHead arrowHead)
{
    m_endArrowHead = arrowHead;
}

QString DrawingElement::startConnectionElementId() const
{
    return m_startConnectionElementId;
}

void DrawingElement::setStartConnectionElementId(const QString &id)
{
    m_startConnectionElementId = id;
}

QString DrawingElement::endConnectionElementId() const
{
    return m_endConnectionElementId;
}

void DrawingElement::setEndConnectionElementId(const QString &id)
{
    m_endConnectionElementId = id;
}

int DrawingElement::arcStartAngle() const
{
    return m_arcStartAngle;
}

void DrawingElement::setArcStartAngle(int angle)
{
    m_arcStartAngle = angle % 360;
}

int DrawingElement::arcSpanAngle() const
{
    return m_arcSpanAngle;
}

void DrawingElement::setArcSpanAngle(int angle)
{
    m_arcSpanAngle = qBound(-359, angle, 359);
    if (m_arcSpanAngle == 0)
        m_arcSpanAngle = 1;
}

int DrawingElement::groupId() const
{
    return m_groupId;
}

void DrawingElement::setGroupId(int groupId)
{
    m_groupId = qMax(0, groupId);
}

qreal DrawingElement::rotationDegrees() const
{
    return m_rotationDegrees;
}

void DrawingElement::setRotationDegrees(qreal degrees)
{
    m_rotationDegrees = normalizedRotation(degrees);
}

QRectF DrawingElement::bounds() const
{
    if (usesPointGeometry(m_type)) {
        if (m_points.isEmpty())
            return QRectF();
        qreal left = m_points.first().x();
        qreal right = left;
        qreal top = m_points.first().y();
        qreal bottom = top;
        for (const QPointF &point : m_points) {
            left = qMin(left, point.x());
            right = qMax(right, point.x());
            top = qMin(top, point.y());
            bottom = qMax(bottom, point.y());
        }
        return QRectF(QPointF(left, top), QPointF(right, bottom)).normalized();
    }
    if (m_type == ElementType::Circle || m_type == ElementType::Arc)
        return squareRect(m_rect);
    return m_rect.normalized();
}

QVector<QPointF> DrawingElement::visualCorners() const
{
    const QRectF rect = bounds();
    QVector<QPointF> corners = {
        rect.topLeft(),
        rect.topRight(),
        rect.bottomRight(),
        rect.bottomLeft(),
    };
    if (usesPointGeometry(m_type) || m_type == ElementType::Circle || qFuzzyIsNull(m_rotationDegrees))
        return corners;
    for (QPointF &point : corners)
        point = rotatePoint(point, rect.center(), m_rotationDegrees);
    return corners;
}

QRectF DrawingElement::visualBounds() const
{
    if (usesPointGeometry(m_type) || m_type == ElementType::Circle || qFuzzyIsNull(m_rotationDegrees))
        return bounds();
    const QVector<QPointF> corners = visualCorners();
    if (corners.isEmpty())
        return QRectF();
    qreal left = corners.first().x();
    qreal right = left;
    qreal top = corners.first().y();
    qreal bottom = top;
    for (const QPointF &point : corners) {
        left = qMin(left, point.x());
        right = qMax(right, point.x());
        top = qMin(top, point.y());
        bottom = qMax(bottom, point.y());
    }
    return QRectF(QPointF(left, top), QPointF(right, bottom)).normalized();
}

QPointF DrawingElement::mapToVisual(const QPointF &point) const
{
    if (usesPointGeometry(m_type) || m_type == ElementType::Circle || qFuzzyIsNull(m_rotationDegrees))
        return point;
    return rotatePoint(point, bounds().center(), m_rotationDegrees);
}

QPointF DrawingElement::mapFromVisual(const QPointF &point) const
{
    if (usesPointGeometry(m_type) || m_type == ElementType::Circle || qFuzzyIsNull(m_rotationDegrees))
        return point;
    return rotatePoint(point, bounds().center(), -m_rotationDegrees);
}

void DrawingElement::moveBy(const QPointF &delta)
{
    if (usesPointGeometry(m_type)) {
        for (QPointF &point : m_points)
            point += delta;
        return;
    }
    m_rect.translate(delta);
}

void DrawingElement::resizeTo(const QRectF &rect)
{
    const QRectF target = rect.normalized();
    if (target.width() <= 1.0 || target.height() <= 1.0)
        return;

    if (m_type == ElementType::Freehand || m_type == ElementType::Polyline || m_type == ElementType::Bezier) {
        const QRectF source = bounds();
        const qreal sourceWidth = qMax<qreal>(1.0, source.width());
        const qreal sourceHeight = qMax<qreal>(1.0, source.height());
        for (QPointF &point : m_points) {
            const qreal xRatio = (point.x() - source.left()) / sourceWidth;
            const qreal yRatio = (point.y() - source.top()) / sourceHeight;
            point = QPointF(target.left() + xRatio * target.width(), target.top() + yRatio * target.height());
        }
        return;
    }

    if (m_type == ElementType::Line) {
        if (m_points.size() >= 2)
            m_points = {target.topLeft(), target.bottomRight()};
        return;
    }

    m_rect = (m_type == ElementType::Circle || m_type == ElementType::Arc) ? squareRect(target) : target;
}

void DrawingElement::rotateBy(qreal degrees, const QPointF &pivot)
{
    if (qFuzzyIsNull(degrees))
        return;

    if (usesPointGeometry(m_type)) {
        for (QPointF &point : m_points)
            point = rotatePoint(point, pivot, degrees);
    } else {
        const QPointF center = bounds().center();
        m_rect.translate(rotatePoint(center, pivot, degrees) - center);
    }
    if (m_type != ElementType::Circle)
        setRotationDegrees(m_rotationDegrees + degrees);
}

void DrawingElement::scaleBy(qreal scaleFactor, const QPointF &pivot)
{
    if (scaleFactor <= 0.0 || qFuzzyCompare(scaleFactor, 1.0))
        return;
    if (usesPointGeometry(m_type)) {
        for (QPointF &point : m_points)
            point = pivot + (point - pivot) * scaleFactor;
        return;
    }

    const QRectF geometry = bounds();
    const QPointF center = pivot + (geometry.center() - pivot) * scaleFactor;
    const QSizeF size = geometry.size() * scaleFactor;
    const QRectF target(center - QPointF(size.width() / 2.0, size.height() / 2.0), size);
    m_rect = (m_type == ElementType::Circle || m_type == ElementType::Arc) ? squareRect(target) : target;
}

bool DrawingElement::intersects(const QPointF &point, qreal radius) const
{
    const QPointF geometryPoint = mapFromVisual(point);
    const QRectF hitRect(geometryPoint.x() - radius, geometryPoint.y() - radius, radius * 2.0, radius * 2.0);
    if (m_type == ElementType::Freehand || m_type == ElementType::Line || m_type == ElementType::Polyline) {
        if (m_type == ElementType::Polyline && m_closed && m_fillColor.alpha() > 0 && m_points.size() > 2) {
            QPainterPath path(m_points.first());
            for (int i = 1; i < m_points.size(); ++i)
                path.lineTo(m_points.at(i));
            path.closeSubpath();
            if (path.contains(geometryPoint))
                return true;
        }
        for (int i = 0; i < m_points.size(); ++i) {
            if (QLineF(m_points.at(i), geometryPoint).length() <= radius)
                return true;
            if (i > 0 && distanceToSegment(geometryPoint, m_points.at(i - 1), m_points.at(i)) <= radius)
                return true;
        }
        if (m_type == ElementType::Polyline && m_closed && m_points.size() > 2 && distanceToSegment(geometryPoint, m_points.last(), m_points.first()) <= radius)
            return true;
        return false;
    }
    if (m_type == ElementType::Bezier) {
        const QPainterPath path = bezierPath(m_points, m_closed);
        if (m_closed && m_fillColor.alpha() > 0 && path.contains(geometryPoint))
            return true;
        return pathIntersectsPoint(path, geometryPoint, radius);
    }
    if (m_type == ElementType::Arc)
        return pathIntersectsPoint(arcPath(bounds(), m_arcStartAngle, m_arcSpanAngle), geometryPoint, radius);
    if (m_type == ElementType::Text || m_type == ElementType::Image)
        return bounds().adjusted(-radius, -radius, radius, radius).contains(geometryPoint);
    if (m_type == ElementType::RoundedRectangle) {
        const QPainterPath path = roundedRectanglePath(bounds(), m_cornerRadius);
        if (m_fillColor.alpha() > 0 && path.contains(geometryPoint))
            return true;
        return pathIntersectsPoint(path, geometryPoint, radius);
    }
    return bounds().adjusted(-radius, -radius, radius, radius).intersects(hitRect);
}

QJsonObject DrawingElement::toJson() const
{
    QJsonArray pointArray;
    for (const QPointF &point : m_points)
        pointArray.append(pointToJson(point));

    QJsonObject object{
        {QStringLiteral("id"), m_id},
        {QStringLiteral("type"), elementTypeToString(m_type)},
        {QStringLiteral("points"), pointArray},
        {QStringLiteral("rect"), rectToJson(m_rect)},
        {QStringLiteral("text"), m_text},
        {QStringLiteral("color"), m_color.name(QColor::HexArgb)},
        {QStringLiteral("fillColor"), m_fillColor.name(QColor::HexArgb)},
        {QStringLiteral("strokeWidth"), m_strokeWidth},
        {QStringLiteral("cornerRadius"), m_cornerRadius},
        {QStringLiteral("strokeStyle"), strokeStyleToString(m_strokeStyle)},
        {QStringLiteral("fontFamily"), m_font.family()},
        {QStringLiteral("fontPointSize"), m_font.pointSize()},
        {QStringLiteral("closed"), m_closed},
        {QStringLiteral("startArrowHead"), arrowHeadToString(m_startArrowHead)},
        {QStringLiteral("endArrowHead"), arrowHeadToString(m_endArrowHead)},
        {QStringLiteral("startConnectionElementId"), m_startConnectionElementId},
        {QStringLiteral("endConnectionElementId"), m_endConnectionElementId},
        {QStringLiteral("arcStartAngle"), m_arcStartAngle},
        {QStringLiteral("arcSpanAngle"), m_arcSpanAngle},
        {QStringLiteral("groupId"), m_groupId},
        {QStringLiteral("rotationDegrees"), m_rotationDegrees},
    };
    if (m_type == ElementType::Image) {
        object.insert(QStringLiteral("imageData"), QString::fromLatin1(m_imageData.toBase64()));
        object.insert(QStringLiteral("imageMimeType"), m_imageMimeType);
    }
    return object;
}

DrawingElement DrawingElement::fromJson(const QJsonObject &object)
{
    DrawingElement element;
    element.setId(object.value(QStringLiteral("id")).toString());
    element.setType(elementTypeFromString(object.value(QStringLiteral("type")).toString()));

    QVector<QPointF> points;
    const QJsonArray pointArray = object.value(QStringLiteral("points")).toArray();
    points.reserve(pointArray.size());
    for (const QJsonValue &value : pointArray)
        points.append(pointFromJson(value.toObject()));
    element.setPoints(points);

    element.setRect(rectFromJson(object.value(QStringLiteral("rect")).toObject()));
    element.setText(object.value(QStringLiteral("text")).toString());
    element.setImageData(QByteArray::fromBase64(object.value(QStringLiteral("imageData")).toString().toLatin1()));
    element.setImageMimeType(object.value(QStringLiteral("imageMimeType")).toString());
    element.setColor(QColor(object.value(QStringLiteral("color")).toString(QStringLiteral("#ff344054"))));
    element.setFillColor(QColor(object.value(QStringLiteral("fillColor")).toString(QStringLiteral("#00000000"))));
    element.setStrokeWidth(object.value(QStringLiteral("strokeWidth")).toInt(3));
    element.setCornerRadius(object.value(QStringLiteral("cornerRadius")).toInt(24));
    element.setStrokeStyle(strokeStyleFromString(object.value(QStringLiteral("strokeStyle")).toString()));
    element.setFont(QFont(
        object.value(QStringLiteral("fontFamily")).toString(QStringLiteral("Sans Serif")),
        object.value(QStringLiteral("fontPointSize")).toInt(18)));
    element.setClosed(object.value(QStringLiteral("closed")).toBool(false));
    element.setStartArrowHead(arrowHeadFromString(object.value(QStringLiteral("startArrowHead")).toString()));
    element.setEndArrowHead(arrowHeadFromString(object.value(QStringLiteral("endArrowHead")).toString()));
    element.setStartConnectionElementId(object.value(QStringLiteral("startConnectionElementId")).toString());
    element.setEndConnectionElementId(object.value(QStringLiteral("endConnectionElementId")).toString());
    element.setArcStartAngle(object.value(QStringLiteral("arcStartAngle")).toInt(0));
    element.setArcSpanAngle(object.value(QStringLiteral("arcSpanAngle")).toInt(120));
    element.setGroupId(object.value(QStringLiteral("groupId")).toInt(0));
    element.setRotationDegrees(object.value(QStringLiteral("rotationDegrees")).toDouble(0.0));
    return element;
}
