#include "qrc_images.cpp"
#include "chip.cpp"
#include "view.cpp"
#include "mainwindow.cpp"
#include "moc_mainwindow.cpp"
#include "moc_view.cpp"
#include "main.cpp"

void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
