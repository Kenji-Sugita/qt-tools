#include "receiver.h"

Receiver::Receiver(QObject* parent)
    : QObject(parent)
{
}

void Receiver::aSlot()
{
    qDebug("A Slot");
}

void Receiver::anOtherSlot(int, QObject*)
{
    qDebug("anOtherSlot");
}
