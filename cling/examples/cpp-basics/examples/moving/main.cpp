#include "moving.h"
#include <QPushButton>
#include <QLayout>
#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QWidget harness;
    const auto topLayout = new QVBoxLayout(&harness);

    const auto moving = new Moving();
    const auto movingLayout = new QHBoxLayout;
    movingLayout->addStretch();
    movingLayout->addWidget(moving);
    movingLayout->addStretch();
    topLayout->addLayout(movingLayout);
    topLayout->addStretch();

    const auto moveButton = new QPushButton("Move");
    QObject::connect(moveButton, &QPushButton::clicked, moving, &Moving::move);
    const auto resetButton = new QPushButton("Reset");
    QObject::connect(resetButton, &QPushButton::clicked, moving, &Moving::reset);
    const auto buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(moveButton);
    buttonLayout->addSpacing(12);
    buttonLayout->addWidget(resetButton);
    buttonLayout->addStretch();
    topLayout->addLayout(buttonLayout);

    harness.show();

    return app.exec();
}
