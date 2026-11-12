#include "colortester.h"
#include <QLabel>
#include <QPushButton>
#include <QLayout>

ColorTester::ColorTester(QWidget* parent)
    : QWidget(parent), colorLabel(new QLabel("Not Set"))
{
    colorLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    const auto colorSelectButton = new QPushButton("Select Color");

    const auto topLayout = new QVBoxLayout(this);

    const auto colorNameLayout = new QHBoxLayout;
    colorNameLayout->addWidget(new QLabel("Color is: "), 0, Qt::AlignRight);
    colorNameLayout->addWidget(colorLabel, 0, Qt::AlignLeft);

    topLayout->addLayout(colorNameLayout, 1);
    topLayout->addWidget(colorSelectButton, 0, Qt::AlignHCenter);
}
