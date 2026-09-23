#include "droparea.cpp"
#include "dropsitewindow.cpp"
#include "moc_droparea.cpp"
#include "moc_dropsitewindow.cpp"
#include "main.cpp"

void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
