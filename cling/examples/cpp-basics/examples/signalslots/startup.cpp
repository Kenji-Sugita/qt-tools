#include "emitter.cpp"
#include "receiver.cpp"
#include "moc_emitter.cpp"
#include "moc_receiver.cpp"
#include "main.cpp"

void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
