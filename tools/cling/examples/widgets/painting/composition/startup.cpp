#include "composition.cpp"
#include "moc_composition.cpp"
#include "qrc_composition.cpp"
#include "main.cpp"

void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
