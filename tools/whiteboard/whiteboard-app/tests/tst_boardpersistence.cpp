#include "BoardPersistence.h"
#include "BoardDocumentStorage.h"
#include "BoardModel.h"

#include <QFile>
#include <QTemporaryDir>
#include <QTest>

class BoardPersistenceTest : public QObject {
    Q_OBJECT
private slots:
    void missingFileAndRestoreProtection();
    void failedSaveStopsAutomaticAttemptsAndRetriesLatestState();
    void settingsAndPendingInputAreUnsavedWithoutRevisionChange();
    void reloadIsAtomicAndNeverDiscardsUsableDocument();
    void firstDirectoryFailureCanRetry();
    void protectsOutputAliases();
};

void BoardPersistenceTest::missingFileAndRestoreProtection()
{
    QTemporaryDir directory;
    const QString path = directory.filePath("board.json");
    BoardModel model;
    BoardPersistence initial(path);
    QVERIFY(initial.restore(model));
    QCOMPARE(initial.state(), BoardPersistence::State::Ready);
    QVERIFY(!initial.status(model.document())["persisted"].toBool());
    QVERIFY(initial.save(model.document()));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("{broken");
    file.close();
    BoardPersistence blocked(path);
    QVERIFY(!blocked.restore(model));
    QCOMPARE(blocked.state(), BoardPersistence::State::RestoreBlocked);
    QVERIFY(!blocked.save(model.document(), true));
    QVERIFY(QFile::remove(path));
    QVERIFY(!blocked.restore(model, true));
    QCOMPARE(blocked.state(), BoardPersistence::State::RestoreBlocked);
    QVERIFY(!QFile::exists(path));
}

void BoardPersistenceTest::failedSaveStopsAutomaticAttemptsAndRetriesLatestState()
{
    QTemporaryDir directory;
    const QString path = directory.filePath("board.json");
    BoardModel model;
    QVERIFY(BoardDocumentStorage::write(path, model.document()).ok());
    int attempts = 0;
    bool fail = true;
    BoardPersistence persistence(path, {}, [&](const QString &target, const BoardDocument &doc) {
        ++attempts;
        if (fail) return BoardFileResult{BoardFileResult::Status::Failure, "commit", "Injected commit failure"};
        return BoardDocumentStorage::write(target, doc);
    });
    QVERIFY(persistence.restore(model));
    model.addRectangle(QRectF(10, 20, 30, 40));
    const auto first = model.document().toJson();
    QVERIFY(!persistence.save(model.document()));
    QCOMPARE(model.document().toJson(), first);
    QVERIFY(model.canUndo());
    model.addText(QPointF(40, 50), "after failure");
    model.setStrokeWidth(8);
    for (int i = 0; i < 4; ++i) QVERIFY(!persistence.save(model.document()));
    QCOMPARE(attempts, 1);
    QCOMPARE(persistence.state(), BoardPersistence::State::SaveFailed);
    fail = false;
    const auto latest = model.document().toJson();
    QVERIFY(persistence.save(model.document(), true));
    QCOMPARE(attempts, 2);
    QCOMPARE(persistence.state(), BoardPersistence::State::Ready);
    QVERIFY(persistence.status(model.document())["persisted"].toBool());
    BoardDocument loaded;
    QVERIFY(BoardDocumentStorage::read(path, &loaded).ok());
    QCOMPARE(loaded.toJson(), latest);
    QVERIFY(model.undo());
    QCOMPARE(model.currentPage().elements().size(), 1);
}

void BoardPersistenceTest::settingsAndPendingInputAreUnsavedWithoutRevisionChange()
{
    QTemporaryDir directory;
    BoardModel model;
    BoardPersistence persistence(directory.filePath("board.json"));
    QVERIFY(persistence.restore(model));
    QVERIFY(persistence.save(model.document()));
    const auto revision = model.document().revision();
    model.setStrokeWidth(7);
    QCOMPARE(model.document().revision(), revision);
    QVERIFY(!persistence.status(model.document())["persisted"].toBool());
    QVERIFY(persistence.save(model.document()));
    model.setCurrentPageScrollPosition(QPoint(10, 20));
    QVERIFY(!persistence.status(model.document())["persisted"].toBool());
    QVERIFY(persistence.save(model.document()));
    model.setWindowSize(QSize(1000, 700));
    QVERIFY(!persistence.status(model.document())["persisted"].toBool());
    QVERIFY(persistence.save(model.document()));
    QVERIFY(!persistence.status(model.document(), true)["persisted"].toBool());
    QVERIFY(persistence.status(model.document())["persisted"].toBool());
}

void BoardPersistenceTest::reloadIsAtomicAndNeverDiscardsUsableDocument()
{
    QTemporaryDir directory;
    const QString path = directory.filePath("board.json");
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("{}");
    file.close();
    BoardModel model;
    model.addRectangle(QRectF(1, 2, 3, 4));
    const auto before = model.document().toJson();
    BoardPersistence persistence(path);
    QVERIFY(!persistence.restore(model));
    QCOMPARE(model.document().toJson(), before);
    QVERIFY(!persistence.restore(model, true));
    QCOMPARE(model.document().toJson(), before);
    BoardModel repaired;
    repaired.addText(QPointF(100, 100), "restored");
    QVERIFY(BoardDocumentStorage::write(path, repaired.document()).ok());
    QVERIFY(persistence.restore(model, true));
    QCOMPARE(model.document().toJson(), repaired.document().toJson());
    QVERIFY(!model.canUndo());
    model.addRectangle(QRectF(10, 20, 30, 40));
    const auto edited = model.document().toJson();
    QVERIFY(!persistence.restore(model, true));
    QVERIFY(!persistence.restore(model));
    QCOMPARE(model.document().toJson(), edited);
}

void BoardPersistenceTest::firstDirectoryFailureCanRetry()
{
    QTemporaryDir directory;
    const QString parent = directory.filePath("parent");
    QFile obstacle(parent);
    QVERIFY(obstacle.open(QIODevice::WriteOnly));
    obstacle.close();
    // Simulate verified absence followed by a race that prevents mkdir.
    BoardPersistence persistence(parent + "/board.json", [](const QString &, BoardDocument *) {
        return BoardFileResult{BoardFileResult::Status::NotFound, "open", "missing"};
    });
    BoardModel model;
    QVERIFY(!persistence.restore(model));
    QCOMPARE(persistence.state(), BoardPersistence::State::SaveFailed);
    QVERIFY(persistence.documentAvailable());
    QVERIFY(QFile::remove(parent));
    model.addText(QPointF(10, 20), "new document");
    QVERIFY(persistence.save(model.document(), true));
    QCOMPARE(persistence.state(), BoardPersistence::State::Ready);
}

void BoardPersistenceTest::protectsOutputAliases()
{
    QTemporaryDir directory;
    const QString path = directory.filePath("board.json");
    BoardModel model;
    BoardPersistence persistence(path);
    QVERIFY(persistence.restore(model));
    QVERIFY(persistence.save(model.document()));
    QString error;
    QVERIFY(!persistence.allowsOutput(path, &error));
    QVERIFY(!error.isEmpty());
    const QString alias = directory.filePath("export.png");
    QVERIFY(QFile::link(path, alias));
    QVERIFY(!persistence.allowsOutput(alias, &error));
    QVERIFY(persistence.allowsOutput(directory.filePath("rescue.whiteboard"), &error));
}

QTEST_MAIN(BoardPersistenceTest)
#include "tst_boardpersistence.moc"
