#include <QApplication>

extern "C" void cling_set_periodic_callback(void (*callbackFunction)(),
                                            int intervalMilliseconds);
extern "C" void cling_clear_periodic_callback();

QApplication* ensureApplication()
{
    if (QApplication::instance()) {
        return nullptr;
    }

    static int argumentCount = 1;
    static char applicationName[] = "qt";
    static char* argumentValues[] = {applicationName, nullptr};
    static QApplication application{argumentCount, argumentValues};
    application.setQuitOnLastWindowClosed(false);
    return &application;
}

void processQtEventsPeriodically()
{
    if (!QApplication::instance()) {
        return;
    }

    QApplication::processEvents();
}

void enablePeriodicQtEvents()
{
    ensureApplication();
    cling_set_periodic_callback(&processQtEventsPeriodically, 10);
}

void disablePeriodicQtEvents()
{
    cling_clear_periodic_callback();
}

enablePeriodicQtEvents();
