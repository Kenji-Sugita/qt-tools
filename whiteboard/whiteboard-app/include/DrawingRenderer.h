#pragma once

#include "DrawingElement.h"

#include <QPainterPath>

class QPainter;

class DrawingRenderer {
public:
    static void drawElement(QPainter &painter, const DrawingElement &element);
    static QPainterPath bezierPath(const QVector<QPointF> &points, bool closed = false);
    static QPainterPath arcPath(const QRectF &rect, int startAngle, int spanAngle);
    static QPainterPath roundedRectanglePath(const QRectF &rect, int radius);
};
