#pragma once

#include "BoardEnums.h"

#include <QColor>
#include <QByteArray>
#include <QFont>
#include <QJsonObject>
#include <QPointF>
#include <QRectF>
#include <QVector>

class DrawingElement {
public:
    QString id() const;
    void setId(const QString &id);

    ElementType type() const;
    void setType(ElementType type);

    QVector<QPointF> points() const;
    void setPoints(const QVector<QPointF> &points);

    QRectF rect() const;
    void setRect(const QRectF &rect);

    QString text() const;
    void setText(const QString &text);

    QByteArray imageData() const;
    void setImageData(const QByteArray &data);

    QString imageMimeType() const;
    void setImageMimeType(const QString &mimeType);

    QColor color() const;
    void setColor(const QColor &color);

    QColor fillColor() const;
    void setFillColor(const QColor &color);

    int strokeWidth() const;
    void setStrokeWidth(int strokeWidth);

    int cornerRadius() const;
    void setCornerRadius(int cornerRadius);

    StrokeStyle strokeStyle() const;
    void setStrokeStyle(StrokeStyle strokeStyle);

    QFont font() const;
    void setFont(const QFont &font);

    bool closed() const;
    void setClosed(bool closed);

    ArrowHead startArrowHead() const;
    void setStartArrowHead(ArrowHead arrowHead);

    ArrowHead endArrowHead() const;
    void setEndArrowHead(ArrowHead arrowHead);

    QString startConnectionElementId() const;
    void setStartConnectionElementId(const QString &id);

    QString endConnectionElementId() const;
    void setEndConnectionElementId(const QString &id);

    int arcStartAngle() const;
    void setArcStartAngle(int angle);

    int arcSpanAngle() const;
    void setArcSpanAngle(int angle);

    int groupId() const;
    void setGroupId(int groupId);

    qreal rotationDegrees() const;
    void setRotationDegrees(qreal degrees);

    QRectF bounds() const;
    QRectF visualBounds() const;
    QVector<QPointF> visualCorners() const;
    QPointF mapToVisual(const QPointF &point) const;
    QPointF mapFromVisual(const QPointF &point) const;
    void moveBy(const QPointF &delta);
    void resizeTo(const QRectF &rect);
    void rotateBy(qreal degrees, const QPointF &pivot);
    void scaleBy(qreal scaleFactor, const QPointF &pivot);
    bool intersects(const QPointF &point, qreal radius) const;
    QJsonObject toJson() const;
    static DrawingElement fromJson(const QJsonObject &object);

private:
    QString m_id;
    ElementType m_type = ElementType::Freehand;
    QVector<QPointF> m_points;
    QRectF m_rect;
    QString m_text;
    QByteArray m_imageData;
    QString m_imageMimeType;
    QColor m_color = QColor(QStringLiteral("#344054"));
    QColor m_fillColor = QColor(0, 0, 0, 0);
    int m_strokeWidth = 3;
    int m_cornerRadius = 24;
    StrokeStyle m_strokeStyle = StrokeStyle::Solid;
    QFont m_font = QFont(QStringLiteral("Sans Serif"), 18);
    bool m_closed = false;
    ArrowHead m_startArrowHead = ArrowHead::None;
    ArrowHead m_endArrowHead = ArrowHead::None;
    QString m_startConnectionElementId;
    QString m_endConnectionElementId;
    int m_arcStartAngle = 0;
    int m_arcSpanAngle = 120;
    int m_groupId = 0;
    qreal m_rotationDegrees = 0.0;
};
