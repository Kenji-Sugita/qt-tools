#pragma once

#include <QFrame>
#include <QLabel>
#include <QSlider>
#include <QWidget>

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);

private slots:
    void updateColor();

private:
    QFrame *colorFrame = nullptr;
    QLabel *valueLabel = nullptr;
    QSlider *redSlider = nullptr;
};

