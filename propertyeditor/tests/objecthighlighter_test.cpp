#include "objecthighlighter.h"

#include <QtGui/QGuiApplication>
#include <QtTest/QTest>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

namespace {

int visibleHighlighterOverlayCount(QWidget *windowWidget)
{
    int overlayCount = 0;
    const auto childWidgets = windowWidget->findChildren<QWidget *>();
    for (QWidget *childWidget : childWidgets) {
        if (childWidget->property("objectHighlighterOverlay").toBool() && childWidget->isVisible()) {
            ++overlayCount;
        }
    }

    return overlayCount;
}

} // namespace

class ObjectHighlighterTest : public QObject
{
    Q_OBJECT

private slots:
    void multipleHighlightersRemainIndependent();
};

void ObjectHighlighterTest::multipleHighlightersRemainIndependent()
{
    if (QGuiApplication::screens().isEmpty()) {
        QSKIP("No screens available in this environment");
    }

    QWidget windowWidget;
    windowWidget.resize(320, 120);
    auto *layout = new QHBoxLayout(&windowWidget);
    auto *leftLabel = new QLabel(QStringLiteral("Left"), &windowWidget);
    auto *rightLabel = new QLabel(QStringLiteral("Right"), &windowWidget);
    layout->addWidget(leftLabel);
    layout->addWidget(rightLabel);
    windowWidget.show();
    QVERIFY(QTest::qWaitForWindowExposed(&windowWidget));

    ObjectHighlighter firstHighlighter;
    ObjectHighlighter secondHighlighter;

    firstHighlighter.setTargetObject(leftLabel);
    secondHighlighter.setTargetObject(rightLabel);
    QCoreApplication::processEvents();

    QCOMPARE(visibleHighlighterOverlayCount(&windowWidget), 2);

    firstHighlighter.clear();
    QCoreApplication::processEvents();

    QCOMPARE(visibleHighlighterOverlayCount(&windowWidget), 1);
    QCOMPARE(secondHighlighter.targetObject(), static_cast<QObject *>(rightLabel));

    secondHighlighter.clear();
    QCoreApplication::processEvents();

    QCOMPARE(visibleHighlighterOverlayCount(&windowWidget), 0);
}

QTEST_MAIN(ObjectHighlighterTest)

#include "objecthighlighter_test.moc"
