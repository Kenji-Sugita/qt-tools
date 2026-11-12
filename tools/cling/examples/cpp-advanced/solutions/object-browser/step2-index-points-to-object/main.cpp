#include "ObjectBrowserModel.h"
#include "ui_test.h"
#include <QApplication>
#include <QTreeView>
#include <QDialog>
#include <QModelIndex>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    const auto dialog = new QDialog;  // Potential memory leak
    const auto ui = new Ui::Test;
    ui->setupUi(dialog);
    delete(ui);
    dialog->show();

    QTreeView view;
    const auto model = new ObjectBrowserModel(dialog, &view);
    view.setModel(model);
    view.resize(400, 600);
    view.show();

    return app.exec();
}
