#pragma once

#include <QWidget>

class Widget : public QWidget {
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr) : QWidget(parent) {}

public slots:
    void updateTitle() {
        setWindowTitle("clicked");
    }
};

