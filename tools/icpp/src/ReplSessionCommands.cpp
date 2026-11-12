#include "icpp/ReplSessionImpl.h"
#include "icpp/LineEdit.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QLibraryInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTextStream>
#include <QXmlStreamReader>

#include <algorithm>
#include <cstdlib>
#include <optional>
#include <utility>

namespace icpp {

bool ReplSessionImpl::requireQtClingCommand(const QString& commandName)
{
    if (currentEngine == Engine::QtCling) {
        return true;
    }

    reportError(trMessage(QStringLiteral("%1 には --engine qtcling が必要です。"),
                          QStringLiteral("%1 requires --engine qtcling."))
                    .arg(commandName));
    return false;
}

void ReplSessionImpl::showRecentErrors()
{
    if (recentErrors.isEmpty()) {
        standardOutput << trMessage(
                              QStringLiteral("icpp コマンドエラーは記録されていません。interpreter の診断はそのまま転送されます。"),
                              QStringLiteral("No icpp command errors recorded. Interpreter diagnostics are forwarded directly."))
                       << Qt::endl;
        return;
    }

    for (const QString& errorMessage : recentErrors) {
        standardOutput << errorMessage << Qt::endl;
    }
}

void ReplSessionImpl::handleBufferAppendMode(const QString& commandName,
                            const QString& argument,
                            const QString& historyEntry)
{
    const QString normalizedArgument = argument.trimmed().toLower();
    if (normalizedArgument.isEmpty()) {
        if (commandName == QStringLiteral(".b")) {
            appendNormalInputToBuffer = !appendNormalInputToBuffer;
        }
        showBufferAppendMode();
        addHistoryEntry(historyEntry);
        return;
    }

    if (normalizedArgument == QStringLiteral("on")) {
        appendNormalInputToBuffer = true;
        showBufferAppendMode();
        addHistoryEntry(historyEntry);
        return;
    }

    if (normalizedArgument == QStringLiteral("off")) {
        appendNormalInputToBuffer = false;
        showBufferAppendMode();
        addHistoryEntry(historyEntry);
        return;
    }

    reportError(trMessage(QStringLiteral("%1 には 'on' または 'off' が必要です。"),
                          QStringLiteral("%1 requires 'on' or 'off'."))
                    .arg(commandName));
}

void ReplSessionImpl::handleAutogenMode(const QString& argument, const QString& historyEntry)
{
    const QString normalizedArgument = argument.trimmed().toLower();
    if (normalizedArgument.isEmpty()) {
        showAutogenMode();
        addHistoryEntry(historyEntry);
        return;
    }

    if (normalizedArgument == QStringLiteral("on")) {
        autogenEnabled = true;
        showAutogenMode();
        addHistoryEntry(historyEntry);
        return;
    }

    if (normalizedArgument == QStringLiteral("off")) {
        autogenEnabled = false;
        showAutogenMode();
        addHistoryEntry(historyEntry);
        return;
    }

    reportError(trMessage(QStringLiteral(".autogen には 'on' または 'off' が必要です。"),
                          QStringLiteral(".autogen requires 'on' or 'off'.")));
}

void ReplSessionImpl::handleQuietMode(const QString& argument, const QString& historyEntry)
{
    const QString normalizedArgument = argument.trimmed().toLower();
    if (normalizedArgument.isEmpty()) {
        showQuietMode();
        addHistoryEntry(historyEntry);
        return;
    }

    if (normalizedArgument == QStringLiteral("on")) {
        quietMode = true;
        QSettings(QStringLiteral("icpp"), QStringLiteral("icpp"))
            .setValue(QStringLiteral("quiet"), quietMode);
        showQuietMode();
        addHistoryEntry(historyEntry);
        return;
    }

    if (normalizedArgument == QStringLiteral("off")) {
        quietMode = false;
        QSettings(QStringLiteral("icpp"), QStringLiteral("icpp"))
            .setValue(QStringLiteral("quiet"), quietMode);
        showQuietMode();
        addHistoryEntry(historyEntry);
        return;
    }

    reportError(trMessage(QStringLiteral(".quiet には 'on' または 'off' が必要です。"),
                          QStringLiteral(".quiet requires 'on' or 'off'.")));
}

void ReplSessionImpl::handleNewFileNameMode(const QString& argument, const QString& historyEntry)
{
    const QString normalizedArgument = argument.trimmed().toLower();
    if (normalizedArgument.isEmpty()) {
        showNewFileNameMode();
        addHistoryEntry(historyEntry);
        return;
    }

    if (normalizedArgument == QStringLiteral("lower")
        || normalizedArgument == QStringLiteral("asis")) {
        newFileNameMode = normalizedArgument;
        QSettings(QStringLiteral("icpp"), QStringLiteral("icpp"))
            .setValue(QStringLiteral("newFileName"), newFileNameMode);
        showNewFileNameMode();
        addHistoryEntry(historyEntry);
        return;
    }

    reportError(trMessage(QStringLiteral(".newname には 'lower' または 'asis' が必要です。"),
                          QStringLiteral(".newname requires 'lower' or 'asis'.")));
}

void ReplSessionImpl::handleHelpLanguageMode(const QString& argument, const QString& historyEntry)
{
    const QString normalizedArgument = argument.trimmed().toLower();
    if (normalizedArgument.isEmpty()) {
        showHelpLanguage();
        addHistoryEntry(historyEntry);
        return;
    }

    const std::optional<HelpLanguage> parsedLanguage =
        parseHelpLanguageName(normalizedArgument);
    if (!parsedLanguage) {
        reportError(trMessage(QStringLiteral("不正なヘルプ言語です。ja または en を指定してください。"),
                              QStringLiteral("Invalid help language. Use ja or en.")));
        return;
    }

    helpLanguage = *parsedLanguage;
    interpreter.setHelpLanguage(helpLanguage);
    activeHelpLanguage() = helpLanguage;
    QSettings settings(QStringLiteral("icpp"), QStringLiteral("icpp"));
    settings.setValue(QStringLiteral("helpLanguage"),
                      helpLanguage == HelpLanguage::Japanese ? QStringLiteral("ja")
                                                             : QStringLiteral("en"));
    settings.sync();
    showHelpLanguage();
    addHistoryEntry(historyEntry);
}

void ReplSessionImpl::showBufferAppendMode() const
{
    standardOutput << trMessage(QStringLiteral("入力バッファ追加: %1"),
                                QStringLiteral("Input buffer append: %1"))
                          .arg(appendNormalInputToBuffer ? QStringLiteral("on")
                                                         : QStringLiteral("off"))
                   << Qt::endl;
}

void ReplSessionImpl::showAutogenMode() const
{
    standardOutput << trMessage(QStringLiteral("autogen: %1"),
                                QStringLiteral("Autogen: %1"))
                          .arg(autogenEnabled ? QStringLiteral("on") : QStringLiteral("off"))
                   << Qt::endl;
}

void ReplSessionImpl::showQuietMode() const
{
    standardOutput << trMessage(QStringLiteral("quiet: %1"),
                                QStringLiteral("quiet: %1"))
                          .arg(quietMode ? QStringLiteral("on") : QStringLiteral("off"))
                   << Qt::endl;
}

void ReplSessionImpl::showNewFileNameMode() const
{
    standardOutput << trMessage(QStringLiteral("新規ファイル名: %1"),
                                QStringLiteral("new file name: %1"))
                          .arg(newFileNameMode)
                   << Qt::endl;
}

void ReplSessionImpl::showHelpLanguage() const
{
    standardOutput << trMessage(QStringLiteral("ヘルプ言語: %1"),
                                QStringLiteral("help language: %1"))
                          .arg(helpLanguage == HelpLanguage::Japanese ? QStringLiteral("ja")
                                                                      : QStringLiteral("en"))
                   << Qt::endl;
}

void ReplSessionImpl::showStatus() const
{
    const QString sourceCode = visibleSourceCode();
    const int lineCount = sourceCode.isEmpty() ? 0 : sourceCode.count(QLatin1Char('\n')) + 1;
    const QString runAllPath = QStandardPaths::findExecutable(QStringLiteral("run_all"));

    standardOutput << trMessage(QStringLiteral("エンジン: %1"), QStringLiteral("engine: %1"))
                          .arg(engineName(currentEngine))
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("ヘルプ言語: %1"),
                                QStringLiteral("help language: %1"))
                          .arg(helpLanguage == HelpLanguage::Japanese ? QStringLiteral("ja")
                                                                      : QStringLiteral("en"))
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("入力バッファ追加: %1"),
                                QStringLiteral("input buffer append: %1"))
                          .arg(appendNormalInputToBuffer ? QStringLiteral("on")
                                                         : QStringLiteral("off"))
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("autogen: %1"),
                                QStringLiteral("autogen: %1"))
                          .arg(autogenEnabled ? QStringLiteral("on") : QStringLiteral("off"))
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("quiet: %1"),
                                QStringLiteral("quiet: %1"))
                          .arg(quietMode ? QStringLiteral("on") : QStringLiteral("off"))
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("新規ファイル名: %1"),
                                QStringLiteral("new file name: %1"))
                          .arg(newFileNameMode)
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("編集バッファ: %1 行, %2"),
                                QStringLiteral("edit buffer: %1 lines, %2"))
                          .arg(lineCount)
                          .arg(visibleBufferHasUnsavedChanges()
                                   ? trMessage(QStringLiteral("変更あり"), QStringLiteral("modified"))
                                   : trMessage(QStringLiteral("変更なし"), QStringLiteral("clean")))
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("保存先: %1"), QStringLiteral("save target: %1"))
                          .arg(rememberedVisibleBufferFilePath.isEmpty()
                                   ? trMessage(QStringLiteral("(なし)"), QStringLiteral("(none)"))
                                   : rememberedVisibleBufferFilePath)
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("登録ファイル数: %1"),
                                QStringLiteral("registered files: %1"))
                          .arg(registeredSourceFilePaths.size())
                   << Qt::endl;
    for (int index = 0; index < registeredSourceFilePaths.size(); ++index) {
        standardOutput << QStringLiteral("  %1  %2").arg(index + 1).arg(registeredSourceFilePaths.at(index))
                       << Qt::endl;
    }
    standardOutput << trMessage(QStringLiteral("インクルードパス数: %1"),
                                QStringLiteral("include paths: %1"))
                          .arg(includePathPragmas.size())
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("操作したファイル数: %1"),
                                QStringLiteral("touched files: %1"))
                          .arg(touchedSourceFilePaths.size())
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("Qt 生成物: %1"),
                                QStringLiteral("qt generated files: %1"))
                          .arg(qtGeneratedFilesMayBeNeeded()
                                   ? trMessage(QStringLiteral("必要そうです"), QStringLiteral("needed"))
                                   : trMessage(QStringLiteral("未検出"), QStringLiteral("not detected")))
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("run_all: %1"), QStringLiteral("run_all: %1"))
                          .arg(runAllPath.isEmpty()
                                   ? trMessage(QStringLiteral("(見つかりません)"),
                                               QStringLiteral("(not found)"))
                                   : runAllPath)
                   << Qt::endl;
}

bool ReplSessionImpl::confirmYesNo(const QString& promptText)
{
    while (true) {
        standardError << promptText << Qt::flush;
        const std::optional<QString> answer = readConsoleLine(QString());
        if (!answer.has_value()) {
            standardError << Qt::endl;
            return false;
        }

        const QString normalizedAnswer = answer->trimmed().toLower();
        if (normalizedAnswer == QStringLiteral("y") || normalizedAnswer == QStringLiteral("yes")) {
            return true;
        }
        if (normalizedAnswer.isEmpty() || normalizedAnswer == QStringLiteral("n")
            || normalizedAnswer == QStringLiteral("no")) {
            return false;
        }

        standardError << trMessage(QStringLiteral("Y または N で答えてください。"),
                                   QStringLiteral("Please answer Y or N."))
                      << Qt::endl;
    }
}

void ReplSessionImpl::reportError(const QString& errorMessage)
{
    recentErrors.append(errorMessage);
    while (recentErrors.size() > 20) {
        recentErrors.removeFirst();
    }
    standardError << errorMessage << Qt::endl;
}

bool ReplSessionImpl::visibleBufferHasUnsavedChanges() const
{
    const QString sourceCode = visibleSourceCode();
    return !sourceCode.isEmpty() && sourceCode != lastSavedVisibleSourceCode;
}

bool ReplSessionImpl::confirmQuitIfVisibleBufferHasUnsavedChanges()
{
    if (!visibleBufferHasUnsavedChanges()) {
        return true;
    }

    return confirmYesNo(trMessage(
        QStringLiteral("警告: 編集バッファに未保存の変更があります。保存せずに終了しますか? [Y/N] "),
        QStringLiteral("Warning: edit buffer has unsaved changes. Quit without saving? [Y/N] ")));
}

void ReplSessionImpl::addHistoryEntry(const QString& historyEntry)
{
    if (historyEntry.trimmed().isEmpty()) {
        return;
    }

#if defined(HAVE_READLINE)
    ::add_history(historyEntry.toLocal8Bit().constData());
#else
    Q_UNUSED(historyEntry);
#endif
}

} // namespace icpp
