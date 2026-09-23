#include "CanvasWidget.h"

#include <QCoreApplication>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QImage>
#include <QKeyEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QPlainTextEdit>
#include <QTemporaryDir>
#include <QTest>
#include <QUrl>

class CanvasWidgetSelectionTest : public QObject {
    Q_OBJECT

private slots:
    void moveSelectionWithArrowKeys();
    void moveSelectedPointWithArrowKeys();
    void groupSelectionWithShortcut();
    void rotateSelectionWithHandle();
    void snapRotationWithShiftHeldBeforeDrag();
    void snapRotationWhenShiftPressedDuringDrag();
    void editPolylinePointWithShiftClick();
    void editBezierPointWithShiftClick();
    void dropImageFileAddsImageAtDropPosition();
    void dropImageFileIsRejectedWhenPageLocked();
};

void CanvasWidgetSelectionTest::rotateSelectionWithHandle()
{
    BoardModel model;
    model.addRectangle(QRectF(120, 120, 100, 50));
    model.selectTool(Tool::Select);

    CanvasWidget canvas;
    canvas.resize(320, 240);
    canvas.setModel(&model);

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(140, 140));
    QVERIFY(model.hasSelection());

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(170, 88));
    QTest::mouseMove(&canvas, QPoint(230, 145));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(230, 145));
    QVERIFY(qAbs(model.currentPage().elements().first().rotationDegrees() - 90.0) < 1.0);

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(195, 95));
    QTest::mouseMove(&canvas, QPoint(215, 75));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(215, 75));
    const QRectF resized = model.currentPage().elements().first().bounds();
    QVERIFY(qAbs(resized.width() - 120.0) < 0.01);
    QVERIFY(qAbs(resized.height() - 70.0) < 0.01);

    QVERIFY(model.undo());
    QVERIFY(qAbs(model.currentPage().elements().first().rotationDegrees() - 90.0) < 1.0);
    QVERIFY(model.undo());
    QCOMPARE(model.currentPage().elements().first().rotationDegrees(), 0.0);
}

void CanvasWidgetSelectionTest::snapRotationWithShiftHeldBeforeDrag()
{
    BoardModel model;
    model.addRectangle(QRectF(120, 120, 100, 50));
    model.selectTool(Tool::Select);

    CanvasWidget canvas;
    canvas.resize(320, 240);
    canvas.setModel(&model);

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(140, 140));
    QTest::keyPress(&canvas, Qt::Key_Shift);
    QTest::mousePress(&canvas, Qt::LeftButton, Qt::ShiftModifier, QPoint(170, 88));
    QVERIFY(model.hasSelection());
    QTest::mouseMove(&canvas, QPoint(220, 115));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(220, 115));
    QTest::keyRelease(&canvas, Qt::Key_Shift);

    QCOMPARE(model.currentPage().elements().first().rotationDegrees(), 60.0);
}

void CanvasWidgetSelectionTest::snapRotationWhenShiftPressedDuringDrag()
{
    BoardModel model;
    model.addRectangle(QRectF(120, 120, 100, 50));
    model.selectTool(Tool::Select);

    CanvasWidget canvas;
    canvas.resize(320, 240);
    canvas.setModel(&model);

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(140, 140));
    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(170, 88));
    QTest::mouseMove(&canvas, QPoint(218, 114));
    QTest::keyPress(&canvas, Qt::Key_Shift);
    QTest::mouseMove(&canvas, QPoint(220, 115));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(220, 115));
    QTest::keyRelease(&canvas, Qt::Key_Shift);

    QCOMPARE(model.currentPage().elements().first().rotationDegrees(), 60.0);
}

void CanvasWidgetSelectionTest::moveSelectionWithArrowKeys()
{
    BoardModel model;
    model.addRectangle(QRectF(10, 20, 100, 80));
    model.selectTool(Tool::Select);

    CanvasWidget canvas;
    canvas.setModel(&model);

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 30));
    QVERIFY(model.hasSelection());

    QTest::keyClick(&canvas, Qt::Key_Right);
    QCOMPARE(model.currentPage().elements().first().bounds(), QRectF(11, 20, 100, 80));

    QTest::keyClick(&canvas, Qt::Key_Down);
    QCOMPARE(model.currentPage().elements().first().bounds(), QRectF(11, 21, 100, 80));

    QVERIFY(model.undo());
    QCOMPARE(model.currentPage().elements().first().bounds(), QRectF(11, 20, 100, 80));
}

void CanvasWidgetSelectionTest::moveSelectedPointWithArrowKeys()
{
    BoardModel model;
    model.addPolyline({QPointF(20, 30), QPointF(80, 30), QPointF(120, 80)});
    model.selectTool(Tool::Select);

    CanvasWidget canvas;
    canvas.resize(160, 120);
    canvas.setModel(&model);

    QVERIFY(model.selectAt(QPointF(50, 30)));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(80, 30));
    QCOMPARE(model.currentPage().elements().first().points().at(1), QPointF(80, 30));

    QImage image(canvas.size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);
    canvas.render(&image);
    const QColor selectedPointColor = image.pixelColor(80, 30);
    QVERIFY(selectedPointColor.red() < 80);
    QVERIFY(selectedPointColor.green() < 80);
    QVERIFY(selectedPointColor.blue() < 80);

    QTest::keyClick(&canvas, Qt::Key_Right);
    QCOMPARE(model.currentPage().elements().first().points().at(1), QPointF(81, 30));
    QCOMPARE(model.currentPage().elements().first().points().at(0), QPointF(20, 30));

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(81, 30));
    QTest::mouseMove(&canvas, QPoint(82, 31));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(82, 31));
    QCOMPARE(model.currentPage().elements().first().points().at(1), QPointF(81, 30));

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(81, 30));
    QTest::mouseMove(&canvas, QPoint(90, 40));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(90, 40));
    QCOMPARE(model.currentPage().elements().first().points().at(1), QPointF(90, 40));
}

void CanvasWidgetSelectionTest::dropImageFileAddsImageAtDropPosition()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString filePath = directory.filePath(QStringLiteral("sample.png"));
    QImage source(60, 40, QImage::Format_RGB32);
    source.fill(QColor(QStringLiteral("#22c55e")));
    QVERIFY(source.save(filePath));

    BoardModel model;
    CanvasWidget canvas;
    canvas.setModel(&model);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QMimeData mimeData;
    mimeData.setUrls({QUrl::fromLocalFile(filePath)});
    QDragEnterEvent dragEnterEvent(QPoint(200, 150), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&canvas, &dragEnterEvent);
    QVERIFY(dragEnterEvent.isAccepted());
    QDropEvent dropEvent(QPointF(200, 150), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&canvas, &dropEvent);

    QCOMPARE(model.currentPage().elements().size(), 1);
    const DrawingElement element = model.currentPage().elements().first();
    QCOMPARE(element.type(), ElementType::Image);
    QCOMPARE(element.rect(), QRectF(170, 130, 60, 40));
    QVERIFY(dropEvent.isAccepted());
}

void CanvasWidgetSelectionTest::dropImageFileIsRejectedWhenPageLocked()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString filePath = directory.filePath(QStringLiteral("sample.png"));
    QImage source(60, 40, QImage::Format_RGB32);
    source.fill(QColor(QStringLiteral("#22c55e")));
    QVERIFY(source.save(filePath));

    BoardModel model;
    QVERIFY(model.setCurrentPageLocked(true));
    CanvasWidget canvas;
    canvas.setModel(&model);
    canvas.show();
    QVERIFY(QTest::qWaitForWindowExposed(&canvas));

    QMimeData mimeData;
    mimeData.setUrls({QUrl::fromLocalFile(filePath)});
    QDragEnterEvent dragEnterEvent(QPoint(200, 150), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&canvas, &dragEnterEvent);
    QVERIFY(!dragEnterEvent.isAccepted());
    QDropEvent dropEvent(QPointF(200, 150), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&canvas, &dropEvent);

    QVERIFY(model.currentPage().elements().isEmpty());
    QVERIFY(!dropEvent.isAccepted());
}

void CanvasWidgetSelectionTest::groupSelectionWithShortcut()
{
    BoardModel model;
    model.addRectangle(QRectF(10, 20, 40, 40));
    model.addEllipse(QRectF(80, 20, 40, 40));
    model.selectTool(Tool::Select);

    CanvasWidget canvas;
    canvas.resize(160, 100);
    canvas.setModel(&model);
    canvas.setFocus();

    QVERIFY(model.selectInRect(QRectF(0, 0, 140, 80)));
    QTest::keyClick(&canvas, Qt::Key_G, Qt::ControlModifier);

    model.clearSelection();
    QVERIFY(model.selectAt(QPointF(20, 30)));
    QCOMPARE(model.selectedElementIndexes(), QVector<int>({0, 1}));

    QTest::keyClick(&canvas, Qt::Key_G, Qt::ControlModifier | Qt::ShiftModifier);

    model.clearSelection();
    QVERIFY(model.selectAt(QPointF(20, 30)));
    QCOMPARE(model.selectedElementIndexes(), QVector<int>({0}));
}

void CanvasWidgetSelectionTest::editPolylinePointWithShiftClick()
{
    BoardModel model;
    model.addPolyline({QPointF(20, 20), QPointF(120, 20), QPointF(120, 80)});
    model.selectTool(Tool::Select);

    CanvasWidget canvas;
    canvas.resize(180, 120);
    canvas.setModel(&model);

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::ShiftModifier, QPoint(70, 24));
    QCOMPARE(model.currentPage().elements().first().points().size(), 4);

    model.undo();
    model.clearSelection();
    QVERIFY(model.selectAt(QPointF(80, 20)));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::ShiftModifier, QPoint(70, 24));
    QCOMPARE(model.currentPage().elements().first().points().size(), 4);

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::ShiftModifier, QPoint(70, 24));
    QCOMPARE(model.currentPage().elements().first().points().size(), 3);
}

void CanvasWidgetSelectionTest::editBezierPointWithShiftClick()
{
    BoardModel model;
    model.addBezier({QPointF(20, 20), QPointF(120, 20), QPointF(120, 80), QPointF(160, 80)});
    model.selectTool(Tool::Select);

    CanvasWidget canvas;
    canvas.resize(200, 120);
    canvas.setModel(&model);

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::ShiftModifier, QPoint(95, 45));
    QCOMPARE(model.currentPage().elements().first().points().size(), 5);

    model.undo();
    model.clearSelection();
    QVERIFY(model.selectAt(QPointF(95, 45)));
    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::ShiftModifier, QPoint(70, 24));
    QCOMPARE(model.currentPage().elements().first().points().size(), 5);

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::ShiftModifier, QPoint(70, 24));
    QCOMPARE(model.currentPage().elements().first().points().size(), 4);
}

QTEST_MAIN(CanvasWidgetSelectionTest)

#include "tst_canvaswidget_selection.moc"
