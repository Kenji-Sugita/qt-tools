#include "CanvasWidget.h"

#include <QCoreApplication>
#include <QImage>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPlainTextEdit>
#include <QTest>

class CanvasWidgetShapeTextTest : public QObject {
    Q_OBJECT

private slots:
    void drawRoundedRectangleWithDrag();
    void commitTextWithControlReturn();
    void commitTextViewportWithControlReturn();
    void commitTextWithMetaReturn();
    void drawTextWhenStrokeWidthIsZero();
    void selectedRectangleShowsOuterSelectionFrame();
    void pendingTextEditNotifiesStateChanges();
};

void CanvasWidgetShapeTextTest::drawRoundedRectangleWithDrag()
{
    BoardModel model;
    model.selectTool(Tool::RoundedRectangle);
    model.setCornerRadius(32);

    CanvasWidget canvas;
    canvas.setModel(&model);

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QMouseEvent moveEvent(QEvent::MouseMove, QPointF(140, 90), QPointF(140, 90), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&canvas, &moveEvent);
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(140, 90));

    QCOMPARE(model.currentPage().elements().size(), 1);
    const DrawingElement element = model.currentPage().elements().first();
    QCOMPARE(element.type(), ElementType::RoundedRectangle);
    QCOMPARE(element.rect(), QRectF(20, 20, 120, 70));
    QCOMPARE(element.cornerRadius(), 32);

    model.clearSelection();
    model.selectTool(Tool::RoundedRectangle);
    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(180, 30));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(260, 100));
    QCOMPARE(model.currentPage().elements().size(), 2);
    QCOMPARE(model.currentPage().elements().last().type(), ElementType::RoundedRectangle);
    QCOMPARE(model.currentPage().elements().last().rect(), QRectF(180, 30, 80, 70));
}

void CanvasWidgetShapeTextTest::commitTextWithControlReturn()
{
    BoardModel model;
    model.selectTool(Tool::Text);

    CanvasWidget canvas;
    canvas.setModel(&model);

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(40, 50));
    auto *editor = canvas.findChild<QPlainTextEdit *>();
    QVERIFY(editor);

    editor->setPlainText(QStringLiteral("aaa"));
    editor->setFocus();
    QTest::keyClick(editor, Qt::Key_Return, Qt::ControlModifier);
    QCoreApplication::processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

    QCOMPARE(model.currentPage().elements().size(), 1);
    QCOMPARE(model.currentPage().elements().first().text(), QStringLiteral("aaa"));
    QVERIFY(!canvas.findChild<QPlainTextEdit *>());
}

void CanvasWidgetShapeTextTest::commitTextViewportWithControlReturn()
{
    BoardModel model;
    model.selectTool(Tool::Text);

    CanvasWidget canvas;
    canvas.setModel(&model);

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(40, 50));
    auto *editor = canvas.findChild<QPlainTextEdit *>();
    QVERIFY(editor);

    editor->setPlainText(QStringLiteral("aaa"));
    editor->viewport()->setFocus();
    QTest::keyClick(editor->viewport(), Qt::Key_Return, Qt::ControlModifier);
    QCoreApplication::processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

    QCOMPARE(model.currentPage().elements().size(), 1);
    QCOMPARE(model.currentPage().elements().first().text(), QStringLiteral("aaa"));
    QVERIFY(!canvas.findChild<QPlainTextEdit *>());
}

void CanvasWidgetShapeTextTest::commitTextWithMetaReturn()
{
    BoardModel model;
    model.selectTool(Tool::Text);

    CanvasWidget canvas;
    canvas.setModel(&model);

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(40, 50));
    auto *editor = canvas.findChild<QPlainTextEdit *>();
    QVERIFY(editor);

    editor->setPlainText(QStringLiteral("aaa"));
    QKeyEvent shortcutOverride(QEvent::ShortcutOverride, Qt::Key_Return, Qt::MetaModifier);
    QCoreApplication::sendEvent(editor->viewport(), &shortcutOverride);
    QCoreApplication::processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

    QCOMPARE(model.currentPage().elements().size(), 1);
    QCOMPARE(model.currentPage().elements().first().text(), QStringLiteral("aaa"));
    QVERIFY(!canvas.findChild<QPlainTextEdit *>());
}

void CanvasWidgetShapeTextTest::drawTextWhenStrokeWidthIsZero()
{
    BoardModel model;
    model.setStrokeWidth(0);
    model.addText(QPointF(20, 30), QStringLiteral("aaa"));

    CanvasWidget canvas;
    canvas.resize(240, 120);
    canvas.setModel(&model);

    QImage image(canvas.size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);
    canvas.render(&image);

    int textPixels = 0;
    const QRect textArea(15, 20, 100, 60);
    for (int y = textArea.top(); y <= textArea.bottom(); ++y) {
        for (int x = textArea.left(); x <= textArea.right(); ++x) {
            const QColor pixel = image.pixelColor(x, y);
            if (pixel.alpha() > 0 && qGray(pixel.rgb()) < 245)
                ++textPixels;
        }
    }

    QVERIFY(textPixels > 0);
}

void CanvasWidgetShapeTextTest::selectedRectangleShowsOuterSelectionFrame()
{
    BoardModel model;
    model.addRectangle(QRectF(20, 30, 120, 80));
    model.selectTool(Tool::Select);

    CanvasWidget canvas;
    canvas.resize(220, 160);
    canvas.setModel(&model);

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(40, 50));
    QVERIFY(model.hasSelection());

    QImage image(canvas.size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);
    canvas.render(&image);

    int outerSelectionPixels = 0;
    const QRect outsideElementArea(10, 20, 140, 100);
    const QRect elementBounds(20, 30, 120, 80);
    for (int y = outsideElementArea.top(); y <= outsideElementArea.bottom(); ++y) {
        for (int x = outsideElementArea.left(); x <= outsideElementArea.right(); ++x) {
            if (elementBounds.adjusted(-1, -1, 1, 1).contains(x, y))
                continue;
            const QColor pixel = image.pixelColor(x, y);
            if (pixel.green() > 90 && pixel.red() < 80 && pixel.blue() < 120)
                ++outerSelectionPixels;
        }
    }

    QVERIFY(outerSelectionPixels > 0);
}

void CanvasWidgetShapeTextTest::pendingTextEditNotifiesStateChanges()
{
    BoardModel model;
    model.selectTool(Tool::Text);

    CanvasWidget canvas;
    canvas.setModel(&model);
    int notifications = 0;
    canvas.setChangeCallback([&notifications]() { ++notifications; });

    QTest::mouseClick(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(40, 50));
    QVERIFY(canvas.hasPendingTextEdit());
    QCOMPARE(notifications, 1);

    auto *editor = canvas.findChild<QPlainTextEdit *>();
    QVERIFY(editor);
    QTest::keyClick(editor, Qt::Key_Escape);
    QCoreApplication::processEvents();
    QVERIFY(!canvas.hasPendingTextEdit());
    QCOMPARE(notifications, 2);
}

QTEST_MAIN(CanvasWidgetShapeTextTest)

#include "tst_canvaswidget_shape_text.moc"
