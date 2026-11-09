#include "MainWindow.h"
#include "BoardDocumentStorage.h"
#include "CanvasWidget.h"
#include "WhiteboardMcpController.h"

#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QResource>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
#include <QTranslator>

class WhiteboardPersistenceUiTest : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void restorationFailureProtectsOriginal_data();
    void restorationFailureProtectsOriginal();
    void successfulReloadEnablesEditing();
    void closeFailureChoices_data();
    void closeFailureChoices();
    void pendingTextAndUiOnlyChangesAreNotReportedSaved();
    void failedSaveNoticeIsQueuedAndNotRepeated();
    void mcpReportsAppliedButUnsavedAndProtectsPendingText();
    void failureNoticeSurvivesNestedApproval();
    void localizedProtectionViews();
};

namespace {
void capture(QWidget &widget, const QString &name)
{
    const QString directory = qEnvironmentVariable("WHITEBOARD_TEST_SCREENSHOT_DIR");
    if (directory.isEmpty()) return;
    QDir().mkpath(directory);
    widget.grab().save(directory + QLatin1Char('/') + name + QStringLiteral(".png"));
}
QByteArray fileBytes(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return {};
    return file.readAll();
}

void clickDialogButton(const QString &text)
{
    for (QWidget *widget : QApplication::topLevelWidgets()) {
        auto *box = qobject_cast<QMessageBox *>(widget);
        if (!box || !box->isVisible()) continue;
        for (auto *button : box->buttons()) {
            if (button->text() == text) { button->click(); return; }
        }
    }
    QFAIL("Expected persistence dialog button was not found");
}
}

void WhiteboardPersistenceUiTest::initTestCase()
{
    const QString resources = qEnvironmentVariable("WHITEBOARD_TEST_RESOURCES");
    if (!resources.isEmpty()) QVERIFY(QResource::registerResource(resources));
}

void WhiteboardPersistenceUiTest::restorationFailureProtectsOriginal_data()
{
    QTest::addColumn<QByteArray>("bytes");
    QTest::newRow("empty") << QByteArray();
    QTest::newRow("truncated") << QByteArray("{\"pages\":[");
    QTest::newRow("array-root") << QByteArray("[]");
    QTest::newRow("empty-object") << QByteArray("{}");
    QTest::newRow("bad-pages") << QByteArray("{\"pages\":42}");
}

void WhiteboardPersistenceUiTest::restorationFailureProtectsOriginal()
{
    QFETCH(QByteArray, bytes);
    QTemporaryDir directory;
    const QString path = directory.filePath("board.json");
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QCOMPARE(file.write(bytes), bytes.size());
    file.close();
    MainWindow window(nullptr, path);
    window.m_persistenceNoticePending = false;
    window.show();
    QCoreApplication::processEvents();
    QCOMPARE(window.m_persistence.state(), BoardPersistence::State::RestoreBlocked);
    QVERIFY(!window.m_scrollArea->isEnabled());
    QVERIFY(!window.m_pageAddAction->isEnabled());
    QVERIFY(window.m_restoreMessage->isVisible());
    capture(window, QStringLiteral("restore-blocked"));
    const auto initial = window.m_model.document().toJson();
    window.performAddPage();
    window.performPaste();
    QVERIFY(!window.navigatePage("next"));
    QVERIFY(!window.saveDocument(true, false));
    QCOMPARE(window.persistenceAccess("whiteboard/state"), QString("restore_blocked"));
    QCOMPARE(window.m_model.document().toJson(), initial);
    QVERIFY(window.close());
    QCOMPARE(fileBytes(path), bytes);
}

void WhiteboardPersistenceUiTest::successfulReloadEnablesEditing()
{
    QTemporaryDir directory;
    const QString path = directory.filePath("board.json");
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("{}");
    file.close();
    MainWindow window(nullptr, path);
    window.m_persistenceNoticePending = false;
    BoardModel repaired;
    repaired.addText(QPointF(20, 30), "repaired");
    QVERIFY(BoardDocumentStorage::write(path, repaired.document()).ok());
    window.reloadSavedData();
    QCOMPARE(window.m_persistence.state(), BoardPersistence::State::Ready);
    QVERIFY(window.m_scrollArea->isEnabled());
    QVERIFY(window.m_pageAddAction->isEnabled());
    QCOMPARE(window.m_model.currentPage().elements().size(), 1);
    window.performAddPage();
    QCOMPARE(window.m_model.document().pages().size(), 2);
    QVERIFY(window.close());
    BoardDocument loaded;
    QVERIFY(BoardDocumentStorage::read(path, &loaded).ok());
    QCOMPARE(loaded.pages().size(), 2);
}

void WhiteboardPersistenceUiTest::closeFailureChoices_data()
{
    QTest::addColumn<QString>("choice");
    QTest::addColumn<bool>("accepted");
    QTest::addColumn<int>("attempts");
    QTest::newRow("cancel") << QString("Cancel Closing") << false << 1;
    QTest::newRow("discard") << QString("Exit Without Saving") << true << 1;
    QTest::newRow("retry") << QString("Retry Saving") << true << 2;
}

void WhiteboardPersistenceUiTest::closeFailureChoices()
{
    QFETCH(QString, choice);
    QFETCH(bool, accepted);
    QFETCH(int, attempts);
    QTemporaryDir directory;
    const QString path = directory.filePath("board.json");
    BoardModel seed;
    QVERIFY(BoardDocumentStorage::write(path, seed.document()).ok());
    const QByteArray original = fileBytes(path);
    MainWindow window(nullptr, path);
    int writes = 0;
    window.m_persistence = BoardPersistence(path, {}, [&](const QString &target, const BoardDocument &doc) {
        ++writes;
        if (writes == 1) return BoardFileResult{BoardFileResult::Status::Failure, "write", "Injected failure"};
        return BoardDocumentStorage::write(target, doc);
    });
    QVERIFY(window.m_persistence.restore(window.m_model));
    window.m_model.addRectangle(QRectF(10, 20, 30, 40));
    QVERIFY(!window.saveDocument(false, false));
    const auto beforeClose = window.m_model.document().toJson();
    window.show();
    QCoreApplication::processEvents();
    capture(window, QStringLiteral("save-stopped"));
    QTimer::singleShot(0, &window, [choice]() {
        for (auto *widget : QApplication::topLevelWidgets())
            if (widget->objectName() == QStringLiteral("persistenceCloseDialog")) capture(*widget, "close-confirmation");
        clickDialogButton(choice);
    });
    QCOMPARE(window.close(), accepted);
    QCOMPARE(writes, attempts);
    QCOMPARE(window.m_model.currentPage().elements().size(), 1);
    QVERIFY(window.m_model.canUndo());
    if (choice != "Retry Saving") {
        QCOMPARE(fileBytes(path), original);
        window.m_exitWithoutSaving = true;
        QVERIFY(window.close());
        QCOMPARE(writes, attempts);
    } else {
        BoardDocument loaded;
        QVERIFY(BoardDocumentStorage::read(path, &loaded).ok());
        QCOMPARE(loaded.currentPage().elements().size(), 1);
    }
}

void WhiteboardPersistenceUiTest::pendingTextAndUiOnlyChangesAreNotReportedSaved()
{
    QTemporaryDir directory;
    const QString path = directory.filePath("board.json");
    MainWindow window(nullptr, path);
    window.show();
    QCoreApplication::processEvents();
    QVERIFY(window.saveDocument(false, false));
    QVERIFY(window.persistenceStatus()["persisted"].toBool());
    const auto revision = window.m_model.document().revision();
    window.resize(window.width() - 100, window.height() - 50);
    QVERIFY(!window.persistenceStatus()["persisted"].toBool());
    QCOMPARE(window.m_model.document().revision(), revision);
    QVERIFY(window.saveDocument(false, false));
    window.m_model.selectTool(Tool::Text);
    window.updateUiState();
    QTest::mouseClick(window.m_canvas, Qt::LeftButton, {}, QPoint(100, 100));
    auto *editor = window.m_canvas->findChild<QPlainTextEdit *>();
    QVERIFY(editor);
    editor->setPlainText("pending text survives");
    const auto beforeStatus = window.m_model.document().toJson();
    QVERIFY(!window.persistenceStatus()["persisted"].toBool());
    QCOMPARE(window.m_model.document().toJson(), beforeStatus);
    QCOMPARE(window.persistenceAccess("whiteboard/diagram/apply"), QString("persistence_busy"));
    QCOMPARE(window.persistenceAccess("whiteboard/page/navigate"), QString("persistence_busy"));
    QCOMPARE(window.persistenceAccess("whiteboard/state"), QString());
    bool fail = true;
    int attempts = 0;
    window.m_persistence = BoardPersistence(path, {}, [&](const QString &target, const BoardDocument &document) {
        ++attempts;
        if (fail) return BoardFileResult{BoardFileResult::Status::Failure, "commit", "Injected failure"};
        return BoardDocumentStorage::write(target, document);
    });
    // Initialize the new persistence baseline without replacing the window's editing model.
    BoardModel baseline;
    QVERIFY(window.m_persistence.restore(baseline));
    const QByteArray original = fileBytes(path);
    QTimer::singleShot(0, &window, []() { clickDialogButton("Cancel Closing"); });
    QVERIFY(!window.close());
    QVERIFY(!window.m_canvas->hasPendingTextEdit());
    QCOMPARE(window.m_model.currentPage().elements().size(), 1);
    QCOMPARE(window.m_model.currentPage().elements().first().text(), QString("pending text survives"));
    QVERIFY(!window.saveDocument(false, false));
    QCOMPARE(attempts, 1);
    QCOMPARE(window.m_model.currentPage().elements().size(), 1);
    QTimer::singleShot(0, &window, [&window]() {
        clickDialogButton("Retry Saving");
        QTimer::singleShot(0, &window, []() {
            for (QWidget *widget : QApplication::topLevelWidgets()) {
                auto *box = qobject_cast<QMessageBox *>(widget);
                if (box && box->isVisible() && box->button(QMessageBox::Ok)) {
                    box->button(QMessageBox::Ok)->click();
                    return;
                }
            }
            QFAIL("Expected retry failure notification was not found");
        });
    });
    QVERIFY(!window.close());
    QCOMPARE(attempts, 2);
    QVERIFY(window.isVisible());
    QVERIFY(window.m_model.canUndo());
    QCOMPARE(fileBytes(path), original);
    fail = false;
    QTimer::singleShot(0, &window, []() { clickDialogButton("Retry Saving"); });
    QVERIFY(window.close());
    QCOMPARE(attempts, 3);
    MainWindow reopened(nullptr, path);
    QCOMPARE(reopened.m_persistence.state(), BoardPersistence::State::Ready);
    QCOMPARE(reopened.m_model.currentPage().elements().size(), 1);
    QCOMPARE(reopened.m_model.currentPage().elements().first().text(), QString("pending text survives"));
    QVERIFY(reopened.close());
}

void WhiteboardPersistenceUiTest::failedSaveNoticeIsQueuedAndNotRepeated()
{
    QTemporaryDir directory;
    const QString path = directory.filePath("board.json");
    MainWindow window(nullptr, path);
    int writes = 0;
    window.m_persistence = BoardPersistence(path, {}, [&](const QString &, const BoardDocument &) {
        ++writes;
        return BoardFileResult{BoardFileResult::Status::Failure, "open", "Injected failure"};
    });
    QVERIFY(window.m_persistence.restore(window.m_model));
    QVERIFY(!window.saveDocument());
    QVERIFY(window.m_persistenceNoticePending);
    QVERIFY(!window.m_persistenceDialogActive);
    QTimer::singleShot(0, &window, [&window]() {
        QCOMPARE(window.persistenceAccess("whiteboard/diagram/apply"), QString("persistence_busy"));
        QCOMPARE(window.persistenceAccess("whiteboard/state"), QString());
        clickDialogButton("Continue Editing");
    });
    QCoreApplication::processEvents();
    QVERIFY(!window.m_persistenceNoticePending);
    for (int i = 0; i < 4; ++i) QVERIFY(!window.saveDocument());
    QCOMPARE(writes, 1);
    QVERIFY(!window.m_persistenceNoticePending);
    window.m_exitWithoutSaving = true;
    QVERIFY(window.close());
}

void WhiteboardPersistenceUiTest::mcpReportsAppliedButUnsavedAndProtectsPendingText()
{
    QTemporaryDir directory;
    const QString path = directory.filePath("board.json");
    BoardModel seed;
    QVERIFY(BoardDocumentStorage::write(path, seed.document()).ok());
    const QByteArray original = fileBytes(path);
    MainWindow window(nullptr, path);
    int writes = 0;
    window.m_persistence = BoardPersistence(path, {}, [&](const QString &, const BoardDocument &) {
        ++writes;
        return BoardFileResult{BoardFileResult::Status::Failure, "commit", "Injected failure"};
    });
    QVERIFY(window.m_persistence.restore(window.m_model));
    WhiteboardMcpController controller(&window.m_model, window.m_canvas,
        [&window]() { window.handleMcpChange(); },
        [&window](const QString &direction) { return window.navigatePage(direction); },
        [&window](const QString &tool, const QJsonObject &args, QString *error) { return window.approveMcpRequest(tool, args, error); });
    controller.setPersistenceCallbacks([&window]() { return window.persistenceStatus(); },
        [&window](const QString &tool) { return window.persistenceAccess(tool); },
        [&window](const QString &target, QString *error) { return window.m_persistence.allowsOutput(target, error); });
    QVERIFY(controller.startMessageTransport());
    auto call = [&controller](const QString &method, const QJsonObject &params) {
        QJsonObject response;
        bool hasResponse = false;
        const auto request = QJsonDocument(QJsonObject{{"jsonrpc", "2.0"}, {"id", 1}, {"method", method}, {"params", params}}).toJson();
        if (!controller.processMessage(request, &response, &hasResponse) || !hasResponse) return QJsonObject();
        return response.value("result").toObject();
    };
    QVERIFY(!call("initialize", {}).isEmpty());
    const QJsonObject rectangle{{"type", "rectangle"}, {"rect", QJsonObject{{"x", 10}, {"y", 20}, {"width", 30}, {"height", 40}}}};
    QJsonObject arguments{{"mode", "append_current"}, {"elements", QJsonArray{rectangle}}};
    auto applied = call("tools/call", {{"name", "whiteboard/diagram/apply"}, {"arguments", arguments}});
    QVERIFY(applied["applied"].toBool());
    QVERIFY(!applied["isError"].toBool());
    QCOMPARE(applied["persistence"].toObject()["status"].toString(), QString("save_failed"));
    QVERIFY(!applied["persistence"].toObject()["persisted"].toBool());
    QCOMPARE(window.m_model.currentPage().elements().size(), 1);
    QCOMPARE(writes, 1);
    QCOMPARE(fileBytes(path), original);
    window.m_persistenceNoticePending = false;
    window.show();
    QCoreApplication::processEvents();
    window.m_model.selectTool(Tool::Text);
    window.updateUiState();
    QTest::mouseClick(window.m_canvas, Qt::LeftButton, {}, QPoint(100, 100));
    auto *editor = window.m_canvas->findChild<QPlainTextEdit *>();
    QVERIFY(editor);
    editor->setPlainText("stay on the original page");
    const auto before = window.m_model.document().toJson();
    arguments["mode"] = "new_page";
    auto blocked = call("tools/call", {{"name", "whiteboard/diagram/apply"}, {"arguments", arguments}});
    QVERIFY(blocked["isError"].toBool());
    QCOMPARE(blocked["structuredContent"].toObject()["error"].toObject()["code"].toString(), QString("persistence_busy"));
    QCOMPARE(window.m_model.document().toJson(), before);
    QVERIFY(window.m_canvas->hasPendingTextEdit());
    QCOMPARE(writes, 1);
    window.m_canvas->commitPendingTextEdit();
    QCOMPARE(window.m_model.document().pages().size(), 1);
    QCOMPARE(window.m_model.currentPage().elements().size(), 2);
    window.m_exitWithoutSaving = true;
    QVERIFY(window.close());
}

void WhiteboardPersistenceUiTest::failureNoticeSurvivesNestedApproval()
{
    QTemporaryDir directory;
    const QString path = directory.filePath("board.json");
    MainWindow window(nullptr, path);
    window.m_persistence = BoardPersistence(path, {}, [](const QString &, const BoardDocument &) {
        return BoardFileResult{BoardFileResult::Status::Failure, "write", "Injected failure"};
    });
    QVERIFY(window.m_persistence.restore(window.m_model));
    QVERIFY(!window.saveDocument()); // queues a notice, before the approval event loop starts
    QTimer::singleShot(0, &window, []() {
        for (auto *widget : QApplication::topLevelWidgets()) {
            auto *box = qobject_cast<QMessageBox *>(widget);
            if (box && box->button(QMessageBox::No)) { box->button(QMessageBox::No)->click(); return; }
        }
        QFAIL("Approval dialog missing");
    });
    QString error;
    QVERIFY(!window.approveMcpRequest("whiteboard/page/add", {}, &error));
    QVERIFY(window.m_persistenceNoticePending);
    QTimer::singleShot(0, &window, []() { clickDialogButton("Continue Editing"); });
    QCoreApplication::processEvents();
    QVERIFY(!window.m_persistenceNoticePending);
    QVERIFY(!window.m_persistenceDialogActive);
    window.m_exitWithoutSaving = true;
    QVERIFY(window.close());
}

void WhiteboardPersistenceUiTest::localizedProtectionViews()
{
    const QString translation = qEnvironmentVariable("WHITEBOARD_TEST_TRANSLATION");
    if (translation.isEmpty()) QSKIP("Optional Japanese visual verification");
    QTranslator translator;
    QVERIFY(translator.load(translation));
    qApp->installTranslator(&translator);
    QTemporaryDir directory;
    const QString path = directory.filePath("board.json");
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("{}");
    file.close();
    MainWindow window(nullptr, path);
    window.m_persistenceNoticePending = false;
    window.show();
    QTest::qWait(100);
    capture(window, "restore-blocked-ja");
    BoardModel repaired;
    repaired.addRectangle(QRectF(50, 50, 150, 100));
    QVERIFY(BoardDocumentStorage::write(path, repaired.document()).ok());
    window.reloadSavedData();
    window.m_persistence = BoardPersistence(path, {}, [](const QString &, const BoardDocument &) {
        return BoardFileResult{BoardFileResult::Status::Failure, "write", "Injected visual verification failure"};
    });
    BoardModel baseline;
    QVERIFY(window.m_persistence.restore(baseline));
    QVERIFY(!window.saveDocument(false, false));
    QTest::qWait(100);
    capture(window, "save-stopped-ja");
    QTimer::singleShot(0, &window, []() {
        for (auto *widget : QApplication::topLevelWidgets())
            if (widget->objectName() == QStringLiteral("persistenceCloseDialog")) capture(*widget, "close-confirmation-ja");
        clickDialogButton(QCoreApplication::translate("MainWindow", "Cancel Closing"));
    });
    QVERIFY(!window.close());
    window.m_exitWithoutSaving = true;
    QVERIFY(window.close());
    qApp->removeTranslator(&translator);
}

QTEST_MAIN(WhiteboardPersistenceUiTest)
#include "tst_persistence_ui.moc"
