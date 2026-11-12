#include "BoardSettings.h"

#include <QJsonObject>
#include <QtGlobal>

int BoardSettings::zoomPercent() const
{
    return m_zoomPercent;
}

void BoardSettings::setZoomPercent(int zoomPercent)
{
    m_zoomPercent = zoomPercent;
}

Tool BoardSettings::selectedTool() const
{
    return m_selectedTool;
}

void BoardSettings::setSelectedTool(Tool selectedTool)
{
    m_selectedTool = selectedTool;
}

QColor BoardSettings::selectedColor() const
{
    return m_selectedColor;
}

void BoardSettings::setSelectedColor(const QColor &selectedColor)
{
    if (selectedColor.isValid())
        m_selectedColor = selectedColor;
}

QColor BoardSettings::fillColor() const
{
    return m_fillColor;
}

void BoardSettings::setFillColor(const QColor &fillColor)
{
    if (fillColor.isValid())
        m_fillColor = fillColor;
}

int BoardSettings::strokeWidth() const
{
    return m_strokeWidth;
}

void BoardSettings::setStrokeWidth(int strokeWidth)
{
    m_strokeWidth = qMax(0, strokeWidth);
}

int BoardSettings::cornerRadius() const
{
    return m_cornerRadius;
}

void BoardSettings::setCornerRadius(int cornerRadius)
{
    m_cornerRadius = qBound(0, cornerRadius, 200);
}

StrokeStyle BoardSettings::strokeStyle() const
{
    return m_strokeStyle;
}

void BoardSettings::setStrokeStyle(StrokeStyle strokeStyle)
{
    m_strokeStyle = strokeStyle;
}

QFont BoardSettings::font() const
{
    return m_font;
}

void BoardSettings::setFont(const QFont &font)
{
    m_font = font;
}

QSize BoardSettings::windowSize() const
{
    return m_windowSize;
}

void BoardSettings::setWindowSize(const QSize &windowSize)
{
    if (windowSize.isValid())
        m_windowSize = QSize(qMax(800, windowSize.width()), qMax(600, windowSize.height()));
}

int BoardSettings::floatingActionDockY() const
{
    return m_floatingActionDockY;
}

void BoardSettings::setFloatingActionDockY(int dockY)
{
    m_floatingActionDockY = dockY;
}

ArrowHead BoardSettings::startArrowHead() const
{
    return m_startArrowHead;
}

void BoardSettings::setStartArrowHead(ArrowHead arrowHead)
{
    m_startArrowHead = arrowHead;
}

ArrowHead BoardSettings::endArrowHead() const
{
    return m_endArrowHead;
}

void BoardSettings::setEndArrowHead(ArrowHead arrowHead)
{
    m_endArrowHead = arrowHead;
}

QJsonObject BoardSettings::toJson() const
{
    return {
        {QStringLiteral("zoomPercent"), m_zoomPercent},
        {QStringLiteral("selectedTool"), toolToString(m_selectedTool)},
        {QStringLiteral("selectedColor"), m_selectedColor.name(QColor::HexArgb)},
        {QStringLiteral("fillColor"), m_fillColor.name(QColor::HexArgb)},
        {QStringLiteral("strokeWidth"), m_strokeWidth},
        {QStringLiteral("cornerRadius"), m_cornerRadius},
        {QStringLiteral("strokeStyle"), strokeStyleToString(m_strokeStyle)},
        {QStringLiteral("fontFamily"), m_font.family()},
        {QStringLiteral("fontPointSize"), m_font.pointSize()},
        {QStringLiteral("windowWidth"), m_windowSize.width()},
        {QStringLiteral("windowHeight"), m_windowSize.height()},
        {QStringLiteral("floatingActionDockY"), m_floatingActionDockY},
        {QStringLiteral("startArrowHead"), arrowHeadToString(m_startArrowHead)},
        {QStringLiteral("endArrowHead"), arrowHeadToString(m_endArrowHead)},
    };
}

BoardSettings BoardSettings::fromJson(const QJsonObject &object)
{
    BoardSettings settings;
    settings.setZoomPercent(object.value(QStringLiteral("zoomPercent")).toInt(100));
    settings.setSelectedTool(toolFromString(object.value(QStringLiteral("selectedTool")).toString()));
    settings.setSelectedColor(QColor(object.value(QStringLiteral("selectedColor")).toString(QStringLiteral("#ff344054"))));
    settings.setFillColor(QColor(object.value(QStringLiteral("fillColor")).toString(QStringLiteral("#00000000"))));
    settings.setStrokeWidth(object.value(QStringLiteral("strokeWidth")).toInt(3));
    settings.setCornerRadius(object.value(QStringLiteral("cornerRadius")).toInt(24));
    settings.setStrokeStyle(strokeStyleFromString(object.value(QStringLiteral("strokeStyle")).toString()));
    const QString family = object.value(QStringLiteral("fontFamily")).toString(QStringLiteral("Sans Serif"));
    const int pointSize = object.value(QStringLiteral("fontPointSize")).toInt(18);
    settings.setFont(QFont(family, pointSize));
    settings.setWindowSize(QSize(object.value(QStringLiteral("windowWidth")).toInt(1850),
                                 object.value(QStringLiteral("windowHeight")).toInt(900)));
    settings.setFloatingActionDockY(object.value(QStringLiteral("floatingActionDockY")).toInt(-1));
    settings.setStartArrowHead(arrowHeadFromString(object.value(QStringLiteral("startArrowHead")).toString()));
    settings.setEndArrowHead(arrowHeadFromString(object.value(QStringLiteral("endArrowHead")).toString()));
    return settings;
}
