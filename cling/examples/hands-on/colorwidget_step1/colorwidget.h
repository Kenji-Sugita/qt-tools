#pragma once

#include <QWidget>
#include "ui_colorwidget.h"

class ColorWidget : public QWidget, private Ui::ColorWidget
{
    Q_OBJECT

public:
    explicit ColorWidget(QWidget* parent = nullptr);

private slots:
    void updateColor();
};
