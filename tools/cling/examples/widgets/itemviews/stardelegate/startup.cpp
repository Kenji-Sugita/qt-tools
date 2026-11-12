#include "stardelegate.cpp"
#include "stareditor.cpp"
#include "starrating.cpp"
#include "moc_stardelegate.cpp"
#include "moc_stareditor.cpp"
#include "main.cpp"

void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
