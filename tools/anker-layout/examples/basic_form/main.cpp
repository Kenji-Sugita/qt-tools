#include "AnkerFormLayout.h"

#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

using namespace Anker;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Anker Basic Form");

    auto *layout = new FormLayout(&window);
    layout->setContentsMargins(12, 12, 12, 12);

    auto *nameLabel = new QLabel("Name:");
    auto *nameEdit = new QLineEdit;
    auto *okButton = new QPushButton("OK");
    auto *cancelButton = new QPushButton("Cancel");

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

    window.resize(420, 160);
    window.show();
    return app.exec();
}
