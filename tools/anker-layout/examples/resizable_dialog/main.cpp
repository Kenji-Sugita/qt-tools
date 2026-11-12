#include "AnkerFormLayout.h"

#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QListWidget>
#include <QTextEdit>

using namespace Anker;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QDialog dialog;
    dialog.setWindowTitle("Anker Resizable Dialog");

    auto *layout = new FormLayout(&dialog);
    layout->setContentsMargins(12, 12, 12, 12);

    auto *listLabel = new QLabel("Items");
    auto *detailLabel = new QLabel("Details");
    auto *list = new QListWidget;
    auto *detail = new QTextEdit;
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    list->addItems({"Alpha", "Beta", "Gamma", "Delta"});
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    layout->addWidget(listLabel, {
        .left = attachForm(),
        .top = attachForm(),
    });
    layout->addWidget(detailLabel, {
        .left = attachPosition(35, 8),
        .top = attachOppositeWidget(listLabel),
    });
    layout->addWidget(list, {
        .left = attachForm(),
        .right = attachPosition(35, 4),
        .top = attachWidget(listLabel, 6),
        .bottom = attachWidget(buttons, 12),
    });
    layout->addWidget(detail, {
        .left = attachPosition(35, 8),
        .right = attachForm(),
        .top = attachWidget(detailLabel, 6),
        .bottom = attachWidget(buttons, 12),
    });
    layout->addWidget(buttons, {
        .right = attachForm(),
        .bottom = attachForm(),
    });

    dialog.resize(620, 400);
    dialog.show();
    return app.exec();
}
