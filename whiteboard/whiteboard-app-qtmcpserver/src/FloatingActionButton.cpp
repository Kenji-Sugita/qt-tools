#include "FloatingActionButton.h"

#include <QApplication>
#include <QMouseEvent>

FloatingActionButton::FloatingActionButton(QWidget *parent)
    : QToolButton(parent)
{
}

void FloatingActionButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QToolButton::mousePressEvent(event);
        return;
    }

    m_pressed = true;
    m_moved = false;
    m_dragOffsetY = event->pos().y();
    m_pressGlobalPos = event->globalPosition().toPoint();
    setDown(true);
    grabMouse();
    event->accept();
}

void FloatingActionButton::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_pressed) {
        QToolButton::mouseMoveEvent(event);
        return;
    }

    if (!m_moved
        && (event->globalPosition().toPoint() - m_pressGlobalPos).manhattanLength()
            < QApplication::startDragDistance()) {
        event->accept();
        return;
    }

    m_moved = true;
    if (QWidget *parent = parentWidget()) {
        QWidget *coordinateParent = parent->parentWidget() ? parent->parentWidget() : parent;
        emit draggedTo(coordinateParent->mapFromGlobal(event->globalPosition().toPoint()).y() - m_dragOffsetY);
    }
    event->accept();
}

void FloatingActionButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_pressed || event->button() != Qt::LeftButton) {
        QToolButton::mouseReleaseEvent(event);
        return;
    }

    releaseMouse();
    setDown(false);
    const bool movedOnRelease = (event->globalPosition().toPoint() - m_pressGlobalPos).manhattanLength()
        >= QApplication::startDragDistance();
    const bool activate = !m_moved && !movedOnRelease;
    m_pressed = false;
    m_moved = false;
    if (activate)
        emit activated();
    event->accept();
}
