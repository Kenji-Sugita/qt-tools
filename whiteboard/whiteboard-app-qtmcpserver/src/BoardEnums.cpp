#include "BoardEnums.h"

#include <QString>

QString toolToString(Tool tool)
{
    switch (tool) {
    case Tool::Select:
        return QStringLiteral("select");
    case Tool::Pen:
        return QStringLiteral("pen");
    case Tool::Eraser:
        return QStringLiteral("eraser");
    case Tool::Line:
        return QStringLiteral("line");
    case Tool::Rectangle:
        return QStringLiteral("rectangle");
    case Tool::RoundedRectangle:
        return QStringLiteral("roundedRectangle");
    case Tool::Ellipse:
        return QStringLiteral("ellipse");
    case Tool::Bezier:
        return QStringLiteral("bezier");
    case Tool::Arc:
        return QStringLiteral("arc");
    case Tool::Circle:
        return QStringLiteral("circle");
    case Tool::Polyline:
        return QStringLiteral("polyline");
    case Tool::Text:
        return QStringLiteral("text");
    }
    return QStringLiteral("pen");
}

Tool toolFromString(const QString &value)
{
    if (value == QStringLiteral("select"))
        return Tool::Select;
    if (value == QStringLiteral("eraser"))
        return Tool::Eraser;
    if (value == QStringLiteral("line"))
        return Tool::Line;
    if (value == QStringLiteral("rectangle"))
        return Tool::Rectangle;
    if (value == QStringLiteral("roundedRectangle"))
        return Tool::RoundedRectangle;
    if (value == QStringLiteral("ellipse"))
        return Tool::Ellipse;
    if (value == QStringLiteral("bezier"))
        return Tool::Bezier;
    if (value == QStringLiteral("arc"))
        return Tool::Arc;
    if (value == QStringLiteral("circle"))
        return Tool::Circle;
    if (value == QStringLiteral("polyline"))
        return Tool::Polyline;
    if (value == QStringLiteral("text"))
        return Tool::Text;
    return Tool::Pen;
}

QString elementTypeToString(ElementType type)
{
    switch (type) {
    case ElementType::Freehand:
        return QStringLiteral("freehand");
    case ElementType::Line:
        return QStringLiteral("line");
    case ElementType::Rectangle:
        return QStringLiteral("rectangle");
    case ElementType::RoundedRectangle:
        return QStringLiteral("roundedRectangle");
    case ElementType::Ellipse:
        return QStringLiteral("ellipse");
    case ElementType::Bezier:
        return QStringLiteral("bezier");
    case ElementType::Arc:
        return QStringLiteral("arc");
    case ElementType::Circle:
        return QStringLiteral("circle");
    case ElementType::Polyline:
        return QStringLiteral("polyline");
    case ElementType::Text:
        return QStringLiteral("text");
    }
    return QStringLiteral("freehand");
}

ElementType elementTypeFromString(const QString &value)
{
    if (value == QStringLiteral("line"))
        return ElementType::Line;
    if (value == QStringLiteral("rectangle"))
        return ElementType::Rectangle;
    if (value == QStringLiteral("roundedRectangle"))
        return ElementType::RoundedRectangle;
    if (value == QStringLiteral("ellipse"))
        return ElementType::Ellipse;
    if (value == QStringLiteral("bezier"))
        return ElementType::Bezier;
    if (value == QStringLiteral("arc"))
        return ElementType::Arc;
    if (value == QStringLiteral("circle"))
        return ElementType::Circle;
    if (value == QStringLiteral("polyline"))
        return ElementType::Polyline;
    if (value == QStringLiteral("text"))
        return ElementType::Text;
    return ElementType::Freehand;
}

QString arrowHeadToString(ArrowHead arrowHead)
{
    switch (arrowHead) {
    case ArrowHead::None:
        return QStringLiteral("none");
    case ArrowHead::Triangle:
        return QStringLiteral("triangle");
    case ArrowHead::Open:
        return QStringLiteral("open");
    case ArrowHead::Diamond:
        return QStringLiteral("diamond");
    }
    return QStringLiteral("none");
}

ArrowHead arrowHeadFromString(const QString &value)
{
    if (value == QStringLiteral("triangle"))
        return ArrowHead::Triangle;
    if (value == QStringLiteral("open"))
        return ArrowHead::Open;
    if (value == QStringLiteral("diamond"))
        return ArrowHead::Diamond;
    return ArrowHead::None;
}

QString strokeStyleToString(StrokeStyle strokeStyle)
{
    switch (strokeStyle) {
    case StrokeStyle::Solid:
        return QStringLiteral("solid");
    case StrokeStyle::Dotted:
        return QStringLiteral("dotted");
    }
    return QStringLiteral("solid");
}

StrokeStyle strokeStyleFromString(const QString &value)
{
    if (value == QStringLiteral("dotted"))
        return StrokeStyle::Dotted;
    return StrokeStyle::Solid;
}
