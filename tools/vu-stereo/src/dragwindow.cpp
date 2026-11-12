// DragWindow の実装です。フレームレスウィンドウでも自然に移動できるよう、
// 子ウィジェット上で発生したマウス操作を拾い、トップレベルウィンドウの位置へ
// 反映します。描画やアプリケーション固有の状態は持ちません。

#include "dragwindow.h"

#include <QEvent>
#include <QMouseEvent>

bool DragWindow::eventFilter(QObject *object, QEvent *event)
{
    auto *widget = qobject_cast<QWidget *>(object);
    if (!widget || widget->window() != this) {
        return QWidget::eventFilter(object, event);
    }

    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            m_dragging = true;
            m_dragStartGlobal = mouseEvent->globalPosition();
            m_windowStartPosition = frameGeometry().topLeft();
        }
        break;
    }
    case QEvent::MouseMove: {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (m_dragging && (mouseEvent->buttons() & Qt::LeftButton)) {
            const QPoint delta = (mouseEvent->globalPosition() - m_dragStartGlobal).toPoint();
            move(m_windowStartPosition + delta);
        }
        break;
    }
    case QEvent::MouseButtonRelease: {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            m_dragging = false;
        }
        break;
    }
    default:
        break;
    }

    return QWidget::eventFilter(object, event);
}
