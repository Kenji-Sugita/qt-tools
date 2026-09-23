#include <QtTest>

class SubdirTest : public QObject
{
    Q_OBJECT

private slots:
    void passes() { QVERIFY(true); }
};

QTEST_MAIN(SubdirTest)
#include "tst_validation.moc"
