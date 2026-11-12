#include "BoardModel.h"

#include <QTemporaryDir>
#include <QTest>

class BoardModelDrawingTest : public QObject {
    Q_OBJECT

private slots:
    void toolSelection();
    void drawingUpdates();
    void arrowsAndNewShapes();
    void lineMidpointSelection();
};

void BoardModelDrawingTest::toolSelection()
{
    BoardModel model;
    model.selectTool(Tool::Text);
    QCOMPARE(model.settings().selectedTool(), Tool::Text);
}

void BoardModelDrawingTest::drawingUpdates()
{
    BoardModel model;
    model.addFreehand({QPointF(10, 10), QPointF(20, 20)});
    QCOMPARE(model.currentPage().elements().size(), 1);
    QCOMPARE(model.currentPage().elements().first().type(), ElementType::Freehand);

    model.addLine(QPointF(0, 0), QPointF(40, 40));
    model.addPolyline({QPointF(0, 10), QPointF(20, 30), QPointF(40, 10)});
    model.addRectangle(QRectF(10, 10, 100, 80));
    model.addRoundedRectangle(QRectF(15, 15, 100, 80));
    model.addEllipse(QRectF(20, 20, 100, 80));
    model.addText(QPointF(30, 30), QStringLiteral("text"));
    QCOMPARE(model.currentPage().elements().size(), 7);

    QVERIFY(model.eraseAt(QPointF(10, 10)));
    QCOMPARE(model.currentPage().elements().size(), 6);
}

void BoardModelDrawingTest::arrowsAndNewShapes()
{
    BoardModel model;
    model.setStartArrowHead(ArrowHead::Triangle);
    model.setEndArrowHead(ArrowHead::Diamond);
    model.setStrokeStyle(StrokeStyle::Dotted);

    model.addLine(QPointF(0, 0), QPointF(100, 0));
    QCOMPARE(model.currentPage().elements().first().startArrowHead(), ArrowHead::Triangle);
    QCOMPARE(model.currentPage().elements().first().endArrowHead(), ArrowHead::Diamond);
    QCOMPARE(model.currentPage().elements().first().strokeStyle(), StrokeStyle::Dotted);

    model.addBezier({QPointF(10, 10), QPointF(60, 20), QPointF(110, 80), QPointF(150, 40)});
    QCOMPARE(model.currentPage().elements().last().type(), ElementType::Bezier);
    QCOMPARE(model.currentPage().elements().last().points().size(), 4);

    model.addArc(QRectF(0, 0, 160, 80));
    QCOMPARE(model.currentPage().elements().last().type(), ElementType::Arc);
    QCOMPARE(model.currentPage().elements().last().bounds().width(), model.currentPage().elements().last().bounds().height());

    QVERIFY(model.selectAt(QPointF(75, 20)));
    QCOMPARE(model.selectedElementType(), ElementType::Arc);
    model.beginSelectionEdit();
    QVERIFY(model.updateSelectedArcAngles(45, 180));
    model.endSelectionEdit();
    QCOMPARE(model.currentPage().elements().last().arcStartAngle(), 45);
    QCOMPARE(model.currentPage().elements().last().arcSpanAngle(), 180);

    model.addCircle(QRectF(0, 0, 120, 60));
    QCOMPARE(model.currentPage().elements().last().type(), ElementType::Circle);
    QCOMPARE(model.currentPage().elements().last().bounds().width(), model.currentPage().elements().last().bounds().height());
    QVERIFY(model.selectAt(QPointF(20, 20)));
    QVERIFY(model.resizeSelectedTo(QRectF(0, 0, 180, 90)));
    QCOMPARE(model.currentPage().elements().last().bounds().width(), model.currentPage().elements().last().bounds().height());
    QVERIFY(model.updateSelectedStrokeStyle(StrokeStyle::Solid));
    QCOMPARE(model.currentPage().elements().last().strokeStyle(), StrokeStyle::Solid);

    model.setCornerRadius(36);
    model.addRoundedRectangle(QRectF(10, 10, 120, 80));
    QCOMPARE(model.currentPage().elements().last().type(), ElementType::RoundedRectangle);
    QCOMPARE(model.currentPage().elements().last().cornerRadius(), 36);
    QVERIFY(model.selectAt(QPointF(12, 50)));
    QCOMPARE(model.selectedElementType(), ElementType::RoundedRectangle);
    QVERIFY(model.updateSelectedCornerRadius(12));
    QCOMPARE(model.currentPage().elements().last().cornerRadius(), 12);
    QVERIFY(model.undo());
    QCOMPARE(model.currentPage().elements().last().cornerRadius(), 36);
    QVERIFY(model.redo());
    QCOMPARE(model.currentPage().elements().last().cornerRadius(), 12);
}

void BoardModelDrawingTest::lineMidpointSelection()
{
    BoardModel model;
    model.addLine(QPointF(0, 0), QPointF(100, 0));

    QVERIFY(model.selectAt(QPointF(50, 2)));
    QCOMPARE(model.selectedElementType(), ElementType::Line);

    model.beginSelectionEdit();
    QVERIFY(model.moveSelectedVertexTo(0, QPointF(20, -40)));
    model.endSelectionEdit();
    QCOMPARE(model.currentPage().elements().first().points().first(), QPointF(20, -40));
    QCOMPARE(model.currentPage().elements().first().points().last(), QPointF(100, 0));

    QVERIFY(model.undo());
    QCOMPARE(model.currentPage().elements().first().points().first(), QPointF(0, 0));
    QVERIFY(model.redo());
    QCOMPARE(model.currentPage().elements().first().points().first(), QPointF(20, -40));
}

QTEST_MAIN(BoardModelDrawingTest)

#include "tst_boardmodel_drawing.moc"
