#include "highlighter.cpp"
#include "mainwindow.cpp"
#include "moc_highlighter.cpp"
#include "moc_mainwindow.cpp"
#include "main.cpp"

void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
