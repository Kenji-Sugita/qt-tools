#pragma once

#include "BoardDocument.h"

#include <QColor>
#include <QFont>
#include <QPoint>
#include <QRectF>
#include <QSize>
#include <QSizeF>
#include <QString>
#include <QVector>

class BoardModel {
public:
    enum class PageElementApplyMode {
        AppendCurrent,
        ReplaceCurrent,
        NewPage
    };

    static constexpr int CanvasWidth = 1920;
    static constexpr int CanvasHeight = 1080;
    static constexpr int MinZoomPercent = 10;
    static constexpr int MaxZoomPercent = 400;
    static constexpr int MaxPages = BoardDocument::MaxPages;
    inline static const QColor DefaultColor = QColor(QStringLiteral("#344054"));
    inline static const QColor DefaultFillColor = QColor(0, 0, 0, 0);
    inline static const QFont DefaultFont = QFont(QStringLiteral("Sans Serif"), 18);
    static constexpr int DefaultStrokeWidth = 3;
    static constexpr int DefaultCornerRadius = 24;
    static constexpr StrokeStyle DefaultStrokeStyle = StrokeStyle::Solid;
    static constexpr ArrowHead DefaultStartArrowHead = ArrowHead::None;
    static constexpr ArrowHead DefaultEndArrowHead = ArrowHead::None;

    const BoardDocument &document() const;
    BoardDocument &document();
    void setDocument(const BoardDocument &document);

    BoardSettings settings() const;
    Page currentPage() const;

    void selectTool(Tool tool);
    void setSelectedColor(const QColor &color);
    void setFillColor(const QColor &color);
    void setStrokeWidth(int strokeWidth);
    void setCornerRadius(int cornerRadius);
    void setStrokeStyle(StrokeStyle strokeStyle);
    void setFont(const QFont &font);
    void setWindowSize(const QSize &windowSize);
    void setFloatingActionDockY(int dockY);
    void setStartArrowHead(ArrowHead arrowHead);
    void setEndArrowHead(ArrowHead arrowHead);
    void resetDefaultStyle();
    int setZoomPercent(int zoomPercent);

    void addFreehand(const QVector<QPointF> &points);
    void addLine(const QPointF &start, const QPointF &end);
    void addPolyline(const QVector<QPointF> &points, bool closed = false);
    void addBezier(const QVector<QPointF> &points, bool closed = false);
    void addBezier(const QPointF &start, const QPointF &end);
    void addArc(const QRectF &rect);
    void addCircle(const QRectF &rect);
    void addRectangle(const QRectF &rect);
    void addRoundedRectangle(const QRectF &rect);
    void addEllipse(const QRectF &rect);
    void addText(const QPointF &position, const QString &text);
    bool eraseAt(const QPointF &point);

    int selectedElementIndex() const;
    QVector<int> selectedElementIndexes() const;
    bool hasSelection() const;
    QRectF selectedElementBounds() const;
    QRectF selectedElementGeometryBounds() const;
    QVector<QRectF> selectedElementBoundsList() const;
    QVector<QPointF> selectedElementVisualCorners() const;
    QVector<QPointF> selectedElementPoints() const;
    qreal selectedElementRotationDegrees() const;
    bool selectedElementsContain(const QPointF &point, qreal radius = 0.0) const;
    bool selectionCanRotate() const;
    QString selectedElementText() const;
    QColor selectedElementColor() const;
    QColor selectedElementFillColor() const;
    int selectedElementStrokeWidth() const;
    int selectedElementCornerRadius() const;
    StrokeStyle selectedElementStrokeStyle() const;
    QFont selectedElementFont() const;
    ElementType selectedElementType() const;
    bool selectedElementClosed() const;
    bool selectedSelectionHasGroup() const;
    ArrowHead selectedElementStartArrowHead() const;
    ArrowHead selectedElementEndArrowHead() const;
    int selectedElementArcStartAngle() const;
    int selectedElementArcSpanAngle() const;
    void clearSelection();
    bool selectAt(const QPointF &point);
    bool addToSelectionAt(const QPointF &point);
    bool selectInRect(const QRectF &rect);
    int selectOpenPolylineEndpointAt(const QPointF &point, qreal radius);
    int selectBezierEndpointAt(const QPointF &point, qreal radius);
    int selectClosedPolylineVertexAt(const QPointF &point, qreal radius);
    void beginSelectionEdit();
    bool beginRotationEdit(const QPointF &pivot);
    void endSelectionEdit();
    bool moveSelectedBy(const QPointF &delta);
    bool resizeSelectedTo(const QRectF &rect);
    bool scaleSelectedBy(qreal scaleFactor);
    bool rotateSelectionTo(qreal degrees);
    bool resetSelectedRotation();
    bool moveSelectedVertexTo(int vertexIndex, const QPointF &point);
    bool updateSelectedArcAngles(int startAngle, int spanAngle);
    bool updateSelectedArrowHeads(ArrowHead startArrowHead, ArrowHead endArrowHead);
    bool updateSelectedPolyline(const QVector<QPointF> &points, bool closed);
    bool insertSelectedPolylinePoint(const QPointF &point, qreal hitRadius);
    bool deleteSelectedPolylinePoint(int pointIndex);
    bool updateSelectedBezier(const QVector<QPointF> &points, bool closed = false);
    bool insertSelectedBezierControlPoint(const QPointF &point, qreal hitRadius);
    bool deleteSelectedBezierControlPoint(int pointIndex);
    bool closeSelectedPolyline();
    bool openSelectedPolyline();
    bool openSelectedPolylineAtVertex(int vertexIndex);
    bool closeSelectedBezier();
    bool openSelectedBezier();
    bool deleteSelectedElement();
    bool moveSelectedForward();
    bool moveSelectedBackward();
    bool moveSelectedToFront();
    bool moveSelectedToBack();
    QVector<DrawingElement> copySelectedElements() const;
    bool pasteElements(const QVector<DrawingElement> &elements);
    bool updateSelectedText(const QString &text);
    bool updateSelectedColor(const QColor &color);
    bool updateSelectedTextColor(const QColor &color);
    bool updateSelectedTextFont(const QFont &font);
    bool updateSelectedStrokeWidth(int strokeWidth);
    bool updateSelectedCornerRadius(int cornerRadius);
    bool updateSelectedStrokeStyle(StrokeStyle strokeStyle);
    bool updateSelectedFillColor(const QColor &color);
    bool resetSelectedStyle();
    bool groupSelectedElements();
    bool ungroupSelectedElements();

    bool canUndo() const;
    bool canRedo() const;
    bool undo();
    bool redo();

    bool canAddPage() const;
    bool canDeletePage() const;
    bool addPage();
    bool deleteCurrentPage();
    void nextPage();
    void previousPage();
    QPoint currentPageScrollPosition() const;
    void setCurrentPageScrollPosition(const QPoint &scrollPosition);
    bool currentPageLocked() const;
    bool setCurrentPageLocked(bool locked);
    bool applyElementsToPage(const QVector<DrawingElement> &elements, PageElementApplyMode mode);

    bool saveToFile(const QString &filePath) const;
    bool loadFromFile(const QString &filePath);

private:
    void pushUndoState();
    void clearRedoState();
    DrawingElement makeElement(ElementType type) const;
    QVector<int> selectedIndexesOrFocused() const;
    void expandSelectionForGroups();
    int nextGroupId() const;
    QSizeF textSize(const QString &text, const QFont &font) const;

    BoardDocument m_document;
    QVector<BoardDocument> m_undoStack;
    QVector<BoardDocument> m_redoStack;
    int m_selectedElementIndex = -1;
    QVector<int> m_selectedElementIndexes;
    bool m_selectionEditActive = false;
    bool m_selectionEditSnapshotPushed = false;
    QPointF m_rotationPivot;
    QVector<int> m_rotationElementIndexes;
    QVector<DrawingElement> m_rotationStartElements;
    bool m_rotationEditActive = false;
};
