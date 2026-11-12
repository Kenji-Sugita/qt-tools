#pragma once

#include "BoardEnums.h"

#include <QColor>
#include <QFont>
#include <QJsonObject>
#include <QSize>

class BoardSettings {
public:
    int zoomPercent() const;
    void setZoomPercent(int zoomPercent);

    Tool selectedTool() const;
    void setSelectedTool(Tool selectedTool);

    QColor selectedColor() const;
    void setSelectedColor(const QColor &selectedColor);

    QColor fillColor() const;
    void setFillColor(const QColor &fillColor);

    int strokeWidth() const;
    void setStrokeWidth(int strokeWidth);

    int cornerRadius() const;
    void setCornerRadius(int cornerRadius);

    StrokeStyle strokeStyle() const;
    void setStrokeStyle(StrokeStyle strokeStyle);

    QFont font() const;
    void setFont(const QFont &font);

    QSize windowSize() const;
    void setWindowSize(const QSize &windowSize);

    int floatingActionDockY() const;
    void setFloatingActionDockY(int dockY);

    ArrowHead startArrowHead() const;
    void setStartArrowHead(ArrowHead arrowHead);

    ArrowHead endArrowHead() const;
    void setEndArrowHead(ArrowHead arrowHead);

    QJsonObject toJson() const;
    static BoardSettings fromJson(const QJsonObject &object);

private:
    int m_zoomPercent = 100;
    Tool m_selectedTool = Tool::Pen;
    QColor m_selectedColor = QColor(QStringLiteral("#344054"));
    QColor m_fillColor = QColor(0, 0, 0, 0);
    int m_strokeWidth = 3;
    int m_cornerRadius = 24;
    StrokeStyle m_strokeStyle = StrokeStyle::Solid;
    QFont m_font = QFont(QStringLiteral("Sans Serif"), 18);
    QSize m_windowSize = QSize(1850, 900);
    int m_floatingActionDockY = -1;
    ArrowHead m_startArrowHead = ArrowHead::None;
    ArrowHead m_endArrowHead = ArrowHead::None;
};
