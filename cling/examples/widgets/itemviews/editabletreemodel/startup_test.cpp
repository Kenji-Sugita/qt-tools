#include "treeitem.cpp"
#include "treemodel.cpp"
#include "moc_treemodel.cpp"
#include "test.cpp"

void startup_test()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
