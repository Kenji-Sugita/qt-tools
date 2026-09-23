#include "button.cpp"
#include "calculator.cpp"
#include "moc_button.cpp"
#include "moc_calculator.cpp"
#include "main.cpp"

void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
