#include "BoardModel.h"

#include <QTemporaryDir>
#include <QTest>

class BoardModelCurveTest : public QObject {
    Q_OBJECT

private slots:
    void polylineUpdates();
    void closedPolylineUpdates();
    void closedBezierUpdates();
    void openClosedPolyline();
    void openClosedPolylineAtVertex();
    void selectOpenPolylineEndpoint();
    void extendSelectedPolyline();
    void editPolylinePoints();
    void extendSelectedBezier();
    void editBezierControlPoints();
    void movePolylineVertex();
};

void BoardModelCurveTest::polylineUpdates()
{
    BoardModel model;
    model.addPolyline({QPointF(0, 0), QPointF(50, 40), QPointF(100, 0)});

    QCOMPARE(model.currentPage().elements().size(), 1);
    QCOMPARE(model.currentPage().elements().first().type(), ElementType::Polyline);
    QVERIFY(model.selectAt(QPointF(25, 20)));
    QCOMPARE(model.selectedElementType(), ElementType::Polyline);
    model.clearSelection();
    QVERIFY(model.selectAt(QPointF(75, 20)));
    QCOMPARE(model.selectedElementType(), ElementType::Polyline);
}

void BoardModelCurveTest::closedPolylineUpdates()
{
    BoardModel model;
    model.addPolyline({QPointF(0, 0), QPointF(100, 0), QPointF(100, 100)}, true);

    QCOMPARE(model.currentPage().elements().size(), 1);
    QCOMPARE(model.currentPage().elements().first().type(), ElementType::Polyline);
    QVERIFY(model.currentPage().elements().first().closed());
    QVERIFY(model.selectAt(QPointF(50, 50)));
    QCOMPARE(model.selectedElementType(), ElementType::Polyline);

    QVERIFY(model.undo());
    QCOMPARE(model.currentPage().elements().size(), 0);

    model.addPolyline({QPointF(0, 0), QPointF(100, 0), QPointF(100, 100)});
    QVERIFY(model.selectAt(QPointF(50, 0)));
    QVERIFY(model.closeSelectedPolyline());
    QVERIFY(model.currentPage().elements().first().closed());
    QVERIFY(model.undo());
    QVERIFY(!model.currentPage().elements().first().closed());
    QVERIFY(model.redo());
    QVERIFY(model.currentPage().elements().first().closed());
}

void BoardModelCurveTest::closedBezierUpdates()
{
    BoardModel model;
    model.setStartArrowHead(ArrowHead::Triangle);
    model.setEndArrowHead(ArrowHead::Diamond);
    model.addBezier({QPointF(0, 0), QPointF(50, 80), QPointF(100, 0)}, true);

    QCOMPARE(model.currentPage().elements().size(), 1);
    QCOMPARE(model.currentPage().elements().first().type(), ElementType::Bezier);
    QVERIFY(model.currentPage().elements().first().closed());
    QCOMPARE(model.currentPage().elements().first().startArrowHead(), ArrowHead::None);

    QVERIFY(model.selectAt(QPointF(50, 80)));
    QCOMPARE(model.selectedElementType(), ElementType::Bezier);
    QVERIFY(model.updateSelectedFillColor(QColor(QStringLiteral("#80335577"))));
    QCOMPARE(model.currentPage().elements().first().fillColor(), QColor(QStringLiteral("#80335577")));
    QVERIFY(!model.updateSelectedArrowHeads(ArrowHead::Open, ArrowHead::Diamond));

    QVERIFY(model.openSelectedBezier());
    QVERIFY(!model.currentPage().elements().first().closed());
    QVERIFY(model.updateSelectedArrowHeads(ArrowHead::Open, ArrowHead::Diamond));
    QCOMPARE(model.currentPage().elements().first().startArrowHead(), ArrowHead::Open);
    QVERIFY(model.closeSelectedBezier());
    QVERIFY(model.currentPage().elements().first().closed());
    QCOMPARE(model.currentPage().elements().first().startArrowHead(), ArrowHead::None);
    QCOMPARE(model.currentPage().elements().first().endArrowHead(), ArrowHead::None);

    QVERIFY(model.undo());
    QVERIFY(!model.currentPage().elements().first().closed());
    QVERIFY(model.redo());
    QVERIFY(model.currentPage().elements().first().closed());
}

void BoardModelCurveTest::openClosedPolyline()
{
    BoardModel model;
    model.addPolyline({QPointF(0, 0), QPointF(100, 0), QPointF(100, 100)}, true);
    QVERIFY(model.selectAt(QPointF(50, 50)));
    QVERIFY(model.openSelectedPolyline());
    QVERIFY(!model.currentPage().elements().first().closed());

    QVERIFY(model.undo());
    QVERIFY(model.currentPage().elements().first().closed());
    QVERIFY(model.redo());
    QVERIFY(!model.currentPage().elements().first().closed());
    QVERIFY(!model.openSelectedPolyline());
}

void BoardModelCurveTest::openClosedPolylineAtVertex()
{
    BoardModel model;
    model.addPolyline({QPointF(0, 0), QPointF(100, 0), QPointF(100, 100), QPointF(0, 100)}, true);

    QCOMPARE(model.selectClosedPolylineVertexAt(QPointF(100, 100), 12), 2);
    QVERIFY(model.openSelectedPolylineAtVertex(2));
    QVERIFY(!model.currentPage().elements().first().closed());
    QCOMPARE(model.currentPage().elements().first().points().first(), QPointF(100, 100));
    QCOMPARE(model.currentPage().elements().first().points().last(), QPointF(100, 0));

    QVERIFY(model.undo());
    QVERIFY(model.currentPage().elements().first().closed());
    QVERIFY(model.redo());
    QVERIFY(!model.currentPage().elements().first().closed());
}

void BoardModelCurveTest::selectOpenPolylineEndpoint()
{
    BoardModel model;
    model.addPolyline({QPointF(0, 0), QPointF(50, 40), QPointF(100, 0)});
    model.addRectangle(QRectF(90, -10, 40, 40));

    QCOMPARE(model.selectOpenPolylineEndpointAt(QPointF(100, 0), 12), 2);
    QCOMPARE(model.selectedElementType(), ElementType::Polyline);
    QCOMPARE(model.selectOpenPolylineEndpointAt(QPointF(50, 40), 12), -1);

    model.addPolyline({QPointF(200, 0), QPointF(240, 40), QPointF(280, 0)}, true);
    QCOMPARE(model.selectOpenPolylineEndpointAt(QPointF(200, 0), 12), -1);
}

void BoardModelCurveTest::extendSelectedPolyline()
{
    BoardModel model;
    model.addPolyline({QPointF(0, 0), QPointF(50, 40), QPointF(100, 0)});
    QVERIFY(model.selectAt(QPointF(100, 0)));
    QVERIFY(model.updateSelectedPolyline({QPointF(0, 0), QPointF(50, 40), QPointF(100, 0), QPointF(140, 20)}, false));
    QCOMPARE(model.currentPage().elements().first().points().size(), 4);
    QCOMPARE(model.currentPage().elements().first().points().last(), QPointF(140, 20));

    QVERIFY(model.updateSelectedPolyline({QPointF(-40, 20), QPointF(0, 0), QPointF(50, 40), QPointF(100, 0), QPointF(140, 20)}, false));
    QCOMPARE(model.currentPage().elements().first().points().first(), QPointF(-40, 20));

    QVERIFY(model.updateSelectedPolyline({QPointF(-40, 20), QPointF(0, 0), QPointF(50, 40), QPointF(100, 0), QPointF(140, 20)}, true));
    QVERIFY(model.currentPage().elements().first().closed());
    QCOMPARE(model.currentPage().elements().first().points().size(), 5);
    QCOMPARE(model.currentPage().elements().first().points().first(), QPointF(-40, 20));
    QCOMPARE(model.currentPage().elements().first().points().last(), QPointF(140, 20));

    QVERIFY(model.undo());
    QVERIFY(!model.currentPage().elements().first().closed());
    QCOMPARE(model.currentPage().elements().first().points().first(), QPointF(-40, 20));
    QVERIFY(model.redo());
    QVERIFY(model.currentPage().elements().first().closed());
    QCOMPARE(model.currentPage().elements().first().points().first(), QPointF(-40, 20));
}

void BoardModelCurveTest::editPolylinePoints()
{
    BoardModel model;
    model.addPolyline({QPointF(0, 0), QPointF(100, 0)});
    QVERIFY(model.selectAt(QPointF(50, 0)));

    QVERIFY(model.insertSelectedPolylinePoint(QPointF(50, 5), 10));
    QCOMPARE(model.currentPage().elements().first().points().size(), 3);
    QCOMPARE(model.currentPage().elements().first().points().at(1), QPointF(50, 5));

    QVERIFY(model.deleteSelectedPolylinePoint(1));
    QCOMPARE(model.currentPage().elements().first().points().size(), 2);
    QVERIFY(!model.deleteSelectedPolylinePoint(1));

    QVERIFY(model.undo());
    QCOMPARE(model.currentPage().elements().first().points().size(), 3);
    QVERIFY(model.redo());
    QCOMPARE(model.currentPage().elements().first().points().size(), 2);

    model.addPolyline({QPointF(0, 100), QPointF(50, 150), QPointF(100, 100)}, true);
    QVERIFY(model.selectAt(QPointF(50, 145)));
    QVERIFY(!model.deleteSelectedPolylinePoint(1));
    QVERIFY(model.insertSelectedPolylinePoint(QPointF(50, 100), 20));
    QCOMPARE(model.currentPage().elements().last().points().size(), 4);
    QVERIFY(model.deleteSelectedPolylinePoint(1));
    QCOMPARE(model.currentPage().elements().last().points().size(), 3);
}

void BoardModelCurveTest::extendSelectedBezier()
{
    BoardModel model;
    model.addBezier({QPointF(0, 0), QPointF(50, 40), QPointF(100, 0)});

    QCOMPARE(model.selectBezierEndpointAt(QPointF(100, 0), 12), 2);
    QVERIFY(model.updateSelectedBezier({QPointF(0, 0), QPointF(50, 40), QPointF(100, 0), QPointF(140, 20)}));
    QCOMPARE(model.currentPage().elements().first().points().size(), 4);
    QCOMPARE(model.currentPage().elements().first().points().last(), QPointF(140, 20));

    QCOMPARE(model.selectBezierEndpointAt(QPointF(0, 0), 12), 0);
    QVERIFY(model.updateSelectedBezier({QPointF(-40, 20), QPointF(0, 0), QPointF(50, 40), QPointF(100, 0), QPointF(140, 20)}));
    QCOMPARE(model.currentPage().elements().first().points().first(), QPointF(-40, 20));

    QVERIFY(model.undo());
    QCOMPARE(model.currentPage().elements().first().points().first(), QPointF(0, 0));
    QVERIFY(model.redo());
    QCOMPARE(model.currentPage().elements().first().points().first(), QPointF(-40, 20));
}

void BoardModelCurveTest::editBezierControlPoints()
{
    BoardModel model;
    model.addBezier({QPointF(0, 0), QPointF(100, 0)});
    QVERIFY(model.selectAt(QPointF(50, 0)));

    QVERIFY(model.insertSelectedBezierControlPoint(QPointF(50, 5), 10));
    QCOMPARE(model.currentPage().elements().first().points().size(), 3);
    QCOMPARE(model.currentPage().elements().first().points().at(1), QPointF(50, 5));

    QVERIFY(model.deleteSelectedBezierControlPoint(1));
    QCOMPARE(model.currentPage().elements().first().points().size(), 2);
    QVERIFY(!model.deleteSelectedBezierControlPoint(1));

    QVERIFY(model.undo());
    QCOMPARE(model.currentPage().elements().first().points().size(), 3);
    QVERIFY(model.redo());
    QCOMPARE(model.currentPage().elements().first().points().size(), 2);

    model.addBezier({QPointF(0, 100), QPointF(50, 150), QPointF(100, 100)}, true);
    QVERIFY(model.selectAt(QPointF(50, 140)));
    QVERIFY(!model.deleteSelectedBezierControlPoint(1));
    QVERIFY(model.insertSelectedBezierControlPoint(QPointF(50, 95), 20));
    QCOMPARE(model.currentPage().elements().last().points().size(), 4);
    QVERIFY(model.deleteSelectedBezierControlPoint(1));
    QCOMPARE(model.currentPage().elements().last().points().size(), 3);
}

void BoardModelCurveTest::movePolylineVertex()
{
    BoardModel model;
    model.addPolyline({QPointF(0, 0), QPointF(50, 40), QPointF(100, 0)});

    QVERIFY(model.selectAt(QPointF(50, 40)));
    model.beginSelectionEdit();
    QVERIFY(model.moveSelectedVertexTo(1, QPointF(60, 80)));
    model.endSelectionEdit();
    QCOMPARE(model.currentPage().elements().first().points().at(1), QPointF(60, 80));

    QVERIFY(model.undo());
    QCOMPARE(model.currentPage().elements().first().points().at(1), QPointF(50, 40));
    QVERIFY(model.redo());
    QCOMPARE(model.currentPage().elements().first().points().at(1), QPointF(60, 80));
}

QTEST_MAIN(BoardModelCurveTest)

#include "tst_boardmodel_curve.moc"
