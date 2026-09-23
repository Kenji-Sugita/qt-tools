#include "object.h"

Object::Object(const QString& name) noexcept
{
    d.name = name;
    qDebug() << "ctor:" << *this;
}

Object::Object(const Object& other) noexcept
{
    d.name = other.d.name + " copy";
    qDebug() << "copy ctor:" << *this;
}

Object::~Object()
{
    qDebug() << "dtor:" << *this;
}

QString Object::name() const noexcept { return d.name; }
void Object::setName(const QString& name) noexcept { d.name = name; }

QDebug operator<<(QDebug debug, const Object& object)
{
    QDebugStateSaver debugStateSaver(debug);

    debug.nospace() << "Object(" << object.name() << ")";

    return debug;
}
