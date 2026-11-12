#include "moving.h"
#include <QPainter>
#include <QPaintEvent>
#include <QRandomGenerator>
#include <QDebug>
#include <stdlib.h>

Moving::Moving(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(300, 120);

    startPosition = QPoint(0, 0);
    endPosition = QPoint(minimumWidth() - sideLength, 2 * sideLength);
    currentPosition = startPosition;
}

void Moving::paintEvent(QPaintEvent* event)
{
    qDebug() << event->rect() << event->region();
    QPainter painter(this);
    painter.fillRect(QRect(currentPosition, QSize(sideLength, sideLength)), Qt::red);
}

void Moving::move()
{
    update(QRect(currentPosition, QSize(sideLength, sideLength)));
    currentPosition.rx() = double(endPosition.x()) * (double(QRandomGenerator::system()->generate()) / double(UINT32_MAX));
    currentPosition.ry() = double(endPosition.y()) * (double(QRandomGenerator::system()->generate()) / double(UINT32_MAX));
    update(QRect(currentPosition, QSize(sideLength, sideLength)));
}

void Moving::reset()
{
    update(QRect(currentPosition, QSize(sideLength, sideLength)));
    currentPosition = startPosition;
    update(QRect(currentPosition, QSize(sideLength, sideLength)));
}

