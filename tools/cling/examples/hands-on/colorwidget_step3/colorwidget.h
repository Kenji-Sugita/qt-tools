#pragma once

#include <QWidget>
#include <QColor>
#include "ui_colorwidget.h"

class ColorWidget : public QWidget, private Ui::ColorWidget
{
    Q_OBJECT

public:
    explicit ColorWidget(QWidget* parent = nullptr);

    QColor color() const { return m_color; }

public slots:
    void setColor(const QColor& color);

signals:
    void colorChanged(const QColor& color);

private slots:
    void updateColor();

private:
    void applyToFrame(const QColor& color);
    void syncSlidersFromColor(const QColor& color);
    QColor m_color;
};
