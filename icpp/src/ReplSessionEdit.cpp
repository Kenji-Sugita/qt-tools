#include "icpp/ReplSessionImpl.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QLibraryInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTextStream>
#include <QXmlStreamReader>

#include <algorithm>
#include <cstdlib>
#include <optional>
#include <utility>

namespace icpp {

QString ReplSessionImpl::visibleSourceCode() const
{
    return joinSourceCode(committedSourceCode);
}

void ReplSessionImpl::replaceVisibleSourceCode(const QString& sourceCode)
{
    committedSourceCode.clear();
    const QString normalized = normalizeSourceCode(sourceCode);
    if (!normalized.isEmpty()) {
        committedSourceCode.append(normalized);
    }
}

void ReplSessionImpl::appendSource(const QString& sourceCode)
{
    const QString normalized = normalizeSourceCode(sourceCode);
    if (!normalized.isEmpty()) {
        committedSourceCode.append(normalized);
    }
}

void ReplSessionImpl::showVisibleSourceCode(const QString& rangeArgument)
{
    const QString sourceCodeToShow = visibleSourceCode();
    if (sourceCodeToShow.isEmpty()) {
        standardOutput << trMessage(QStringLiteral("編集バッファは空です。"),
                                    QStringLiteral("Edit buffer is empty."))
                       << Qt::endl;
        return;
    }

    const QStringList allLines = sourceCodeToShow.split(QLatin1Char('\n'));
    const std::optional<ShowRange> showRange = parseShowRange(rangeArgument, allLines.size());
    if (!showRange.has_value()) {
        reportError(trMessage(QStringLiteral("範囲指定が不正です。.show、.show 3、.show 3:8 の形で指定してください。"),
                              QStringLiteral("Invalid range. Use .show, .show 3, or .show 3:8")));
        return;
    }

    const int lineNumberWidth = QString::number(showRange->endLine).size();
    for (int lineNumber = showRange->startLine; lineNumber <= showRange->endLine; ++lineNumber) {
        standardOutput << QStringLiteral("%1  %2")
                              .arg(lineNumber, lineNumberWidth)
                              .arg(allLines.at(lineNumber - 1))
                       << Qt::endl;
    }
}

void ReplSessionImpl::discardVisibleSourceCode()
{
    if (committedSourceCode.isEmpty()) {
        standardOutput << trMessage(QStringLiteral("編集バッファはすでに空です。"),
                                    QStringLiteral("Edit buffer is already empty."))
                       << Qt::endl;
        return;
    }
    if (visibleBufferHasUnsavedChanges()
        && !confirmYesNo(trMessage(
            QStringLiteral("警告: 未保存の編集バッファを消します。続けますか? [Y/N] "),
            QStringLiteral("Warning: edit buffer has unsaved changes and will be discarded. Continue? [Y/N] ")))) {
        return;
    }

    committedSourceCode.clear();
    rememberedVisibleBufferFilePath.clear();
    lastSavedVisibleSourceCode.clear();
    standardOutput << trMessage(QStringLiteral("編集バッファを消しました。"),
                                QStringLiteral("Edit buffer cleared."))
                   << Qt::endl;
}

void ReplSessionImpl::pasteClipboardIntoVisibleBuffer(const QString& historyEntry)
{
    const std::optional<ClipboardCommand> command = findClipboardReadCommand();
    if (!command.has_value()) {
        reportError(trMessage(QStringLiteral("クリップボード貼り付けコマンドが見つかりません。"),
                              QStringLiteral("Cannot find a clipboard paste command.")));
        return;
    }

    QProcess clipboardProcess;
    clipboardProcess.start(command->program, command->arguments);
    if (!clipboardProcess.waitForStarted()) {
        reportError(trMessage(QStringLiteral("クリップボード貼り付けコマンドを起動できません: %1"),
                              QStringLiteral("Cannot start clipboard paste command: %1"))
                        .arg(command->program));
        return;
    }
    if (!clipboardProcess.waitForFinished(3000)) {
        clipboardProcess.kill();
        clipboardProcess.waitForFinished(1000);
        reportError(trMessage(QStringLiteral("クリップボード貼り付けコマンドがタイムアウトしました。"),
                              QStringLiteral("Clipboard paste command timed out.")));
        return;
    }
    if (clipboardProcess.exitStatus() != QProcess::NormalExit || clipboardProcess.exitCode() != 0) {
        const QString errorText = QString::fromLocal8Bit(clipboardProcess.readAllStandardError()).trimmed();
        reportError(errorText.isEmpty()
                        ? trMessage(QStringLiteral("クリップボード貼り付けコマンドが失敗しました。"),
                                    QStringLiteral("Clipboard paste command failed."))
                        : trMessage(QStringLiteral("クリップボード貼り付けコマンドが失敗しました: %1"),
                                    QStringLiteral("Clipboard paste command failed: %1"))
                              .arg(errorText));
        return;
    }

    const QString clipboardText = QString::fromLocal8Bit(clipboardProcess.readAllStandardOutput());
    const QString normalizedClipboardText = normalizeSourceCode(clipboardText);
    if (normalizedClipboardText.isEmpty()) {
        reportError(trMessage(QStringLiteral("クリップボードは空です。"),
                              QStringLiteral("Clipboard is empty.")));
        return;
    }

    appendSource(normalizedClipboardText);
    const int pastedLineCount = normalizedClipboardText.count(QLatin1Char('\n')) + 1;
    standardOutput << trMessage(QStringLiteral("%1 行を貼り付けました。"),
                                QStringLiteral("Pasted %1 lines."))
                          .arg(pastedLineCount)
                   << Qt::endl;
    addHistoryEntry(historyEntry);
}

void ReplSessionImpl::copyVisibleBufferToClipboard(const QString& historyEntry)
{
    const QString sourceCode = visibleSourceCode();
    if (sourceCode.isEmpty()) {
        reportError(trMessage(QStringLiteral("編集バッファは空です。"),
                              QStringLiteral("Edit buffer is empty.")));
        return;
    }

    const std::optional<ClipboardCommand> command = findClipboardWriteCommand();
    if (!command.has_value()) {
        reportError(trMessage(QStringLiteral("クリップボードコピーコマンドが見つかりません。"),
                              QStringLiteral("Cannot find a clipboard copy command.")));
        return;
    }

    QProcess clipboardProcess;
    clipboardProcess.start(command->program, command->arguments);
    if (!clipboardProcess.waitForStarted()) {
        reportError(trMessage(QStringLiteral("クリップボードコピーコマンドを起動できません: %1"),
                              QStringLiteral("Cannot start clipboard copy command: %1"))
                        .arg(command->program));
        return;
    }

    clipboardProcess.write(sourceCode.toLocal8Bit());
    clipboardProcess.closeWriteChannel();
    if (!clipboardProcess.waitForFinished(3000)) {
        clipboardProcess.kill();
        clipboardProcess.waitForFinished(1000);
        reportError(trMessage(QStringLiteral("クリップボードコピーコマンドがタイムアウトしました。"),
                              QStringLiteral("Clipboard copy command timed out.")));
        return;
    }
    if (clipboardProcess.exitStatus() != QProcess::NormalExit || clipboardProcess.exitCode() != 0) {
        const QString errorText = QString::fromLocal8Bit(clipboardProcess.readAllStandardError()).trimmed();
        reportError(errorText.isEmpty()
                        ? trMessage(QStringLiteral("クリップボードコピーコマンドが失敗しました。"),
                                    QStringLiteral("Clipboard copy command failed."))
                        : trMessage(QStringLiteral("クリップボードコピーコマンドが失敗しました: %1"),
                                    QStringLiteral("Clipboard copy command failed: %1"))
                              .arg(errorText));
        return;
    }

    const int copiedLineCount = sourceCode.count(QLatin1Char('\n')) + 1;
    standardOutput << trMessage(QStringLiteral("%1 行をコピーしました。"),
                                QStringLiteral("Copied %1 lines."))
                          .arg(copiedLineCount)
                   << Qt::endl;
    addHistoryEntry(historyEntry);
}

void ReplSessionImpl::editSourceFile(const QString& filePathOrNumber, const QString& historyEntry)
{
    std::optional<QString> filePath;
    bool createdFile = false;

    bool isNumber = false;
    filePathOrNumber.trimmed().toInt(&isNumber);
    if (isNumber) {
        filePath = resolveRegisteredSourceFilePath(filePathOrNumber, QStringLiteral(".edit"));
        if (!filePath.has_value()) {
            return;
        }
    } else {
        const QString target = filePathOrNumber.trimmed();
        if (target.isEmpty()) {
            reportError(trMessage(QStringLiteral(".edit にはファイルパスまたは番号が必要です。"),
                                  QStringLiteral(".edit requires a file path or number.")));
            return;
        }

        const QFileInfo fileInfo(target);
        const QDir parentDirectory = fileInfo.absoluteDir();
        if (!parentDirectory.exists()) {
            reportError(trMessage(QStringLiteral("%1: ディレクトリが見つかりません。"),
                                  QStringLiteral("%1: Directory not found."))
                            .arg(parentDirectory.absolutePath()));
            return;
        }

        const QString absoluteFilePath = fileInfo.absoluteFilePath();
        if (!fileInfo.exists()) {
            if (!writeTextFile(absoluteFilePath, QString())) {
                return;
            }
            createdFile = true;
            standardOutput << trMessage(QStringLiteral("作成しました: %1"),
                                        QStringLiteral("Created: %1"))
                                  .arg(absoluteFilePath)
                           << Qt::endl;
        } else if (!fileInfo.isFile()) {
            reportError(trMessage(QStringLiteral("%1: ファイルではありません。"),
                                  QStringLiteral("%1: Not a file."))
                            .arg(target));
            return;
        }

        if (shouldAutoRegisterEditedFile(QFileInfo(absoluteFilePath))
            && !registeredSourceFilePaths.contains(absoluteFilePath)) {
            registeredSourceFilePaths.append(absoluteFilePath);
            standardOutput << trMessage(QStringLiteral("登録しました: %1"),
                                        QStringLiteral("Added: %1"))
                                  .arg(absoluteFilePath)
                           << Qt::endl;
        }
        filePath = absoluteFilePath;
    }

    if (!runEditorForFile(*filePath)) {
        if (createdFile) {
            standardOutput << trMessage(QStringLiteral("作成したファイルは残っています: %1"),
                                        QStringLiteral("Created file remains: %1"))
                                  .arg(*filePath)
                           << Qt::endl;
        }
        return;
    }

    if (!readTextFile(*filePath).has_value()) {
        return;
    }

    rememberTouchedSourceFile(*filePath);
    standardOutput << trMessage(QStringLiteral("編集しました: %1"),
                                QStringLiteral("Edited: %1"))
                          .arg(QFileInfo(*filePath).fileName())
                   << Qt::endl;
    if (!autogenEnabled) {
        warnIfQtGeneratedFilesMayBeStale();
    }
    if (registeredSourceFilePaths.isEmpty() && visibleSourceCode().isEmpty()
        && !shouldAutoRegisterEditedFile(QFileInfo(*filePath))) {
        standardOutput
            << trMessage(QStringLiteral("評価する C++ ファイルまたは編集バッファがありません。.e <file.cpp> または .add <file.cpp> の後に .gen を実行してください。"),
                         QStringLiteral("No C++ files or edit buffer to run. Use .e <file.cpp> or .add <file.cpp>, then run .gen."))
            << Qt::endl;
        addHistoryEntry(historyEntry);
        return;
    }
    runVisibleSourceCode();
    addHistoryEntry(historyEntry);
}

void ReplSessionImpl::openFileIntoVisibleBuffer(const QString& filePath, const QString& historyEntry)
{
    if (filePath.isEmpty()) {
        reportError(trMessage(QStringLiteral(".open にはファイルパスが必要です。"),
                              QStringLiteral(".open requires a file path.")));
        return;
    }
    if (visibleBufferHasUnsavedChanges()
        && !confirmYesNo(trMessage(
            QStringLiteral("警告: 未保存の編集バッファを置き換えます。続けますか? [Y/N] "),
            QStringLiteral("Warning: edit buffer has unsaved changes and will be replaced. Continue? [Y/N] ")))) {
        return;
    }

    const std::optional<QString> sourceCode = readTextFile(filePath);
    if (!sourceCode.has_value()) {
        return;
    }

    replaceVisibleSourceCode(*sourceCode);
    rememberedVisibleBufferFilePath = QFileInfo(filePath).absoluteFilePath();
    rememberTouchedSourceFile(rememberedVisibleBufferFilePath);
    lastSavedVisibleSourceCode = visibleSourceCode();
    standardOutput << trMessage(QStringLiteral("開きました: %1"),
                                QStringLiteral("Opened: %1"))
                          .arg(QFileInfo(filePath).fileName())
                   << Qt::endl;
    addHistoryEntry(historyEntry);
}

void ReplSessionImpl::loadAndEvaluateFile(const QString& filePath, const QString& historyEntry)
{
    if (filePath.isEmpty()) {
        reportError(trMessage(QStringLiteral(".load にはファイルパスが必要です。"),
                              QStringLiteral(".load requires a file path.")));
        return;
    }

    const std::optional<QString> sourceCode = readTextFile(filePath);
    if (!sourceCode.has_value()) {
        return;
    }

    if (sourceAppearsIncomplete(*sourceCode, QFileInfo(filePath).fileName())) {
        return;
    }

    const bool wasVisibleBufferEmpty = visibleSourceCode().isEmpty();
    appendSource(*sourceCode);
    if (interpreter.sendSource(*sourceCode)) {
        rememberedVisibleBufferFilePath = QFileInfo(filePath).absoluteFilePath();
        rememberTouchedSourceFile(rememberedVisibleBufferFilePath);
        if (wasVisibleBufferEmpty) {
            lastSavedVisibleSourceCode = visibleSourceCode();
        }
        standardOutput << trMessage(QStringLiteral("読み込みました: %1"),
                                    QStringLiteral("Loaded: %1"))
                              .arg(QFileInfo(filePath).fileName())
                       << Qt::endl;
        addHistoryEntry(historyEntry);
    }
}

void ReplSessionImpl::saveVisibleBuffer(const QString& filePathArgument, const QString& historyEntry)
{
    const QString sourceCodeToWrite = visibleSourceCode();
    if (sourceCodeToWrite.isEmpty()) {
        reportError(trMessage(QStringLiteral("編集バッファは空です。"),
                              QStringLiteral("Edit buffer is empty.")));
        return;
    }

    QString filePath = filePathArgument.trimmed();
    if (filePath.isEmpty()) {
        filePath = rememberedVisibleBufferFilePath;
        if (!filePath.isEmpty() && shouldConfirmImplicitSave()
            && !confirmYesNo(
                trMessage(QStringLiteral("警告: 複数のファイルを扱っています。編集バッファを %1 に保存しますか? [Y/N] "),
                          QStringLiteral("Warning: multiple files are in use. Save edit buffer to %1? [Y/N] "))
                    .arg(filePath))) {
            return;
        }
    }
    if (filePath.isEmpty()) {
        reportError(trMessage(QStringLiteral("最初の .save にはファイルパスが必要です。"),
                              QStringLiteral(".save requires a file path the first time.")));
        return;
    }

    if (!writeTextFile(filePath, sourceCodeToWrite)) {
        return;
    }

    rememberedVisibleBufferFilePath = QFileInfo(filePath).absoluteFilePath();
    rememberTouchedSourceFile(rememberedVisibleBufferFilePath);
    lastSavedVisibleSourceCode = sourceCodeToWrite;
    standardOutput << trMessage(QStringLiteral("保存しました: %1"),
                                QStringLiteral("Saved: %1"))
                          .arg(QFileInfo(filePath).fileName())
                   << Qt::endl;
    addHistoryEntry(historyEntry.isEmpty() ? QStringLiteral(".save %1").arg(filePath) : historyEntry);
}

void ReplSessionImpl::openEditor(const QString& target, const QString& historyEntry)
{
    if (!target.trimmed().isEmpty()) {
        editSourceFile(target, historyEntry);
        return;
    }

    QTemporaryFile temporaryFile(QDir::tempPath() + QStringLiteral("/icpp_XXXXXX.cpp"));
    if (!temporaryFile.open()) {
        reportError(trMessage(QStringLiteral("一時ファイルを開けません。"),
                              QStringLiteral("Cannot open a temporary file.")));
        return;
    }

    QTextStream outputStream(&temporaryFile);
    outputStream << visibleSourceCode();
    if (!visibleSourceCode().isEmpty()) {
        outputStream << Qt::endl;
    }
    outputStream.flush();

    const QString temporaryFilePath = temporaryFile.fileName();
    temporaryFile.close();

    if (!runEditorForFile(temporaryFilePath)) {
        return;
    }

    const std::optional<QString> editedSourceCode = readTextFile(temporaryFilePath);
    if (!editedSourceCode.has_value()) {
        reportError(trMessage(QStringLiteral("編集後の一時ファイルを開けません。"),
                              QStringLiteral("Cannot open the edited temporary file.")));
        return;
    }

    replaceVisibleSourceCode(*editedSourceCode);
    if (!autogenEnabled) {
        warnIfQtGeneratedFilesMayBeStale();
    }
    runVisibleSourceCode();
    addHistoryEntry(historyEntry);
}

bool ReplSessionImpl::runEditorForFile(const QString& filePath)
{
    const QString editorCommand = determineEditorCommand();
    QStringList editorCommandParts = QProcess::splitCommand(editorCommand);
    if (editorCommandParts.isEmpty()) {
        reportError(trMessage(QStringLiteral("エディタコマンドを決定できません。"),
                              QStringLiteral("Cannot determine the editor command.")));
        return false;
    }

    const QString editorProgram = editorCommandParts.takeFirst();
    editorCommandParts.append(filePath);

    QProcess editorProcess;
    editorProcess.setInputChannelMode(QProcess::ForwardedInputChannel);
    editorProcess.setProcessChannelMode(QProcess::ForwardedChannels);
    editorProcess.start(editorProgram, editorCommandParts);

    if (!editorProcess.waitForStarted()) {
        reportError(trMessage(QStringLiteral("エディタを起動できません: %1"),
                              QStringLiteral("Cannot start the editor: %1"))
                        .arg(editorProgram));
        return false;
    }
    editorProcess.waitForFinished(-1);
    return true;
}

QString ReplSessionImpl::determineEditorCommand() const
{
    QString editorCommand = qEnvironmentVariable("ICPP_EDITOR");
    if (editorCommand.isEmpty()) {
        editorCommand = qEnvironmentVariable("VISUAL");
    }
    if (editorCommand.isEmpty()) {
        editorCommand = qEnvironmentVariable("EDITOR");
    }
    if (editorCommand.isEmpty()) {
        editorCommand = QString::fromLatin1(kDefaultEditor);
    }
    return editorCommand;
}

void ReplSessionImpl::rememberTouchedSourceFile(const QString& filePath)
{
    if (filePath.isEmpty()) {
        return;
    }

    const QString absoluteFilePath = QFileInfo(filePath).absoluteFilePath();
    if (!touchedSourceFilePaths.contains(absoluteFilePath)) {
        touchedSourceFilePaths.append(absoluteFilePath);
    }
}

bool ReplSessionImpl::shouldConfirmImplicitSave() const
{
    return touchedSourceFilePaths.size() > 1 || registeredSourceFilePaths.size() > 1;
}

} // namespace icpp
