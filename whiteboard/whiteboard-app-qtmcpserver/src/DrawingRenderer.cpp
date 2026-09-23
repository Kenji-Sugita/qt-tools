#include "DrawingRenderer.h"

#include <QLineF>
#include <QPainter>
#include <QPolygonF>
#include <QTransform>
#include <QtMath>

#include <cmath>

namespace {
bool usesPointGeometry(ElementType type)
{
    return type == ElementType::Freehand || type == ElementType::Line
        || type == ElementType::Polyline || type == ElementType::Bezier;
}

Qt::PenStyle toQtPenStyle(StrokeStyle strokeStyle)
{
    return strokeStyle == StrokeStyle::Dotted ? Qt::DotLine : Qt::SolidLine;
}

bool supportsArrowHeads(const DrawingElement &element)
{
    if (element.type() == ElementType::Line || element.type() == ElementType::Arc)
        return true;
    if (element.type() == ElementType::Bezier)
        return !element.closed();
    return element.type() == ElementType::Polyline && !element.closed();
}

bool supportsFillColor(const DrawingElement &element)
{
    if (element.type() == ElementType::Rectangle || element.type() == ElementType::RoundedRectangle
        || element.type() == ElementType::Ellipse || element.type() == ElementType::Circle) {
        return true;
    }
    if (element.type() == ElementType::Bezier)
        return element.closed();
    return element.type() == ElementType::Polyline && element.closed();
}

QPointF pointOnEllipse(const QRectF &rect, int angleDegrees)
{
    const qreal radians = qDegreesToRadians(static_cast<qreal>(angleDegrees));
    const QPointF center = rect.center();
    return QPointF(
        center.x() + std::cos(radians) * rect.width() / 2.0,
        center.y() - std::sin(radians) * rect.height() / 2.0);
}

void drawArrowHead(QPainter &painter,
                   const QPointF &tip,
                   const QPointF &tail,
                   ArrowHead arrowHead,
                   const QColor &color,
                   qreal strokeWidth)
{
    if (arrowHead == ArrowHead::None)
        return;
    QLineF direction(tail, tip);
    if (direction.length() <= 0.5)
        return;
    direction.setLength(qMax<qreal>(12.0, strokeWidth * 4.0));

    const qreal angle = std::atan2(tip.y() - tail.y(), tip.x() - tail.x());
    const qreal sideAngle = qDegreesToRadians(28.0);
    const qreal length = direction.length();
    const QPointF left(
        tip.x() - std::cos(angle - sideAngle) * length,
        tip.y() - std::sin(angle - sideAngle) * length);
    const QPointF right(
        tip.x() - std::cos(angle + sideAngle) * length,
        tip.y() - std::sin(angle + sideAngle) * length);

    QPen pen(color, strokeWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    if (arrowHead == ArrowHead::Open) {
        painter.drawLine(tip, left);
        painter.drawLine(tip, right);
        return;
    }

    if (arrowHead == ArrowHead::Triangle) {
        painter.setBrush(color);
        painter.drawPolygon(QPolygonF{tip, left, right});
        return;
    }

    const QPointF back(
        tip.x() - std::cos(angle) * length * 1.35,
        tip.y() - std::sin(angle) * length * 1.35);
    painter.setBrush(Qt::NoBrush);
    painter.drawPolygon(QPolygonF{tip, left, back, right});
}

void drawArrowHeads(QPainter &painter, const DrawingElement &element)
{
    if (!supportsArrowHeads(element) || element.strokeWidth() <= 0)
        return;
    const QColor color = element.color();
    const qreal width = element.strokeWidth();
    if (element.type() == ElementType::Line || element.type() == ElementType::Polyline
        || element.type() == ElementType::Bezier) {
        const QVector<QPointF> points = element.points();
        if (points.size() < 2)
            return;
        drawArrowHead(painter, points.first(), points.at(1), element.startArrowHead(), color, width);
        drawArrowHead(painter, points.last(), points.at(points.size() - 2), element.endArrowHead(), color, width);
        return;
    }

    const QRectF rect = element.bounds();
    const int start = element.arcStartAngle();
    const int end = start + element.arcSpanAngle();
    const int step = element.arcSpanAngle() >= 0 ? 4 : -4;
    drawArrowHead(painter, pointOnEllipse(rect, start), pointOnEllipse(rect, start + step), element.startArrowHead(), color, width);
    drawArrowHead(painter, pointOnEllipse(rect, end), pointOnEllipse(rect, end - step), element.endArrowHead(), color, width);
}
}

QPainterPath DrawingRenderer::bezierPath(const QVector<QPointF> &points, bool closed)
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
            path.cubicTo(p1 + (p2 - p0) / 6.0, p2 - (p3 - p1) / 6.0, p2);
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
        path.cubicTo(p1 + (p2 - p0) / 6.0, p2 - (p3 - p1) / 6.0, p2);
    }
    return path;
}

QPainterPath DrawingRenderer::arcPath(const QRectF &rect, int startAngle, int spanAngle)
{
    QPainterPath path;
    if (rect.isEmpty())
        return path;
    path.moveTo(pointOnEllipse(rect, startAngle));
    path.arcTo(rect, startAngle, spanAngle);
    return path;
}

QPainterPath DrawingRenderer::roundedRectanglePath(const QRectF &rect, int radius)
{
    QPainterPath path;
    const QRectF normalized = rect.normalized();
    const qreal clampedRadius = qMin<qreal>(qMax(0, radius), qMin(normalized.width(), normalized.height()) / 2.0);
    path.addRoundedRect(normalized, clampedRadius, clampedRadius);
    return path;
}

void DrawingRenderer::drawElement(QPainter &painter, const DrawingElement &element)
{
    painter.save();
    const bool transformElement = !usesPointGeometry(element.type())
        && element.type() != ElementType::Circle && !qFuzzyIsNull(element.rotationDegrees());
    if (transformElement) {
        const QPointF center = element.bounds().center();
        painter.translate(center);
        painter.rotate(element.rotationDegrees());
        painter.translate(-center);
    }
    QPen pen(element.color(), element.strokeWidth(), toQtPenStyle(element.strokeStyle()), Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(element.strokeWidth() > 0 ? pen : QPen(Qt::NoPen));
    painter.setBrush((supportsFillColor(element) && element.fillColor().alpha() > 0)
                         ? QBrush(element.fillColor()) : QBrush(Qt::NoBrush));

    switch (element.type()) {
    case ElementType::Freehand: {
        const QVector<QPointF> points = element.points();
        if (!points.isEmpty()) {
            QPainterPath path(points.first());
            for (int i = 1; i < points.size(); ++i)
                path.lineTo(points.at(i));
            painter.drawPath(path);
        }
        break;
    }
    case ElementType::Line: {
        const QVector<QPointF> points = element.points();
        if (points.size() >= 2) {
            painter.drawLine(points.at(0), points.at(1));
            drawArrowHeads(painter, element);
        }
        break;
    }
    case ElementType::Polyline: {
        const QVector<QPointF> points = element.points();
        if (element.closed() && points.size() >= 3) {
            painter.drawPolygon(QPolygonF(points));
        } else {
            for (int i = 1; i < points.size(); ++i)
                painter.drawLine(points.at(i - 1), points.at(i));
        }
        drawArrowHeads(painter, element);
        break;
    }
    case ElementType::Bezier:
        if (element.points().size() >= 2) {
            painter.drawPath(bezierPath(element.points(), element.closed()));
            drawArrowHeads(painter, element);
        }
        break;
    case ElementType::Rectangle:
        painter.drawRect(element.rect());
        break;
    case ElementType::RoundedRectangle:
        painter.drawPath(roundedRectanglePath(element.rect(), element.cornerRadius()));
        break;
    case ElementType::Ellipse:
        painter.drawEllipse(element.rect());
        break;
    case ElementType::Arc:
        painter.drawArc(element.bounds(), element.arcStartAngle() * 16, element.arcSpanAngle() * 16);
        drawArrowHeads(painter, element);
        break;
    case ElementType::Circle:
        painter.drawEllipse(element.bounds());
        break;
    case ElementType::Text:
        painter.setFont(element.font());
        painter.setPen(QPen(element.color()));
        painter.setBrush(Qt::NoBrush);
        painter.drawText(element.rect(), Qt::AlignLeft | Qt::AlignTop, element.text());
        break;
    }
    painter.restore();
}
