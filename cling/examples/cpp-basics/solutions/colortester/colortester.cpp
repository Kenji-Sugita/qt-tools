#include "colortester.h"
#include <QLabel>
#include <QColorDialog>
#include <QPushButton>
#include <QLayout>

ColorTester::ColorTester(QWidget* parent)
    : QWidget(parent), colorLabel(new QLabel("Not Set"))
{
    colorLabel->setAutoFillBackground(true);
    colorLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    const auto colorSelectButton = new QPushButton("Select Color");
    connect(colorSelectButton, &QPushButton::clicked, this, &ColorTester::slotSelectColor);

    const auto topLayout = new QVBoxLayout(this);

    const auto colorNameLayout = new QHBoxLayout;
    colorNameLayout->addWidget(new QLabel("Color is: "), 0, Qt::AlignRight);
    colorNameLayout->addWidget(colorLabel, 0, Qt::AlignLeft);

    topLayout->addLayout(colorNameLayout, 1);
    topLayout->addWidget(colorSelectButton, 0, Qt::AlignHCenter);
}

void ColorTester::slotSelectColor()
{
    QColor currentColor(colorLabel->text());
    if (!currentColor.isValid()) {
        currentColor = Qt::black;
    }
    const QColor newColor = QColorDialog::getColor(currentColor, this);

    if (newColor.isValid()) {
        colorLabel->setText(newColor.name());

        QPalette newPalette = colorLabel->palette();
        newPalette.setColor(QPalette::Window, newColor);
        colorLabel->setPalette(newPalette);
    }
}
