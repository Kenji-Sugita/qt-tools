// qtcling -c startup_test.cpp -I/usr/local/qt/Qt/6.11.0/macos/lib/QtTest.framework/Headers -F/usr/local/qt/Qt/6.11.0/macos/lib /usr/local/qt/Qt/6.11.0/macos/lib/QtTest.framework/Versions/A/QtTest

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
