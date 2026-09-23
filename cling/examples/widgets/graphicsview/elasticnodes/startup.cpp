#include "node.cpp"
#include "edge.cpp"
#include "graphwidget.cpp"
#include "moc_graphwidget.cpp"
#include "main.cpp"

void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
