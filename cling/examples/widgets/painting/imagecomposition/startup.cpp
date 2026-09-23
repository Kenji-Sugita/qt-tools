#include "imagecomposer.cpp"
#include "moc_imagecomposer.cpp"
#include "qrc_imagecomposition.cpp"
#include "main.cpp"

void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
