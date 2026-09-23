#pragma once

#include <QLabel>

class FloatingActionBubbleLabel : public QLabel {
    Q_OBJECT

public:
    explicit FloatingActionBubbleLabel(QWidget *parent = nullptr);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
};
