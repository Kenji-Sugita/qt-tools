#include "imagewidget.cpp"
#include "mainwidget.cpp"
#include "moc_imagewidget.cpp"
#include "moc_mainwidget.cpp"
#include "main.cpp"

void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
