#include "PersistentToolBar.h"

#include <QAction>
#include <QEvent>
#include <QLabel>
#include <QMainWindow>
#include <QSpinBox>
#include <QToolBar>
#include <QToolButton>
#include <QtTest/QtTest>

#include <algorithm>

class PersistentToolBarTest : public QObject {
    Q_OBJECT

private slots:
    void extensionButtonTogglesPersistentPanel();
    void embeddedInToolBarUsesAvailableWidth();
    void fixedWidgetWidthIsUsedForPanelWidth();
    void actualItemGeometrySetsPanelWidth();
    void macOsItemsUseWidgetRect();
    void wrappedRowsStartAtSameLeftEdge();
    void widgetInstanceMovesBetweenRows();
    void wideningToolbarRemovesOverflow();
};

void PersistentToolBarTest::extensionButtonTogglesPersistentPanel()
{
    QWidget window;
    window.resize(260, 220);
    PersistentToolBar toolbar(&window, &window);
    toolbar.setGeometry(0, 0, 180, 48);

    QAction *lastAction = nullptr;
    for (int i = 0; i < 6; ++i) {
        lastAction = new QAction(QStringLiteral("Action %1").arg(i + 1), &toolbar);
        toolbar.addAction(lastAction);
    }

    window.show();
    toolbar.show();
    toolbar.refreshOverflow();
    QCoreApplication::processEvents();

    QVERIFY(toolbar.hasOverflow());
    QVERIFY(!toolbar.overflowExpanded());
    auto *button = toolbar.findChild<QToolButton *>(QStringLiteral("persistentToolbarExtensionButton"));
    QVERIFY(button);
    QVERIFY(button->isVisible());

    QTest::mouseClick(button, Qt::LeftButton);
    QCoreApplication::processEvents();
    QVERIFY(toolbar.overflowExpanded());

    QSignalSpy triggeredSpy(lastAction, &QAction::triggered);
    auto *overflowActionButton = qobject_cast<QToolButton *>(toolbar.widgetForAction(lastAction));
    QVERIFY(overflowActionButton);
    QTest::mouseClick(overflowActionButton, Qt::LeftButton);
    QCOMPARE(triggeredSpy.count(), 1);
    QVERIFY(toolbar.overflowExpanded());

    QEvent leaveEvent(QEvent::Leave);
    QApplication::sendEvent(&window, &leaveEvent);
    QCoreApplication::processEvents();
    QVERIFY(toolbar.overflowExpanded());

    QTest::mouseClick(button, Qt::LeftButton);
    QCoreApplication::processEvents();
    QVERIFY(!toolbar.overflowExpanded());
}

void PersistentToolBarTest::embeddedInToolBarUsesAvailableWidth()
{
    QMainWindow window;
    window.resize(620, 300);
    auto *outerToolBar = window.addToolBar(QStringLiteral("Tools"));
    auto *toolbar = new PersistentToolBar(&window, outerToolBar);
    outerToolBar->addWidget(toolbar);
    for (int i = 0; i < 16; ++i)
        toolbar->addAction(new QAction(QStringLiteral("Action %1").arg(i + 1), toolbar));

    window.show();
    QCoreApplication::processEvents();
    toolbar->refreshOverflow();

    QVERIFY(toolbar->width() > 500);
    QVERIFY(toolbar->hasOverflow());
    auto *button = toolbar->findChild<QToolButton *>(QStringLiteral("persistentToolbarExtensionButton"));
    QVERIFY(button);
    QTest::mouseClick(button, Qt::LeftButton);
    QCoreApplication::processEvents();
    QVERIFY(toolbar->overflowExpanded());

    QWidget *firstOverflowWidget = nullptr;
    for (QAction *action : toolbar->actions()) {
        QWidget *actionWidget = toolbar->widgetForAction(action);
        if (actionWidget
            && actionWidget->parentWidget()->objectName() == QStringLiteral("persistentToolbarOverflowPanel")) {
            if (!firstOverflowWidget)
                firstOverflowWidget = actionWidget;
        }
    }
    QVERIFY(firstOverflowWidget);
    const int toolbarLeft = toolbar->mapTo(&window, QPoint(0, 0)).x();
    const int overflowItemLeft = firstOverflowWidget->mapTo(&window, QPoint(0, 0)).x();
    QVERIFY(overflowItemLeft - toolbarLeft <= 1);

    auto *overflowPanel = window.findChild<QWidget *>(QStringLiteral("persistentToolbarOverflowPanel"));
    QVERIFY(overflowPanel);
    const int outerToolBarLeft = outerToolBar->mapTo(&window, QPoint(0, 0)).x();
    const int overflowPanelLeft = overflowPanel->mapTo(&window, QPoint(0, 0)).x();
    QCOMPARE(overflowPanelLeft, outerToolBarLeft);
    int overflowItemRight = 0;
    for (QAction *action : toolbar->actions()) {
        QWidget *actionWidget = toolbar->widgetForAction(action);
        if (actionWidget && actionWidget->parentWidget() == overflowPanel) {
            overflowItemRight = std::max(
                overflowItemRight,
                actionWidget->mapTo(&window, QPoint(actionWidget->width(), 0)).x());
        }
    }
    const int overflowPanelRight = overflowPanel->mapTo(&window, QPoint(overflowPanel->width(), 0)).x();
    const int leftBackgroundInset = overflowItemLeft - overflowPanelLeft;
    const int rightBackgroundInset = overflowPanelRight - overflowItemRight;
    QCOMPARE(rightBackgroundInset, leftBackgroundInset);
}

void PersistentToolBarTest::fixedWidgetWidthIsUsedForPanelWidth()
{
    QWidget window;
    window.resize(320, 220);
    PersistentToolBar toolbar(&window, &window);
    toolbar.setGeometry(0, 0, 130, 48);

    for (int i = 0; i < 3; ++i)
        toolbar.addAction(new QAction(QStringLiteral("Action %1").arg(i + 1), &toolbar));
    auto *label = new QLabel(QStringLiteral("A deliberately wide size hint"), &toolbar);
    label->setFixedWidth(20);
    toolbar.addWidget(label);

    window.show();
    toolbar.show();
    toolbar.refreshOverflow();
    toolbar.setOverflowExpanded(true);
    QCoreApplication::processEvents();

    auto *panel = window.findChild<QWidget *>(QStringLiteral("persistentToolbarOverflowPanel"));
    QVERIFY(panel);
    QCOMPARE(label->width(), 20);
    QCOMPARE(label->parentWidget(), panel);

    int firstItemLeft = panel->width();
    int lastItemRight = 0;
    for (QWidget *child : panel->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly)) {
        if (!child->isVisible())
            continue;
        firstItemLeft = std::min(firstItemLeft, child->geometry().left());
        lastItemRight = std::max(lastItemRight, child->geometry().right() + 1);
    }
    QCOMPARE(panel->width() - lastItemRight, firstItemLeft);
}

void PersistentToolBarTest::actualItemGeometrySetsPanelWidth()
{
    QWidget window;
    window.resize(1000, 220);
    QToolBar outerToolBar(&window);
    outerToolBar.setGeometry(0, 0, 1000, 60);
    PersistentToolBar toolbar(&window, &outerToolBar);
    toolbar.setGeometry(10, 8, 130, 44);
    for (int i = 0; i < 3; ++i)
        toolbar.addAction(new QAction(QStringLiteral("Action %1").arg(i + 1), &toolbar));
    auto *label = new QLabel(QStringLiteral("3 / 13"), &toolbar);
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    label->setIndent(0);
    label->setMargin(0);
    toolbar.addWidget(label);

    window.show();
    outerToolBar.show();
    toolbar.show();
    QCoreApplication::processEvents();
    toolbar.refreshOverflow();
    toolbar.setOverflowExpanded(true);
    QCoreApplication::processEvents();

    auto *panel = window.findChild<QWidget *>(QStringLiteral("persistentToolbarOverflowPanel"));
    QVERIFY(panel);
    QCOMPARE(label->parentWidget(), panel);

    int firstItemLeft = panel->width();
    for (QWidget *child : panel->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly)) {
        if (child->isVisible())
            firstItemLeft = std::min(firstItemLeft, child->geometry().left());
    }
    const int labelRight = label->geometry().right() + 1;
    QCOMPARE(panel->width() - labelRight, firstItemLeft);
}

void PersistentToolBarTest::macOsItemsUseWidgetRect()
{
#ifndef Q_OS_MACOS
    QSKIP("macOS-specific layout normalization");
#else
    QWidget window;
    PersistentToolBar toolbar(&window, &window);
    auto *action = new QAction(QStringLiteral("Action"), &toolbar);
    toolbar.addAction(action);
    auto *label = new QLabel(QStringLiteral("Page"), &toolbar);
    toolbar.addWidget(label);

    QWidget *actionWidget = toolbar.widgetForAction(action);
    QVERIFY(actionWidget);
    QVERIFY(actionWidget->testAttribute(Qt::WA_LayoutUsesWidgetRect));
    QVERIFY(label->testAttribute(Qt::WA_LayoutUsesWidgetRect));
    auto *extensionButton = toolbar.findChild<QToolButton *>(QStringLiteral("persistentToolbarExtensionButton"));
    QVERIFY(extensionButton);
    QVERIFY(extensionButton->testAttribute(Qt::WA_LayoutUsesWidgetRect));
#endif
}

void PersistentToolBarTest::wrappedRowsStartAtSameLeftEdge()
{
    QWidget window;
    window.resize(220, 320);
    PersistentToolBar toolbar(&window, &window);
    toolbar.setGeometry(0, 0, 80, 48);
    for (int i = 0; i < 12; ++i)
        toolbar.addAction(new QAction(QStringLiteral("Action %1").arg(i + 1), &toolbar));

    window.show();
    toolbar.show();
    toolbar.refreshOverflow();
    toolbar.setOverflowExpanded(true);
    QCoreApplication::processEvents();

    auto *panel = window.findChild<QWidget *>(QStringLiteral("persistentToolbarOverflowPanel"));
    QVERIFY(panel);
    QHash<int, int> leftByRow;
    for (QWidget *child : panel->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly)) {
        if (!child->isVisible())
            continue;
        const int top = child->geometry().top();
        leftByRow.insert(top, std::min(leftByRow.value(top, panel->width()), child->geometry().left()));
    }
    QVERIFY(leftByRow.size() > 1);
    const QList<int> rowLefts = leftByRow.values();
    for (int left : rowLefts)
        QCOMPARE(left, rowLefts.first());
}

void PersistentToolBarTest::widgetInstanceMovesBetweenRows()
{
    QWidget window;
    window.resize(500, 220);
    PersistentToolBar toolbar(&window, &window);
    toolbar.setGeometry(0, 0, 140, 48);
    toolbar.addAction(new QAction(QStringLiteral("Action"), &toolbar));
    auto *spinBox = new QSpinBox(&toolbar);
    spinBox->setFixedWidth(100);
    toolbar.addWidget(spinBox);

    window.show();
    toolbar.show();
    toolbar.refreshOverflow();
    toolbar.setOverflowExpanded(true);
    QCoreApplication::processEvents();

    QVERIFY(toolbar.hasOverflow());
    QVERIFY(spinBox->isVisible());
    QCOMPARE(spinBox->parentWidget()->objectName(), QStringLiteral("persistentToolbarOverflowPanel"));

    toolbar.resize(400, 48);
    toolbar.refreshOverflow();
    QCoreApplication::processEvents();

    QVERIFY(!toolbar.hasOverflow());
    QCOMPARE(spinBox->parentWidget(), &toolbar);
}

void PersistentToolBarTest::wideningToolbarRemovesOverflow()
{
    QWidget window;
    window.resize(900, 220);
    PersistentToolBar toolbar(&window, &window);
    toolbar.setGeometry(0, 0, 160, 48);

    for (int i = 0; i < 4; ++i)
        toolbar.addAction(new QAction(QStringLiteral("Action %1").arg(i + 1), &toolbar));

    window.show();
    toolbar.show();
    toolbar.refreshOverflow();
    QVERIFY(toolbar.hasOverflow());
    toolbar.setOverflowExpanded(true);
    QVERIFY(toolbar.overflowExpanded());

    toolbar.resize(800, 48);
    toolbar.refreshOverflow();
    QCoreApplication::processEvents();

    QVERIFY(!toolbar.hasOverflow());
    QVERIFY(!toolbar.overflowExpanded());
}

QTEST_MAIN(PersistentToolBarTest)
#include "tst_persistenttoolbar.moc"
