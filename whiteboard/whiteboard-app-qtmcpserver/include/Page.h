#pragma once

#include "DrawingElement.h"

#include <QJsonObject>
#include <QPoint>
#include <QVector>

class Page {
public:
    int zoomPercent() const;
    void setZoomPercent(int zoomPercent);

    QPoint scrollPosition() const;
    void setScrollPosition(const QPoint &scrollPosition);

    bool locked() const;
    void setLocked(bool locked);

    QVector<DrawingElement> elements() const;
    void setElements(const QVector<DrawingElement> &elements);

    void addElement(const DrawingElement &element);
    QVector<int> addElements(const QVector<DrawingElement> &elements);
    bool eraseAt(const QPointF &point, qreal radius);
    bool removeElementAt(int index);
    bool removeElementsAt(const QVector<int> &indexes);
    int hitTest(const QPointF &point, qreal radius) const;
    QVector<int> hitTestRect(const QRectF &rect) const;
    DrawingElement *elementAt(int index);
    const DrawingElement *elementAt(int index) const;

    QJsonObject toJson() const;
    static Page fromJson(const QJsonObject &object);

private:
    int m_zoomPercent = 100;
    QPoint m_scrollPosition;
    bool m_locked = false;
    QVector<DrawingElement> m_elements;
};
