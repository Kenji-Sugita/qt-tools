#include "mainwindow.cpp"
#include "treemodelcompleter.cpp"
#include "moc_mainwindow.cpp"
#include "moc_treemodelcompleter.cpp"
#include "qrc_treemodelcompleter.cpp"
#include "main.cpp"

void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
