#include "AnkerFormLayout.h"

#include <QTest>
#include <QWidget>

using namespace Anker;

class AnkerFormLayoutTest : public QObject
{
    Q_OBJECT

private slots:
    void formAttachmentsStretchWidget();
    void widgetAttachmentsUseDefaultSpacing();
    void positionAttachmentsUseFractionBase();
    void oppositeFormAttachmentsUseFarFormEdge();
    void oppositeWidgetAttachmentsShareEdges();
    void selfAttachmentsCanRubberPosition();
};

void AnkerFormLayoutTest::formAttachmentsStretchWidget()
{
    QWidget parent;
    auto *layout = new FormLayout(&parent);
    layout->setContentsMargins(10, 20, 30, 40);

    auto *child = new QWidget;
    child->setMinimumSize(20, 10);
    layout->addWidget(child, {
        .left = attachForm(1),
        .right = attachForm(2),
        .top = attachForm(3),
        .bottom = attachForm(4),
    });

    layout->setGeometry(QRect(0, 0, 200, 100));

    QCOMPARE(child->geometry(), QRect(11, 23, 157, 33));
}

void AnkerFormLayoutTest::widgetAttachmentsUseDefaultSpacing()
{
    QWidget parent;
    auto *layout = new FormLayout(&parent);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setHorizontalSpacing(7);

    auto *left = new QWidget;
    left->setFixedSize(20, 10);
    auto *right = new QWidget;
    right->setFixedSize(30, 10);

    layout->addWidget(left, {
        .left = attachForm(),
        .top = attachForm(),
    });
    layout->addWidget(right, {
        .left = attachWidget(left),
        .top = attachOppositeWidget(left),
    });

    layout->setGeometry(QRect(0, 0, 100, 50));

    QCOMPARE(left->geometry(), QRect(0, 0, 20, 10));
    QCOMPARE(right->geometry(), QRect(27, 0, 30, 10));
}

void AnkerFormLayoutTest::positionAttachmentsUseFractionBase()
{
    QWidget parent;
    auto *layout = new FormLayout(&parent);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setFractionBase(10);

    auto *child = new QWidget;
    child->setMinimumSize(1, 1);
    layout->addWidget(child, {
        .left = attachPosition(2),
        .right = attachPosition(8),
        .top = attachPosition(1),
        .bottom = attachPosition(9),
    });

    layout->setGeometry(QRect(0, 0, 100, 50));

    QCOMPARE(child->geometry(), QRect(20, 5, 60, 40));
}

void AnkerFormLayoutTest::oppositeFormAttachmentsUseFarFormEdge()
{
    QWidget parent;
    auto *layout = new FormLayout(&parent);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *child = new QWidget;
    child->setFixedSize(10, 10);
    layout->addWidget(child, {
        .left = attachOppositeForm(3),
        .top = attachOppositeForm(4),
    });

    layout->setGeometry(QRect(0, 0, 100, 50));

    QCOMPARE(child->geometry(), QRect(103, 54, 10, 10));
}

void AnkerFormLayoutTest::oppositeWidgetAttachmentsShareEdges()
{
    QWidget parent;
    auto *layout = new FormLayout(&parent);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *source = new QWidget;
    source->setFixedSize(40, 20);
    auto *aligned = new QWidget;
    aligned->setFixedSize(10, 10);

    layout->addWidget(source, {
        .left = attachForm(15),
        .top = attachForm(12),
    });
    layout->addWidget(aligned, {
        .left = attachOppositeWidget(source),
        .top = attachOppositeWidget(source),
    });

    layout->setGeometry(QRect(0, 0, 100, 80));

    QCOMPARE(source->geometry(), QRect(15, 12, 40, 20));
    QCOMPARE(aligned->geometry(), QRect(15, 12, 10, 10));
}

void AnkerFormLayoutTest::selfAttachmentsCanRubberPosition()
{
    QWidget parent;
    auto *layout = new FormLayout(&parent);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setRubberPositioning(true);

    auto *child = new QWidget(&parent);
    child->setGeometry(25, 10, 20, 10);
    child->setFixedSize(20, 10);
    layout->addWidget(child, {
        .left = attachSelf(),
        .top = attachSelf(),
    });

    layout->setGeometry(QRect(0, 0, 100, 50));
    QCOMPARE(child->geometry(), QRect(25, 10, 20, 10));

    layout->setGeometry(QRect(0, 0, 200, 100));
    QCOMPARE(child->geometry(), QRect(50, 20, 20, 10));
}

QTEST_MAIN(AnkerFormLayoutTest)

#include "tst_formlayout.moc"
