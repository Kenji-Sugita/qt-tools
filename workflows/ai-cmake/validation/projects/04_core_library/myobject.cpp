#include "myobject.h"

MyObject::MyObject(QObject *parent) : QObject(parent) {}

QString MyObject::name() const
{
    return QStringLiteral("validation");
}
