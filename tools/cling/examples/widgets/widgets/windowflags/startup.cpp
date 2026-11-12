#include "controllerwindow.cpp"
#include "previewwindow.cpp"
#include "moc_controllerwindow.cpp"
#include "moc_previewwindow.cpp"
#include "main.cpp"

void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
