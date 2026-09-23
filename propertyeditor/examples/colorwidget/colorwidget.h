#pragma once

#include <QWidget>
#include <QColor>
#include "ui_colorwidget.h"
#include "widget_global.h"

class WIDGET_EXPORT ColorWidget
    : public QWidget, private Ui::ColorWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

public:
    explicit ColorWidget(QWidget* parent = nullptr);
    explicit ColorWidget(const QColor& initialColor, QWidget* parent = nullptr);

    QColor color() const { return m_color; }

public slots:
    void setColor(const QColor& color);

signals:
    void colorChanged(const QColor& color);

private slots:
    void updateColor();

private:
    void init();
    void applyToFrame(const QColor& color);
    void syncSlidersFromColor(const QColor& color);
    QColor m_color;
};
