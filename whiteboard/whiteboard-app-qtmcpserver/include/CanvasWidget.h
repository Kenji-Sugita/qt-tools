#pragma once

#include "BoardModel.h"

#include <QPointF>
#include <QWidget>

#include <functional>

class QPlainTextEdit;
class QKeyEvent;
class QFocusEvent;
class QImage;

class CanvasWidget : public QWidget {
public:
    explicit CanvasWidget(QWidget *parent = nullptr);

    void setModel(BoardModel *model);
    void setChangeCallback(std::function<void()> callback);
    void refreshSize();
    QImage renderPageImage();
    void commitPendingTextEdit();
    bool hasPendingTextEdit() const;
    bool closeActivePolyline();
    bool closeActiveBezier();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    enum class DragMode {
        None,
        Draw,
        Move,
        Resize,
        Rotate,
        PolylineVertex,
        ArcAngle,
        RubberBand
    };

    enum class ResizeHandle {
        None,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        Top,
        Right,
        Bottom,
        Left
    };

    QPointF toCanvasPoint(const QPointF &widgetPoint) const;
    qreal handleSize() const;
    ResizeHandle hitResizeHandle(const QPointF &canvasPoint) const;
    bool hitRotationHandle(const QPointF &canvasPoint) const;
    QPointF rotationHandlePoint() const;
    int hitPolylineVertexHandle(const QPointF &canvasPoint) const;
    int hitArcAngleHandle(const QPointF &canvasPoint) const;
    QRectF resizedRect(const QPointF &canvasPoint) const;
    void startTextEditor(const QPointF &position, const QString &text, bool editingExistingText);
    void commitTextEditor();
    QRect textEditorGeometry(const QRectF &canvasRect) const;
    QPointF polylineCloseTarget() const;
    QPointF bezierCloseTarget() const;
    void addPolylinePoint(const QPointF &point);
    void addBezierPoint(const QPointF &point);
    bool tryOpenPolylineAtVertex(const QPointF &point, Qt::KeyboardModifiers modifiers);
    bool tryEditPolylinePoint(const QPointF &point, Qt::KeyboardModifiers modifiers);
    bool tryEditBezierControlPoint(const QPointF &point, Qt::KeyboardModifiers modifiers);
    bool tryStartPolylineExtension(const QPointF &point, Qt::KeyboardModifiers modifiers);
    bool tryStartBezierExtension(const QPointF &point, Qt::KeyboardModifiers modifiers);
    void handleSelectMousePress(const QPointF &point, Qt::KeyboardModifiers modifiers);
    void commitPolyline(bool closed = false);
    void commitBezier(bool closed = false);
    void cancelPolyline();
    void cancelBezier();
    void commitShape(const QPointF &endPoint);
    void notifyChanged();
    void drawActivePolylinePreview(QPainter &painter);
    void drawActiveBezierPreview(QPainter &painter);
    void drawShapePreview(QPainter &painter);
    void drawRubberBandPreview(QPainter &painter);
    void drawElement(QPainter &painter, const DrawingElement &element);
    void drawSelection(QPainter &painter);

    BoardModel *m_model = nullptr;
    QVector<QPointF> m_currentStroke;
    QVector<QPointF> m_polylinePoints;
    QVector<QPointF> m_bezierPoints;
    QVector<QPointF> m_extensionBasePoints;
    QPointF m_dragStart;
    QPointF m_dragCurrent;
    QPointF m_vertexDragStartPoint;
    QRectF m_rubberBandRect;
    QPointF m_lastDragPoint;
    QRectF m_resizeStartRect;
    QPointF m_resizeAnchor;
    qreal m_resizeRotationDegrees = 0.0;
    QPointF m_rotationCenter;
    qreal m_rotationStartPointerAngle = 0.0;
    qreal m_rotationDragDegrees = 0.0;
    ResizeHandle m_resizeHandle = ResizeHandle::None;
    DragMode m_dragMode = DragMode::None;
    int m_selectedPointIndex = -1;
    int m_polylineVertexIndex = -1;
    int m_arcAngleHandle = -1;
    QPlainTextEdit *m_textEditor = nullptr;
    QPointF m_textEditorPosition;
    bool m_editingExistingText = false;
    bool m_textEditorAppFilterInstalled = false;
    bool m_polylineActive = false;
    bool m_bezierActive = false;
    bool m_polylineClosedPreview = false;
    bool m_bezierClosedPreview = false;
    bool m_extendingPolyline = false;
    bool m_extendingBezier = false;
    bool m_extendAtStart = false;
    bool m_dragging = false;
    bool m_vertexDragMoved = false;
    bool m_shiftPressed = false;
    std::function<void()> m_changeCallback;
};
