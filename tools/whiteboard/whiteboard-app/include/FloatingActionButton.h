#pragma once

#include <QPoint>
#include <QToolButton>

class QMouseEvent;

class FloatingActionButton : public QToolButton {
    Q_OBJECT

public:
    explicit FloatingActionButton(QWidget *parent = nullptr);

signals:
    void activated();
    void draggedTo(int topY);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    bool m_pressed = false;
    bool m_moved = false;
    int m_dragOffsetY = 0;
    QPoint m_pressGlobalPos;
};
