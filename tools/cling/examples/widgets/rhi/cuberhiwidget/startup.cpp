#include "examplewidget.cpp"
#include "moc_examplewidget.cpp"
#include "qrc_cuberhiwidget.cpp"
#include "main.cpp"

void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
