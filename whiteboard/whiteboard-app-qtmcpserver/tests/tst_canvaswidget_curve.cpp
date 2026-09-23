#include "CanvasWidget.h"

#include <QCoreApplication>
#include <QImage>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPlainTextEdit>
#include <QTest>

class CanvasWidgetCurveTest : public QObject {
    Q_OBJECT

private slots:
    void openClosedPolylineAtVertexWithMouse();
    void closeExtendedPolylineAtOppositeEndpointWithMouse();
    void extendBezierEndpointWithMouse();
    void closeBezierWithMouse();
    void closeBezierWithControlReturn();
};

void CanvasWidgetCurveTest::openClosedPolylineAtVertexWithMouse()
{
    BoardModel model;
    model.addPolyline({QPointF(0, 0), QPointF(100, 0), QPointF(100, 100), QPointF(0, 100)}, true);
    model.selectTool(Tool::Select);

    CanvasWidget canvas;
    canvas.setModel(&model);

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::ControlModifier | Qt::ShiftModifier, QPoint(100, 100));

    const DrawingElement element = model.currentPage().elements().first();
    QVERIFY(!element.closed());
    QCOMPARE(element.points().first(), QPointF(100, 100));
    QCOMPARE(element.points().last(), QPointF(100, 0));
}

void CanvasWidgetCurveTest::closeExtendedPolylineAtOppositeEndpointWithMouse()
{
    BoardModel model;
    model.addPolyline({QPointF(0, 0), QPointF(100, 0), QPointF(100, 100)});
    model.selectTool(Tool::Select);

    CanvasWidget canvas;
    canvas.setModel(&model);

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::ControlModifier, QPoint(100, 100));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(150, 50));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(2, 2));

    const DrawingElement element = model.currentPage().elements().first();
    QVERIFY(element.closed());
    QCOMPARE(element.points().size(), 4);
    QCOMPARE(element.points().last(), QPointF(150, 50));
}

void CanvasWidgetCurveTest::extendBezierEndpointWithMouse()
{
    BoardModel model;
    model.addBezier({QPointF(0, 0), QPointF(50, 40), QPointF(100, 0)});
    model.selectTool(Tool::Select);

    CanvasWidget canvas;
    canvas.setModel(&model);

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::ControlModifier, QPoint(100, 0));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(140, 30));
    QTest::mouseDClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(180, 0));

    const DrawingElement element = model.currentPage().elements().first();
    QCOMPARE(element.type(), ElementType::Bezier);
    QCOMPARE(element.points().size(), 5);
    QCOMPARE(element.points().at(3), QPointF(140, 30));
    QCOMPARE(element.points().last(), QPointF(180, 0));
}

void CanvasWidgetCurveTest::closeBezierWithMouse()
{
    BoardModel model;
    model.selectTool(Tool::Bezier);

    CanvasWidget canvas;
    canvas.setModel(&model);

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(80, 80));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(140, 20));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(21, 21));

    QCOMPARE(model.currentPage().elements().size(), 1);
    const DrawingElement element = model.currentPage().elements().first();
    QCOMPARE(element.type(), ElementType::Bezier);
    QVERIFY(element.closed());
    QCOMPARE(element.points().size(), 3);
}

void CanvasWidgetCurveTest::closeBezierWithControlReturn()
{
    BoardModel model;
    model.selectTool(Tool::Bezier);

    CanvasWidget canvas;
    canvas.setModel(&model);

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(80, 80));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(140, 20));
    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Return, Qt::ControlModifier);
    QCoreApplication::sendEvent(&canvas, &keyPress);

    QCOMPARE(model.currentPage().elements().size(), 1);
    const DrawingElement element = model.currentPage().elements().first();
    QCOMPARE(element.type(), ElementType::Bezier);
    QVERIFY(element.closed());
}

QTEST_MAIN(CanvasWidgetCurveTest)

#include "tst_canvaswidget_curve.moc"
