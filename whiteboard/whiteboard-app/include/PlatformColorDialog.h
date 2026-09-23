#pragma once

#include <QColor>

class QWidget;

class PlatformColorDialog {
public:
    static QColor getFillColor(const QColor &initialColor, QWidget *parent);
};
