#pragma once

#include <QPointer>
#include <QToolButton>

class FloatingActionBubbleLabel;

class FloatingActionChildButton : public QToolButton {
    Q_OBJECT

public:
    explicit FloatingActionChildButton(QWidget *parent = nullptr, QWidget *labelParent = nullptr);

    void setBubbleText(const QString &text);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void ensureBubble();
    void showHoverState();
    void hideHoverState();
    void updateBubbleGeometry();

    QPointer<QWidget> m_labelParent;
    QPointer<FloatingActionBubbleLabel> m_bubbleLabel;
    QString m_bubbleText;
    bool m_hovered = false;
};
