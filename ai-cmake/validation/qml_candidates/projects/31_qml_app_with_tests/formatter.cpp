#include "formatter.h"

QString Formatter::label(int value) const
{
    return QStringLiteral("value:%1").arg(value);
}
