#include "harness.h"
#include "widget.h"
#include <QLayout>
#include <QDebug>

QWidget* makeWidget(QColor color)
{
    auto widget = new Widget;
    widget->setAutoFillBackground(true);
    auto palette = widget->palette();
    palette.setColor(QPalette::Window, color);
    widget->setPalette(palette);

    return widget;
}

Harness::Harness(QWidget* parent)
    : QWidget(parent)
{
    auto topLayout = new QHBoxLayout(this);

    auto leftWidget = makeWidget("Red");
    auto rightWidget = makeWidget("Blue");

    topLayout->addWidget(leftWidget);
    topLayout->addWidget(rightWidget);
}
