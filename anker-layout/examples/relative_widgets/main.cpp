#include "AnkerFormLayout.h"

#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QWidget>

using namespace Anker;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Anker Relative Widgets");

    auto *layout = new FormLayout(&window);
    layout->setContentsMargins(12, 12, 12, 12);

    auto *titleLabel = new QLabel("Title:");
    auto *titleEdit = new QLineEdit;
    auto *bodyLabel = new QLabel("Body:");
    auto *bodyEdit = new QTextEdit;

    layout->addWidget(titleLabel, {
        .left = attachForm(),
        .top = attachForm(),
    });
    layout->addWidget(titleEdit, {
        .left = attachWidget(titleLabel, 8),
        .right = attachForm(),
        .top = attachOppositeWidget(titleLabel),
    });
    layout->addWidget(bodyLabel, {
        .left = attachOppositeWidget(titleLabel),
        .top = attachWidget(titleLabel, 16),
    });
    layout->addWidget(bodyEdit, {
        .left = attachOppositeWidget(titleEdit),
        .right = attachForm(),
        .top = attachOppositeWidget(bodyLabel),
        .bottom = attachForm(),
    });

    window.resize(520, 320);
    window.show();
    return app.exec();
}
