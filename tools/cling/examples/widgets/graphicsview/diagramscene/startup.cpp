#include "arrow.cpp"
#include "diagramitem.cpp"
#include "diagramscene.cpp"
#include "diagramtextitem.cpp"
#include "mainwindow.cpp"
#include "qrc_diagramscene.cpp"
#include "moc_diagramscene.cpp"
#include "moc_diagramtextitem.cpp"
#include "moc_mainwindow.cpp"
#include "main.cpp"


void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
