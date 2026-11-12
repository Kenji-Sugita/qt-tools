#include "scribblearea.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

ScribbleArea::ScribbleArea(QWidget* parent)
    : QWidget(parent)
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
    const int penWidth = 3;

    QPainter painter(&renderBuffer);
    painter.setPen(QPen(Qt::black, penWidth));
    painter.drawLine(lastMousePoint, event->pos());
#if 1
    // Updating minimum region.
    const QRect updatingRect = QRect(lastMousePoint, event->pos()).normalized().adjusted(-penWidth, -penWidth, penWidth, penWidth);
    update(updatingRect);

    // Illustrating the updating region with a red-bordered square.
    //painter.setPen(QPen(Qt::red, 1));
    //painter.drawRect(updatingRect.adjusted(1, 1, -1, -1));
#else
    // Updating whole region. It's expensive.
    update();
#endif

    lastMousePoint = event->pos();
}

void ScribbleArea::paintEvent(QPaintEvent* event)
{
    qDebug() << Q_FUNC_INFO << event->rect() << event->region();
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
