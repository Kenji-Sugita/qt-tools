#include "Page.h"

#include <QJsonArray>
#include <QtGlobal>
#include <algorithm>

int Page::zoomPercent() const
{
    return m_zoomPercent;
}

void Page::setZoomPercent(int zoomPercent)
{
    m_zoomPercent = qBound(10, zoomPercent, 400);
}

QPoint Page::scrollPosition() const
{
    return m_scrollPosition;
}

void Page::setScrollPosition(const QPoint &scrollPosition)
{
    m_scrollPosition = QPoint(qMax(0, scrollPosition.x()), qMax(0, scrollPosition.y()));
}

bool Page::locked() const
{
    return m_locked;
}

void Page::setLocked(bool locked)
{
    m_locked = locked;
}

QVector<DrawingElement> Page::elements() const
{
    return m_elements;
}

void Page::setElements(const QVector<DrawingElement> &elements)
{
    m_elements = elements;
}

void Page::addElement(const DrawingElement &element)
{
    m_elements.append(element);
}

QVector<int> Page::addElements(const QVector<DrawingElement> &elements)
{
    QVector<int> indexes;
    indexes.reserve(elements.size());
    for (const DrawingElement &element : elements) {
        indexes.append(m_elements.size());
        m_elements.append(element);
    }
    return indexes;
}

bool Page::eraseAt(const QPointF &point, qreal radius)
{
    for (int i = m_elements.size() - 1; i >= 0; --i) {
        if (m_elements.at(i).intersects(point, radius)) {
            m_elements.removeAt(i);
            return true;
        }
    }
    return false;
}

bool Page::removeElementAt(int index)
{
    if (index < 0 || index >= m_elements.size())
        return false;
    m_elements.removeAt(index);
    return true;
}

bool Page::removeElementsAt(const QVector<int> &indexes)
{
    QVector<int> sorted = indexes;
    std::sort(sorted.begin(), sorted.end(), std::greater<int>());
    bool removed = false;
    int last = -1;
    for (int index : sorted) {
        if (index == last || index < 0 || index >= m_elements.size())
            continue;
        m_elements.removeAt(index);
        last = index;
        removed = true;
    }
    return removed;
}

int Page::hitTest(const QPointF &point, qreal radius) const
{
    for (int i = m_elements.size() - 1; i >= 0; --i) {
        if (m_elements.at(i).intersects(point, radius))
            return i;
    }
    return -1;
}

QVector<int> Page::hitTestRect(const QRectF &rect) const
{
    const QRectF normalized = rect.normalized();
    QVector<int> indexes;
    for (int i = 0; i < m_elements.size(); ++i) {
        if (normalized.intersects(m_elements.at(i).visualBounds()))
            indexes.append(i);
    }
    return indexes;
}

DrawingElement *Page::elementAt(int index)
{
    if (index < 0 || index >= m_elements.size())
        return nullptr;
    return &m_elements[index];
}

const DrawingElement *Page::elementAt(int index) const
{
    if (index < 0 || index >= m_elements.size())
        return nullptr;
    return &m_elements[index];
}

QJsonObject Page::toJson() const
{
    QJsonArray elements;
    for (const DrawingElement &element : m_elements)
        elements.append(element.toJson());
    return {
        {QStringLiteral("zoomPercent"), m_zoomPercent},
        {QStringLiteral("scrollX"), m_scrollPosition.x()},
        {QStringLiteral("scrollY"), m_scrollPosition.y()},
        {QStringLiteral("locked"), m_locked},
        {QStringLiteral("elements"), elements},
    };
}

Page Page::fromJson(const QJsonObject &object)
{
    Page page;
    page.setZoomPercent(object.value(QStringLiteral("zoomPercent")).toInt(100));
    page.setScrollPosition(QPoint(object.value(QStringLiteral("scrollX")).toInt(0),
                                  object.value(QStringLiteral("scrollY")).toInt(0)));
    page.setLocked(object.value(QStringLiteral("locked")).toBool(false));
    QVector<DrawingElement> elements;
    const QJsonArray array = object.value(QStringLiteral("elements")).toArray();
    elements.reserve(array.size());
    for (const QJsonValue &value : array)
        elements.append(DrawingElement::fromJson(value.toObject()));
    page.setElements(elements);
    return page;
}
