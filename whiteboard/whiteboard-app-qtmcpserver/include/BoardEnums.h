#pragma once

#include <QString>

enum class Tool {
    Select,
    Pen,
    Eraser,
    Line,
    Rectangle,
    RoundedRectangle,
    Ellipse,
    Bezier,
    Arc,
    Circle,
    Polyline,
    Text
};

enum class ElementType {
    Freehand,
    Line,
    Rectangle,
    RoundedRectangle,
    Ellipse,
    Bezier,
    Arc,
    Circle,
    Polyline,
    Text
};

enum class ArrowHead {
    None,
    Triangle,
    Open,
    Diamond
};

enum class StrokeStyle {
    Solid,
    Dotted
};

QString toolToString(Tool tool);
Tool toolFromString(const QString &value);
QString elementTypeToString(ElementType type);
ElementType elementTypeFromString(const QString &value);
QString arrowHeadToString(ArrowHead arrowHead);
ArrowHead arrowHeadFromString(const QString &value);
QString strokeStyleToString(StrokeStyle strokeStyle);
StrokeStyle strokeStyleFromString(const QString &value);
