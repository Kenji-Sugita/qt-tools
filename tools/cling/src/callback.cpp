#include <QCoreApplication>

extern "C" void cling_set_periodic_callback(void (*callbackFunction)(),
                                            int intervalMilliseconds);
extern "C" void cling_clear_periodic_callback();

void processQtEventsPeriodically()
{
    if (QCoreApplication::instance() == nullptr) {
        return;
    }

    QCoreApplication::processEvents();
}

void enablePeriodicQtEvents()
{
    cling_set_periodic_callback(&processQtEventsPeriodically, 100);
}

void disablePeriodicQtEvents()
{
    cling_clear_periodic_callback();
}
