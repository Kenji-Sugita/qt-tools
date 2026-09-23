#include "commands.cpp"
#include "diagramitem.cpp"
#include "diagramscene.cpp"
#include "mainwindow.cpp"
#include "moc_diagramscene.cpp"
#include "moc_mainwindow.cpp"
#include "qrc_undoframework.cpp"
#include "main.cpp"

void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
