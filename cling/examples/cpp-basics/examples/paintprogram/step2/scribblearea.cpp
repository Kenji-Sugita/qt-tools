#include "scribblearea.h"
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPixmap>
#include <QResizeEvent>
#include <QPainter>

ScribbleArea::ScribbleArea(QWidget* parent)
    : QWidget(parent),
      penStyle(Qt::SolidLine),
      penWidth(3)
{
}

QSize ScribbleArea::sizeHint() const
{
    return QSize(400, 400);
}

void ScribbleArea::mousePressEvent(QMouseEvent* event)
{
    lastMousePoint = event->pos();
}

void ScribbleArea::mouseMoveEvent(QMouseEvent* event)
{
    QPainter painter(&renderBuffer);
    QPen pen(penColor, penWidth);
    pen.setStyle(penStyle);
    painter.setPen(pen);
    painter.drawLine(lastMousePoint, event->pos());
    const QRect updatingRect = QRect(lastMousePoint, event->pos()).normalized().adjusted(-penWidth, -penWidth, penWidth, penWidth);
    update(updatingRect);

    lastMousePoint = event->pos();
}

void ScribbleArea::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.drawPixmap(0, 0, renderBuffer);
}

void ScribbleArea::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    if (width() > renderBuffer.width() || height() > renderBuffer.height()) {
        QPixmap newBuffer(size());
        newBuffer.fill(Qt::white);

        QPainter painter(&newBuffer);
        painter.drawPixmap(0, 0, renderBuffer);
        renderBuffer = newBuffer;
    }
}

void ScribbleArea::slotChangeColor(const QColor& color)
{
    penColor = color;
}

void ScribbleArea::setPenStyle(const Qt::PenStyle style)
{
    penStyle = style;
}

void ScribbleArea::setPenWidth(int width)
{
    penWidth = width;
}
