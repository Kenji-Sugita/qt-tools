#include "../shared/arthurstyle.cpp"
#include "../shared/arthurwidgets.cpp"
#include "../shared/hoverpoints.cpp"
#include "../shared/moc_arthurwidgets.cpp"
#include "../shared/moc_hoverpoints.cpp"
//#include "../shared/qrc_shared.cpp"
#include "qrc_affine.cpp"
#include "xform.cpp"
#include "moc_xform.cpp"
#include "main.cpp"

void startup()
{
    int argc = 1;
    static char appName[] = "qtcling";
    static char* argv[] = {appName, nullptr};

    main(argc, argv);
}
