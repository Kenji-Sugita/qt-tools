#include "mainwindow.cpp"
#include "tabletapplication.cpp"
#include "tabletcanvas.cpp"
#include "moc_mainwindow.cpp"
#include "moc_tabletapplication.cpp"
#include "moc_tabletcanvas.cpp"
#include "qrc_images.cpp"
#include "main.cpp"

void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
