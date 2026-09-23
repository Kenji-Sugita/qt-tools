#include "CanvasWidget.h"

#include "DrawingRenderer.h"

#include <QApplication>
#include <QEvent>
#include <QFrame>
#include <QImage>
#include <QKeyEvent>
#include <QLineF>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPlainTextEdit>
#include <QPolygonF>
#include <QScrollBar>
#include <QSizeF>
#include <QTransform>
#include <QtMath>

#include <cmath>
#include <functional>

namespace {
bool usesPointGeometry(ElementType type)
{
    return type == ElementType::Freehand || type == ElementType::Line
        || type == ElementType::Polyline || type == ElementType::Bezier;
}

QPointF rotatePoint(const QPointF &point, const QPointF &pivot, qreal degrees)
{
    QTransform transform;
    transform.translate(pivot.x(), pivot.y());
    transform.rotate(degrees);
    transform.translate(-pivot.x(), -pivot.y());
    return transform.map(point);
}

QPointF rotateVector(const QPointF &vector, qreal degrees)
{
    QTransform transform;
    transform.rotate(degrees);
    return transform.map(vector);
}

qreal pointerAngle(const QPointF &center, const QPointF &point)
{
    return qRadiansToDegrees(std::atan2(point.y() - center.y(), point.x() - center.x()));
}

qreal normalizedAngleDelta(qreal degrees)
{
    while (degrees > 180.0)
        degrees -= 360.0;
    while (degrees < -180.0)
        degrees += 360.0;
    return degrees;
}

QRectF squareRect(const QRectF &rect)
{
    const QRectF normalized = rect.normalized();
    const qreal side = qMin(normalized.width(), normalized.height());
    return QRectF(normalized.topLeft(), QSizeF(side, side)).normalized();
}

QPointF pointOnEllipse(const QRectF &rect, int angleDegrees)
{
    const qreal radians = qDegreesToRadians(static_cast<qreal>(angleDegrees));
    const QPointF center = rect.center();
    return QPointF(
        center.x() + std::cos(radians) * rect.width() / 2.0,
        center.y() - std::sin(radians) * rect.height() / 2.0);
}

int angleFromPoint(const QRectF &rect, const QPointF &point)
{
    const QPointF center = rect.center();
    const qreal angle = qRadiansToDegrees(std::atan2(center.y() - point.y(), point.x() - center.x()));
    return qRound(angle);
}

bool isShapeTool(Tool tool)
{
    return tool == Tool::Line
        || tool == Tool::Rectangle
        || tool == Tool::RoundedRectangle
        || tool == Tool::Ellipse
        || tool == Tool::Arc
        || tool == Tool::Circle;
}

bool isCommitTextKey(QKeyEvent *event)
{
    const Qt::KeyboardModifiers modifiers = event->modifiers();
    const bool commandModifier = modifiers.testFlag(Qt::ControlModifier) || modifiers.testFlag(Qt::MetaModifier);
    return commandModifier && (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter);
}

class TextEditor : public QPlainTextEdit {
public:
    explicit TextEditor(std::function<void()> commitCallback, QWidget *parent = nullptr)
        : QPlainTextEdit(parent)
        , m_commitCallback(std::move(commitCallback))
    {
    }

protected:
    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::ShortcutOverride) {
            auto *keyEvent = static_cast<QKeyEvent *>(event);
            if (isCommitTextKey(keyEvent)) {
                event->accept();
                if (m_commitCallback)
                    m_commitCallback();
                return true;
            }
        }
        return QPlainTextEdit::event(event);
    }

    void keyPressEvent(QKeyEvent *event) override
    {
        if (isCommitTextKey(event)) {
            event->accept();
            if (m_commitCallback)
                m_commitCallback();
            return;
        }
        QPlainTextEdit::keyPressEvent(event);
    }

private:
    std::function<void()> m_commitCallback;
};

QPainterPath bezierPath(const QVector<QPointF> &points, bool closed = false)
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

QPainterPath roundedRectanglePath(const QRectF &rect, int radius)
{
    QPainterPath path;
    const QRectF normalized = rect.normalized();
    const qreal clampedRadius = qMin<qreal>(qMax(0, radius), qMin(normalized.width(), normalized.height()) / 2.0);
    path.addRoundedRect(normalized, clampedRadius, clampedRadius);
    return path;
}

}

CanvasWidget::CanvasWidget(QWidget *parent)
    : QWidget(parent)
{
    setAutoFillBackground(false);
    setFocusPolicy(Qt::StrongFocus);
    refreshSize();
}

void CanvasWidget::setModel(BoardModel *model)
{
    m_model = model;
    refreshSize();
    update();
}

void CanvasWidget::setChangeCallback(std::function<void()> callback)
{
    m_changeCallback = std::move(callback);
}

void CanvasWidget::refreshSize()
{
    const int zoom = m_model ? m_model->settings().zoomPercent() : 100;
    const QSize canvasSize(BoardModel::CanvasWidth * zoom / 100, BoardModel::CanvasHeight * zoom / 100);
    setFixedSize(canvasSize);
    updateGeometry();
    update();
}

QImage CanvasWidget::renderPageImage()
{
    QImage image(QSize(BoardModel::CanvasWidth, BoardModel::CanvasHeight), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);
    if (!m_model)
        return image;

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    const QVector<DrawingElement> elements = m_model->currentPage().elements();
    for (const DrawingElement &element : elements)
        drawElement(painter, element);
    return image;
}

void CanvasWidget::commitPendingTextEdit()
{
    commitTextEditor();
}

bool CanvasWidget::hasPendingTextEdit() const
{
    return m_textEditor != nullptr;
}

bool CanvasWidget::closeActivePolyline()
{
    if (!m_polylineActive || m_polylinePoints.size() < 3)
        return false;
    commitPolyline(true);
    return true;
}

bool CanvasWidget::closeActiveBezier()
{
    const int pointCount = m_extendingBezier
        ? m_extensionBasePoints.size() + qMax(0, m_bezierPoints.size() - 1)
        : m_bezierPoints.size();
    if (!m_bezierActive || pointCount < 3)
        return false;
    commitBezier(true);
    return true;
}

void CanvasWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.fillRect(rect(), QColor(QStringLiteral("#ffffff")));

    if (!m_model)
        return;

    const qreal scale = m_model->settings().zoomPercent() / 100.0;
    painter.save();
    painter.scale(scale, scale);

    painter.setRenderHint(QPainter::Antialiasing);
    const QVector<DrawingElement> elements = m_model->currentPage().elements();
    for (int i = 0; i < elements.size(); ++i) {
        if (m_editingExistingText && i == m_model->selectedElementIndex())
            continue;
        drawElement(painter, elements.at(i));
    }

    drawSelection(painter);

    drawActivePolylinePreview(painter);
    drawActiveBezierPreview(painter);
    drawShapePreview(painter);
    drawRubberBandPreview(painter);

    painter.restore();
}

void CanvasWidget::mousePressEvent(QMouseEvent *event)
{
    const bool controlClick = (event->button() == Qt::RightButton)
        && ((event->modifiers() & (Qt::ControlModifier | Qt::MetaModifier)) != 0);
    if (!m_model || (event->button() != Qt::LeftButton && !controlClick))
        return;
    setFocus();
    commitTextEditor();

    const QPointF canvasPoint = toCanvasPoint(event->position());
    const Tool tool = m_model->settings().selectedTool();
    const bool locked = m_model->currentPageLocked();
    if (m_polylineActive) {
        addPolylinePoint(canvasPoint);
        return;
    }
    if (m_bezierActive) {
        addBezierPoint(canvasPoint);
        return;
    }
    if (tryOpenPolylineAtVertex(canvasPoint, event->modifiers()))
        return;
    if (tool == Tool::Select) {
        handleSelectMousePress(canvasPoint, event->modifiers());
        update();
    } else if (locked) {
        m_model->selectAt(canvasPoint);
        update();
    } else if (tool == Tool::Pen) {
        m_currentStroke = {canvasPoint};
        m_dragging = true;
        m_dragMode = DragMode::Draw;
    } else if (tool == Tool::Eraser) {
        if (m_model->eraseAt(canvasPoint))
            notifyChanged();
    } else if (isShapeTool(tool)) {
        cancelPolyline();
        m_dragStart = canvasPoint;
        m_dragCurrent = canvasPoint;
        m_dragging = true;
        m_dragMode = DragMode::Draw;
    } else if (tool == Tool::Bezier) {
        cancelPolyline();
        const bool extensionModifier = (event->modifiers() & (Qt::ControlModifier | Qt::MetaModifier)) != 0;
        if (!tryStartBezierExtension(canvasPoint, event->modifiers()) && !extensionModifier)
            addBezierPoint(canvasPoint);
    } else if (tool == Tool::Polyline) {
        const bool extensionModifier = (event->modifiers() & (Qt::ControlModifier | Qt::MetaModifier)) != 0;
        if (!tryStartPolylineExtension(canvasPoint, event->modifiers()) && !extensionModifier)
            addPolylinePoint(canvasPoint);
    } else if (tool == Tool::Text) {
        cancelPolyline();
        startTextEditor(canvasPoint, QString(), false);
    }
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_model)
        return;

    const QPointF canvasPoint = toCanvasPoint(event->position());
    const Tool tool = m_model->settings().selectedTool();
    const bool locked = m_model->currentPageLocked();
    if (m_polylineActive) {
        m_dragCurrent = canvasPoint;
        m_polylineClosedPreview = QLineF(polylineCloseTarget(), canvasPoint).length() <= handleSize() * 2.0;
        update();
    } else if (m_bezierActive) {
        m_dragCurrent = canvasPoint;
        m_bezierClosedPreview = QLineF(bezierCloseTarget(), canvasPoint).length() <= handleSize() * 2.0;
        update();
    } else if (tool == Tool::Select && m_dragging && m_dragMode == DragMode::Move) {
        if (m_model->moveSelectedBy(canvasPoint - m_lastDragPoint)) {
            m_lastDragPoint = canvasPoint;
            update();
        }
    } else if (tool == Tool::Select && m_dragging && m_dragMode == DragMode::Resize) {
        if (m_model->resizeSelectedTo(resizedRect(canvasPoint)))
            update();
    } else if (tool == Tool::Select && m_dragging && m_dragMode == DragMode::Rotate) {
        qreal degrees = normalizedAngleDelta(pointerAngle(m_rotationCenter, canvasPoint) - m_rotationStartPointerAngle);
        if (m_shiftPressed || event->modifiers().testFlag(Qt::ShiftModifier))
            degrees = qRound(degrees / 15.0) * 15.0;
        m_rotationDragDegrees = degrees;
        if (m_model->rotateSelectionTo(degrees))
            update();
    } else if (tool == Tool::Select && m_dragging && m_dragMode == DragMode::PolylineVertex) {
        const qreal dragThreshold = handleSize() * 0.8;
        if (!m_vertexDragMoved && QLineF(m_vertexDragStartPoint, canvasPoint).length() < dragThreshold)
            return;
        m_vertexDragMoved = true;
        if (m_model->moveSelectedVertexTo(m_polylineVertexIndex, canvasPoint))
            update();
    } else if (tool == Tool::Select && m_dragging && m_dragMode == DragMode::ArcAngle) {
        const QRectF bounds = m_model->selectedElementGeometryBounds();
        const QPointF localPoint = rotatePoint(canvasPoint, bounds.center(), -m_model->selectedElementRotationDegrees());
        const int angle = angleFromPoint(bounds, localPoint);
        if (m_arcAngleHandle == 0) {
            const int endAngle = m_model->selectedElementArcStartAngle() + m_model->selectedElementArcSpanAngle();
            if (m_model->updateSelectedArcAngles(angle, endAngle - angle))
                update();
        } else {
            if (m_model->updateSelectedArcAngles(m_model->selectedElementArcStartAngle(), angle - m_model->selectedElementArcStartAngle()))
                update();
        }
    } else if (tool == Tool::Select && m_dragging && m_dragMode == DragMode::RubberBand) {
        m_dragCurrent = canvasPoint;
        m_rubberBandRect = QRectF(m_dragStart, m_dragCurrent).normalized();
        m_model->selectInRect(m_rubberBandRect);
        update();
    } else if (!locked && tool == Tool::Pen && m_dragging) {
        m_currentStroke.append(canvasPoint);
        update();
    } else if (!locked && tool == Tool::Eraser) {
        if (m_model->eraseAt(canvasPoint))
            notifyChanged();
    } else if (!locked && isShapeTool(tool) && m_dragging && m_dragMode == DragMode::Draw) {
        m_dragCurrent = canvasPoint;
        update();
    }
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_model || event->button() != Qt::LeftButton || !m_dragging)
        return;

    const QPointF canvasPoint = toCanvasPoint(event->position());
    const Tool tool = m_model->settings().selectedTool();
    if (tool == Tool::Select) {
        if (m_dragMode == DragMode::Move)
            m_model->moveSelectedBy(canvasPoint - m_lastDragPoint);
        else if (m_dragMode == DragMode::Resize)
            m_model->resizeSelectedTo(resizedRect(canvasPoint));
        else if (m_dragMode == DragMode::Rotate) {
            qreal degrees = normalizedAngleDelta(pointerAngle(m_rotationCenter, canvasPoint) - m_rotationStartPointerAngle);
            if (m_shiftPressed || event->modifiers().testFlag(Qt::ShiftModifier))
                degrees = qRound(degrees / 15.0) * 15.0;
            m_rotationDragDegrees = degrees;
            m_model->rotateSelectionTo(degrees);
        }
        else if (m_dragMode == DragMode::PolylineVertex) {
            m_selectedPointIndex = m_polylineVertexIndex;
        }
        else if (m_dragMode == DragMode::ArcAngle) {
            const QRectF bounds = m_model->selectedElementGeometryBounds();
            const QPointF localPoint = rotatePoint(canvasPoint, bounds.center(), -m_model->selectedElementRotationDegrees());
            const int angle = angleFromPoint(bounds, localPoint);
            if (m_arcAngleHandle == 0) {
                const int endAngle = m_model->selectedElementArcStartAngle() + m_model->selectedElementArcSpanAngle();
                m_model->updateSelectedArcAngles(angle, endAngle - angle);
            } else {
                m_model->updateSelectedArcAngles(m_model->selectedElementArcStartAngle(), angle - m_model->selectedElementArcStartAngle());
            }
        } else if (m_dragMode == DragMode::RubberBand) {
            m_rubberBandRect = QRectF(m_dragStart, canvasPoint).normalized();
            m_model->selectInRect(m_rubberBandRect);
        }
        m_model->endSelectionEdit();
        m_dragging = false;
        m_dragMode = DragMode::None;
        m_resizeHandle = ResizeHandle::None;
        m_polylineVertexIndex = -1;
        m_vertexDragMoved = false;
        m_arcAngleHandle = -1;
        notifyChanged();
    } else if (tool == Tool::Pen) {
        m_currentStroke.append(canvasPoint);
        m_model->addFreehand(m_currentStroke);
        m_currentStroke.clear();
        m_dragging = false;
        m_dragMode = DragMode::None;
        notifyChanged();
    } else if (isShapeTool(tool) && m_dragMode == DragMode::Draw) {
        m_dragCurrent = canvasPoint;
        commitShape(canvasPoint);
        m_dragging = false;
        m_dragMode = DragMode::None;
        notifyChanged();
    }
}

void CanvasWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!m_model || event->button() != Qt::LeftButton)
        return;
    commitTextEditor();

    const QPointF canvasPoint = toCanvasPoint(event->position());
    if (m_polylineActive) {
        addPolylinePoint(canvasPoint);
        commitPolyline();
        return;
    }
    if (m_bezierActive) {
        addBezierPoint(canvasPoint);
        commitBezier();
        return;
    }

    if (m_model->settings().selectedTool() != Tool::Select)
        return;
    if (m_model->currentPageLocked())
        return;

    if (hitRotationHandle(canvasPoint)) {
        if (m_model->resetSelectedRotation())
            notifyChanged();
        return;
    }

    if (!m_model->selectedElementsContain(canvasPoint, handleSize()))
        m_model->selectAt(canvasPoint);

    if (m_model->hasSelection() && m_model->selectedElementType() == ElementType::Text)
        startTextEditor(m_model->selectedElementBounds().topLeft(), m_model->selectedElementText(), true);
}

void CanvasWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Shift)
        m_shiftPressed = true;

    const Qt::KeyboardModifiers modifiers = event->modifiers();
    const bool commandModifier = (modifiers & (Qt::ControlModifier | Qt::MetaModifier)) != 0;
    if (m_model && commandModifier && !modifiers.testFlag(Qt::AltModifier) && event->key() == Qt::Key_G) {
        const bool changed = modifiers.testFlag(Qt::ShiftModifier)
            ? m_model->ungroupSelectedElements()
            : m_model->groupSelectedElements();
        if (changed)
            notifyChanged();
        event->accept();
        return;
    }
    if (m_model && commandModifier && modifiers.testFlag(Qt::ShiftModifier) && event->key() == Qt::Key_O) {
        if (m_model->openSelectedPolyline() || m_model->openSelectedBezier()) {
            notifyChanged();
            event->accept();
            return;
        }
    }
    if (m_model && m_model->settings().selectedTool() == Tool::Select && m_model->hasSelection()
        && (modifiers & (Qt::ShiftModifier | Qt::ControlModifier | Qt::MetaModifier | Qt::AltModifier)) == Qt::NoModifier) {
        QPointF delta;
        if (event->key() == Qt::Key_Left)
            delta = QPointF(-1, 0);
        else if (event->key() == Qt::Key_Right)
            delta = QPointF(1, 0);
        else if (event->key() == Qt::Key_Up)
            delta = QPointF(0, -1);
        else if (event->key() == Qt::Key_Down)
            delta = QPointF(0, 1);

        if (!delta.isNull()) {
            bool moved = false;
            const QVector<QPointF> points = m_model->selectedElementPoints();
            if (m_selectedPointIndex >= 0 && m_selectedPointIndex < points.size()) {
                m_model->beginSelectionEdit();
                moved = m_model->moveSelectedVertexTo(m_selectedPointIndex, points.at(m_selectedPointIndex) + delta);
                m_model->endSelectionEdit();
            } else {
                m_selectedPointIndex = -1;
                m_model->beginSelectionEdit();
                moved = m_model->moveSelectedBy(delta);
                m_model->endSelectionEdit();
            }
            if (moved)
                notifyChanged();
            event->accept();
            return;
        }
    }
    if (m_model && m_polylineActive) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            commitPolyline(commandModifier);
            event->accept();
            return;
        }
        if (event->key() == Qt::Key_Escape) {
            cancelPolyline();
            event->accept();
            return;
        }
    }
    if (m_model && m_bezierActive) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            commitBezier(commandModifier);
            event->accept();
            return;
        }
        if (event->key() == Qt::Key_Escape) {
            cancelBezier();
            event->accept();
            return;
        }
    }
    QWidget::keyPressEvent(event);
}

void CanvasWidget::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Shift)
        m_shiftPressed = false;
    QWidget::keyReleaseEvent(event);
}

void CanvasWidget::focusOutEvent(QFocusEvent *event)
{
    m_shiftPressed = false;
    QWidget::focusOutEvent(event);
}

bool CanvasWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (m_textEditor && (event->type() == QEvent::ShortcutOverride || event->type() == QEvent::KeyPress)) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (isCommitTextKey(keyEvent)) {
            event->accept();
            commitTextEditor();
            return true;
        }
    }

    const bool textEditorEvent = m_textEditor && (watched == m_textEditor || watched == m_textEditor->viewport());
    if (textEditorEvent) {
        if (event->type() == QEvent::FocusOut) {
            commitTextEditor();
            return false;
        }
        if (event->type() == QEvent::KeyPress) {
            auto *keyEvent = static_cast<QKeyEvent *>(event);
            if (isCommitTextKey(keyEvent)) {
                event->accept();
                commitTextEditor();
                return true;
            }
            if (keyEvent->key() == Qt::Key_Escape) {
                qApp->removeEventFilter(this);
                m_textEditorAppFilterInstalled = false;
                m_textEditor->viewport()->removeEventFilter(this);
                m_textEditor->removeEventFilter(this);
                m_textEditor->deleteLater();
                m_textEditor = nullptr;
                m_editingExistingText = false;
                update();
                if (m_changeCallback)
                    m_changeCallback();
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

QPointF CanvasWidget::toCanvasPoint(const QPointF &widgetPoint) const
{
    if (!m_model)
        return widgetPoint;
    const qreal scale = m_model->settings().zoomPercent() / 100.0;
    return widgetPoint / scale;
}

qreal CanvasWidget::handleSize() const
{
    if (!m_model)
        return 8.0;
    const qreal scale = m_model->settings().zoomPercent() / 100.0;
    return 8.0 / scale;
}

CanvasWidget::ResizeHandle CanvasWidget::hitResizeHandle(const QPointF &canvasPoint) const
{
    if (!m_model || !m_model->hasSelection())
        return ResizeHandle::None;

    if (m_model->selectedElementIndexes().size() > 1)
        return ResizeHandle::None;
    const QRectF bounds = m_model->selectedElementGeometryBounds();
    const qreal size = handleSize();
    const auto contains = [canvasPoint, size](const QPointF &center) {
        return QRectF(center.x() - size, center.y() - size, size * 2.0, size * 2.0).contains(canvasPoint);
    };
    QVector<QPointF> corners = m_model->selectedElementVisualCorners();
    if (corners.size() != 4)
        return ResizeHandle::None;
    if (contains(corners.at(0)))
        return ResizeHandle::TopLeft;
    if (contains(corners.at(1)))
        return ResizeHandle::TopRight;
    if (contains(corners.at(2)))
        return ResizeHandle::BottomRight;
    if (contains(corners.at(3)))
        return ResizeHandle::BottomLeft;
    const ElementType type = m_model->selectedElementType();
    if (type == ElementType::Rectangle || type == ElementType::RoundedRectangle || type == ElementType::Ellipse || type == ElementType::Circle) {
        if (contains((corners.at(0) + corners.at(1)) / 2.0))
            return ResizeHandle::Top;
        if (contains((corners.at(1) + corners.at(2)) / 2.0))
            return ResizeHandle::Right;
        if (contains((corners.at(2) + corners.at(3)) / 2.0))
            return ResizeHandle::Bottom;
        if (contains((corners.at(3) + corners.at(0)) / 2.0))
            return ResizeHandle::Left;
    }
    return ResizeHandle::None;
}

QPointF CanvasWidget::rotationHandlePoint() const
{
    if (!m_model || !m_model->hasSelection())
        return QPointF();
    const QRectF bounds = m_model->selectedElementBounds();
    const qreal offset = handleSize() * 4.0;
    const qreal margin = handleSize() * 1.5;
    const qreal above = bounds.top() - offset;
    const qreal below = bounds.bottom() + offset;
    const qreal preferredY = above >= margin ? above : below;
    const qreal y = qBound(margin, preferredY, BoardModel::CanvasHeight - margin);
    const qreal x = qBound(margin, bounds.center().x(), BoardModel::CanvasWidth - margin);
    return QPointF(x, y);
}

bool CanvasWidget::hitRotationHandle(const QPointF &canvasPoint) const
{
    if (!m_model || m_model->currentPageLocked() || !m_model->selectionCanRotate())
        return false;
    const qreal size = handleSize() * 1.5;
    const QPointF handle = rotationHandlePoint();
    return QRectF(handle.x() - size, handle.y() - size, size * 2.0, size * 2.0).contains(canvasPoint);
}

int CanvasWidget::hitPolylineVertexHandle(const QPointF &canvasPoint) const
{
    if (!m_model || !m_model->hasSelection() || (m_model->selectedElementType() != ElementType::Line && m_model->selectedElementType() != ElementType::Polyline && m_model->selectedElementType() != ElementType::Bezier))
        return -1;

    const QVector<QPointF> points = m_model->selectedElementPoints();
    const qreal size = handleSize() * 1.5;
    for (int i = 0; i < points.size(); ++i) {
        const QPointF point = points.at(i);
        if (QRectF(point.x() - size, point.y() - size, size * 2.0, size * 2.0).contains(canvasPoint))
            return i;
    }
    return -1;
}

int CanvasWidget::hitArcAngleHandle(const QPointF &canvasPoint) const
{
    if (!m_model || !m_model->hasSelection() || m_model->selectedElementType() != ElementType::Arc)
        return -1;

    const QRectF bounds = m_model->selectedElementGeometryBounds();
    const qreal size = handleSize() * 1.8;
    const qreal rotation = m_model->selectedElementRotationDegrees();
    const QPointF start = rotatePoint(pointOnEllipse(bounds, m_model->selectedElementArcStartAngle()), bounds.center(), rotation);
    const QPointF end = rotatePoint(pointOnEllipse(bounds, m_model->selectedElementArcStartAngle() + m_model->selectedElementArcSpanAngle()), bounds.center(), rotation);
    if (QRectF(start.x() - size, start.y() - size, size * 2.0, size * 2.0).contains(canvasPoint))
        return 0;
    if (QRectF(end.x() - size, end.y() - size, size * 2.0, size * 2.0).contains(canvasPoint))
        return 1;
    return -1;
}

QRectF CanvasWidget::resizedRect(const QPointF &canvasPoint) const
{
    if (m_resizeHandle == ResizeHandle::None)
        return m_resizeStartRect;
    QPointF localDelta = rotateVector(canvasPoint - m_resizeAnchor, -m_resizeRotationDegrees);
    if (m_resizeHandle == ResizeHandle::Top || m_resizeHandle == ResizeHandle::Bottom)
        localDelta.setX(0.0);
    if (m_resizeHandle == ResizeHandle::Left || m_resizeHandle == ResizeHandle::Right)
        localDelta.setY(0.0);

    const qreal width = (m_resizeHandle == ResizeHandle::Top || m_resizeHandle == ResizeHandle::Bottom)
        ? m_resizeStartRect.width() : qAbs(localDelta.x());
    const qreal height = (m_resizeHandle == ResizeHandle::Left || m_resizeHandle == ResizeHandle::Right)
        ? m_resizeStartRect.height() : qAbs(localDelta.y());
    const QPointF localCenterOffset(localDelta.x() / 2.0, localDelta.y() / 2.0);
    const QPointF center = m_resizeAnchor + rotateVector(localCenterOffset, m_resizeRotationDegrees);
    return QRectF(center - QPointF(width / 2.0, height / 2.0), QSizeF(width, height));
}

void CanvasWidget::startTextEditor(const QPointF &position, const QString &text, bool editingExistingText)
{
    if (!m_model)
        return;
    if (m_model->currentPageLocked())
        return;
    commitTextEditor();

    m_textEditorPosition = position;
    m_editingExistingText = editingExistingText;
    const QRectF canvasRect = editingExistingText
        ? m_model->selectedElementBounds()
        : QRectF(position, QSizeF(360, 120));

    m_textEditor = new TextEditor([this]() { commitTextEditor(); }, this);
    m_textEditor->setPlainText(text);
    m_textEditor->setFont(editingExistingText ? m_model->selectedElementFont() : m_model->settings().font());
    QPalette editorPalette = m_textEditor->palette();
    editorPalette.setColor(QPalette::Text, editingExistingText ? m_model->selectedElementColor() : m_model->settings().selectedColor());
    m_textEditor->setPalette(editorPalette);
    m_textEditor->setStyleSheet(QStringLiteral(
        "QPlainTextEdit {"
        "background: rgba(255, 255, 255, 235);"
        "border: 1px solid #0f766e;"
        "border-radius: 3px;"
        "padding: 4px;"
        "}"));
    m_textEditor->setFrameShape(QFrame::NoFrame);
    m_textEditor->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_textEditor->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_textEditor->setGeometry(textEditorGeometry(canvasRect));
    m_textEditor->installEventFilter(this);
    m_textEditor->viewport()->installEventFilter(this);
    qApp->installEventFilter(this);
    m_textEditorAppFilterInstalled = true;
    m_textEditor->show();
    m_textEditor->setFocus();
    m_textEditor->selectAll();
    update();
    if (m_changeCallback)
        m_changeCallback();
}

void CanvasWidget::commitTextEditor()
{
    if (!m_textEditor || !m_model)
        return;

    const QString text = m_textEditor->toPlainText();
    const bool changed = m_editingExistingText
        ? m_model->updateSelectedText(text)
        : (!text.trimmed().isEmpty() ? (m_model->addText(m_textEditorPosition, text), true) : false);

    if (m_textEditorAppFilterInstalled) {
        qApp->removeEventFilter(this);
        m_textEditorAppFilterInstalled = false;
    }
    m_textEditor->viewport()->removeEventFilter(this);
    m_textEditor->removeEventFilter(this);
    m_textEditor->deleteLater();
    m_textEditor = nullptr;
    m_editingExistingText = false;

    if (changed)
        notifyChanged();
    else {
        update();
        if (m_changeCallback)
            m_changeCallback();
    }
}

QPointF CanvasWidget::polylineCloseTarget() const
{
    if (m_extendingPolyline && !m_extensionBasePoints.isEmpty())
        return m_extendAtStart ? m_extensionBasePoints.last() : m_extensionBasePoints.first();
    return m_polylinePoints.isEmpty() ? QPointF() : m_polylinePoints.first();
}

QPointF CanvasWidget::bezierCloseTarget() const
{
    if (m_extendingBezier && !m_extensionBasePoints.isEmpty())
        return m_extendAtStart ? m_extensionBasePoints.last() : m_extensionBasePoints.first();
    return m_bezierPoints.isEmpty() ? QPointF() : m_bezierPoints.first();
}

void CanvasWidget::addPolylinePoint(const QPointF &point)
{
    if (!m_model)
        return;
    if (!m_polylineActive) {
        m_polylinePoints.clear();
        m_extensionBasePoints.clear();
        m_extendingPolyline = false;
        m_extendAtStart = false;
        m_polylineActive = true;
        m_polylineClosedPreview = false;
    }
    const int totalPointCount = m_extendingPolyline
        ? m_extensionBasePoints.size() + qMax(0, m_polylinePoints.size() - 1)
        : m_polylinePoints.size();
    if (totalPointCount >= 3 && QLineF(polylineCloseTarget(), point).length() <= handleSize() * 2.0) {
        commitPolyline(true);
        return;
    }
    if (m_polylinePoints.isEmpty() || QLineF(m_polylinePoints.last(), point).length() > 1.0)
        m_polylinePoints.append(point);
    m_dragCurrent = point;
    m_polylineClosedPreview = false;
    update();
}

void CanvasWidget::addBezierPoint(const QPointF &point)
{
    if (!m_model)
        return;
    if (!m_bezierActive) {
        m_bezierPoints.clear();
        m_extensionBasePoints.clear();
        m_extendingBezier = false;
        m_extendAtStart = false;
        m_bezierActive = true;
        m_bezierClosedPreview = false;
    }
    const int totalPointCount = m_extendingBezier
        ? m_extensionBasePoints.size() + qMax(0, m_bezierPoints.size() - 1)
        : m_bezierPoints.size();
    if (totalPointCount >= 3 && QLineF(bezierCloseTarget(), point).length() <= handleSize() * 2.0) {
        commitBezier(true);
        return;
    }
    if (m_bezierPoints.isEmpty() || QLineF(m_bezierPoints.last(), point).length() > 1.0)
        m_bezierPoints.append(point);
    m_dragCurrent = point;
    m_bezierClosedPreview = false;
    update();
}

bool CanvasWidget::tryOpenPolylineAtVertex(const QPointF &point, Qt::KeyboardModifiers modifiers)
{
    const bool commandModifier = (modifiers & (Qt::ControlModifier | Qt::MetaModifier)) != 0;
    if (!m_model || !commandModifier || !modifiers.testFlag(Qt::ShiftModifier))
        return false;

    const int vertexIndex = m_model->selectClosedPolylineVertexAt(point, handleSize() * 2.0);
    if (vertexIndex < 0)
        return false;
    if (!m_model->openSelectedPolylineAtVertex(vertexIndex))
        return false;

    notifyChanged();
    return true;
}

bool CanvasWidget::tryEditPolylinePoint(const QPointF &point, Qt::KeyboardModifiers modifiers)
{
    if (!modifiers.testFlag(Qt::ShiftModifier) || modifiers.testFlag(Qt::ControlModifier) || modifiers.testFlag(Qt::MetaModifier) || modifiers.testFlag(Qt::AltModifier))
        return false;
    if (!m_model)
        return false;
    if (!m_model->hasSelection() || m_model->selectedElementType() != ElementType::Polyline) {
        if (!m_model->selectAt(point) || m_model->selectedElementType() != ElementType::Polyline)
            return false;
    }

    const int vertexIndex = hitPolylineVertexHandle(point);
    if (vertexIndex >= 0) {
        if (m_model->deleteSelectedPolylinePoint(vertexIndex))
            notifyChanged();
        return true;
    }

    if (m_model->insertSelectedPolylinePoint(point, handleSize() * 3.0)) {
        notifyChanged();
        return true;
    }
    return false;
}

bool CanvasWidget::tryEditBezierControlPoint(const QPointF &point, Qt::KeyboardModifiers modifiers)
{
    if (!modifiers.testFlag(Qt::ShiftModifier) || modifiers.testFlag(Qt::ControlModifier) || modifiers.testFlag(Qt::MetaModifier) || modifiers.testFlag(Qt::AltModifier))
        return false;
    if (!m_model)
        return false;
    if (!m_model->hasSelection() || m_model->selectedElementType() != ElementType::Bezier) {
        if (!m_model->selectAt(point) || m_model->selectedElementType() != ElementType::Bezier)
            return false;
    }

    const int vertexIndex = hitPolylineVertexHandle(point);
    if (vertexIndex >= 0) {
        if (m_model->deleteSelectedBezierControlPoint(vertexIndex))
            notifyChanged();
        return true;
    }

    if (m_model->insertSelectedBezierControlPoint(point, handleSize() * 5.0)) {
        notifyChanged();
        return true;
    }
    return false;
}

bool CanvasWidget::tryStartPolylineExtension(const QPointF &point, Qt::KeyboardModifiers modifiers)
{
    if ((modifiers & (Qt::ControlModifier | Qt::MetaModifier)) == 0)
        return false;
    if (!m_model || m_polylineActive)
        return false;
    if (m_model->currentPageLocked())
        return false;

    const qreal hitRadius = handleSize() * 2.0;
    const int endpointIndex = m_model->selectOpenPolylineEndpointAt(point, hitRadius);
    if (endpointIndex < 0)
        return false;

    const QVector<QPointF> points = m_model->selectedElementPoints();
    if (points.size() < 2)
        return false;

    const bool hitStart = endpointIndex == 0;
    m_extensionBasePoints = points;
    m_polylinePoints = {hitStart ? points.first() : points.last()};
    m_extendingPolyline = true;
    m_extendAtStart = hitStart;
    m_polylineActive = true;
    m_polylineClosedPreview = false;
    m_dragCurrent = point;
    update();
    return true;
}

bool CanvasWidget::tryStartBezierExtension(const QPointF &point, Qt::KeyboardModifiers modifiers)
{
    if ((modifiers & (Qt::ControlModifier | Qt::MetaModifier)) == 0)
        return false;
    if (!m_model || m_bezierActive)
        return false;
    if (m_model->currentPageLocked())
        return false;

    const qreal hitRadius = handleSize() * 2.0;
    const int endpointIndex = m_model->selectBezierEndpointAt(point, hitRadius);
    if (endpointIndex < 0)
        return false;

    const QVector<QPointF> points = m_model->selectedElementPoints();
    if (points.size() < 2)
        return false;

    const bool hitStart = endpointIndex == 0;
    m_extensionBasePoints = points;
    m_bezierPoints = {hitStart ? points.first() : points.last()};
    m_extendingBezier = true;
    m_extendAtStart = hitStart;
    m_bezierActive = true;
    m_bezierClosedPreview = false;
    m_dragCurrent = point;
    update();
    return true;
}

void CanvasWidget::handleSelectMousePress(const QPointF &point, Qt::KeyboardModifiers modifiers)
{
    const bool locked = m_model->currentPageLocked();
    const bool extensionModifier = (modifiers & (Qt::ControlModifier | Qt::MetaModifier)) != 0;
    if (locked) {
        if (extensionModifier)
            m_model->addToSelectionAt(point);
        else
            m_model->selectAt(point);
        m_selectedPointIndex = -1;
        m_dragging = false;
        m_dragMode = DragMode::None;
        update();
        return;
    }

    if (hitRotationHandle(point)) {
        m_selectedPointIndex = -1;
        m_rotationCenter = m_model->selectedElementBounds().center();
        m_rotationStartPointerAngle = pointerAngle(m_rotationCenter, point);
        m_rotationDragDegrees = 0.0;
        if (m_model->beginRotationEdit(m_rotationCenter)) {
            m_dragging = true;
            m_dragMode = DragMode::Rotate;
        }
    } else if (tryEditPolylinePoint(point, modifiers)) {
        m_selectedPointIndex = -1;
        update();
    } else if (tryEditBezierControlPoint(point, modifiers)) {
        m_selectedPointIndex = -1;
        update();
    } else if (tryStartPolylineExtension(point, modifiers) || tryStartBezierExtension(point, modifiers)) {
        m_selectedPointIndex = -1;
        update();
    } else if (extensionModifier) {
        m_model->addToSelectionAt(point);
        m_selectedPointIndex = -1;
        update();
    } else if (const int arcHandle = hitArcAngleHandle(point); arcHandle >= 0) {
        m_selectedPointIndex = -1;
        m_arcAngleHandle = arcHandle;
        m_model->beginSelectionEdit();
        m_dragging = true;
        m_dragMode = DragMode::ArcAngle;
    } else if (const int vertexIndex = hitPolylineVertexHandle(point); vertexIndex >= 0) {
        m_selectedPointIndex = vertexIndex;
        m_polylineVertexIndex = vertexIndex;
        m_vertexDragStartPoint = point;
        m_vertexDragMoved = false;
        m_model->beginSelectionEdit();
        m_dragging = true;
        m_dragMode = DragMode::PolylineVertex;
    } else if (const ResizeHandle handle = hitResizeHandle(point); handle != ResizeHandle::None) {
        m_selectedPointIndex = -1;
        m_resizeHandle = handle;
        m_resizeStartRect = m_model->selectedElementGeometryBounds();
        m_resizeRotationDegrees = usesPointGeometry(m_model->selectedElementType())
            ? 0.0 : m_model->selectedElementRotationDegrees();
        const QPointF localAnchor = (handle == ResizeHandle::TopLeft) ? m_resizeStartRect.bottomRight()
            : (handle == ResizeHandle::TopRight) ? m_resizeStartRect.bottomLeft()
            : (handle == ResizeHandle::BottomLeft) ? m_resizeStartRect.topRight()
            : (handle == ResizeHandle::BottomRight) ? m_resizeStartRect.topLeft()
            : (handle == ResizeHandle::Top) ? QPointF(m_resizeStartRect.center().x(), m_resizeStartRect.bottom())
            : (handle == ResizeHandle::Right) ? QPointF(m_resizeStartRect.left(), m_resizeStartRect.center().y())
            : (handle == ResizeHandle::Bottom) ? QPointF(m_resizeStartRect.center().x(), m_resizeStartRect.top())
                                               : QPointF(m_resizeStartRect.right(), m_resizeStartRect.center().y());
        m_resizeAnchor = rotatePoint(localAnchor, m_resizeStartRect.center(), m_resizeRotationDegrees);
        m_model->beginSelectionEdit();
        m_dragging = true;
        m_dragMode = DragMode::Resize;
    } else {
        const qreal hitMargin = handleSize() * 2.0;
        if (!m_model->selectedElementsContain(point, hitMargin)) {
            m_model->selectAt(point);
            m_selectedPointIndex = -1;
        }
        if (m_model->selectedElementsContain(point, hitMargin)) {
            m_selectedPointIndex = -1;
            m_lastDragPoint = point;
            m_model->beginSelectionEdit();
            m_dragging = true;
            m_dragMode = DragMode::Move;
        } else {
            m_model->clearSelection();
            m_selectedPointIndex = -1;
            m_dragStart = point;
            m_dragCurrent = point;
            m_rubberBandRect = QRectF(m_dragStart, m_dragCurrent).normalized();
            m_dragging = true;
            m_dragMode = DragMode::RubberBand;
        }
    }
}

void CanvasWidget::commitPolyline(bool closed)
{
    if (!m_model || !m_polylineActive)
        return;
    if (m_extendingPolyline && m_polylinePoints.size() >= 2) {
        QVector<QPointF> updatedPoints = m_extensionBasePoints;
        const QVector<QPointF> addedPoints = m_polylinePoints.mid(1);
        if (m_extendAtStart) {
            for (const QPointF &point : addedPoints)
                updatedPoints.prepend(point);
        } else {
            updatedPoints += addedPoints;
        }
        m_model->updateSelectedPolyline(updatedPoints, closed);
        notifyChanged();
    } else if (m_polylinePoints.size() >= 2 && (!closed || m_polylinePoints.size() >= 3)) {
        m_model->addPolyline(m_polylinePoints, closed);
        notifyChanged();
    }
    m_polylinePoints.clear();
    m_extensionBasePoints.clear();
    m_polylineActive = false;
    m_polylineClosedPreview = false;
    m_extendingPolyline = false;
    m_extendAtStart = false;
    update();
}

void CanvasWidget::commitBezier(bool closed)
{
    if (!m_model || !m_bezierActive)
        return;
    QVector<QPointF> commitPoints = m_bezierPoints;
    if (!closed && !commitPoints.isEmpty() && QLineF(commitPoints.last(), m_dragCurrent).length() > 1.0)
        commitPoints.append(m_dragCurrent);
    if (m_extendingBezier && (commitPoints.size() >= 2 || (closed && commitPoints.size() >= 1))) {
        QVector<QPointF> updatedPoints = m_extensionBasePoints;
        const QVector<QPointF> addedPoints = commitPoints.mid(1);
        if (m_extendAtStart) {
            for (const QPointF &point : addedPoints)
                updatedPoints.prepend(point);
        } else {
            updatedPoints += addedPoints;
        }
        m_model->updateSelectedBezier(updatedPoints, closed);
        notifyChanged();
    } else if (commitPoints.size() >= 2 && (!closed || commitPoints.size() >= 3)) {
        m_model->addBezier(commitPoints, closed);
        notifyChanged();
    }
    m_bezierPoints.clear();
    m_extensionBasePoints.clear();
    m_bezierActive = false;
    m_extendingBezier = false;
    m_bezierClosedPreview = false;
    m_extendAtStart = false;
    update();
}

void CanvasWidget::cancelPolyline()
{
    if (!m_polylineActive)
        return;
    m_polylinePoints.clear();
    m_extensionBasePoints.clear();
    m_polylineActive = false;
    m_polylineClosedPreview = false;
    m_extendingPolyline = false;
    m_extendAtStart = false;
    update();
}

void CanvasWidget::cancelBezier()
{
    if (!m_bezierActive)
        return;
    m_bezierPoints.clear();
    m_extensionBasePoints.clear();
    m_bezierActive = false;
    m_extendingBezier = false;
    m_bezierClosedPreview = false;
    m_extendAtStart = false;
    update();
}

QRect CanvasWidget::textEditorGeometry(const QRectF &canvasRect) const
{
    const qreal scale = m_model ? m_model->settings().zoomPercent() / 100.0 : 1.0;
    QRect rect = QRectF(canvasRect.topLeft() * scale, canvasRect.size() * scale).toAlignedRect();
    rect.setWidth(qMax(rect.width(), qRound(180 * scale)));
    rect.setHeight(qMax(rect.height(), qRound(72 * scale)));
    return rect.adjusted(-2, -2, 2, 2);
}

void CanvasWidget::commitShape(const QPointF &endPoint)
{
    const Tool tool = m_model->settings().selectedTool();
    if (tool == Tool::Line) {
        m_model->addLine(m_dragStart, endPoint);
    } else if (tool == Tool::Rectangle) {
        m_model->addRectangle(QRectF(m_dragStart, endPoint));
    } else if (tool == Tool::RoundedRectangle) {
        m_model->addRoundedRectangle(QRectF(m_dragStart, endPoint));
    } else if (tool == Tool::Ellipse) {
        m_model->addEllipse(QRectF(m_dragStart, endPoint));
    } else if (tool == Tool::Arc) {
        m_model->addArc(QRectF(m_dragStart, endPoint));
    } else if (tool == Tool::Circle) {
        m_model->addCircle(QRectF(m_dragStart, endPoint));
    }
}

void CanvasWidget::drawActivePolylinePreview(QPainter &painter)
{
    if (!m_polylineActive || m_polylinePoints.isEmpty())
        return;

    QPen previewPen(QColor(QStringLiteral("#0f766e")));
    previewPen.setWidth(2);
    previewPen.setStyle(Qt::DashLine);
    painter.setPen(previewPen);
    painter.setBrush(Qt::NoBrush);
    for (int i = 1; i < m_polylinePoints.size(); ++i)
        painter.drawLine(m_polylinePoints.at(i - 1), m_polylinePoints.at(i));
    if (m_polylineClosedPreview)
        painter.drawLine(m_polylinePoints.last(), polylineCloseTarget());
    else
        painter.drawLine(m_polylinePoints.last(), m_dragCurrent);
}

void CanvasWidget::drawActiveBezierPreview(QPainter &painter)
{
    if (!m_bezierActive || m_bezierPoints.isEmpty())
        return;

    QPen previewPen(QColor(QStringLiteral("#0f766e")));
    previewPen.setWidth(2);
    previewPen.setStyle(Qt::DashLine);
    painter.setPen(previewPen);
    painter.setBrush(Qt::NoBrush);
    QVector<QPointF> previewPoints = m_bezierPoints;
    if (!m_bezierClosedPreview && !previewPoints.isEmpty() && QLineF(previewPoints.last(), m_dragCurrent).length() > 1.0)
        previewPoints.append(m_dragCurrent);
    painter.drawPath(bezierPath(previewPoints, m_bezierClosedPreview));
}

void CanvasWidget::drawShapePreview(QPainter &painter)
{
    if (!m_model || !m_dragging || m_dragMode != DragMode::Draw)
        return;

    QPen previewPen(QColor(QStringLiteral("#0f766e")));
    previewPen.setWidth(2);
    previewPen.setStyle(Qt::DashLine);
    painter.setPen(previewPen);
    painter.setBrush(Qt::NoBrush);
    const QRectF previewRect(m_dragStart, m_dragCurrent);
    switch (m_model->settings().selectedTool()) {
    case Tool::Line:
        painter.drawLine(m_dragStart, m_dragCurrent);
        break;
    case Tool::Rectangle:
        painter.drawRect(previewRect.normalized());
        break;
    case Tool::RoundedRectangle:
        painter.drawPath(roundedRectanglePath(previewRect.normalized(), m_model->settings().cornerRadius()));
        break;
    case Tool::Ellipse:
        painter.drawEllipse(previewRect.normalized());
        break;
    case Tool::Arc:
        painter.drawArc(squareRect(previewRect), 30 * 16, 240 * 16);
        break;
    case Tool::Circle:
        painter.drawEllipse(squareRect(previewRect));
        break;
    default:
        break;
    }
}

void CanvasWidget::drawRubberBandPreview(QPainter &painter)
{
    if (!m_dragging || m_dragMode != DragMode::RubberBand)
        return;

    QPen bandPen(QColor(QStringLiteral("#0f766e")));
    bandPen.setWidthF(1.5);
    bandPen.setStyle(Qt::DashLine);
    painter.setPen(bandPen);
    painter.setBrush(QColor(15, 118, 110, 28));
    painter.drawRect(m_rubberBandRect.normalized());
}

void CanvasWidget::drawSelection(QPainter &painter)
{
    if (!m_model || !m_model->hasSelection())
        return;

    const QRectF bounds = m_model->selectedElementBounds();
    const qreal selectionInset = handleSize() * 0.7;
    QPen selectionPen(QColor(QStringLiteral("#0f766e")));
    selectionPen.setWidthF(1.5);
    selectionPen.setStyle(Qt::DashLine);
    painter.setBrush(Qt::NoBrush);
    const QVector<QRectF> selectedBounds = m_model->selectedElementBoundsList();
    QPen haloPen(QColor(QStringLiteral("#ffffff")));
    haloPen.setWidthF(4.5);
    haloPen.setStyle(Qt::SolidLine);
    const bool orientedOutline = selectedBounds.size() == 1 && !usesPointGeometry(m_model->selectedElementType())
        && m_model->selectedElementType() != ElementType::Circle;
    if (orientedOutline) {
        const QPolygonF outline(m_model->selectedElementVisualCorners());
        painter.setPen(haloPen);
        painter.drawPolygon(outline);
        painter.setPen(selectionPen);
        painter.drawPolygon(outline);
    } else {
        for (const QRectF &selectedRect : selectedBounds) {
            const QRectF visualRect = selectedRect.adjusted(-selectionInset, -selectionInset, selectionInset, selectionInset);
            painter.setPen(haloPen);
            painter.drawRect(visualRect);
            painter.setPen(selectionPen);
            painter.drawRect(visualRect);
        }
    }

    if (m_model->selectionCanRotate() && !m_model->currentPageLocked()) {
        const QPointF rotationHandle = rotationHandlePoint();
        const QPointF connector = rotationHandle.y() < bounds.center().y()
            ? QPointF(bounds.center().x(), bounds.top())
            : QPointF(bounds.center().x(), bounds.bottom());
        painter.setPen(QPen(QColor(QStringLiteral("#0f766e")), 1.5));
        painter.drawLine(connector, rotationHandle);
        painter.setBrush(QColor(QStringLiteral("#ffffff")));
        const qreal rotationSize = handleSize() * 1.25;
        painter.drawEllipse(rotationHandle, rotationSize, rotationSize);
        if (m_dragMode == DragMode::Rotate) {
            painter.setPen(QColor(QStringLiteral("#344054")));
            painter.setFont(QFont(QStringLiteral("Sans Serif"), 11));
            const QString angleText = QStringLiteral("%1%2")
                .arg(qRound(m_rotationDragDegrees))
                .arg(QChar(0x00b0));
            painter.drawText(QRectF(rotationHandle + QPointF(14, -14), QSizeF(80, 28)), angleText);
        }
    }

    if (selectedBounds.size() > 1)
        return;

    const qreal size = handleSize();
    painter.setPen(QPen(QColor(QStringLiteral("#0f766e")), 1.2));
    painter.setBrush(QColor(QStringLiteral("#ffffff")));
    QVector<QPointF> handles;
    if (usesPointGeometry(m_model->selectedElementType())) {
        handles = m_model->selectedElementPoints();
    } else {
        handles = m_model->selectedElementVisualCorners();
        const ElementType type = m_model->selectedElementType();
        if (type == ElementType::Rectangle || type == ElementType::RoundedRectangle || type == ElementType::Ellipse || type == ElementType::Circle) {
            const QVector<QPointF> corners = handles;
            handles += {
                (corners.at(0) + corners.at(1)) / 2.0,
                (corners.at(1) + corners.at(2)) / 2.0,
                (corners.at(2) + corners.at(3)) / 2.0,
                (corners.at(3) + corners.at(0)) / 2.0,
            };
        }
        if (type == ElementType::Arc) {
            const QRectF geometryBounds = m_model->selectedElementGeometryBounds();
            const qreal rotation = m_model->selectedElementRotationDegrees();
            handles += {
                rotatePoint(pointOnEllipse(geometryBounds, m_model->selectedElementArcStartAngle()), geometryBounds.center(), rotation),
                rotatePoint(pointOnEllipse(geometryBounds, m_model->selectedElementArcStartAngle() + m_model->selectedElementArcSpanAngle()), geometryBounds.center(), rotation),
            };
        }
    }
    for (int i = 0; i < handles.size(); ++i) {
        const QPointF point = handles.at(i);
        const bool selectedPoint = i == m_selectedPointIndex
            && (m_model->selectedElementType() == ElementType::Line
                || m_model->selectedElementType() == ElementType::Polyline
                || m_model->selectedElementType() == ElementType::Bezier);
        painter.setBrush(selectedPoint ? QColor(QStringLiteral("#111827")) : QColor(QStringLiteral("#ffffff")));
        painter.drawRect(QRectF(point.x() - size / 2.0, point.y() - size / 2.0, size, size));
    }
}

void CanvasWidget::notifyChanged()
{
    refreshSize();
    update();
    if (m_changeCallback)
        m_changeCallback();
}

void CanvasWidget::drawElement(QPainter &painter, const DrawingElement &element)
{
    DrawingRenderer::drawElement(painter, element);
}
