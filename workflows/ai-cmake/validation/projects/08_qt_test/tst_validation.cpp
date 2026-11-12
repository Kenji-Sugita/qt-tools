#include <QtTest>

class ValidationTest : public QObject
{
    Q_OBJECT

private slots:
    void passes() { QVERIFY(true); }
};

QTEST_MAIN(ValidationTest)
#include "tst_validation.moc"
