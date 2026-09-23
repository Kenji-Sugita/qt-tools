#include "objectpicker.h"

#include <QtTest/QTest>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

class ObjectPickerTest : public QObject
{
    Q_OBJECT

private slots:
    void startRequiresVisibleRoot();
    void stopResetsState();
    void escapeKeepsNoSelectionWhenNothingWasConfirmed();
    void escapeClearsPreviouslySelectedObject();
};

void ObjectPickerTest::startRequiresVisibleRoot()
{
    QWidget hiddenRoot;
    ObjectPicker picker;

    QVERIFY(!picker.start(&hiddenRoot));
    QVERIFY(!picker.isActive());
}

void ObjectPickerTest::stopResetsState()
{
    QWidget rootWidget;
    rootWidget.resize(240, 120);
    auto *layout = new QVBoxLayout(&rootWidget);
    auto *label = new QLabel(QStringLiteral("Pick me"), &rootWidget);
    layout->addWidget(label);
    rootWidget.show();
    QVERIFY(QTest::qWaitForWindowExposed(&rootWidget));

    ObjectPicker picker;
    QVERIFY(picker.start(&rootWidget));
    QVERIFY(picker.isActive());
    QCOMPARE(picker.rootWidget(), &rootWidget);

    picker.stop();

    QVERIFY(!picker.isActive());
    QVERIFY(picker.rootWidget() == nullptr);
    QVERIFY(picker.hoveredWidget() == nullptr);
}

void ObjectPickerTest::escapeKeepsNoSelectionWhenNothingWasConfirmed()
{
    QWidget rootWidget;
    rootWidget.resize(240, 120);
    auto *layout = new QVBoxLayout(&rootWidget);
    auto *label = new QLabel(QStringLiteral("Pick me"), &rootWidget);
    layout->addWidget(label);
    rootWidget.show();
    QVERIFY(QTest::qWaitForWindowExposed(&rootWidget));

    ObjectPicker picker;
    QVERIFY(picker.selectedObject() == nullptr);
    QVERIFY(picker.start(&rootWidget));

    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    QVERIFY(QCoreApplication::sendEvent(qApp, &keyEvent));

    QVERIFY(!picker.isActive());
    QVERIFY(picker.selectedObject() == nullptr);
}

void ObjectPickerTest::escapeClearsPreviouslySelectedObject()
{
    QWidget rootWidget;
    rootWidget.resize(240, 120);
    auto *layout = new QVBoxLayout(&rootWidget);
    auto *label = new QLabel(QStringLiteral("Pick me"), &rootWidget);
    layout->addWidget(label);
    rootWidget.show();
    QVERIFY(QTest::qWaitForWindowExposed(&rootWidget));

    ObjectPicker picker;
    picker.setSelectedObject(label);
    QCOMPARE(picker.selectedObject(), static_cast<QObject *>(label));
    QVERIFY(picker.start(&rootWidget));

    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    QVERIFY(QCoreApplication::sendEvent(qApp, &keyEvent));

    QVERIFY(!picker.isActive());
    QVERIFY(picker.selectedObject() == nullptr);
}

QTEST_MAIN(ObjectPickerTest)

#include "objectpicker_test.moc"
