#include "backend.h"

Backend::Backend(QObject *parent) : QObject(parent) {}

QString Backend::message() const
{
    return QStringLiteral("backend");
}
