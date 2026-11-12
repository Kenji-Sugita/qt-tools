#include <QtCore>

using namespace Qt::Literals::StringLiterals;

void string()
{
    QString x = u"Nine pineapples"_s;

    x.slice(5);     // x == "pineapples"
    qDebug() << x;

    x.slice(4, 3);  // x == "app"
    qDebug() << x;
}
