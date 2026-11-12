#include "updating.h"
#include <QApplication>
#include <QPushButton>
#include <QLayout>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QWidget top;

    const auto updating = new Updating;

    const auto updatingButton = new QPushButton("Update");
    QObject::connect(updatingButton, &QPushButton::clicked, updating, &Updating::updating);
    QPushButton* const repaintingButton = new QPushButton("Repaint");
    QObject::connect(repaintingButton, &QPushButton::clicked, updating, &Updating::updating);

    const auto topLayout = new QVBoxLayout(&top);

    const auto updatingLayout = new QHBoxLayout;
    updatingLayout->addStretch();
    updatingLayout->addWidget(updating);
    updatingLayout->addStretch();
    topLayout->addLayout(updatingLayout);

    const auto buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(updatingButton, 0, Qt::AlignRight);
    buttonLayout->addWidget(repaintingButton, 0, Qt::AlignLeft);
    topLayout->addLayout(buttonLayout);

    top.show();

    return app.exec();
}
