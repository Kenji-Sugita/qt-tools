#include "findDialog.h"
#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    FindDialog dialog;

    dialog.exec();
}
