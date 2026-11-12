#include "adddialog.cpp"
#include "adddialog.h"
#include "addresswidget.cpp"
#include "addresswidget.h"
#include "mainwindow.cpp"
#include "mainwindow.h"
#include "newaddresstab.cpp"
#include "newaddresstab.h"
#include "tablemodel.cpp"
#include "tablemodel.h"
#include "moc_adddialog.cpp"
#include "moc_addresswidget.cpp"
#include "moc_mainwindow.cpp"
#include "moc_newaddresstab.cpp"
#include "moc_tablemodel.cpp"
#include "main.cpp"

void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
