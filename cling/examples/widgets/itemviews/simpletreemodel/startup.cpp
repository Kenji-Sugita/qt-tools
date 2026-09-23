#include "treeitem.cpp"
#include "treemodel.cpp"
#include "moc_treemodel.cpp"
#include "qrc_simpletreemodel.cpp"
#include "main.cpp"

void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
