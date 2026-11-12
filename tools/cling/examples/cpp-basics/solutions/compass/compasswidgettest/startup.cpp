#include "../compasswidget/compasswidget.cpp"
#include "../compasswidget/compasswidget2.cpp"
#include "../compasswidget/moc_compasswidget.cpp"
#include "../compasswidget/moc_compasswidget2.cpp"
#include "main.cpp"

void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
