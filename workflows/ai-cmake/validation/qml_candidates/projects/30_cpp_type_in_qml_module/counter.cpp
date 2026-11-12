#include "counter.h"

Counter::Counter(QObject *parent) : QObject(parent) {}

int Counter::value() const
{
    return 42;
}
