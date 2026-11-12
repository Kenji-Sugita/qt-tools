#include "notepad.cpp"
#include "qrc_notepad.cpp"
#include "moc_notepad.cpp"
#include "main.cpp"

void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
