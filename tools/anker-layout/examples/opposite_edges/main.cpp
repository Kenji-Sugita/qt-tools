#include "AnkerFormLayout.h"

#include <QApplication>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>

using namespace Anker;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Anker Opposite Edges");

    auto *layout = new FormLayout(&window);
    layout->setContentsMargins(12, 12, 12, 12);

    auto *hostLabel = new QLabel("Host:");
    auto *hostEdit = new QLineEdit;
    auto *portLabel = new QLabel("Port:");
    auto *portEdit = new QLineEdit;
    auto *protocolLabel = new QLabel("Protocol:");
    auto *protocolCombo = new QComboBox;
    protocolCombo->addItems({"HTTP", "HTTPS", "SSH"});

    layout->addWidget(hostLabel, {
        .left = attachForm(),
        .top = attachForm(),
    });
    layout->addWidget(hostEdit, {
        .left = attachPosition(25),
        .right = attachForm(),
        .top = attachOppositeWidget(hostLabel),
    });
    layout->addWidget(portLabel, {
        .left = attachOppositeWidget(hostLabel),
        .top = attachWidget(hostLabel, 14),
    });
    layout->addWidget(portEdit, {
        .left = attachOppositeWidget(hostEdit),
        .right = attachPosition(50),
        .top = attachOppositeWidget(portLabel),
    });
    layout->addWidget(protocolLabel, {
        .left = attachOppositeWidget(hostLabel),
        .top = attachWidget(portLabel, 14),
    });
    layout->addWidget(protocolCombo, {
        .left = attachOppositeWidget(hostEdit),
        .right = attachPosition(50),
        .top = attachOppositeWidget(protocolLabel),
    });

    window.resize(480, 180);
    window.show();
    return app.exec();
}
