// DragWindow は、標準タイトルバーを持たないウィンドウをドラッグ移動できるようにする
// 小さな基底クラスです。マウスイベントをアプリケーション全体のイベントフィルターで
// 受け取り、同じトップレベルウィンドウに属する子ウィジェット上のドラッグでも
// ウィンドウ全体を移動できるようにします。

#pragma once

#include <QPoint>
#include <QPointF>
#include <QWidget>

class DragWindow : public QWidget {
public:
    using QWidget::QWidget;

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    bool m_dragging = false;
    QPointF m_dragStartGlobal;
    QPoint m_windowStartPosition;
};
