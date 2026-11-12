#include "ObjectBrowserModel.h"
#include "ui_test.h"
#include <QApplication>
#include <QTableView>
#include <QDialog>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    const auto dialog = new QDialog;
    const auto ui = new Ui::Test;
    ui->setupUi(dialog);
    delete ui;
    dialog->show();

    QTableView view;
    const auto model = new ObjectBrowserModel(dialog, &view);
    view.setModel(model);
    view.resize(400, 600);
    view.resizeColumnsToContents();
    view.show();

    const int returnCode = app.exec();
    delete model;
    delete dialog;
    return returnCode;
}
