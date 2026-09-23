#include "mainwindow.cpp"
#include "textedit.cpp"
#include "moc_mainwindow.cpp"
#include "moc_textedit.cpp"
#include "qrc_customcompleter.cpp"
#include "main.cpp"

void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
