// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// Q_OBJECT を持つ header-only Widget を定義し、paintEvent で赤い四角を描いてください。
// main() は書かず、moc 生成が必要な教材として使える形にしてください。

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

