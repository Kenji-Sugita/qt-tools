#include <QtTest>
#include "formatter.h"

class FormatterTest : public QObject
{
    Q_OBJECT

private slots:
    void label() { QCOMPARE(Formatter().label(3), QStringLiteral("value:3")); }
};

QTEST_MAIN(FormatterTest)
#include "tst_formatter.moc"
