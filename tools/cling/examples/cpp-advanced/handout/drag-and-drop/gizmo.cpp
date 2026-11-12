#include "gizmo.h"
#include <QPainter>
#include <QKeyEvent>
#include <QDebug>
#include <QApplication>
#include <cmath>

Gizmo::Gizmo(const QColor& color1, const QColor& color2, Qt::Orientation orientation, QWidget* parent)
    : QWidget(parent)
{
    this->color1 = color1;
    this->color2 = color2;
    this->orientation = orientation;
    setFocusPolicy(Qt::StrongFocus);
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
