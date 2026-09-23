#include "gizmo.h"
#include "GizmoData.h"
#include <QPainter>
#include <QKeyEvent>
#include <QDebug>
#include <QApplication>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QClipboard>
#include <cmath>

Gizmo::Gizmo(const QColor& color1, const QColor& color2, Qt::Orientation orientation, QWidget* parent)
    : QWidget(parent)
{
    this->color1 = color1;
    this->color2 = color2;
    this->orientation = orientation;
    setFocusPolicy(Qt::StrongFocus);
    setAcceptDrops(true);
}

void Gizmo::paintEvent(QPaintEvent*)
{
    paint(this, false);
}

QSize Gizmo::sizeHint() const
{
    return QSize(100, 100);
}

void Gizmo::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_C && event->modifiers() & Qt::ControlModifier) {
        GizmoData* const data = new GizmoData(this);
        QApplication::clipboard()->setMimeData(data);
    } else if (event->key() == Qt::Key_V && event->modifiers() & Qt::ControlModifier) {
        GizmoData::decode(this, QApplication::clipboard()->mimeData());
        update();
    }
}

bool Gizmo::shouldStartDrag(QMouseEvent* event) const
{
    return ((event->buttons() & Qt::LeftButton)
            && (pressPos - event->pos()).manhattanLength() > QApplication::startDragDistance());
}

void Gizmo::mousePressEvent(QMouseEvent* event)
{
    pressPos = event->pos();
}

void Gizmo::mouseMoveEvent(QMouseEvent* event)
{
    if (shouldStartDrag(event)) {
        QDrag* const drag = new QDrag(this);
        GizmoData* const mimeData = new GizmoData(this);
        drag->setMimeData(mimeData);

        QPixmap pixmap(size());
        paint(&pixmap, true);
        drag->setPixmap(pixmap);

        Qt::DropAction dropAction = drag->exec(Qt::CopyAction | Qt::MoveAction);
        if (dropAction == Qt::MoveAction) {
            // Delete the data dragged.
        }
    }
    QWidget::mouseMoveEvent(event);
}

void Gizmo::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasText() || event->mimeData()->hasFormat("x-gizmo/x-drag")) {
        event->accept();
    }
}

void Gizmo::dropEvent(QDropEvent* event)
{
    if (!(event->possibleActions() & (Qt::CopyAction | Qt::MoveAction))) {
        return;
    }

    const QMimeData* const mimeData = event->mimeData();
    if (GizmoData::decode(this, mimeData)) {
        if (event->possibleActions() & Qt::CopyAction) {
            event->setDropAction(Qt::CopyAction);
            event->accept();
        } else {
            // Omit checking a proposed action.
            event->acceptProposedAction();
        }
        update();
    } else if (mimeData->hasText()) { // Dead code
        QString text = mimeData->text();
        qDebug() << text;
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
}

void Gizmo::paint(QPaintDevice* device, bool enableFocusRing) const
{
    QRect rect1;
    QRect rect2;

    if (orientation == Qt::Vertical) {
        rect1 = QRect(0, 0, width()/2, height());
        rect2 = QRect(width()/2, 0, width()/2, height());
    } else {
        rect1 = QRect(0, 0, width(), height()/2);
        rect2 = QRect(0, height()/2, width(), height()/2);
    }

    QPainter painter(device);

    painter.setPen(color1);
    painter.setBrush(QBrush(color1, Qt::SolidPattern));
    painter.drawRect(rect1);

    painter.setPen(color2);
    painter.setBrush(QBrush(color2, Qt::SolidPattern));
    painter.drawRect(rect2);

    if (!enableFocusRing && hasFocus()) {
        qreal penWidth = 1.0;
        if (penWidth == 0.0) {
            penWidth = 1.0 / device->devicePixelRatio();
        }
        QRectF focusRect(0, 0, device->width(), device->height());
        if (device->devicePixelRatio() == 1) {
            const qreal topLeftDelta = int(penWidth)/2;
            const qreal bottomRightDelta = -((int(penWidth) + 1)/2);
            focusRect.adjust(topLeftDelta, topLeftDelta, bottomRightDelta, bottomRightDelta);
        } else if (device->devicePixelRatio() == 2) {
            const qreal topLeftDelta = floor(penWidth)/2.0;
            const qreal bottomRightDelta = -floor(penWidth + 0.5)/2.0;
            focusRect.adjust(topLeftDelta, topLeftDelta, bottomRightDelta, bottomRightDelta);
        } else {
            qWarning() << "Not yet implemented.";
        }

        painter.setBrush(Qt::NoBrush);
        QPen pen(Qt::white, penWidth);
        painter.setPen(pen);
        painter.drawRect(focusRect);

        pen.setColor(Qt::black);
        pen.setStyle(Qt::DotLine);
        painter.setPen(pen);
        painter.drawRect(focusRect);
    }
}
