#include "paintwindow.cpp"
#include "scribblearea.cpp"
#include "moc_paintwindow.cpp"
#include "moc_scribblearea.cpp"
#include "main.cpp"

void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
