#include "BoardModel.h"

#include <QFile>
#include <QFontMetricsF>
#include <QHash>
#include <QJsonDocument>
#include <QLineF>
#include <QIODevice>
#include <QSet>
#include <QStringList>
#include <QtGlobal>

#include <algorithm>
#include <limits>

namespace {
QRectF squareRect(const QRectF &rect)
{
    const QRectF normalized = rect.normalized();
    const qreal side = qMin(normalized.width(), normalized.height());
    return QRectF(normalized.topLeft(), QSizeF(side, side)).normalized();
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
    if (element.type() == ElementType::Rectangle || element.type() == ElementType::RoundedRectangle || element.type() == ElementType::Ellipse || element.type() == ElementType::Circle)
        return true;
    if (element.type() == ElementType::Bezier)
        return element.closed();
    return element.type() == ElementType::Polyline && element.closed();
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

QVector<int> validSelectedIndexes(const Page &page, const QVector<int> &selectedIndexes, int selectedElementIndex)
{
    QVector<int> indexes = !selectedIndexes.isEmpty() ? selectedIndexes : QVector<int>{selectedElementIndex};
    std::sort(indexes.begin(), indexes.end());
    indexes.erase(std::unique(indexes.begin(), indexes.end()), indexes.end());

    QVector<int> validIndexes;
    validIndexes.reserve(indexes.size());
    for (int index : indexes) {
        if (page.elementAt(index))
            validIndexes.append(index);
    }
    return validIndexes;
}
}

const BoardDocument &BoardModel::document() const
{
    return m_document;
}

BoardDocument &BoardModel::document()
{
    return m_document;
}

void BoardModel::setDocument(const BoardDocument &document)
{
    m_document = document;
    clearSelection();
}

BoardSettings BoardModel::settings() const
{
    BoardSettings settings = m_document.settings();
    settings.setZoomPercent(m_document.currentPage().zoomPercent());
    return settings;
}

Page BoardModel::currentPage() const
{
    return m_document.currentPage();
}

void BoardModel::selectTool(Tool tool)
{
    BoardSettings settings = m_document.settings();
    settings.setSelectedTool(tool);
    m_document.setSettings(settings);
    if (tool != Tool::Select && tool != Tool::Polyline)
        clearSelection();
}

void BoardModel::setSelectedColor(const QColor &color)
{
    BoardSettings settings = m_document.settings();
    settings.setSelectedColor(color);
    m_document.setSettings(settings);
}

void BoardModel::setFillColor(const QColor &color)
{
    BoardSettings settings = m_document.settings();
    settings.setFillColor(color);
    m_document.setSettings(settings);
}

void BoardModel::setStrokeWidth(int strokeWidth)
{
    BoardSettings settings = m_document.settings();
    settings.setStrokeWidth(strokeWidth);
    m_document.setSettings(settings);
}

void BoardModel::setCornerRadius(int cornerRadius)
{
    BoardSettings settings = m_document.settings();
    settings.setCornerRadius(cornerRadius);
    m_document.setSettings(settings);
}

void BoardModel::setStrokeStyle(StrokeStyle strokeStyle)
{
    BoardSettings settings = m_document.settings();
    settings.setStrokeStyle(strokeStyle);
    m_document.setSettings(settings);
}

void BoardModel::setFont(const QFont &font)
{
    BoardSettings settings = m_document.settings();
    settings.setFont(font);
    m_document.setSettings(settings);
}

void BoardModel::setWindowSize(const QSize &windowSize)
{
    BoardSettings settings = m_document.settings();
    settings.setWindowSize(windowSize);
    m_document.setSettings(settings);
}

void BoardModel::setFloatingActionDockY(int dockY)
{
    BoardSettings settings = m_document.settings();
    settings.setFloatingActionDockY(dockY);
    m_document.setSettings(settings);
}

void BoardModel::setStartArrowHead(ArrowHead arrowHead)
{
    BoardSettings settings = m_document.settings();
    settings.setStartArrowHead(arrowHead);
    m_document.setSettings(settings);
}

void BoardModel::setEndArrowHead(ArrowHead arrowHead)
{
    BoardSettings settings = m_document.settings();
    settings.setEndArrowHead(arrowHead);
    m_document.setSettings(settings);
}

void BoardModel::resetDefaultStyle()
{
    BoardSettings settings = m_document.settings();
    settings.setSelectedColor(DefaultColor);
    settings.setFillColor(DefaultFillColor);
    settings.setStrokeWidth(DefaultStrokeWidth);
    settings.setCornerRadius(DefaultCornerRadius);
    settings.setStrokeStyle(DefaultStrokeStyle);
    settings.setFont(DefaultFont);
    settings.setStartArrowHead(DefaultStartArrowHead);
    settings.setEndArrowHead(DefaultEndArrowHead);
    m_document.setSettings(settings);
}

int BoardModel::setZoomPercent(int zoomPercent)
{
    const int clamped = qBound(MinZoomPercent, zoomPercent, MaxZoomPercent);
    m_document.currentPage().setZoomPercent(clamped);
    return clamped;
}

void BoardModel::addFreehand(const QVector<QPointF> &points)
{
    if (currentPageLocked())
        return;
    if (points.isEmpty())
        return;
    clearSelection();
    pushUndoState();
    DrawingElement element = makeElement(ElementType::Freehand);
    element.setPoints(points);
    m_document.currentPage().addElement(element);
    clearRedoState();
}

void BoardModel::addLine(const QPointF &start, const QPointF &end)
{
    if (currentPageLocked())
        return;
    clearSelection();
    pushUndoState();
    DrawingElement element = makeElement(ElementType::Line);
    element.setPoints({start, end});
    m_document.currentPage().addElement(element);
    clearRedoState();
}

void BoardModel::addPolyline(const QVector<QPointF> &points, bool closed)
{
    if (currentPageLocked())
        return;
    if (points.size() < 2 || (closed && points.size() < 3))
        return;
    clearSelection();
    pushUndoState();
    DrawingElement element = makeElement(ElementType::Polyline);
    element.setPoints(points);
    element.setClosed(closed);
    m_document.currentPage().addElement(element);
    clearRedoState();
}

void BoardModel::addBezier(const QVector<QPointF> &points, bool closed)
{
    if (currentPageLocked())
        return;
    if (points.size() < 2 || (closed && points.size() < 3))
        return;
    clearSelection();
    pushUndoState();
    DrawingElement element = makeElement(ElementType::Bezier);
    element.setPoints(points);
    element.setClosed(closed);
    if (closed) {
        element.setStartArrowHead(ArrowHead::None);
        element.setEndArrowHead(ArrowHead::None);
    }
    m_document.currentPage().addElement(element);
    clearRedoState();
}

void BoardModel::addBezier(const QPointF &start, const QPointF &end)
{
    addBezier({start, end});
}

void BoardModel::addArc(const QRectF &rect)
{
    if (currentPageLocked())
        return;
    clearSelection();
    pushUndoState();
    DrawingElement element = makeElement(ElementType::Arc);
    element.setRect(squareRect(rect));
    element.setArcStartAngle(30);
    element.setArcSpanAngle(240);
    m_document.currentPage().addElement(element);
    clearRedoState();
}

void BoardModel::addCircle(const QRectF &rect)
{
    if (currentPageLocked())
        return;
    clearSelection();
    pushUndoState();
    DrawingElement element = makeElement(ElementType::Circle);
    element.setRect(squareRect(rect));
    m_document.currentPage().addElement(element);
    clearRedoState();
}

void BoardModel::addRectangle(const QRectF &rect)
{
    if (currentPageLocked())
        return;
    clearSelection();
    pushUndoState();
    DrawingElement element = makeElement(ElementType::Rectangle);
    element.setRect(rect);
    m_document.currentPage().addElement(element);
    clearRedoState();
}

void BoardModel::addRoundedRectangle(const QRectF &rect)
{
    if (currentPageLocked())
        return;
    clearSelection();
    pushUndoState();
    DrawingElement element = makeElement(ElementType::RoundedRectangle);
    element.setRect(rect);
    element.setCornerRadius(m_document.settings().cornerRadius());
    m_document.currentPage().addElement(element);
    clearRedoState();
}

void BoardModel::addEllipse(const QRectF &rect)
{
    if (currentPageLocked())
        return;
    clearSelection();
    pushUndoState();
    DrawingElement element = makeElement(ElementType::Ellipse);
    element.setRect(rect);
    m_document.currentPage().addElement(element);
    clearRedoState();
}

void BoardModel::addText(const QPointF &position, const QString &text)
{
    if (currentPageLocked())
        return;
    if (text.trimmed().isEmpty())
        return;
    clearSelection();
    pushUndoState();
    DrawingElement element = makeElement(ElementType::Text);
    element.setText(text);
    element.setRect(QRectF(position, textSize(text, element.font())));
    m_document.currentPage().addElement(element);
    clearRedoState();
}

bool BoardModel::eraseAt(const QPointF &point)
{
    if (currentPageLocked())
        return false;
    clearSelection();
    pushUndoState();
    const bool erased = m_document.currentPage().eraseAt(point, qMax(8, m_document.settings().strokeWidth() * 3));
    if (erased) {
        clearRedoState();
    } else if (!m_undoStack.isEmpty()) {
        m_undoStack.removeLast();
    }
    return erased;
}

int BoardModel::selectedElementIndex() const
{
    return m_selectedElementIndex;
}

QVector<int> BoardModel::selectedElementIndexes() const
{
    return m_selectedElementIndexes;
}

bool BoardModel::hasSelection() const
{
    for (int index : m_selectedElementIndexes) {
        if (m_document.currentPage().elementAt(index))
            return true;
    }
    return m_document.currentPage().elementAt(m_selectedElementIndex) != nullptr;
}

QRectF BoardModel::selectedElementBounds() const
{
    QRectF bounds;
    bool hasBounds = false;
    for (int index : m_selectedElementIndexes) {
        const DrawingElement *element = m_document.currentPage().elementAt(index);
        if (!element)
            continue;
        bounds = hasBounds ? bounds.united(element->visualBounds()) : element->visualBounds();
        hasBounds = true;
    }
    if (hasBounds)
        return bounds;
    const DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    return element ? element->visualBounds() : QRectF();
}

QRectF BoardModel::selectedElementGeometryBounds() const
{
    if (m_selectedElementIndexes.size() > 1)
        return QRectF();
    const DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    return element ? element->bounds() : QRectF();
}

QVector<QRectF> BoardModel::selectedElementBoundsList() const
{
    QVector<QRectF> bounds;
    for (int index : m_selectedElementIndexes) {
        const DrawingElement *element = m_document.currentPage().elementAt(index);
        if (element)
            bounds.append(element->visualBounds());
    }
    if (bounds.isEmpty()) {
        const DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
        if (element)
            bounds.append(element->visualBounds());
    }
    return bounds;
}

QVector<QPointF> BoardModel::selectedElementVisualCorners() const
{
    if (m_selectedElementIndexes.size() > 1)
        return {};
    const DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    return element ? element->visualCorners() : QVector<QPointF>();
}

QVector<QPointF> BoardModel::selectedElementPoints() const
{
    const DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    return element ? element->points() : QVector<QPointF>();
}

qreal BoardModel::selectedElementRotationDegrees() const
{
    if (m_selectedElementIndexes.size() > 1)
        return 0.0;
    const DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    return element ? element->rotationDegrees() : 0.0;
}

bool BoardModel::selectedElementsContain(const QPointF &point, qreal radius) const
{
    const QVector<int> indexes = validSelectedIndexes(m_document.currentPage(), selectedIndexesOrFocused(), m_selectedElementIndex);
    for (int index : indexes) {
        const DrawingElement *element = m_document.currentPage().elementAt(index);
        if (element && element->intersects(point, radius))
            return true;
    }
    return false;
}

bool BoardModel::selectionCanRotate() const
{
    const QVector<int> indexes = validSelectedIndexes(m_document.currentPage(), selectedIndexesOrFocused(), m_selectedElementIndex);
    if (indexes.isEmpty())
        return false;
    if (indexes.size() != 1)
        return true;
    const DrawingElement *element = m_document.currentPage().elementAt(indexes.first());
    return element && element->type() != ElementType::Circle;
}

QString BoardModel::selectedElementText() const
{
    const DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    return element ? element->text() : QString();
}

QColor BoardModel::selectedElementColor() const
{
    const DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    return element ? element->color() : m_document.settings().selectedColor();
}

QColor BoardModel::selectedElementFillColor() const
{
    const DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    return element ? element->fillColor() : m_document.settings().fillColor();
}

int BoardModel::selectedElementStrokeWidth() const
{
    const DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    return element ? element->strokeWidth() : m_document.settings().strokeWidth();
}

int BoardModel::selectedElementCornerRadius() const
{
    const DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    return element ? element->cornerRadius() : m_document.settings().cornerRadius();
}

StrokeStyle BoardModel::selectedElementStrokeStyle() const
{
    const DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    return element ? element->strokeStyle() : m_document.settings().strokeStyle();
}

QFont BoardModel::selectedElementFont() const
{
    const DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    return element ? element->font() : m_document.settings().font();
}

ElementType BoardModel::selectedElementType() const
{
    const DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    return element ? element->type() : ElementType::Freehand;
}

bool BoardModel::selectedElementClosed() const
{
    const DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    return element ? element->closed() : false;
}

bool BoardModel::selectedSelectionHasGroup() const
{
    const QVector<int> indexes = selectedIndexesOrFocused();
    for (int index : indexes) {
        const DrawingElement *element = m_document.currentPage().elementAt(index);
        if (element && element->groupId() > 0)
            return true;
    }
    return false;
}

ArrowHead BoardModel::selectedElementStartArrowHead() const
{
    const DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    return element ? element->startArrowHead() : m_document.settings().startArrowHead();
}

ArrowHead BoardModel::selectedElementEndArrowHead() const
{
    const DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    return element ? element->endArrowHead() : m_document.settings().endArrowHead();
}

int BoardModel::selectedElementArcStartAngle() const
{
    const DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    return element ? element->arcStartAngle() : 0;
}

int BoardModel::selectedElementArcSpanAngle() const
{
    const DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    return element ? element->arcSpanAngle() : 120;
}

void BoardModel::clearSelection()
{
    m_selectedElementIndex = -1;
    m_selectedElementIndexes.clear();
    m_selectionEditActive = false;
    m_selectionEditSnapshotPushed = false;
}

bool BoardModel::selectAt(const QPointF &point)
{
    m_selectionEditActive = false;
    m_selectedElementIndex = m_document.currentPage().hitTest(point, qMax(12, m_document.settings().strokeWidth() * 4));
    m_selectedElementIndexes = m_selectedElementIndex >= 0 ? QVector<int>{m_selectedElementIndex} : QVector<int>();
    expandSelectionForGroups();
    return m_selectedElementIndex >= 0;
}

bool BoardModel::addToSelectionAt(const QPointF &point)
{
    m_selectionEditActive = false;
    m_selectionEditSnapshotPushed = false;
    const int index = m_document.currentPage().hitTest(point, qMax(12, m_document.settings().strokeWidth() * 4));
    if (index < 0)
        return false;
    if (!m_selectedElementIndexes.contains(index))
        m_selectedElementIndexes.append(index);
    m_selectedElementIndex = index;
    expandSelectionForGroups();
    return true;
}

bool BoardModel::selectInRect(const QRectF &rect)
{
    m_selectionEditActive = false;
    m_selectionEditSnapshotPushed = false;
    m_selectedElementIndexes = m_document.currentPage().hitTestRect(rect);
    m_selectedElementIndex = m_selectedElementIndexes.isEmpty() ? -1 : m_selectedElementIndexes.last();
    expandSelectionForGroups();
    return !m_selectedElementIndexes.isEmpty();
}

int BoardModel::selectOpenPolylineEndpointAt(const QPointF &point, qreal radius)
{
    const QVector<DrawingElement> elements = m_document.currentPage().elements();
    for (int i = elements.size() - 1; i >= 0; --i) {
        const DrawingElement &element = elements.at(i);
        const QVector<QPointF> points = element.points();
        if (element.type() != ElementType::Polyline || element.closed() || points.size() < 2)
            continue;

        const qreal startDistance = QLineF(points.first(), point).length();
        const qreal endDistance = QLineF(points.last(), point).length();
        const bool hitStart = startDistance <= radius;
        const bool hitEnd = endDistance <= radius;
        if (!hitStart && !hitEnd)
            continue;

        m_selectionEditActive = false;
        m_selectionEditSnapshotPushed = false;
        m_selectedElementIndex = i;
        m_selectedElementIndexes = {i};
        return (hitStart && (!hitEnd || startDistance <= endDistance)) ? 0 : points.size() - 1;
    }
    return -1;
}

int BoardModel::selectBezierEndpointAt(const QPointF &point, qreal radius)
{
    const QVector<DrawingElement> elements = m_document.currentPage().elements();
    for (int i = elements.size() - 1; i >= 0; --i) {
        const DrawingElement &element = elements.at(i);
        const QVector<QPointF> points = element.points();
        if (element.type() != ElementType::Bezier || element.closed() || points.size() < 2)
            continue;

        const qreal startDistance = QLineF(points.first(), point).length();
        const qreal endDistance = QLineF(points.last(), point).length();
        const bool hitStart = startDistance <= radius;
        const bool hitEnd = endDistance <= radius;
        if (!hitStart && !hitEnd)
            continue;

        m_selectionEditActive = false;
        m_selectionEditSnapshotPushed = false;
        m_selectedElementIndex = i;
        m_selectedElementIndexes = {i};
        return (hitStart && (!hitEnd || startDistance <= endDistance)) ? 0 : points.size() - 1;
    }
    return -1;
}

int BoardModel::selectClosedPolylineVertexAt(const QPointF &point, qreal radius)
{
    const QVector<DrawingElement> elements = m_document.currentPage().elements();
    for (int i = elements.size() - 1; i >= 0; --i) {
        const DrawingElement &element = elements.at(i);
        const QVector<QPointF> points = element.points();
        if (element.type() != ElementType::Polyline || !element.closed() || points.size() < 3)
            continue;

        int hitVertex = -1;
        qreal bestDistance = radius;
        for (int j = 0; j < points.size(); ++j) {
            const qreal distance = QLineF(points.at(j), point).length();
            if (distance <= bestDistance) {
                bestDistance = distance;
                hitVertex = j;
            }
        }
        if (hitVertex < 0)
            continue;

        m_selectionEditActive = false;
        m_selectionEditSnapshotPushed = false;
        m_selectedElementIndex = i;
        m_selectedElementIndexes = {i};
        return hitVertex;
    }
    return -1;
}

void BoardModel::beginSelectionEdit()
{
    if (currentPageLocked())
        return;
    if (!hasSelection() || m_selectionEditActive)
        return;
    m_selectionEditActive = true;
    m_selectionEditSnapshotPushed = false;
}

bool BoardModel::beginRotationEdit(const QPointF &pivot)
{
    if (currentPageLocked() || !selectionCanRotate())
        return false;
    const QVector<int> indexes = validSelectedIndexes(m_document.currentPage(), selectedIndexesOrFocused(), m_selectedElementIndex);
    if (indexes.isEmpty())
        return false;

    beginSelectionEdit();
    m_rotationPivot = pivot;
    m_rotationElementIndexes = indexes;
    m_rotationStartElements.clear();
    m_rotationStartElements.reserve(indexes.size());
    for (int index : indexes) {
        const DrawingElement *element = m_document.currentPage().elementAt(index);
        if (element)
            m_rotationStartElements.append(*element);
    }
    m_rotationEditActive = m_rotationStartElements.size() == m_rotationElementIndexes.size();
    return m_rotationEditActive;
}

void BoardModel::endSelectionEdit()
{
    m_selectionEditActive = false;
    m_selectionEditSnapshotPushed = false;
    m_rotationEditActive = false;
    m_rotationElementIndexes.clear();
    m_rotationStartElements.clear();
}

bool BoardModel::moveSelectedBy(const QPointF &delta)
{
    if (currentPageLocked())
        return false;
    if (!hasSelection())
        return false;
    if (qFuzzyIsNull(delta.x()) && qFuzzyIsNull(delta.y()))
        return false;
    if (!m_selectionEditActive || !m_selectionEditSnapshotPushed) {
        pushUndoState();
        clearRedoState();
        m_selectionEditSnapshotPushed = m_selectionEditActive;
    }
    bool moved = false;
    const QVector<int> indexes = selectedIndexesOrFocused();
    for (int index : indexes) {
        DrawingElement *element = m_document.currentPage().elementAt(index);
        if (!element)
            continue;
        element->moveBy(delta);
        moved = true;
    }
    return moved;
}

bool BoardModel::resizeSelectedTo(const QRectF &rect)
{
    if (currentPageLocked())
        return false;
    if (m_selectedElementIndexes.size() > 1)
        return false;
    const DrawingElement *currentElement = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!currentElement)
        return false;
    if (rect.normalized() == currentElement->bounds())
        return false;
    if (!m_selectionEditActive || !m_selectionEditSnapshotPushed) {
        pushUndoState();
        clearRedoState();
        m_selectionEditSnapshotPushed = m_selectionEditActive;
    }
    DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!element)
        return false;
    element->resizeTo(rect);
    return true;
}

bool BoardModel::scaleSelectedBy(qreal scaleFactor)
{
    if (currentPageLocked())
        return false;
    if (scaleFactor <= 0.0)
        return false;

    Page &page = m_document.currentPage();
    const QVector<int> indexes = validSelectedIndexes(page, selectedIndexesOrFocused(), m_selectedElementIndex);
    if (indexes.isEmpty())
        return false;

    if (indexes.size() == 1) {
        const DrawingElement *currentElement = page.elementAt(indexes.first());
        if (!currentElement || currentElement->type() == ElementType::Text)
            return false;
    }

    const QRectF bounds = selectedElementBounds();
    if (bounds.width() <= 1.0 || bounds.height() <= 1.0)
        return false;
    const QSizeF scaledSize(bounds.width() * scaleFactor, bounds.height() * scaleFactor);
    if (scaledSize.width() <= 1.0 || scaledSize.height() <= 1.0)
        return false;
    pushUndoState();
    clearRedoState();

    for (int index : indexes) {
        DrawingElement *element = m_document.currentPage().elementAt(index);
        if (!element)
            continue;
        element->scaleBy(scaleFactor, bounds.center());
        if (element->type() == ElementType::Text) {
            QFont font = element->font();
            font.setPointSize(qMax(1, qRound(font.pointSizeF() * scaleFactor)));
            element->setFont(font);
        }
    }
    return true;
}

bool BoardModel::rotateSelectionTo(qreal degrees)
{
    if (currentPageLocked() || !m_rotationEditActive)
        return false;
    if (qFuzzyIsNull(degrees) && !m_selectionEditSnapshotPushed)
        return false;
    if (!m_selectionEditActive || !m_selectionEditSnapshotPushed) {
        pushUndoState();
        clearRedoState();
        m_selectionEditSnapshotPushed = m_selectionEditActive;
    }

    bool rotated = false;
    for (qsizetype i = 0; i < m_rotationElementIndexes.size(); ++i) {
        DrawingElement *element = m_document.currentPage().elementAt(m_rotationElementIndexes.at(i));
        if (!element)
            continue;
        *element = m_rotationStartElements.at(i);
        element->rotateBy(degrees, m_rotationPivot);
        rotated = true;
    }
    return rotated;
}

bool BoardModel::resetSelectedRotation()
{
    if (currentPageLocked() || m_selectedElementIndexes.size() > 1)
        return false;
    DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!element || element->type() == ElementType::Circle || qFuzzyIsNull(element->rotationDegrees()))
        return false;
    pushUndoState();
    clearRedoState();
    element->rotateBy(-element->rotationDegrees(), element->bounds().center());
    return true;
}

bool BoardModel::moveSelectedVertexTo(int vertexIndex, const QPointF &point)
{
    if (currentPageLocked())
        return false;
    const DrawingElement *currentElement = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!currentElement || (currentElement->type() != ElementType::Line && currentElement->type() != ElementType::Polyline && currentElement->type() != ElementType::Bezier) || vertexIndex < 0 || vertexIndex >= currentElement->points().size())
        return false;
    if (QLineF(currentElement->points().at(vertexIndex), point).length() <= 0.5)
        return false;
    if (!m_selectionEditActive || !m_selectionEditSnapshotPushed) {
        pushUndoState();
        clearRedoState();
        m_selectionEditSnapshotPushed = m_selectionEditActive;
    }
    DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!element)
        return false;
    QVector<QPointF> points = element->points();
    points[vertexIndex] = point;
    element->setPoints(points);
    return true;
}

bool BoardModel::updateSelectedArcAngles(int startAngle, int spanAngle)
{
    if (currentPageLocked())
        return false;
    const DrawingElement *currentElement = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!currentElement || currentElement->type() != ElementType::Arc)
        return false;
    if (currentElement->arcStartAngle() == startAngle && currentElement->arcSpanAngle() == spanAngle)
        return false;
    if (!m_selectionEditActive || !m_selectionEditSnapshotPushed) {
        pushUndoState();
        clearRedoState();
        m_selectionEditSnapshotPushed = m_selectionEditActive;
    }
    DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!element)
        return false;
    element->setArcStartAngle(startAngle);
    element->setArcSpanAngle(spanAngle);
    return true;
}

bool BoardModel::updateSelectedArrowHeads(ArrowHead startArrowHead, ArrowHead endArrowHead)
{
    if (currentPageLocked())
        return false;
    const DrawingElement *currentElement = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!currentElement || !supportsArrowHeads(*currentElement))
        return false;
    if (currentElement->startArrowHead() == startArrowHead && currentElement->endArrowHead() == endArrowHead)
        return false;
    pushUndoState();
    clearRedoState();
    DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!element)
        return false;
    element->setStartArrowHead(startArrowHead);
    element->setEndArrowHead(endArrowHead);
    return true;
}

bool BoardModel::updateSelectedPolyline(const QVector<QPointF> &points, bool closed)
{
    if (currentPageLocked())
        return false;
    const DrawingElement *currentElement = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!currentElement || currentElement->type() != ElementType::Polyline || points.size() < 2 || (closed && points.size() < 3))
        return false;
    if (currentElement->points() == points && currentElement->closed() == closed)
        return false;

    pushUndoState();
    clearRedoState();
    DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!element)
        return false;
    element->setPoints(points);
    element->setClosed(closed);
    return true;
}

bool BoardModel::insertSelectedPolylinePoint(const QPointF &point, qreal hitRadius)
{
    if (currentPageLocked())
        return false;
    if (m_selectedElementIndexes.size() > 1)
        return false;
    const DrawingElement *currentElement = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!currentElement || currentElement->type() != ElementType::Polyline)
        return false;
    const QVector<QPointF> currentPoints = currentElement->points();
    if (currentPoints.size() < 2)
        return false;

    const bool closed = currentElement->closed();
    const int segmentCount = closed ? currentPoints.size() : currentPoints.size() - 1;
    int insertIndex = -1;
    qreal bestDistance = std::numeric_limits<qreal>::max();
    for (int i = 0; i < segmentCount; ++i) {
        const QPointF start = currentPoints.at(i);
        const QPointF end = currentPoints.at((i + 1) % currentPoints.size());
        const qreal distance = distanceToSegment(point, start, end);
        if (distance < bestDistance) {
            bestDistance = distance;
            insertIndex = i + 1;
        }
    }
    if (insertIndex < 0 || bestDistance > hitRadius)
        return false;

    pushUndoState();
    clearRedoState();
    DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!element)
        return false;
    QVector<QPointF> points = element->points();
    points.insert(qMin(insertIndex, points.size()), point);
    element->setPoints(points);
    return true;
}

bool BoardModel::deleteSelectedPolylinePoint(int pointIndex)
{
    if (currentPageLocked())
        return false;
    if (m_selectedElementIndexes.size() > 1)
        return false;
    const DrawingElement *currentElement = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!currentElement || currentElement->type() != ElementType::Polyline || pointIndex < 0 || pointIndex >= currentElement->points().size())
        return false;
    const int minimumPointCount = currentElement->closed() ? 3 : 2;
    if (currentElement->points().size() <= minimumPointCount)
        return false;

    pushUndoState();
    clearRedoState();
    DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!element)
        return false;
    QVector<QPointF> points = element->points();
    points.removeAt(pointIndex);
    element->setPoints(points);
    return true;
}

bool BoardModel::updateSelectedBezier(const QVector<QPointF> &points, bool closed)
{
    if (currentPageLocked())
        return false;
    const DrawingElement *currentElement = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!currentElement || currentElement->type() != ElementType::Bezier || points.size() < 2 || (closed && points.size() < 3))
        return false;
    if (currentElement->points() == points && currentElement->closed() == closed)
        return false;

    pushUndoState();
    clearRedoState();
    DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!element)
        return false;
    element->setPoints(points);
    element->setClosed(closed);
    if (closed) {
        element->setStartArrowHead(ArrowHead::None);
        element->setEndArrowHead(ArrowHead::None);
    }
    return true;
}

bool BoardModel::insertSelectedBezierControlPoint(const QPointF &point, qreal hitRadius)
{
    if (currentPageLocked())
        return false;
    if (m_selectedElementIndexes.size() > 1)
        return false;
    const DrawingElement *currentElement = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!currentElement || currentElement->type() != ElementType::Bezier)
        return false;
    const QVector<QPointF> currentPoints = currentElement->points();
    if (currentPoints.size() < 2)
        return false;

    const bool closed = currentElement->closed();
    const int segmentCount = closed ? currentPoints.size() : currentPoints.size() - 1;
    int insertIndex = -1;
    qreal bestDistance = std::numeric_limits<qreal>::max();
    for (int i = 0; i < segmentCount; ++i) {
        const QPointF start = currentPoints.at(i);
        const QPointF end = currentPoints.at((i + 1) % currentPoints.size());
        const qreal distance = distanceToSegment(point, start, end);
        if (distance < bestDistance) {
            bestDistance = distance;
            insertIndex = i + 1;
        }
    }
    if (insertIndex < 0 || bestDistance > hitRadius)
        return false;

    pushUndoState();
    clearRedoState();
    DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!element)
        return false;
    QVector<QPointF> points = element->points();
    points.insert(qMin(insertIndex, points.size()), point);
    element->setPoints(points);
    return true;
}

bool BoardModel::deleteSelectedBezierControlPoint(int pointIndex)
{
    if (currentPageLocked())
        return false;
    if (m_selectedElementIndexes.size() > 1)
        return false;
    const DrawingElement *currentElement = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!currentElement || currentElement->type() != ElementType::Bezier || pointIndex < 0 || pointIndex >= currentElement->points().size())
        return false;
    const int minimumPointCount = currentElement->closed() ? 3 : 2;
    if (currentElement->points().size() <= minimumPointCount)
        return false;

    pushUndoState();
    clearRedoState();
    DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!element)
        return false;
    QVector<QPointF> points = element->points();
    points.removeAt(pointIndex);
    element->setPoints(points);
    return true;
}

bool BoardModel::closeSelectedPolyline()
{
    if (currentPageLocked())
        return false;
    const DrawingElement *currentElement = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!currentElement || currentElement->type() != ElementType::Polyline || currentElement->closed() || currentElement->points().size() < 3)
        return false;

    pushUndoState();
    clearRedoState();
    DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!element)
        return false;
    element->setClosed(true);
    return true;
}

bool BoardModel::openSelectedPolyline()
{
    if (currentPageLocked())
        return false;
    const DrawingElement *currentElement = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!currentElement || currentElement->type() != ElementType::Polyline || !currentElement->closed())
        return false;

    pushUndoState();
    clearRedoState();
    DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!element)
        return false;
    element->setClosed(false);
    return true;
}

bool BoardModel::openSelectedPolylineAtVertex(int vertexIndex)
{
    if (currentPageLocked())
        return false;
    const DrawingElement *currentElement = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!currentElement || currentElement->type() != ElementType::Polyline || !currentElement->closed())
        return false;

    const QVector<QPointF> points = currentElement->points();
    if (vertexIndex < 0 || vertexIndex >= points.size())
        return false;

    pushUndoState();
    clearRedoState();
    DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!element)
        return false;

    QVector<QPointF> openedPoints;
    openedPoints.reserve(points.size());
    for (int i = 0; i < points.size(); ++i)
        openedPoints.append(points.at((vertexIndex + i) % points.size()));
    element->setPoints(openedPoints);
    element->setClosed(false);
    return true;
}

bool BoardModel::closeSelectedBezier()
{
    if (currentPageLocked())
        return false;
    const DrawingElement *currentElement = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!currentElement || currentElement->type() != ElementType::Bezier || currentElement->closed() || currentElement->points().size() < 3)
        return false;

    pushUndoState();
    clearRedoState();
    DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!element)
        return false;
    element->setClosed(true);
    element->setStartArrowHead(ArrowHead::None);
    element->setEndArrowHead(ArrowHead::None);
    return true;
}

bool BoardModel::openSelectedBezier()
{
    if (currentPageLocked())
        return false;
    const DrawingElement *currentElement = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!currentElement || currentElement->type() != ElementType::Bezier || !currentElement->closed())
        return false;

    pushUndoState();
    clearRedoState();
    DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!element)
        return false;
    element->setClosed(false);
    return true;
}

bool BoardModel::deleteSelectedElement()
{
    if (currentPageLocked())
        return false;
    if (!hasSelection())
        return false;
    pushUndoState();
    clearRedoState();
    const QVector<int> indexes = selectedIndexesOrFocused();
    const bool removed = m_document.currentPage().removeElementsAt(indexes);
    clearSelection();
    return removed;
}

bool BoardModel::moveSelectedForward()
{
    if (currentPageLocked())
        return false;
    Page &page = m_document.currentPage();
    QVector<int> selected = validSelectedIndexes(page, selectedIndexesOrFocused(), m_selectedElementIndex);
    if (selected.isEmpty())
        return false;

    QVector<DrawingElement> elements = page.elements();
    QVector<int> moving = selected;
    std::sort(moving.begin(), moving.end(), [](int lhs, int rhs) { return lhs > rhs; });
    int focusedIndex = m_selectedElementIndex;
    bool changed = false;

    for (int index : moving) {
        if (index + 1 >= elements.size() || selected.contains(index + 1))
            continue;
        std::swap(elements[index], elements[index + 1]);
        selected.removeOne(index);
        selected.append(index + 1);
        if (focusedIndex == index)
            focusedIndex = index + 1;
        changed = true;
    }
    if (!changed)
        return false;

    std::sort(selected.begin(), selected.end());
    pushUndoState();
    clearRedoState();
    m_document.currentPage().setElements(elements);
    m_selectedElementIndexes = selected;
    m_selectedElementIndex = selected.contains(focusedIndex) ? focusedIndex : selected.last();
    return true;
}

bool BoardModel::moveSelectedBackward()
{
    if (currentPageLocked())
        return false;
    Page &page = m_document.currentPage();
    QVector<int> selected = validSelectedIndexes(page, selectedIndexesOrFocused(), m_selectedElementIndex);
    if (selected.isEmpty())
        return false;

    QVector<DrawingElement> elements = page.elements();
    QVector<int> moving = selected;
    std::sort(moving.begin(), moving.end());
    int focusedIndex = m_selectedElementIndex;
    bool changed = false;

    for (int index : moving) {
        if (index <= 0 || selected.contains(index - 1))
            continue;
        std::swap(elements[index], elements[index - 1]);
        selected.removeOne(index);
        selected.append(index - 1);
        if (focusedIndex == index)
            focusedIndex = index - 1;
        changed = true;
    }
    if (!changed)
        return false;

    std::sort(selected.begin(), selected.end());
    pushUndoState();
    clearRedoState();
    m_document.currentPage().setElements(elements);
    m_selectedElementIndexes = selected;
    m_selectedElementIndex = selected.contains(focusedIndex) ? focusedIndex : selected.last();
    return true;
}

bool BoardModel::moveSelectedToFront()
{
    if (currentPageLocked())
        return false;
    Page &page = m_document.currentPage();
    QVector<int> selected = validSelectedIndexes(page, selectedIndexesOrFocused(), m_selectedElementIndex);
    if (selected.isEmpty())
        return false;

    const QVector<DrawingElement> elements = page.elements();
    bool alreadyAtFront = true;
    for (int i = 0; i < selected.size(); ++i) {
        if (selected.at(i) != elements.size() - selected.size() + i) {
            alreadyAtFront = false;
            break;
        }
    }
    if (alreadyAtFront)
        return false;

    QVector<DrawingElement> reordered;
    QVector<DrawingElement> moved;
    reordered.reserve(elements.size());
    moved.reserve(selected.size());

    for (int i = 0; i < elements.size(); ++i) {
        if (selected.contains(i))
            moved.append(elements.at(i));
        else
            reordered.append(elements.at(i));
    }
    const int firstMovedIndex = reordered.size();
    reordered += moved;

    QVector<int> newSelected;
    newSelected.reserve(selected.size());
    for (int i = 0; i < selected.size(); ++i)
        newSelected.append(firstMovedIndex + i);
    const int focusedOffset = qMax(0, selected.indexOf(m_selectedElementIndex));

    pushUndoState();
    clearRedoState();
    m_document.currentPage().setElements(reordered);
    m_selectedElementIndexes = newSelected;
    m_selectedElementIndex = newSelected.at(focusedOffset);
    return true;
}

bool BoardModel::moveSelectedToBack()
{
    if (currentPageLocked())
        return false;
    Page &page = m_document.currentPage();
    QVector<int> selected = validSelectedIndexes(page, selectedIndexesOrFocused(), m_selectedElementIndex);
    if (selected.isEmpty())
        return false;

    const QVector<DrawingElement> elements = page.elements();
    bool alreadyAtBack = true;
    for (int i = 0; i < selected.size(); ++i) {
        if (selected.at(i) != i) {
            alreadyAtBack = false;
            break;
        }
    }
    if (alreadyAtBack)
        return false;

    QVector<DrawingElement> reordered;
    QVector<DrawingElement> moved;
    reordered.reserve(elements.size());
    moved.reserve(selected.size());

    for (int i = 0; i < elements.size(); ++i) {
        if (selected.contains(i))
            moved.append(elements.at(i));
        else
            reordered.append(elements.at(i));
    }
    reordered = moved + reordered;

    QVector<int> newSelected;
    newSelected.reserve(selected.size());
    for (int i = 0; i < selected.size(); ++i)
        newSelected.append(i);
    const int focusedOffset = qMax(0, selected.indexOf(m_selectedElementIndex));

    pushUndoState();
    clearRedoState();
    m_document.currentPage().setElements(reordered);
    m_selectedElementIndexes = newSelected;
    m_selectedElementIndex = newSelected.at(focusedOffset);
    return true;
}

QVector<DrawingElement> BoardModel::copySelectedElements() const
{
    QVector<DrawingElement> copied;
    const QVector<int> indexes = selectedIndexesOrFocused();
    copied.reserve(indexes.size());
    for (int index : indexes) {
        const DrawingElement *element = m_document.currentPage().elementAt(index);
        if (element)
            copied.append(*element);
    }
    return copied;
}

bool BoardModel::pasteElements(const QVector<DrawingElement> &elements)
{
    if (currentPageLocked())
        return false;
    if (elements.isEmpty())
        return false;
    clearSelection();
    pushUndoState();
    clearRedoState();
    QVector<DrawingElement> pasted = elements;
    QHash<int, int> pastedGroupIds;
    int groupId = nextGroupId();
    for (DrawingElement &element : pasted) {
        element.moveBy(QPointF(24, 24));
        if (element.groupId() > 0) {
            if (!pastedGroupIds.contains(element.groupId()))
                pastedGroupIds.insert(element.groupId(), groupId++);
            element.setGroupId(pastedGroupIds.value(element.groupId()));
        }
    }
    m_selectedElementIndexes = m_document.currentPage().addElements(pasted);
    m_selectedElementIndex = m_selectedElementIndexes.isEmpty() ? -1 : m_selectedElementIndexes.last();
    return !m_selectedElementIndexes.isEmpty();
}

bool BoardModel::updateSelectedText(const QString &text)
{
    if (currentPageLocked())
        return false;
    const DrawingElement *currentElement = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!currentElement || currentElement->type() != ElementType::Text || text.trimmed().isEmpty() || text == currentElement->text())
        return false;

    pushUndoState();
    clearRedoState();
    DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!element)
        return false;
    element->setText(text);
    element->setRect(QRectF(element->rect().topLeft(), textSize(text, element->font())));
    return true;
}

bool BoardModel::updateSelectedTextColor(const QColor &color)
{
    if (currentPageLocked())
        return false;
    const DrawingElement *currentElement = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!currentElement || currentElement->type() != ElementType::Text || !color.isValid() || color == currentElement->color())
        return false;

    pushUndoState();
    clearRedoState();
    DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!element)
        return false;
    element->setColor(color);
    return true;
}

bool BoardModel::updateSelectedColor(const QColor &color)
{
    if (currentPageLocked())
        return false;
    if (!color.isValid())
        return false;

    const QVector<int> indexes = selectedIndexesOrFocused();
    QVector<int> editableIndexes;
    editableIndexes.reserve(indexes.size());
    for (int index : indexes) {
        const DrawingElement *element = m_document.currentPage().elementAt(index);
        if (element && element->type() != ElementType::Text && element->color() != color)
            editableIndexes.append(index);
    }
    if (editableIndexes.isEmpty())
        return false;

    pushUndoState();
    clearRedoState();
    for (int index : editableIndexes) {
        DrawingElement *element = m_document.currentPage().elementAt(index);
        if (element)
            element->setColor(color);
    }
    return true;
}

bool BoardModel::updateSelectedTextFont(const QFont &font)
{
    if (currentPageLocked())
        return false;
    const DrawingElement *currentElement = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!currentElement || currentElement->type() != ElementType::Text || font == currentElement->font())
        return false;

    pushUndoState();
    clearRedoState();
    DrawingElement *element = m_document.currentPage().elementAt(m_selectedElementIndex);
    if (!element)
        return false;
    element->setFont(font);
    element->setRect(QRectF(element->rect().topLeft(), textSize(element->text(), font)));
    return true;
}

bool BoardModel::updateSelectedStrokeStyle(StrokeStyle strokeStyle)
{
    if (currentPageLocked())
        return false;
    const QVector<int> indexes = selectedIndexesOrFocused();
    QVector<int> editableIndexes;
    editableIndexes.reserve(indexes.size());
    for (int index : indexes) {
        const DrawingElement *element = m_document.currentPage().elementAt(index);
        if (element && element->type() != ElementType::Text && element->strokeStyle() != strokeStyle)
            editableIndexes.append(index);
    }
    if (editableIndexes.isEmpty())
        return false;

    pushUndoState();
    clearRedoState();
    for (int index : editableIndexes) {
        DrawingElement *element = m_document.currentPage().elementAt(index);
        if (element)
            element->setStrokeStyle(strokeStyle);
    }
    return true;
}

bool BoardModel::updateSelectedStrokeWidth(int strokeWidth)
{
    if (currentPageLocked())
        return false;
    const int width = qMax(0, strokeWidth);
    const QVector<int> indexes = selectedIndexesOrFocused();
    QVector<int> editableIndexes;
    editableIndexes.reserve(indexes.size());
    for (int index : indexes) {
        const DrawingElement *element = m_document.currentPage().elementAt(index);
        if (element && element->type() != ElementType::Text && element->strokeWidth() != width)
            editableIndexes.append(index);
    }
    if (editableIndexes.isEmpty())
        return false;

    pushUndoState();
    clearRedoState();
    for (int index : editableIndexes) {
        DrawingElement *element = m_document.currentPage().elementAt(index);
        if (element)
            element->setStrokeWidth(width);
    }
    return true;
}

bool BoardModel::updateSelectedCornerRadius(int cornerRadius)
{
    if (currentPageLocked())
        return false;
    const int radius = qBound(0, cornerRadius, 200);
    const QVector<int> indexes = selectedIndexesOrFocused();
    QVector<int> editableIndexes;
    editableIndexes.reserve(indexes.size());
    for (int index : indexes) {
        const DrawingElement *element = m_document.currentPage().elementAt(index);
        if (element && element->type() == ElementType::RoundedRectangle && element->cornerRadius() != radius)
            editableIndexes.append(index);
    }
    if (editableIndexes.isEmpty())
        return false;

    pushUndoState();
    clearRedoState();
    for (int index : editableIndexes) {
        DrawingElement *element = m_document.currentPage().elementAt(index);
        if (element)
            element->setCornerRadius(radius);
    }
    return true;
}

bool BoardModel::updateSelectedFillColor(const QColor &color)
{
    if (currentPageLocked())
        return false;
    if (!color.isValid())
        return false;

    const QVector<int> indexes = selectedIndexesOrFocused();
    QVector<int> editableIndexes;
    editableIndexes.reserve(indexes.size());
    for (int index : indexes) {
        const DrawingElement *element = m_document.currentPage().elementAt(index);
        if (element && supportsFillColor(*element) && element->fillColor() != color)
            editableIndexes.append(index);
    }
    if (editableIndexes.isEmpty())
        return false;

    pushUndoState();
    clearRedoState();
    for (int index : editableIndexes) {
        DrawingElement *element = m_document.currentPage().elementAt(index);
        if (element)
            element->setFillColor(color);
    }
    return true;
}

bool BoardModel::resetSelectedStyle()
{
    if (currentPageLocked() && hasSelection())
        return false;
    const QVector<int> indexes = selectedIndexesOrFocused();
    QVector<int> editableIndexes;
    editableIndexes.reserve(indexes.size());
    for (int index : indexes) {
        const DrawingElement *element = m_document.currentPage().elementAt(index);
        if (!element)
            continue;
        const bool needsReset = element->color() != DefaultColor
            || element->fillColor() != DefaultFillColor
            || (element->type() != ElementType::Text && element->strokeWidth() != DefaultStrokeWidth)
            || (element->type() == ElementType::RoundedRectangle && element->cornerRadius() != DefaultCornerRadius)
            || (element->type() != ElementType::Text && element->strokeStyle() != DefaultStrokeStyle)
            || (element->type() == ElementType::Text && element->font() != DefaultFont)
            || element->startArrowHead() != DefaultStartArrowHead
            || element->endArrowHead() != DefaultEndArrowHead;
        if (needsReset)
            editableIndexes.append(index);
    }
    if (editableIndexes.isEmpty())
        return false;

    pushUndoState();
    clearRedoState();
    for (int index : editableIndexes) {
        DrawingElement *element = m_document.currentPage().elementAt(index);
        if (!element)
            continue;
        element->setColor(DefaultColor);
        element->setFillColor(DefaultFillColor);
        element->setStartArrowHead(DefaultStartArrowHead);
        element->setEndArrowHead(DefaultEndArrowHead);
        if (element->type() != ElementType::Text) {
            element->setStrokeWidth(DefaultStrokeWidth);
            element->setStrokeStyle(DefaultStrokeStyle);
            if (element->type() == ElementType::RoundedRectangle)
                element->setCornerRadius(DefaultCornerRadius);
        } else {
            element->setFont(DefaultFont);
            element->setRect(QRectF(element->rect().topLeft(), textSize(element->text(), DefaultFont)));
        }
    }
    return true;
}

bool BoardModel::groupSelectedElements()
{
    if (currentPageLocked())
        return false;
    QVector<int> indexes = validSelectedIndexes(m_document.currentPage(), selectedIndexesOrFocused(), m_selectedElementIndex);
    if (indexes.size() < 2)
        return false;

    const int groupId = nextGroupId();
    pushUndoState();
    clearRedoState();
    Page &page = m_document.currentPage();
    for (int index : indexes) {
        DrawingElement *element = page.elementAt(index);
        if (element)
            element->setGroupId(groupId);
    }
    m_selectedElementIndexes = indexes;
    m_selectedElementIndex = indexes.last();
    return true;
}

bool BoardModel::ungroupSelectedElements()
{
    if (currentPageLocked())
        return false;
    const QVector<int> indexes = validSelectedIndexes(m_document.currentPage(), selectedIndexesOrFocused(), m_selectedElementIndex);
    if (indexes.isEmpty())
        return false;

    QSet<int> targetGroupIds;
    for (int index : indexes) {
        const DrawingElement *element = m_document.currentPage().elementAt(index);
        if (element && element->groupId() > 0)
            targetGroupIds.insert(element->groupId());
    }
    if (targetGroupIds.isEmpty())
        return false;

    pushUndoState();
    clearRedoState();
    Page &page = m_document.currentPage();
    QVector<int> ungroupedIndexes;
    const QVector<DrawingElement> elements = page.elements();
    for (int i = 0; i < elements.size(); ++i) {
        if (!targetGroupIds.contains(elements.at(i).groupId()))
            continue;
        DrawingElement *element = page.elementAt(i);
        if (element)
            element->setGroupId(0);
        ungroupedIndexes.append(i);
    }
    m_selectedElementIndexes = ungroupedIndexes;
    m_selectedElementIndex = ungroupedIndexes.isEmpty() ? -1 : ungroupedIndexes.last();
    return true;
}

bool BoardModel::canUndo() const
{
    return !currentPageLocked() && !m_undoStack.isEmpty();
}

bool BoardModel::canRedo() const
{
    return !currentPageLocked() && !m_redoStack.isEmpty();
}

bool BoardModel::undo()
{
    if (currentPageLocked())
        return false;
    if (!canUndo())
        return false;
    clearSelection();
    m_redoStack.append(m_document);
    m_document = m_undoStack.takeLast();
    return true;
}

bool BoardModel::redo()
{
    if (currentPageLocked())
        return false;
    if (!canRedo())
        return false;
    clearSelection();
    m_undoStack.append(m_document);
    m_document = m_redoStack.takeLast();
    return true;
}

bool BoardModel::canAddPage() const
{
    return m_document.pages().size() < MaxPages;
}

bool BoardModel::canDeletePage() const
{
    return !currentPageLocked() && m_document.pages().size() > 1;
}

bool BoardModel::addPage()
{
    if (!canAddPage())
        return false;
    clearSelection();
    pushUndoState();
    QVector<Page> pages = m_document.pages();
    const int insertIndex = qMin(m_document.currentPageIndex() + 1, pages.size());
    pages.insert(insertIndex, Page());
    m_document.setPages(pages);
    m_document.setCurrentPageIndex(insertIndex);
    clearRedoState();
    return true;
}

bool BoardModel::deleteCurrentPage()
{
    if (!canDeletePage())
        return false;
    clearSelection();
    pushUndoState();
    QVector<Page> pages = m_document.pages();
    pages.removeAt(m_document.currentPageIndex());
    m_document.setPages(pages);
    m_document.setCurrentPageIndex(qMin(m_document.currentPageIndex(), pages.size() - 1));
    clearRedoState();
    return true;
}

void BoardModel::nextPage()
{
    clearSelection();
    m_document.setCurrentPageIndex(m_document.currentPageIndex() + 1);
}

void BoardModel::previousPage()
{
    clearSelection();
    m_document.setCurrentPageIndex(m_document.currentPageIndex() - 1);
}

QPoint BoardModel::currentPageScrollPosition() const
{
    return m_document.currentPage().scrollPosition();
}

void BoardModel::setCurrentPageScrollPosition(const QPoint &scrollPosition)
{
    m_document.currentPage().setScrollPosition(scrollPosition);
}

bool BoardModel::currentPageLocked() const
{
    return m_document.currentPage().locked();
}

bool BoardModel::setCurrentPageLocked(bool locked)
{
    if (m_document.currentPage().locked() == locked)
        return false;
    clearSelection();
    pushUndoState();
    m_document.currentPage().setLocked(locked);
    clearRedoState();
    return true;
}

bool BoardModel::applyElementsToPage(const QVector<DrawingElement> &elements, PageElementApplyMode mode)
{
    if (elements.isEmpty())
        return false;
    if (mode != PageElementApplyMode::NewPage && currentPageLocked())
        return false;
    if (mode == PageElementApplyMode::NewPage && !canAddPage())
        return false;

    clearSelection();
    pushUndoState();

    if (mode == PageElementApplyMode::NewPage) {
        QVector<Page> pages = m_document.pages();
        const int newPageIndex = m_document.currentPageIndex() + 1;
        Page page;
        page.setElements(elements);
        pages.insert(newPageIndex, page);
        m_document.setPages(pages);
        m_document.setCurrentPageIndex(newPageIndex);
    } else if (mode == PageElementApplyMode::ReplaceCurrent) {
        m_document.currentPage().setElements(elements);
    } else {
        QVector<DrawingElement> appended = elements;
        QHash<int, int> appendedGroupIds;
        int groupId = nextGroupId();
        for (DrawingElement &element : appended) {
            if (element.groupId() <= 0)
                continue;
            if (!appendedGroupIds.contains(element.groupId()))
                appendedGroupIds.insert(element.groupId(), groupId++);
            element.setGroupId(appendedGroupIds.value(element.groupId()));
        }
        m_document.currentPage().addElements(appended);
    }

    clearRedoState();
    return true;
}

bool BoardModel::saveToFile(const QString &filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    file.write(QJsonDocument(m_document.toJson()).toJson(QJsonDocument::Indented));
    return true;
}

bool BoardModel::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;
    const QJsonDocument json = QJsonDocument::fromJson(file.readAll());
    if (!json.isObject())
        return false;
    m_document = BoardDocument::fromJson(json.object());
    clearSelection();
    m_undoStack.clear();
    m_redoStack.clear();
    return true;
}

void BoardModel::pushUndoState()
{
    m_undoStack.append(m_document);
}

void BoardModel::clearRedoState()
{
    m_redoStack.clear();
}

DrawingElement BoardModel::makeElement(ElementType type) const
{
    DrawingElement element;
    element.setType(type);
    element.setColor(m_document.settings().selectedColor());
    element.setFillColor(m_document.settings().fillColor());
    element.setStrokeWidth(m_document.settings().strokeWidth());
    element.setCornerRadius(m_document.settings().cornerRadius());
    element.setStrokeStyle(m_document.settings().strokeStyle());
    element.setFont(m_document.settings().font());
    element.setStartArrowHead(m_document.settings().startArrowHead());
    element.setEndArrowHead(m_document.settings().endArrowHead());
    return element;
}

QVector<int> BoardModel::selectedIndexesOrFocused() const
{
    return !m_selectedElementIndexes.isEmpty() ? m_selectedElementIndexes : QVector<int>{m_selectedElementIndex};
}

void BoardModel::expandSelectionForGroups()
{
    Page &page = m_document.currentPage();
    QVector<int> sourceIndexes = selectedIndexesOrFocused();
    QVector<int> indexes;
    indexes.reserve(sourceIndexes.size());
    for (int index : sourceIndexes) {
        if (!indexes.contains(index) && page.elementAt(index))
            indexes.append(index);
    }
    if (indexes.isEmpty()) {
        m_selectedElementIndexes.clear();
        m_selectedElementIndex = -1;
        return;
    }

    QSet<int> groupIds;
    for (int index : indexes) {
        const DrawingElement *element = page.elementAt(index);
        if (element && element->groupId() > 0)
            groupIds.insert(element->groupId());
    }
    if (groupIds.isEmpty()) {
        m_selectedElementIndexes = indexes;
        if (!m_selectedElementIndexes.contains(m_selectedElementIndex))
            m_selectedElementIndex = m_selectedElementIndexes.last();
        return;
    }

    const QVector<DrawingElement> elements = page.elements();
    for (int i = 0; i < elements.size(); ++i) {
        if (groupIds.contains(elements.at(i).groupId()) && !indexes.contains(i))
            indexes.append(i);
    }

    std::sort(indexes.begin(), indexes.end());
    m_selectedElementIndexes = indexes;
    if (!m_selectedElementIndexes.contains(m_selectedElementIndex))
        m_selectedElementIndex = m_selectedElementIndexes.last();
}

int BoardModel::nextGroupId() const
{
    int maxGroupId = 0;
    for (const Page &page : m_document.pages()) {
        for (const DrawingElement &element : page.elements())
            maxGroupId = qMax(maxGroupId, element.groupId());
    }
    return maxGroupId + 1;
}

QSizeF BoardModel::textSize(const QString &text, const QFont &font) const
{
    QFontMetricsF metrics(font);
    const QStringList lines = text.split(QLatin1Char('\n'));
    qreal width = 80.0;
    for (const QString &line : lines)
        width = qMax(width, metrics.horizontalAdvance(line.isEmpty() ? QStringLiteral(" ") : line));
    const qreal height = qMax<qreal>(metrics.lineSpacing(), metrics.lineSpacing() * qMax(1, lines.size()));
    return QSizeF(width + 12.0, height + 12.0);
}
