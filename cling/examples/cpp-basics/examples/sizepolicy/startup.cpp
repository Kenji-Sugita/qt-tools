#include "harness.cpp"
#include "widget.cpp"
#include "moc_harness.cpp"
#include "moc_widget.cpp"
#include "main.cpp"

void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
