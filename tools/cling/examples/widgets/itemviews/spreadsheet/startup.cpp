#include "printview.cpp"
#include "spreadsheet.cpp"
#include "spreadsheetdelegate.cpp"
#include "spreadsheetitem.cpp"
#include "moc_printview.cpp"
#include "moc_spreadsheet.cpp"
#include "moc_spreadsheetdelegate.cpp"
#include "qrc_spreadsheet.cpp"
#include "main.cpp"

void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
