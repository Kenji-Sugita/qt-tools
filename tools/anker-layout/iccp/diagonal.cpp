#include "AnkerFormLayout.h"

#include <QPushButton>

using namespace Anker;

QWidget *go()
{
    auto *window = new QWidget;
    window->setWindowTitle("Diagonal Form Layout Example");

    auto *layout = new FormLayout(window);
    layout->setContentsMargins(12, 12, 12, 12);

    auto *leftButton = new QPushButton("Left", window);
    auto *rightButton = new QPushButton("Right", window);

    layout->addWidget(leftButton, {
        .left = attachForm(50),
        .top = attachForm(50),
    });

    layout->addWidget(rightButton, {
        .left = attachOppositeWidget(leftButton, 100),
        .top = attachOppositeWidget(leftButton, 100),
    });

    window->resize(420, 320);
    window->show();
    window->raise();
    return window;
}
