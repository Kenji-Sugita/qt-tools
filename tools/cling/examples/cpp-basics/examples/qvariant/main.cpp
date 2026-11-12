#include "data.h"
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
//#include <QVariant> // Step 1
#endif

int main()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    //QMetaType::registerDebugStreamOperator<Data>(); // Step 3
    //QMetaType::registerEqualsComparator<Data>(); // Step 4
#endif

    Data red;
    red.setType("red");
    red.setSize(QSize(100, 200));
    //QVariant variantRed = QVariant::fromValue(red); // Step 1

    Data Red;
    Red.setType("red");
    Red.setSize(QSize(100, 200));
    // Enabled by cast operator
    //QVariant variantLargeRed = Red; // Step 2
    //qDebug() << variantLargeRed; // Step 2

    Data yellow;
    yellow.setType("yellow");
    yellow.setSize(QSize(100, 200));
    //QVariant variantYellow = QVariant::fromValue(yellow); // Step 1

    qDebug() << "red =" << red;
    qDebug() << "Red =" << Red;
    qDebug() << "yellow =" << yellow;
    qDebug() << "(red == Red) = " << (red == Red);

    // Enabled by Q_DECLARE_METATYPE
    //qDebug() << "variantRed =" << variantRed; // Step 1
    //qDebug() << "variantYellow =" << variantYellow; // Step 1

    // Enabled by QMetaType::registerEqualsComparator<Data>()
    //qDebug() << (variantRed == variantLargeRed); // Step 4
    //qDebug() << (variantRed == variantYellow); // Step 4
}
