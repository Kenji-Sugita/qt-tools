#include "FloatingActionButton.h"

#include <QMouseEvent>
#include <QtTest/QtTest>

class FloatingActionButtonTest : public QObject {
    Q_OBJECT

private slots:
    void clickActivates();
    void dragDoesNotActivate();
};

void FloatingActionButtonTest::clickActivates()
{
    QWidget parent;
    FloatingActionButton button(&parent);
    button.setFixedSize(48, 48);
    parent.resize(80, 120);

    QSignalSpy activatedSpy(&button, &FloatingActionButton::activated);
    QSignalSpy draggedSpy(&button, &FloatingActionButton::draggedTo);

    QMouseEvent pressEvent(QEvent::MouseButtonPress,
                           QPointF(24, 24),
                           QPointF(24, 24),
                           Qt::LeftButton,
                           Qt::LeftButton,
                           Qt::NoModifier);
    QApplication::sendEvent(&button, &pressEvent);
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease,
                             QPointF(24, 24),
                             QPointF(24, 24),
                             Qt::LeftButton,
                             Qt::NoButton,
                             Qt::NoModifier);
    QApplication::sendEvent(&button, &releaseEvent);

    QCOMPARE(activatedSpy.count(), 1);
    QCOMPARE(draggedSpy.count(), 0);
}

void FloatingActionButtonTest::dragDoesNotActivate()
{
    QWidget parent;
    FloatingActionButton button(&parent);
    button.setFixedSize(48, 48);
    parent.resize(80, 160);

    QSignalSpy activatedSpy(&button, &FloatingActionButton::activated);
    QSignalSpy draggedSpy(&button, &FloatingActionButton::draggedTo);

    const QPoint start(24, 24);
    const QPoint end = start + QPoint(0, QApplication::startDragDistance() + 8);
    QMouseEvent pressEvent(QEvent::MouseButtonPress,
                           QPointF(start),
                           QPointF(start),
                           Qt::LeftButton,
                           Qt::LeftButton,
                           Qt::NoModifier);
    QApplication::sendEvent(&button, &pressEvent);
    QMouseEvent moveEvent(QEvent::MouseMove,
                          QPointF(end),
                          QPointF(end),
                          Qt::NoButton,
                          Qt::LeftButton,
                          Qt::NoModifier);
    QApplication::sendEvent(&button, &moveEvent);
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease,
                             QPointF(end),
                             QPointF(end),
                             Qt::LeftButton,
                             Qt::NoButton,
                             Qt::NoModifier);
    QApplication::sendEvent(&button, &releaseEvent);

    QCOMPARE(activatedSpy.count(), 0);
    QVERIFY(draggedSpy.count() > 0);
}

QTEST_MAIN(FloatingActionButtonTest)
#include "tst_floatingactionbutton.moc"
