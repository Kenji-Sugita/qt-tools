#include "BoardModel.h"

#include <QLineF>
#include <QTemporaryDir>
#include <QTest>

class BoardModelSelectionTest : public QObject {
    Q_OBJECT

private slots:
    void selectMoveAndResizeElement();
    void scaleSelectedElement();
    void rubberBandSelectionMoveDeleteAndCopyPaste();
    void groupAndUngroupSelection();
    void rotateSingleAndGroupedElements();
    void moveSelectedElementOrder();
    void deleteSelectedElement();
    void connectorLineFollowsMovedShapes();
    void reconnectDetachedLineEndpoint();
    void copyPasteRemapsConnectorTargets();
};

namespace {
void compareRect(const QRectF &actual, const QRectF &expected)
{
    QVERIFY(qAbs(actual.x() - expected.x()) < 0.001);
    QVERIFY(qAbs(actual.y() - expected.y()) < 0.001);
    QVERIFY(qAbs(actual.width() - expected.width()) < 0.001);
    QVERIFY(qAbs(actual.height() - expected.height()) < 0.001);
}
}

void BoardModelSelectionTest::selectMoveAndResizeElement()
{
    BoardModel model;
    model.addRectangle(QRectF(10, 20, 100, 80));

    QVERIFY(model.selectAt(QPointF(20, 30)));
    QVERIFY(model.hasSelection());
    QCOMPARE(model.selectedElementIndex(), 0);
    QCOMPARE(model.selectedElementBounds(), QRectF(10, 20, 100, 80));

    model.beginSelectionEdit();
    QVERIFY(model.moveSelectedBy(QPointF(15, 25)));
    model.endSelectionEdit();
    QCOMPARE(model.selectedElementBounds(), QRectF(25, 45, 100, 80));

    QVERIFY(model.undo());
    QVERIFY(!model.hasSelection());
    QCOMPARE(model.currentPage().elements().first().bounds(), QRectF(10, 20, 100, 80));
    QVERIFY(model.redo());
    QCOMPARE(model.currentPage().elements().first().bounds(), QRectF(25, 45, 100, 80));

    QVERIFY(model.selectAt(QPointF(30, 50)));
    model.beginSelectionEdit();
    QVERIFY(model.resizeSelectedTo(QRectF(25, 45, 150, 120)));
    model.endSelectionEdit();
    QCOMPARE(model.selectedElementBounds(), QRectF(25, 45, 150, 120));
}

void BoardModelSelectionTest::scaleSelectedElement()
{
    BoardModel model;
    model.setZoomPercent(50);
    model.addRectangle(QRectF(10, 20, 100, 80));
    model.addEllipse(QRectF(300, 300, 50, 50));

    QVERIFY(model.selectAt(QPointF(20, 30)));
    QVERIFY(model.scaleSelectedBy(1.5));
    QCOMPARE(model.settings().zoomPercent(), 50);
    QCOMPARE(model.selectedElementBounds(), QRectF(-15, 0, 150, 120));
    QCOMPARE(model.currentPage().elements().at(1).bounds(), QRectF(300, 300, 50, 50));

    QVERIFY(model.undo());
    QCOMPARE(model.currentPage().elements().first().bounds(), QRectF(10, 20, 100, 80));
    QVERIFY(model.redo());
    QCOMPARE(model.currentPage().elements().first().bounds(), QRectF(-15, 0, 150, 120));

    model.clearSelection();
    QVERIFY(model.selectInRect(QRectF(-20, -5, 400, 360)));
    QCOMPARE(model.selectedElementIndexes().size(), 2);
    QVERIFY(model.scaleSelectedBy(2.0));
    QCOMPARE(model.currentPage().elements().at(0).bounds(), QRectF(-197.5, -175, 300, 240));
    QCOMPARE(model.currentPage().elements().at(1).bounds(), QRectF(432.5, 425, 100, 100));

    BoardModel groupModel;
    groupModel.addRectangle(QRectF(10, 20, 40, 40));
    groupModel.addEllipse(QRectF(90, 20, 40, 40));
    QVERIFY(groupModel.selectInRect(QRectF(0, 0, 140, 80)));
    QVERIFY(groupModel.groupSelectedElements());
    groupModel.clearSelection();
    QVERIFY(groupModel.selectAt(QPointF(20, 30)));
    QVERIFY(groupModel.scaleSelectedBy(2.0));
    QCOMPARE(groupModel.currentPage().elements().at(0).bounds(), QRectF(-50, 0, 80, 80));
    QCOMPARE(groupModel.currentPage().elements().at(1).bounds(), QRectF(110, 0, 80, 80));
    QCOMPARE(groupModel.selectedElementBounds(), QRectF(-50, 0, 240, 80));
    QVERIFY(groupModel.undo());
    QCOMPARE(groupModel.currentPage().elements().at(0).bounds(), QRectF(10, 20, 40, 40));
    QCOMPARE(groupModel.currentPage().elements().at(1).bounds(), QRectF(90, 20, 40, 40));

    BoardModel textGroupModel;
    textGroupModel.addRectangle(QRectF(0, 0, 40, 40));
    textGroupModel.addText(QPointF(80, 0), QStringLiteral("text"));
    QVERIFY(textGroupModel.selectInRect(QRectF(-10, -10, 200, 80)));
    QVERIFY(textGroupModel.groupSelectedElements());
    QVERIFY(textGroupModel.scaleSelectedBy(1.5));
    QCOMPARE(textGroupModel.currentPage().elements().at(1).font().pointSize(), 27);

    model.clearSelection();
    model.addText(QPointF(500, 500), QStringLiteral("text"));
    QVERIFY(model.selectAt(QPointF(505, 505)));
    QCOMPARE(model.selectedElementType(), ElementType::Text);
    QVERIFY(!model.scaleSelectedBy(2.0));
}

void BoardModelSelectionTest::rubberBandSelectionMoveDeleteAndCopyPaste()
{
    BoardModel model;
    model.addRectangle(QRectF(10, 10, 40, 40));
    model.addEllipse(QRectF(80, 10, 40, 40));
    model.addLine(QPointF(200, 10), QPointF(240, 10));

    QVERIFY(model.selectInRect(QRectF(0, 0, 140, 80)));
    QCOMPARE(model.selectedElementIndexes().size(), 2);

    model.clearSelection();
    QVERIFY(model.selectAt(QPointF(20, 20)));
    QVERIFY(model.addToSelectionAt(QPointF(100, 20)));
    QCOMPARE(model.selectedElementIndexes().size(), 2);
    QCOMPARE(model.selectedElementIndexes().at(0), 0);
    QCOMPARE(model.selectedElementIndexes().at(1), 1);
    QVERIFY(model.addToSelectionAt(QPointF(100, 20)));
    QCOMPARE(model.selectedElementIndexes().size(), 2);

    model.beginSelectionEdit();
    QVERIFY(model.moveSelectedBy(QPointF(10, 20)));
    model.endSelectionEdit();
    QCOMPARE(model.currentPage().elements().at(0).bounds(), QRectF(20, 30, 40, 40));
    QCOMPARE(model.currentPage().elements().at(1).bounds(), QRectF(90, 30, 40, 40));

    const QVector<DrawingElement> copied = model.copySelectedElements();
    QCOMPARE(copied.size(), 2);
    QVERIFY(model.pasteElements(copied));
    QCOMPARE(model.currentPage().elements().size(), 5);
    QCOMPARE(model.selectedElementIndexes().size(), 2);
    QCOMPARE(model.currentPage().elements().at(3).bounds(), QRectF(44, 54, 40, 40));

    QVERIFY(model.deleteSelectedElement());
    QCOMPARE(model.currentPage().elements().size(), 3);

    QVERIFY(model.undo());
    QCOMPARE(model.currentPage().elements().size(), 5);
}

void BoardModelSelectionTest::groupAndUngroupSelection()
{
    BoardModel deleteModel;
    deleteModel.addRectangle(QRectF(10, 10, 40, 40));
    deleteModel.addEllipse(QRectF(80, 10, 40, 40));
    deleteModel.addLine(QPointF(200, 10), QPointF(240, 10));
    QVERIFY(deleteModel.selectInRect(QRectF(0, 0, 140, 80)));
    QVERIFY(deleteModel.groupSelectedElements());
    deleteModel.clearSelection();
    QVERIFY(deleteModel.selectAt(QPointF(20, 20)));
    QCOMPARE(deleteModel.selectedElementIndexes(), QVector<int>({0, 1}));
    QVERIFY(deleteModel.deleteSelectedElement());
    QCOMPARE(deleteModel.currentPage().elements().size(), 1);

    BoardModel model;
    model.addRectangle(QRectF(10, 10, 40, 40));
    model.addEllipse(QRectF(80, 10, 40, 40));
    model.addLine(QPointF(200, 10), QPointF(240, 10));

    QVERIFY(model.selectInRect(QRectF(0, 0, 140, 80)));
    QCOMPARE(model.selectedElementIndexes(), QVector<int>({0, 1}));
    QVERIFY(model.groupSelectedElements());
    const int groupId = model.currentPage().elements().at(0).groupId();
    QVERIFY(groupId > 0);
    QCOMPARE(model.currentPage().elements().at(1).groupId(), groupId);
    QCOMPARE(model.currentPage().elements().at(2).groupId(), 0);

    model.clearSelection();
    QVERIFY(model.selectAt(QPointF(20, 20)));
    QCOMPARE(model.selectedElementIndexes(), QVector<int>({0, 1}));

    model.beginSelectionEdit();
    QVERIFY(model.moveSelectedBy(QPointF(20, 30)));
    model.endSelectionEdit();
    QCOMPARE(model.currentPage().elements().at(0).bounds(), QRectF(30, 40, 40, 40));
    QCOMPARE(model.currentPage().elements().at(1).bounds(), QRectF(100, 40, 40, 40));
    QCOMPARE(model.currentPage().elements().at(2).bounds(), QRectF(200, 10, 40, 0));

    const QVector<DrawingElement> copied = model.copySelectedElements();
    QCOMPARE(copied.size(), 2);
    QVERIFY(model.pasteElements(copied));
    QCOMPARE(model.selectedElementIndexes(), QVector<int>({3, 4}));
    const int pastedGroupId = model.currentPage().elements().at(3).groupId();
    QVERIFY(pastedGroupId > 0);
    QVERIFY(pastedGroupId != groupId);
    QCOMPARE(model.currentPage().elements().at(4).groupId(), pastedGroupId);

    model.clearSelection();
    QVERIFY(model.selectAt(QPointF(54, 64)));
    QCOMPARE(model.selectedElementIndexes(), QVector<int>({3, 4}));
    QVERIFY(model.ungroupSelectedElements());
    QCOMPARE(model.currentPage().elements().at(3).groupId(), 0);
    QCOMPARE(model.currentPage().elements().at(4).groupId(), 0);

    QVERIFY(model.undo());
    QCOMPARE(model.currentPage().elements().at(3).groupId(), pastedGroupId);
    QVERIFY(model.redo());
    QCOMPARE(model.currentPage().elements().at(3).groupId(), 0);
}

void BoardModelSelectionTest::rotateSingleAndGroupedElements()
{
    BoardModel model;
    model.addRectangle(QRectF(10, 10, 100, 40));
    QVERIFY(model.selectAt(QPointF(20, 20)));
    const QPointF center = model.selectedElementBounds().center();
    QVERIFY(model.beginRotationEdit(center));
    QVERIFY(model.rotateSelectionTo(90.0));
    model.endSelectionEdit();

    QCOMPARE(model.currentPage().elements().first().rotationDegrees(), 90.0);
    compareRect(model.currentPage().elements().first().bounds(), QRectF(10, 10, 100, 40));
    compareRect(model.selectedElementBounds(), QRectF(40, -20, 40, 100));
    QVERIFY(model.selectAt(QPointF(60, -10)));

    QVERIFY(model.undo());
    QCOMPARE(model.currentPage().elements().first().rotationDegrees(), 0.0);
    QVERIFY(model.redo());
    QCOMPARE(model.currentPage().elements().first().rotationDegrees(), 90.0);
    QVERIFY(model.selectAt(QPointF(60, -10)));
    const QVector<DrawingElement> rotatedCopy = model.copySelectedElements();
    QCOMPARE(rotatedCopy.size(), 1);
    QCOMPARE(rotatedCopy.first().rotationDegrees(), 90.0);
    QVERIFY(model.pasteElements(rotatedCopy));
    QCOMPARE(model.currentPage().elements().last().rotationDegrees(), 90.0);

    BoardModel lineModel;
    lineModel.addLine(QPointF(0, 0), QPointF(100, 0));
    QVERIFY(lineModel.selectAt(QPointF(50, 0)));
    QVERIFY(lineModel.beginRotationEdit(QPointF(50, 0)));
    QVERIFY(lineModel.rotateSelectionTo(90.0));
    lineModel.endSelectionEdit();
    const QVector<QPointF> linePoints = lineModel.currentPage().elements().first().points();
    QVERIFY(QLineF(linePoints.at(0), QPointF(50, -50)).length() < 0.001);
    QVERIFY(QLineF(linePoints.at(1), QPointF(50, 50)).length() < 0.001);
    QVERIFY(lineModel.resetSelectedRotation());
    QCOMPARE(lineModel.currentPage().elements().first().rotationDegrees(), 0.0);

    BoardModel groupModel;
    groupModel.addRectangle(QRectF(0, 0, 20, 20));
    groupModel.addEllipse(QRectF(80, 0, 20, 20));
    QVERIFY(groupModel.selectInRect(QRectF(-5, -5, 110, 30)));
    QVERIFY(groupModel.groupSelectedElements());
    const QPointF groupCenter = groupModel.selectedElementBounds().center();
    QVERIFY(groupModel.beginRotationEdit(groupCenter));
    QVERIFY(groupModel.rotateSelectionTo(90.0));
    groupModel.endSelectionEdit();
    compareRect(groupModel.currentPage().elements().at(0).bounds(), QRectF(40, -40, 20, 20));
    compareRect(groupModel.currentPage().elements().at(1).bounds(), QRectF(40, 40, 20, 20));
    QCOMPARE(groupModel.currentPage().elements().at(0).rotationDegrees(), 90.0);
    QCOMPARE(groupModel.currentPage().elements().at(1).rotationDegrees(), 90.0);

    QVERIFY(groupModel.setCurrentPageLocked(true));
    QVERIFY(!groupModel.beginRotationEdit(groupCenter));
}

void BoardModelSelectionTest::moveSelectedElementOrder()
{
    BoardModel model;
    model.setSelectedColor(QColor(QStringLiteral("#aa0000")));
    model.addRectangle(QRectF(0, 0, 40, 40));
    model.setSelectedColor(QColor(QStringLiteral("#00aa00")));
    model.addEllipse(QRectF(60, 0, 40, 40));
    model.setSelectedColor(QColor(QStringLiteral("#0000aa")));
    model.addCircle(QRectF(120, 0, 40, 40));

    QVERIFY(model.selectAt(QPointF(80, 20)));
    QCOMPARE(model.selectedElementIndex(), 1);
    QVERIFY(model.moveSelectedForward());
    QCOMPARE(model.selectedElementIndex(), 2);
    QCOMPARE(model.currentPage().elements().at(2).type(), ElementType::Ellipse);
    QVERIFY(!model.moveSelectedForward());

    QVERIFY(model.moveSelectedBackward());
    QCOMPARE(model.selectedElementIndex(), 1);
    QCOMPARE(model.currentPage().elements().at(1).type(), ElementType::Ellipse);
    QVERIFY(model.moveSelectedBackward());
    QCOMPARE(model.selectedElementIndex(), 0);
    QCOMPARE(model.currentPage().elements().first().type(), ElementType::Ellipse);

    QVERIFY(model.undo());
    QCOMPARE(model.currentPage().elements().at(1).type(), ElementType::Ellipse);
    QVERIFY(model.redo());
    QCOMPARE(model.currentPage().elements().first().type(), ElementType::Ellipse);

    model.clearSelection();
    QVERIFY(model.selectInRect(QRectF(-10, -10, 110, 60)));
    QCOMPARE(model.selectedElementIndexes(), QVector<int>({0, 1}));
    QVERIFY(model.moveSelectedForward());
    QCOMPARE(model.currentPage().elements().at(1).type(), ElementType::Ellipse);
    QCOMPARE(model.currentPage().elements().at(2).type(), ElementType::Rectangle);
    QCOMPARE(model.selectedElementIndexes(), QVector<int>({1, 2}));

    BoardModel toFrontBackModel;
    toFrontBackModel.addRectangle(QRectF(0, 0, 40, 40));
    toFrontBackModel.addEllipse(QRectF(60, 0, 40, 40));
    toFrontBackModel.addCircle(QRectF(120, 0, 40, 40));
    toFrontBackModel.addText(QPointF(180, 0), QStringLiteral("top"));

    QVERIFY(toFrontBackModel.selectAt(QPointF(80, 20)));
    QVERIFY(toFrontBackModel.moveSelectedToFront());
    QCOMPARE(toFrontBackModel.selectedElementIndex(), 3);
    QCOMPARE(toFrontBackModel.currentPage().elements().last().type(), ElementType::Ellipse);
    QVERIFY(!toFrontBackModel.moveSelectedToFront());

    QVERIFY(toFrontBackModel.moveSelectedToBack());
    QCOMPARE(toFrontBackModel.selectedElementIndex(), 0);
    QCOMPARE(toFrontBackModel.currentPage().elements().first().type(), ElementType::Ellipse);
    QVERIFY(!toFrontBackModel.moveSelectedToBack());

    QVERIFY(toFrontBackModel.undo());
    QCOMPARE(toFrontBackModel.currentPage().elements().last().type(), ElementType::Ellipse);
    QVERIFY(toFrontBackModel.redo());
    QCOMPARE(toFrontBackModel.currentPage().elements().first().type(), ElementType::Ellipse);

    toFrontBackModel.clearSelection();
    QVERIFY(toFrontBackModel.selectInRect(QRectF(-10, -10, 170, 60)));
    QCOMPARE(toFrontBackModel.selectedElementIndexes(), QVector<int>({0, 1, 2}));
    QVERIFY(toFrontBackModel.moveSelectedToFront());
    QCOMPARE(toFrontBackModel.selectedElementIndexes(), QVector<int>({1, 2, 3}));
    QCOMPARE(toFrontBackModel.currentPage().elements().last().type(), ElementType::Circle);

    BoardModel splitSelectionModel;
    splitSelectionModel.addRectangle(QRectF(0, 0, 40, 40));
    splitSelectionModel.addEllipse(QRectF(60, 0, 40, 40));
    splitSelectionModel.addCircle(QRectF(120, 0, 40, 40));
    splitSelectionModel.addRectangle(QRectF(180, 0, 40, 40));

    QVERIFY(splitSelectionModel.selectAt(QPointF(200, 20)));
    QVERIFY(splitSelectionModel.addToSelectionAt(QPointF(20, 20)));
    QCOMPARE(splitSelectionModel.selectedElementIndexes(), QVector<int>({3, 0}));
    QVERIFY(splitSelectionModel.moveSelectedToFront());
    QCOMPARE(splitSelectionModel.selectedElementIndexes(), QVector<int>({2, 3}));
    QCOMPARE(splitSelectionModel.currentPage().elements().at(2).type(), ElementType::Rectangle);
    QCOMPARE(splitSelectionModel.currentPage().elements().last().type(), ElementType::Rectangle);

    QVERIFY(splitSelectionModel.moveSelectedToBack());
    QCOMPARE(splitSelectionModel.selectedElementIndexes(), QVector<int>({0, 1}));
    QCOMPARE(splitSelectionModel.currentPage().elements().first().type(), ElementType::Rectangle);
}

void BoardModelSelectionTest::deleteSelectedElement()
{
    BoardModel model;
    model.addRectangle(QRectF(10, 20, 100, 80));
    model.addEllipse(QRectF(140, 20, 100, 80));
    QCOMPARE(model.currentPage().elements().size(), 2);

    QVERIFY(model.selectAt(QPointF(20, 30)));
    QVERIFY(model.deleteSelectedElement());
    QVERIFY(!model.hasSelection());
    QCOMPARE(model.currentPage().elements().size(), 1);
    QCOMPARE(model.currentPage().elements().first().type(), ElementType::Ellipse);

    QVERIFY(model.undo());
    QCOMPARE(model.currentPage().elements().size(), 2);
    QVERIFY(model.redo());
    QCOMPARE(model.currentPage().elements().size(), 1);
}

void BoardModelSelectionTest::connectorLineFollowsMovedShapes()
{
    BoardModel model;
    model.addRectangle(QRectF(100, 100, 100, 80));
    model.addEllipse(QRectF(400, 120, 100, 80));
    const QString rectangleId = model.currentPage().elements().at(0).id();
    const QString ellipseId = model.currentPage().elements().at(1).id();

    model.addLine(QPointF(150, 140), QPointF(450, 160));
    QCOMPARE(model.currentPage().elements().size(), 3);
    const DrawingElement connector = model.currentPage().elements().at(2);
    QCOMPARE(connector.startConnectionElementId(), rectangleId);
    QCOMPARE(connector.endConnectionElementId(), ellipseId);
    QVERIFY(qAbs(connector.points().first().x() - 200.0) < 0.001);
    QVERIFY(connector.points().last().x() > 399.0);
    QVERIFY(connector.points().last().x() < 405.0);

    QVERIFY(model.selectAt(QPointF(120, 120)));
    model.beginSelectionEdit();
    QVERIFY(model.moveSelectedBy(QPointF(50, 0)));
    model.endSelectionEdit();

    const DrawingElement movedConnector = model.currentPage().elements().at(2);
    QCOMPARE(movedConnector.startConnectionElementId(), rectangleId);
    QCOMPARE(movedConnector.endConnectionElementId(), ellipseId);
    QVERIFY(movedConnector.points().first().x() > connector.points().first().x());
    QVERIFY(movedConnector.points().last().x() > 399.0);
    QVERIFY(movedConnector.points().last().x() < 405.0);

    QVERIFY(model.selectAt(movedConnector.points().first()));
    QVERIFY(model.moveSelectedVertexTo(0, QPointF(260, 50)));
    const DrawingElement detachedConnector = model.currentPage().elements().at(2);
    QVERIFY(detachedConnector.startConnectionElementId().isEmpty());
    QCOMPARE(detachedConnector.endConnectionElementId(), ellipseId);
}

void BoardModelSelectionTest::reconnectDetachedLineEndpoint()
{
    BoardModel model;
    model.addRectangle(QRectF(100, 100, 100, 80));
    model.addEllipse(QRectF(400, 120, 100, 80));
    model.addRoundedRectangle(QRectF(650, 100, 120, 90));
    const QString rectangleId = model.currentPage().elements().at(0).id();
    const QString ellipseId = model.currentPage().elements().at(1).id();
    const QString roundedId = model.currentPage().elements().at(2).id();

    model.addLine(QPointF(150, 140), QPointF(450, 160));
    QVERIFY(model.selectAt(QPointF(300, 150)));
    QCOMPARE(model.currentPage().elements().at(3).startConnectionElementId(), rectangleId);
    QCOMPARE(model.currentPage().elements().at(3).endConnectionElementId(), ellipseId);

    model.beginSelectionEdit();
    QVERIFY(model.moveSelectedVertexTo(1, QPointF(710, 145)));
    QVERIFY(model.reconnectSelectedLineEndpoint(1, QPointF(710, 145), 20.0));
    model.endSelectionEdit();

    const DrawingElement reconnected = model.currentPage().elements().at(3);
    QCOMPARE(reconnected.startConnectionElementId(), rectangleId);
    QCOMPARE(reconnected.endConnectionElementId(), roundedId);
    QVERIFY(reconnected.points().last().x() >= 649.0);
    QVERIFY(reconnected.points().last().x() <= 651.0);

    QVERIFY(model.undo());
    const DrawingElement restored = model.currentPage().elements().at(3);
    QCOMPARE(restored.startConnectionElementId(), rectangleId);
    QCOMPARE(restored.endConnectionElementId(), ellipseId);
}

void BoardModelSelectionTest::copyPasteRemapsConnectorTargets()
{
    BoardModel model;
    model.addRectangle(QRectF(100, 100, 100, 80));
    model.addEllipse(QRectF(400, 120, 100, 80));
    model.addLine(QPointF(150, 140), QPointF(450, 160));

    QVERIFY(model.selectInRect(QRectF(90, 90, 430, 130)));
    QCOMPARE(model.selectedElementIndexes(), QVector<int>({0, 1, 2}));
    const QVector<DrawingElement> copied = model.copySelectedElements();
    QVERIFY(model.pasteElements(copied));
    QCOMPARE(model.currentPage().elements().size(), 6);

    const DrawingElement pastedStart = model.currentPage().elements().at(3);
    const DrawingElement pastedEnd = model.currentPage().elements().at(4);
    const DrawingElement pastedConnector = model.currentPage().elements().at(5);
    QCOMPARE(pastedConnector.startConnectionElementId(), pastedStart.id());
    QCOMPARE(pastedConnector.endConnectionElementId(), pastedEnd.id());
    QVERIFY(pastedConnector.startConnectionElementId() != model.currentPage().elements().at(0).id());
    QVERIFY(pastedConnector.endConnectionElementId() != model.currentPage().elements().at(1).id());
}

QTEST_MAIN(BoardModelSelectionTest)

#include "tst_boardmodel_selection.moc"
