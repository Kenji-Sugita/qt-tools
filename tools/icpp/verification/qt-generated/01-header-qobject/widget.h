#pragma once

#include <QPainter>
#include <QPaintEvent>
#include <QSize>
#include <QWidget>

class Widget : public QWidget {
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowTitle("01 Header + Q_OBJECT");
    }

    QSize sizeHint() const override {
        return QSize(160, 160);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.fillRect(rect().adjusted(24, 24, -24, -24), Qt::red);
    }
};

