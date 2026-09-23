#include <QtTest>
#include "calculator.h"

class CalculatorTest : public QObject
{
    Q_OBJECT

private slots:
    void add() { QCOMPARE(Calculator().add(1, 2), 3); }
};

QTEST_MAIN(CalculatorTest)
#include "tst_calculator.moc"
