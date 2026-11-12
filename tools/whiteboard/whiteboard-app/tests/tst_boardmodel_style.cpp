#include "BoardModel.h"

#include <QTemporaryDir>
#include <QTest>

class BoardModelStyleTest : public QObject {
    Q_OBJECT

private slots:
    void fillColorSettingsAndSelection();
    void resetDefaultAndSelectedStyle();
    void updateSelectedTextElement();
};

void BoardModelStyleTest::fillColorSettingsAndSelection()
{
    BoardModel model;
    const QColor fillColor(QStringLiteral("#80ffaa00"));
    const QColor replacementFillColor(QStringLiteral("#80335577"));
    const QColor replacementStrokeColor(QStringLiteral("#ff224466"));
    model.setFillColor(fillColor);

    model.addRectangle(QRectF(0, 0, 100, 80));
    QCOMPARE(model.currentPage().elements().first().fillColor(), fillColor);

    QVERIFY(model.selectAt(QPointF(50, 40)));
    QCOMPARE(model.selectedElementFillColor(), fillColor);
    QVERIFY(model.updateSelectedColor(replacementStrokeColor));
    QCOMPARE(model.currentPage().elements().first().color(), replacementStrokeColor);
    QVERIFY(model.undo());
    QCOMPARE(model.currentPage().elements().first().color(), BoardModel::DefaultColor);
    QVERIFY(model.redo());
    QCOMPARE(model.currentPage().elements().first().color(), replacementStrokeColor);

    QVERIFY(model.selectAt(QPointF(50, 40)));
    QVERIFY(model.updateSelectedStrokeWidth(0));
    QCOMPARE(model.currentPage().elements().first().strokeWidth(), 0);
    QCOMPARE(model.selectedElementStrokeWidth(), 0);
    QVERIFY(model.undo());
    QCOMPARE(model.currentPage().elements().first().strokeWidth(), 3);
    QVERIFY(model.redo());
    QCOMPARE(model.currentPage().elements().first().strokeWidth(), 0);

    QVERIFY(model.selectAt(QPointF(50, 40)));
    QVERIFY(model.updateSelectedFillColor(replacementFillColor));
    QCOMPARE(model.currentPage().elements().first().fillColor(), replacementFillColor);
    QVERIFY(model.undo());
    QCOMPARE(model.currentPage().elements().first().fillColor(), fillColor);
    QVERIFY(model.redo());
    QCOMPARE(model.currentPage().elements().first().fillColor(), replacementFillColor);

    model.clearSelection();
    model.setFillColor(fillColor);
    model.addPolyline({QPointF(0, 0), QPointF(100, 0), QPointF(100, 100)}, true);
    QVERIFY(model.selectAt(QPointF(80, 50)));
    QCOMPARE(model.selectedElementType(), ElementType::Polyline);

    model.clearSelection();
    model.addLine(QPointF(200, 0), QPointF(300, 0));
    QVERIFY(model.selectAt(QPointF(250, 0)));
    QVERIFY(!model.updateSelectedFillColor(fillColor));
}

void BoardModelStyleTest::resetDefaultAndSelectedStyle()
{
    BoardModel model;
    model.setSelectedColor(QColor(QStringLiteral("#ffffffff")));
    model.setFillColor(QColor(QStringLiteral("#660000ff")));
    model.setStrokeWidth(0);
    model.setStrokeStyle(StrokeStyle::Dotted);
    model.setFont(QFont(QStringLiteral("Courier"), 24));
    model.setStartArrowHead(ArrowHead::Triangle);
    model.setEndArrowHead(ArrowHead::Diamond);

    model.resetDefaultStyle();
    QCOMPARE(model.settings().selectedColor(), BoardModel::DefaultColor);
    QCOMPARE(model.settings().fillColor(), BoardModel::DefaultFillColor);
    QCOMPARE(model.settings().strokeWidth(), BoardModel::DefaultStrokeWidth);
    QCOMPARE(model.settings().strokeStyle(), BoardModel::DefaultStrokeStyle);
    QCOMPARE(model.settings().font(), BoardModel::DefaultFont);
    QCOMPARE(model.settings().startArrowHead(), BoardModel::DefaultStartArrowHead);
    QCOMPARE(model.settings().endArrowHead(), BoardModel::DefaultEndArrowHead);

    model.setSelectedColor(QColor(QStringLiteral("#ffffffff")));
    model.setFillColor(QColor(QStringLiteral("#660000ff")));
    model.setStrokeWidth(0);
    model.setStrokeStyle(StrokeStyle::Dotted);
    model.setStartArrowHead(ArrowHead::Open);
    model.setEndArrowHead(ArrowHead::Diamond);
    model.addLine(QPointF(0, 0), QPointF(100, 0));
    QVERIFY(model.selectAt(QPointF(50, 0)));

    QVERIFY(model.resetSelectedStyle());
    const DrawingElement element = model.currentPage().elements().first();
    QCOMPARE(element.color(), BoardModel::DefaultColor);
    QCOMPARE(element.fillColor(), BoardModel::DefaultFillColor);
    QCOMPARE(element.strokeWidth(), BoardModel::DefaultStrokeWidth);
    QCOMPARE(element.strokeStyle(), BoardModel::DefaultStrokeStyle);
    QCOMPARE(element.startArrowHead(), BoardModel::DefaultStartArrowHead);
    QCOMPARE(element.endArrowHead(), BoardModel::DefaultEndArrowHead);

    QVERIFY(model.undo());
    QCOMPARE(model.currentPage().elements().first().color(), QColor(QStringLiteral("#ffffffff")));

    model.clearSelection();
    model.setSelectedColor(QColor(QStringLiteral("#ffffffff")));
    model.setFont(QFont(QStringLiteral("Courier"), 24));
    model.addText(QPointF(10, 20), QStringLiteral("text"));
    QVERIFY(model.selectAt(QPointF(20, 30)));

    QVERIFY(model.resetSelectedStyle());
    const DrawingElement textElement = model.currentPage().elements().last();
    QCOMPARE(textElement.color(), BoardModel::DefaultColor);
    QCOMPARE(textElement.font(), BoardModel::DefaultFont);
}

void BoardModelStyleTest::updateSelectedTextElement()
{
    BoardModel model;
    model.addText(QPointF(10, 20), QStringLiteral("before"));

    QVERIFY(model.selectAt(QPointF(12, 22)));
    QCOMPARE(model.selectedElementType(), ElementType::Text);
    QCOMPARE(model.selectedElementText(), QStringLiteral("before"));
    QVERIFY(model.updateSelectedText(QStringLiteral("after\nline 2")));
    QCOMPARE(model.currentPage().elements().first().text(), QStringLiteral("after\nline 2"));

    QVERIFY(model.undo());
    QCOMPARE(model.currentPage().elements().first().text(), QStringLiteral("before"));
    QVERIFY(model.redo());
    QCOMPARE(model.currentPage().elements().first().text(), QStringLiteral("after\nline 2"));

    QVERIFY(model.selectAt(QPointF(12, 22)));
    QVERIFY(model.updateSelectedTextColor(QColor(QStringLiteral("#112233"))));
    QCOMPARE(model.currentPage().elements().first().color(), QColor(QStringLiteral("#112233")));

    QFont font(QStringLiteral("Courier"), 24);
    QVERIFY(model.updateSelectedTextFont(font));
    QCOMPARE(model.currentPage().elements().first().font().family(), QStringLiteral("Courier"));
    QCOMPARE(model.currentPage().elements().first().font().pointSize(), 24);
}

QTEST_MAIN(BoardModelStyleTest)

#include "tst_boardmodel_style.moc"
