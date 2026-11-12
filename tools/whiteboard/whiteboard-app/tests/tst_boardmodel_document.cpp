#include "BoardModel.h"

#include <QTemporaryDir>
#include <QTest>

class BoardModelDocumentTest : public QObject {
    Q_OBJECT

private slots:
    void undoRedoEmptyHistory();
    void zoomClamps();
    void pageLimits();
    void pageLockPreventsEditingButAllowsCopy();
    void saveIncludesFontSettings();
    void saveIncludesPageLock();
    void saveIncludesRotationAndLoadsMissingRotationAsZero();
    void assignsPersistentElementIds();
    void doesNotReuseElementIdsAfterUndo();
    void doesNotReuseMcpElementIdsAfterUndo();
    void updatesAndDeletesElementsByPersistentId();
    void revisionPersistsAndAdvancesAcrossHistory();
    void loadsLegacyFilesWithGeneratedElementIds();
    void applyElementsToPageIsOneUndoableChange();
    void applyElementsToPageRemapsConnectorIds();
    void applyElementsToPageHonorsLockAndPageLimit();
};

void BoardModelDocumentTest::applyElementsToPageIsOneUndoableChange()
{
    BoardModel model;
    model.addRectangle(QRectF(10, 10, 40, 30));
    const QString existingId = model.currentPage().elements().first().id();

    DrawingElement line;
    line.setType(ElementType::Line);
    line.setId(QStringLiteral("external-line"));
    line.setPoints({QPointF(100, 100), QPointF(200, 100)});
    DrawingElement text;
    text.setType(ElementType::Text);
    text.setId(existingId);
    text.setText(QStringLiteral("Generated"));
    text.setRect(QRectF(120, 120, 100, 30));

    QVERIFY(model.applyElementsToPage(
        {line, text}, BoardModel::PageElementApplyMode::AppendCurrent));
    QCOMPARE(model.currentPage().elements().size(), 3);
    QVERIFY(!model.currentPage().elements().at(1).id().isEmpty());
    QVERIFY(!model.currentPage().elements().at(2).id().isEmpty());
    QVERIFY(model.currentPage().elements().at(1).id() != QStringLiteral("external-line"));
    QVERIFY(model.currentPage().elements().at(2).id() != existingId);
    QVERIFY(model.currentPage().elements().at(1).id() != model.currentPage().elements().at(2).id());
    QVERIFY(model.undo());
    QCOMPARE(model.currentPage().elements().size(), 1);

    QVERIFY(model.applyElementsToPage(
        {line, text}, BoardModel::PageElementApplyMode::ReplaceCurrent));
    QCOMPARE(model.currentPage().elements().size(), 2);
    QVERIFY(model.currentPage().elements().at(0).id() != QStringLiteral("external-line"));
    QVERIFY(model.currentPage().elements().at(1).id() != existingId);
    QVERIFY(model.undo());
    QCOMPARE(model.currentPage().elements().size(), 1);

    QVERIFY(model.applyElementsToPage(
        {line, text}, BoardModel::PageElementApplyMode::NewPage));
    QCOMPARE(model.document().pages().size(), 2);
    QCOMPARE(model.document().currentPageIndex(), 1);
    QCOMPARE(model.currentPage().elements().size(), 2);
    QVERIFY(!model.currentPage().elements().at(0).id().isEmpty());
    QVERIFY(model.currentPage().elements().at(0).id() != model.currentPage().elements().at(1).id());
    QVERIFY(model.undo());
    QCOMPARE(model.document().pages().size(), 1);
    QCOMPARE(model.currentPage().elements().size(), 1);
}

void BoardModelDocumentTest::applyElementsToPageRemapsConnectorIds()
{
    BoardModel model;
    model.addRectangle(QRectF(10, 10, 40, 30));
    const QString existingId = model.currentPage().elements().first().id();

    DrawingElement appendedLine;
    appendedLine.setType(ElementType::Line);
    appendedLine.setPoints({QPointF(100, 20), QPointF(30, 20)});
    appendedLine.setEndConnectionElementId(existingId);
    QVERIFY(model.applyElementsToPage({appendedLine}, BoardModel::PageElementApplyMode::AppendCurrent));
    QCOMPARE(model.currentPage().elements().at(1).endConnectionElementId(), existingId);

    DrawingElement sourceRectangle;
    sourceRectangle.setType(ElementType::Rectangle);
    sourceRectangle.setId(QStringLiteral("source-rectangle"));
    sourceRectangle.setRect(QRectF(200, 200, 60, 40));
    DrawingElement sourceLine;
    sourceLine.setType(ElementType::Line);
    sourceLine.setId(QStringLiteral("source-line"));
    sourceLine.setPoints({QPointF(260, 220), QPointF(350, 220)});
    sourceLine.setStartConnectionElementId(QStringLiteral("source-rectangle"));

    QVERIFY(model.applyElementsToPage(
        {sourceRectangle, sourceLine}, BoardModel::PageElementApplyMode::NewPage));
    const QVector<DrawingElement> pageElements = model.currentPage().elements();
    QCOMPARE(pageElements.size(), 2);
    QVERIFY(pageElements.at(0).id() != QStringLiteral("source-rectangle"));
    QCOMPARE(pageElements.at(1).startConnectionElementId(), pageElements.at(0).id());
}

void BoardModelDocumentTest::assignsPersistentElementIds()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    BoardModel model;
    model.addRectangle(QRectF(10, 10, 40, 30));
    model.addEllipse(QRectF(80, 10, 40, 30));
    QCOMPARE(model.currentPage().elements().at(0).id(), QStringLiteral("element-1"));
    QCOMPARE(model.currentPage().elements().at(1).id(), QStringLiteral("element-2"));

    QVERIFY(model.selectInRect(QRectF(0, 0, 140, 80)));
    const QVector<DrawingElement> copied = model.copySelectedElements();
    QVERIFY(model.pasteElements(copied));
    QCOMPARE(model.currentPage().elements().at(2).id(), QStringLiteral("element-3"));
    QCOMPARE(model.currentPage().elements().at(3).id(), QStringLiteral("element-4"));

    const QString filePath = directory.filePath(QStringLiteral("ids.json"));
    QVERIFY(model.saveToFile(filePath));

    BoardModel loaded;
    QVERIFY(loaded.loadFromFile(filePath));
    QCOMPARE(loaded.currentPage().elements().at(0).id(), QStringLiteral("element-1"));
    QCOMPARE(loaded.currentPage().elements().at(3).id(), QStringLiteral("element-4"));
    QVERIFY(loaded.document().nextElementId() > 4);
}

void BoardModelDocumentTest::doesNotReuseElementIdsAfterUndo()
{
    BoardModel model;
    model.addRectangle(QRectF(10, 10, 40, 30));
    model.addEllipse(QRectF(80, 10, 40, 30));
    QCOMPARE(model.currentPage().elements().at(1).id(), QStringLiteral("element-2"));

    QVERIFY(model.undo());
    QCOMPARE(model.currentPage().elements().size(), 1);
    model.addCircle(QRectF(140, 10, 30, 30));

    QCOMPARE(model.currentPage().elements().at(1).id(), QStringLiteral("element-3"));
    QVERIFY(!model.canRedo());
}

void BoardModelDocumentTest::doesNotReuseMcpElementIdsAfterUndo()
{
    BoardModel model;
    DrawingElement rectangle;
    rectangle.setType(ElementType::Rectangle);
    rectangle.setRect(QRectF(20, 20, 80, 60));
    DrawingElement ellipse;
    ellipse.setType(ElementType::Ellipse);
    ellipse.setRect(QRectF(140, 20, 80, 60));

    QVERIFY(model.applyElementsToPage(
        {rectangle, ellipse}, BoardModel::PageElementApplyMode::NewPage));
    QCOMPARE(model.currentPage().elements().at(0).id(), QStringLiteral("element-1"));
    QCOMPARE(model.currentPage().elements().at(1).id(), QStringLiteral("element-2"));

    QVERIFY(model.undo());
    QVERIFY(model.applyElementsToPage(
        {rectangle}, BoardModel::PageElementApplyMode::NewPage));

    QCOMPARE(model.currentPage().elements().at(0).id(), QStringLiteral("element-3"));
    QVERIFY(!model.canRedo());
}

void BoardModelDocumentTest::updatesAndDeletesElementsByPersistentId()
{
    BoardModel model;
    model.addRectangle(QRectF(100, 100, 100, 80));
    model.addEllipse(QRectF(400, 120, 100, 80));
    model.addLine(QPointF(150, 140), QPointF(450, 160));
    const QVector<DrawingElement> original = model.currentPage().elements();

    DrawingElement updatedRectangle = original.at(0);
    updatedRectangle.setRect(QRectF(200, 100, 100, 80));
    QVERIFY(model.updateElementsById({updatedRectangle}));
    QCOMPARE(model.currentPage().elements().at(0).id(), original.at(0).id());
    QCOMPARE(model.currentPage().elements().at(0).rect().x(), 200.0);
    QVERIFY(model.currentPage().elements().at(2).points().first() != original.at(2).points().first());
    QVERIFY(model.undo());
    QCOMPARE(model.currentPage().elements().at(0).rect(), original.at(0).rect());

    QStringList detachedConnectorIds;
    QVERIFY(model.deleteElementsById({original.at(0).id()}, &detachedConnectorIds));
    QCOMPARE(detachedConnectorIds, QStringList{original.at(2).id()});
    QCOMPARE(model.currentPage().elements().size(), 2);
    QCOMPARE(model.currentPage().elements().at(1).startConnectionElementId(), QString());
    QVERIFY(model.undo());
    QCOMPARE(model.currentPage().elements().size(), 3);

    DrawingElement missing = original.at(0);
    missing.setId(QStringLiteral("missing-element"));
    QVERIFY(!model.updateElementsById({updatedRectangle, missing}));
    QCOMPARE(model.currentPage().elements().size(), original.size());
    QCOMPARE(model.currentPage().elements().at(0).rect(), original.at(0).rect());
    QVERIFY(!model.deleteElementsById({original.at(0).id(), QStringLiteral("missing-element")}));
    QCOMPARE(model.currentPage().elements().size(), original.size());
    QCOMPARE(model.currentPage().elements().at(0).id(), original.at(0).id());
}

void BoardModelDocumentTest::revisionPersistsAndAdvancesAcrossHistory()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    BoardModel model;
    QCOMPARE(model.document().revision(), 0);
    model.addRectangle(QRectF(10, 10, 40, 30));
    const qint64 createdRevision = model.document().revision();
    QVERIFY(createdRevision > 0);
    QVERIFY(model.undo());
    const qint64 undoRevision = model.document().revision();
    QVERIFY(undoRevision > createdRevision);
    QVERIFY(model.redo());
    QVERIFY(model.document().revision() > undoRevision);

    const QString filePath = directory.filePath(QStringLiteral("revision.json"));
    QVERIFY(model.saveToFile(filePath));
    BoardModel loaded;
    QVERIFY(loaded.loadFromFile(filePath));
    QCOMPARE(loaded.document().revision(), model.document().revision());

    QJsonObject legacyDocument = model.document().toJson();
    legacyDocument.remove(QStringLiteral("revision"));
    QCOMPARE(BoardDocument::fromJson(legacyDocument).revision(), 0);
}

void BoardModelDocumentTest::loadsLegacyFilesWithGeneratedElementIds()
{
    DrawingElement first;
    first.setType(ElementType::Rectangle);
    first.setRect(QRectF(10, 10, 40, 30));
    DrawingElement second = first;

    QJsonObject firstObject = first.toJson();
    QJsonObject secondObject = second.toJson();
    firstObject.remove(QStringLiteral("id"));
    secondObject.insert(QStringLiteral("id"), QStringLiteral("element-1"));

    Page page;
    page.setElements({DrawingElement::fromJson(firstObject), DrawingElement::fromJson(secondObject)});
    BoardDocument document;
    document.setPages({page});
    document.ensureElementIds();

    const QVector<DrawingElement> loadedElements = document.currentPage().elements();
    QCOMPARE(loadedElements.size(), 2);
    QCOMPARE(loadedElements.at(0).id(), QStringLiteral("element-2"));
    QCOMPARE(loadedElements.at(1).id(), QStringLiteral("element-1"));
    QCOMPARE(document.nextElementId(), 3);
}

void BoardModelDocumentTest::applyElementsToPageHonorsLockAndPageLimit()
{
    BoardModel model;
    DrawingElement rectangle;
    rectangle.setType(ElementType::Rectangle);
    rectangle.setRect(QRectF(20, 20, 80, 60));

    QVERIFY(model.setCurrentPageLocked(true));
    QVERIFY(!model.applyElementsToPage(
        {rectangle}, BoardModel::PageElementApplyMode::AppendCurrent));
    QVERIFY(!model.applyElementsToPage(
        {rectangle}, BoardModel::PageElementApplyMode::ReplaceCurrent));
    QVERIFY(model.applyElementsToPage(
        {rectangle}, BoardModel::PageElementApplyMode::NewPage));

    while (model.canAddPage())
        QVERIFY(model.addPage());
    QCOMPARE(model.document().pages().size(), BoardModel::MaxPages);
    QVERIFY(!model.applyElementsToPage(
        {rectangle}, BoardModel::PageElementApplyMode::NewPage));
    QCOMPARE(model.document().pages().size(), BoardModel::MaxPages);
}

void BoardModelDocumentTest::saveIncludesRotationAndLoadsMissingRotationAsZero()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    BoardModel model;
    model.addRoundedRectangle(QRectF(20, 30, 120, 70));
    QVERIFY(model.selectAt(QPointF(30, 40)));
    QVERIFY(model.beginRotationEdit(model.selectedElementBounds().center()));
    QVERIFY(model.rotateSelectionTo(30.0));
    model.endSelectionEdit();

    const QString filePath = directory.filePath(QStringLiteral("rotation.json"));
    QVERIFY(model.saveToFile(filePath));
    BoardModel loaded;
    QVERIFY(loaded.loadFromFile(filePath));
    QCOMPARE(loaded.currentPage().elements().first().rotationDegrees(), 30.0);

    QJsonObject oldElement = loaded.currentPage().elements().first().toJson();
    oldElement.remove(QStringLiteral("rotationDegrees"));
    QCOMPARE(DrawingElement::fromJson(oldElement).rotationDegrees(), 0.0);
}

void BoardModelDocumentTest::undoRedoEmptyHistory()
{
    BoardModel model;
    QVERIFY(!model.undo());
    QVERIFY(!model.redo());

    model.addFreehand({QPointF(1, 1), QPointF(2, 2)});
    QCOMPARE(model.currentPage().elements().size(), 1);
    QVERIFY(model.undo());
    QCOMPARE(model.currentPage().elements().size(), 0);
    QVERIFY(model.redo());
    QCOMPARE(model.currentPage().elements().size(), 1);
}

void BoardModelDocumentTest::zoomClamps()
{
    BoardModel model;
    QCOMPARE(model.setZoomPercent(5), 10);
    QCOMPARE(model.settings().zoomPercent(), 10);
    QCOMPARE(model.setZoomPercent(500), 400);
    QCOMPARE(model.settings().zoomPercent(), 400);
    QCOMPARE(model.setZoomPercent(125), 125);
    QCOMPARE(model.settings().zoomPercent(), 125);
    model.setCurrentPageScrollPosition(QPoint(120, 240));
    QCOMPARE(model.currentPageScrollPosition(), QPoint(120, 240));

    QVERIFY(model.addPage());
    QCOMPARE(model.settings().zoomPercent(), 100);
    QCOMPARE(model.currentPageScrollPosition(), QPoint(0, 0));
    QCOMPARE(model.setZoomPercent(175), 175);
    QCOMPARE(model.settings().zoomPercent(), 175);
    model.setCurrentPageScrollPosition(QPoint(320, 480));

    model.previousPage();
    QCOMPARE(model.settings().zoomPercent(), 125);
    QCOMPARE(model.currentPageScrollPosition(), QPoint(120, 240));
    model.nextPage();
    QCOMPARE(model.settings().zoomPercent(), 175);
    QCOMPARE(model.currentPageScrollPosition(), QPoint(320, 480));
}

void BoardModelDocumentTest::pageLimits()
{
    BoardModel model;
    QVERIFY(!model.canDeletePage());
    QVERIFY(!model.deleteCurrentPage());

    model.addRectangle(QRectF(10, 10, 20, 20));
    QVERIFY(model.addPage());
    model.addEllipse(QRectF(30, 30, 20, 20));
    model.previousPage();
    QCOMPARE(model.document().currentPageIndex(), 0);

    QVERIFY(model.addPage());
    QCOMPARE(model.document().currentPageIndex(), 1);
    QCOMPARE(model.document().pages().size(), 3);
    QCOMPARE(model.document().pages().at(0).elements().first().type(), ElementType::Rectangle);
    QVERIFY(model.currentPage().elements().isEmpty());
    QCOMPARE(model.document().pages().at(2).elements().first().type(), ElementType::Ellipse);

    for (int i = model.document().pages().size(); i < BoardModel::MaxPages; ++i)
        QVERIFY(model.addPage());

    QCOMPARE(model.document().pages().size(), BoardModel::MaxPages);
    QVERIFY(!model.canAddPage());
    QVERIFY(!model.addPage());
    QCOMPARE(model.document().pages().size(), BoardModel::MaxPages);

    QVERIFY(model.canDeletePage());
    QVERIFY(model.deleteCurrentPage());
    QCOMPARE(model.document().pages().size(), BoardModel::MaxPages - 1);
}

void BoardModelDocumentTest::pageLockPreventsEditingButAllowsCopy()
{
    BoardModel model;
    model.addRectangle(QRectF(10, 10, 20, 20));
    QVERIFY(model.addPage());
    model.previousPage();
    QVERIFY(model.selectAt(QPointF(15, 15)));
    const QVector<DrawingElement> copied = model.copySelectedElements();
    QCOMPARE(copied.size(), 1);

    QVERIFY(model.setCurrentPageLocked(true));
    QVERIFY(model.currentPageLocked());
    QVERIFY(model.hasSelection() == false);
    QCOMPARE(model.currentPage().elements().size(), 1);

    QVERIFY(model.selectAt(QPointF(15, 15)));
    QCOMPARE(model.copySelectedElements().size(), 1);
    model.addEllipse(QRectF(40, 40, 20, 20));
    QCOMPARE(model.currentPage().elements().size(), 1);
    QVERIFY(!model.moveSelectedBy(QPointF(10, 0)));
    QVERIFY(!model.deleteSelectedElement());
    QVERIFY(!model.pasteElements(copied));
    QVERIFY(!model.groupSelectedElements());
    QVERIFY(!model.canUndo());
    QVERIFY(!model.canDeletePage());

    QVERIFY(model.setCurrentPageLocked(false));
    QVERIFY(!model.currentPageLocked());
    QVERIFY(model.canDeletePage());
    QVERIFY(model.pasteElements(copied));
    QCOMPARE(model.currentPage().elements().size(), 2);
}

void BoardModelDocumentTest::saveIncludesFontSettings()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    BoardModel model;
    QFont font(QStringLiteral("Courier"), 22);
    model.setFont(font);
    model.setSelectedColor(QColor(QStringLiteral("#123456")));
    model.setStrokeWidth(0);
    model.setCornerRadius(42);
    model.setZoomPercent(150);
    model.setCurrentPageScrollPosition(QPoint(210, 420));
    model.setWindowSize(QSize(1440, 880));
    model.setFloatingActionDockY(360);
    model.setStrokeStyle(StrokeStyle::Dotted);
    model.setFillColor(QColor(QStringLiteral("#80112233")));
    model.setStartArrowHead(ArrowHead::Open);
    model.setEndArrowHead(ArrowHead::Triangle);
    model.addRoundedRectangle(QRectF(0, 0, 120, 80));
    model.addText(QPointF(12, 34), QStringLiteral("saved"));

    const QString filePath = directory.filePath(QStringLiteral("whiteboard.json"));
    QVERIFY(model.saveToFile(filePath));

    BoardModel loaded;
    QVERIFY(loaded.loadFromFile(filePath));
    QCOMPARE(loaded.settings().font().family(), QStringLiteral("Courier"));
    QCOMPARE(loaded.settings().font().pointSize(), 22);
    QCOMPARE(loaded.settings().strokeWidth(), 0);
    QCOMPARE(loaded.settings().cornerRadius(), 42);
    QCOMPARE(loaded.settings().zoomPercent(), 150);
    QCOMPARE(loaded.currentPage().zoomPercent(), 150);
    QCOMPARE(loaded.currentPageScrollPosition(), QPoint(210, 420));
    QCOMPARE(loaded.settings().windowSize(), QSize(1440, 880));
    QCOMPARE(loaded.settings().floatingActionDockY(), 360);
    QCOMPARE(loaded.settings().strokeStyle(), StrokeStyle::Dotted);
    QCOMPARE(loaded.settings().fillColor(), QColor(QStringLiteral("#80112233")));
    QCOMPARE(loaded.settings().startArrowHead(), ArrowHead::Open);
    QCOMPARE(loaded.settings().endArrowHead(), ArrowHead::Triangle);
    QCOMPARE(loaded.currentPage().elements().size(), 2);
    QCOMPARE(loaded.currentPage().elements().first().type(), ElementType::RoundedRectangle);
    QCOMPARE(loaded.currentPage().elements().first().strokeWidth(), 0);
    QCOMPARE(loaded.currentPage().elements().first().cornerRadius(), 42);
    QCOMPARE(loaded.currentPage().elements().first().fillColor(), QColor(QStringLiteral("#80112233")));
}

void BoardModelDocumentTest::saveIncludesPageLock()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    BoardModel model;
    model.addRectangle(QRectF(10, 10, 20, 20));
    QVERIFY(model.setCurrentPageLocked(true));

    const QString filePath = directory.filePath(QStringLiteral("whiteboard.json"));
    QVERIFY(model.saveToFile(filePath));

    BoardModel loaded;
    QVERIFY(loaded.loadFromFile(filePath));
    QVERIFY(loaded.currentPageLocked());
    QCOMPARE(loaded.currentPage().elements().size(), 1);
}

QTEST_MAIN(BoardModelDocumentTest)

#include "tst_boardmodel_document.moc"
