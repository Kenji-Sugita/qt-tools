#include "pathdeform.cpp"
#include "qrc_deform.cpp"
#include "moc_pathdeform.cpp"
#include "main.cpp"

void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
