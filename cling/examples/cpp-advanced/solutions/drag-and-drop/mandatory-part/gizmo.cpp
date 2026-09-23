#include "gizmo.h"
#include <QPainter>
#include <QKeyEvent>
#include <QDebug>
#include <QApplication>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
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
    QRect rect1;
    QRect rect2;

    if (orientation == Qt::Vertical) {
        rect1 = QRect(0, 0, width()/2, height());
        rect2 = QRect(width()/2, 0, width()/2, height());
    } else {
        rect1 = QRect(0, 0, width(), height()/2);
        rect2 = QRect(0, height()/2, width(), height()/2);
    }

    QPainter painter(this);

    painter.setPen(color1);
    painter.setBrush(QBrush(color1, Qt::SolidPattern));
    painter.drawRect(rect1);

    painter.setPen(color2);
    painter.setBrush(QBrush(color2, Qt::SolidPattern));
    painter.drawRect(rect2);

    if (hasFocus()) {
        qreal penWidth = 1.0;
        if (penWidth == 0.0) {
            penWidth = 1.0 / devicePixelRatio();
        }
        QRectF focusRect(0, 0, width(), height());
        if (devicePixelRatio() == 1) {
            const qreal topLeftDelta = int(penWidth)/2;
            const qreal bottomRightDelta = -((int(penWidth) + 1)/2);
            focusRect.adjust(topLeftDelta, topLeftDelta, bottomRightDelta, bottomRightDelta);
        } else if (devicePixelRatio() == 2) {
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

QSize Gizmo::sizeHint() const
{
    return QSize(100, 100);
}

void Gizmo::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_C && event->modifiers() & Qt::ControlModifier) {
        qDebug() << "Copy";
    } else if (event->key() == Qt::Key_V && event->modifiers() & Qt::ControlModifier) {
        qDebug() << "Paste";
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
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream << color1 << color2 << static_cast<quint32>(orientation);

        QDrag* const drag = new QDrag(this);
#if defined(Q_OS_WINDOWS)
	// To suppress the following warning:
	// QPixmap::scaled: Pixmap is a null pixmap
	auto pixmap = QPixmap(1, 1);
	pixmap.fill(Qt::transparent);
	drag->setPixmap(pixmap);
#endif
        QMimeData* const mimeData = new QMimeData;
        mimeData->setData("x-gizmo/x-drag", data);
        mimeData->setText("hello world");
        drag->setMimeData(mimeData);
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
    if (mimeData->hasFormat("x-gizmo/x-drag")) {
        const QByteArray data = mimeData->data("x-gizmo/x-drag");
        QDataStream stream(data);
        Q_ASSERT(sizeof(Qt::Orientation) == sizeof(int));
        stream >> color1 >> color2 >> reinterpret_cast<int&>(orientation);
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
