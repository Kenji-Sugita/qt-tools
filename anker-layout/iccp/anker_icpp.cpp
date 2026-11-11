#include "AnkerFormLayout.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

using namespace Anker;

QWidget *go()
{
    auto *window = new QWidget;
    window->setWindowTitle("Anker Layout in icpp");

    auto *layout = new FormLayout(window);
    layout->setContentsMargins(12, 12, 12, 12);

    auto *nameLabel = new QLabel("Name:", window);
    auto *nameEdit = new QLineEdit(window);
    auto *okButton = new QPushButton("OK", window);
    auto *cancelButton = new QPushButton("Cancel", window);

    layout->addWidget(nameLabel, {
        .left = attachForm(),
        .top = attachForm(),
    });

    layout->addWidget(nameEdit, {
        .left = attachWidget(nameLabel, 8),
        .right = attachForm(),
        .top = attachOppositeWidget(nameLabel),
    });

    layout->addWidget(okButton, {
        .right = attachForm(),
        .bottom = attachForm(),
    });

    layout->addWidget(cancelButton, {
        .right = attachWidget(okButton, 8),
        .bottom = attachOppositeWidget(okButton),
    });

    window->resize(420, 160);
    window->show();
    window->raise();
    return window;
}
