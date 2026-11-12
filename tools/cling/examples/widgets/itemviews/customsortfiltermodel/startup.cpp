#include "filterwidget.cpp"
#include "window.cpp"
#include "moc_filterwidget.cpp"
#include "moc_mysortfilterproxymodel.cpp"
#include "moc_window.cpp"
#include "mysortfilterproxymodel.cpp"
#include "qrc_customsortfiltermodel.cpp"
#include "main.cpp"

void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
