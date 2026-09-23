#include "object.h"

int main()
{
    QList<Object> list1;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    list1.reserve(10);
#endif
    list1 << Object("Foo") << Object("Bar");
    qDebug() << "list1 =" << list1;

    QList<Object> list2 = list1;
    qDebug() << "list2 =" << list2;

    qDebug() << "list1[0] =" << list1[0];
}
