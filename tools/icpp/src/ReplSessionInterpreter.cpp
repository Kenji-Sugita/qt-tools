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
namespace {

QString includePathPragmaForPath(const QString& path)
{
    return QStringLiteral("#pragma cling add_include_path(\"%1\")").arg(quoteForPragma(path));
}

QString unescapePragmaString(const QString& text)
{
    QString result;
    result.reserve(text.size());
    bool escaped = false;
    for (const QChar ch : text) {
        if (escaped) {
            result.append(ch);
            escaped = false;
        } else if (ch == QLatin1Char('\\')) {
            escaped = true;
        } else {
            result.append(ch);
        }
    }
    if (escaped) {
        result.append(QLatin1Char('\\'));
    }
    return result;
}

QString includePathFromPragma(const QString& pragma)
{
    constexpr auto prefix = "#pragma cling add_include_path(\"";
    constexpr auto suffix = "\")";
    if (!pragma.startsWith(QLatin1StringView(prefix)) || !pragma.endsWith(QLatin1StringView(suffix))) {
        return pragma;
    }

    const int start = int(QLatin1StringView(prefix).size());
    const int length = pragma.size() - start - int(QLatin1StringView(suffix).size());
    return unescapePragmaString(pragma.mid(start, length));
}

QStringList includePathsFromPragmas(const QStringList& pragmas)
{
    QStringList paths;
    paths.reserve(pragmas.size());
    for (const QString& pragma : pragmas) {
        paths.append(includePathFromPragma(pragma));
    }
    return paths;
}

} // namespace

void ReplSessionImpl::runVisibleSourceCode()
{
    if (!prepareAutogenIfEnabled()) {
        return;
    }
    runVisibleSourceCodeWithoutAutogen();
}

void ReplSessionImpl::runVisibleSourceCodeWithoutAutogen()
{
    const QString sourceCodeToEvaluate = visibleSourceCode();
    if (registeredSourceFilePaths.isEmpty() && sourceCodeToEvaluate.isEmpty()) {
        reportError(trMessage(QStringLiteral("実行するファイルまたは編集バッファがありません。"),
                              QStringLiteral("No files or edit buffer to run.")));
        return;
    }

    QList<std::pair<QString, QString>> registeredSources;
    registeredSources.reserve(registeredSourceFilePaths.size());
    for (const QString& filePath : registeredSourceFilePaths) {
        const std::optional<QString> sourceCode = readTextFile(filePath);
        if (!sourceCode.has_value()) {
            return;
        }
        if (sourceAppearsIncomplete(*sourceCode, QFileInfo(filePath).fileName())) {
            return;
        }
        registeredSources.append(std::make_pair(filePath, *sourceCode));
    }
    if (!sourceCodeToEvaluate.isEmpty()
        && sourceAppearsIncomplete(sourceCodeToEvaluate,
                                   trMessage(QStringLiteral("編集バッファ"),
                                             QStringLiteral("edit buffer")))) {
        return;
    }

    if (interpreter.restart()) {
        for (const QString& pragma : includePathPragmas) {
            interpreter.sendSource(pragma);
        }
        for (const auto& registeredSource : registeredSources) {
            const QString& filePath = registeredSource.first;
            const QString& sourceCode = registeredSource.second;
            ensureMocIncludePlaceholders(sourceCode, QFileInfo(filePath).absoluteDir());
            interpreter.sendSource(sourceCode);
        }
        if (!sourceCodeToEvaluate.isEmpty()) {
            ensureMocIncludePlaceholders(sourceCodeToEvaluate, QDir::current());
            interpreter.sendSource(sourceCodeToEvaluate);
        }
    }
}

bool ReplSessionImpl::sourceAppearsIncomplete(const QString& sourceCode, const QString& label)
{
    int braceDelta = 0;
    const QStringList lines = normalizeSourceCode(sourceCode).split(QLatin1Char('\n'));
    for (const QString& line : lines) {
        braceDelta += braceDeltaOutsideQuotedText(line);
    }
    if (braceDelta == 0) {
        return false;
    }

    reportError(trMessage(
                    QStringLiteral("入力待ちの可能性があります: %1 の { } が対応していません。コードを閉じてから再実行してください。"),
                    QStringLiteral("Input appears incomplete: unmatched { } in %1. Close the block and run again."))
                    .arg(label));
    return true;
}

void ReplSessionImpl::restartInterpreter()
{
    if (interpreter.restart()) {
        standardOutput << trMessage(QStringLiteral("interpreter を再起動しました。"),
                                    QStringLiteral("Interpreter restarted."))
                       << Qt::endl;
    }
}

void ReplSessionImpl::resetSession()
{
    if (visibleBufferHasUnsavedChanges()
        && !confirmYesNo(trMessage(
            QStringLiteral("警告: 未保存の編集バッファがあります。セッションをリセットしますか? [Y/N] "),
            QStringLiteral("Warning: edit buffer has unsaved changes. Reset the session? [Y/N] ")))) {
        return;
    }

    if (interpreter.restart()) {
        committedSourceCode.clear();
        registeredSourceFilePaths.clear();
        includePathPragmas.clear();
        touchedSourceFilePaths.clear();
        rememberedVisibleBufferFilePath.clear();
        lastSavedVisibleSourceCode.clear();
        standardOutput << trMessage(QStringLiteral("セッションをリセットしました。"),
                                    QStringLiteral("Session reset."))
                       << Qt::endl;
    }
}

void ReplSessionImpl::ensureMocIncludePlaceholders(const QString& sourceCode, const QDir& baseDirectory)
{
    for (const QString& filePath : missingMocIncludes(sourceCode, baseDirectory)) {
        QFileInfo fileInfo(filePath);
        if (!fileInfo.absoluteDir().exists()) {
            continue;
        }
        if (writeTextFile(filePath, QString())) {
            standardOutput << trMessage(QStringLiteral("placeholder を作成しました: %1"),
                                        QStringLiteral("Created placeholder: %1"))
                                  .arg(filePath)
                           << Qt::endl;
        }
    }
}

void ReplSessionImpl::loadLibrary(const QString& path, const QString& historyEntry)
{
    if (path.isEmpty()) {
        reportError(trMessage(QStringLiteral(".loadlib にはライブラリパスが必要です。"),
                              QStringLiteral(".loadlib requires a library path.")));
        return;
    }

    const QString pragma = QStringLiteral("#pragma cling load(\"%1\")").arg(quoteForPragma(path));
    appendSource(pragma);
    interpreter.sendSource(pragma);
    addHistoryEntry(historyEntry);
}

void ReplSessionImpl::includeHeader(const QString& header, const QString& historyEntry)
{
    if (header.isEmpty()) {
        reportError(trMessage(QStringLiteral(".include にはヘッダ名が必要です。"),
                              QStringLiteral(".include requires a header name.")));
        return;
    }

    QString includeDirective;
    if (header.startsWith(QLatin1Char('<')) || header.startsWith(QLatin1Char('"'))) {
        includeDirective = QStringLiteral("#include %1").arg(header);
    } else {
        includeDirective = QStringLiteral("#include <%1>").arg(header);
    }

    appendSource(includeDirective);
    interpreter.sendSource(includeDirective);
    addHistoryEntry(historyEntry);
}

void ReplSessionImpl::addIncludePath(const QString& path, const QString& historyEntry)
{
    if (path.isEmpty()) {
        reportError(trMessage(QStringLiteral(".i には include path が必要です。"),
                              QStringLiteral(".i requires an include path.")));
        return;
    }

    const QString pragma = includePathPragmaForPath(path);
    if (!includePathPragmas.contains(pragma)) {
        includePathPragmas.append(pragma);
    }
    interpreter.sendSource(pragma);
    standardOutput << trMessage(QStringLiteral("include path を追加しました: %1"),
                                QStringLiteral("Added include path: %1"))
                          .arg(path)
                   << Qt::endl;
    addHistoryEntry(historyEntry);
}

void ReplSessionImpl::handleIncludePathCommand(const QString& argument, const QString& historyEntry)
{
    const QString trimmedArgument = argument.trimmed();
    if (trimmedArgument.isEmpty()) {
        showIncludePaths();
        addHistoryEntry(historyEntry);
        return;
    }

    if (trimmedArgument == QStringLiteral("edit")) {
        if (editIncludePaths()) {
            addHistoryEntry(historyEntry);
        }
        return;
    }

    addIncludePath(argument, historyEntry);
}

void ReplSessionImpl::showIncludePaths() const
{
    const QStringList paths = includePathsFromPragmas(includePathPragmas);
    if (paths.isEmpty()) {
        standardOutput << trMessage(QStringLiteral("インクルードパスはありません。"),
                                    QStringLiteral("No include paths."))
                       << Qt::endl;
        return;
    }

    standardOutput << trMessage(QStringLiteral("インクルードパス:"),
                                QStringLiteral("Include paths:"))
                   << Qt::endl;
    const int numberWidth = QString::number(paths.size()).size();
    for (int index = 0; index < paths.size(); ++index) {
        standardOutput << QStringLiteral("%1  %2")
                              .arg(index + 1, numberWidth)
                              .arg(paths.at(index))
                       << Qt::endl;
    }
}

bool ReplSessionImpl::editIncludePaths()
{
    QTemporaryFile includePathFile(QDir::tempPath() + QStringLiteral("/icpp_include_paths_XXXXXX.txt"));
    if (!includePathFile.open()) {
        reportError(trMessage(QStringLiteral("インクルードパス編集用の一時ファイルを開けません。"),
                              QStringLiteral("Cannot open a temporary file for include path editing.")));
        return false;
    }

    QTextStream outputStream(&includePathFile);
    outputStream << trMessage(
                        QStringLiteral("# インクルードパスを編集します。1 行に 1 パスを書いてください。\n"
                                       "# # で始まる行と空行は無視します。\n\n"),
                        QStringLiteral("# Edit include paths. Keep one path per line.\n"
                                       "# Lines starting with # and empty lines are ignored.\n\n"));
    for (const QString& path : includePathsFromPragmas(includePathPragmas)) {
        outputStream << path << Qt::endl;
    }
    outputStream.flush();

    const QString includePathFilePath = includePathFile.fileName();
    includePathFile.close();

    if (!runEditorForFile(includePathFilePath)) {
        return false;
    }

    const std::optional<QString> editedText = readTextFile(includePathFilePath);
    if (!editedText.has_value()) {
        return false;
    }

    const QStringList previousPaths = includePathsFromPragmas(includePathPragmas);
    QStringList editedPaths;
    const QStringList lines = editedText->split(QLatin1Char('\n'));
    for (const QString& line : lines) {
        const QString trimmedLine = line.trimmed();
        if (trimmedLine.isEmpty() || trimmedLine.startsWith(QLatin1Char('#'))) {
            continue;
        }
        if (!editedPaths.contains(trimmedLine)) {
            editedPaths.append(trimmedLine);
        }
    }

    includePathPragmas.clear();
    for (const QString& path : editedPaths) {
        const QString pragma = includePathPragmaForPath(path);
        includePathPragmas.append(pragma);
        interpreter.sendSource(pragma);
    }

    standardOutput << trMessage(QStringLiteral("インクルードパスを更新しました: %1 件"),
                                QStringLiteral("Updated include paths: %1"))
                          .arg(includePathPragmas.size())
                   << Qt::endl;
    for (const QString& previousPath : previousPaths) {
        if (!editedPaths.contains(previousPath)) {
            standardOutput << trMessage(
                                  QStringLiteral("削除したインクルードパスは次回 .r 後に interpreter 側へ反映されます。"),
                                  QStringLiteral("Removed include paths take effect in the interpreter after .r."))
                           << Qt::endl;
            break;
        }
    }
    showIncludePaths();
    return true;
}

void ReplSessionImpl::sendPragma(const QString& argument, const QString& historyEntry)
{
    if (argument.isEmpty()) {
        reportError(trMessage(QStringLiteral(".pragma には引数が必要です。"),
                              QStringLiteral(".pragma requires an argument.")));
        return;
    }

    const QString pragma = QStringLiteral("#pragma cling %1").arg(argument);
    appendSource(pragma);
    interpreter.sendSource(pragma);
    addHistoryEntry(historyEntry);
}

bool ReplSessionImpl::executeExternalCommand(const QString& command)
{
    QProcess process;
    process.setProcessChannelMode(QProcess::ForwardedChannels);

#if defined(Q_OS_WIN)
    const QString shellProgram = QStringLiteral("cmd.exe");
    const QStringList shellArguments{QStringLiteral("/C"), command};
#else
    const QString shellProgram = QStringLiteral("/bin/sh");
    const QStringList shellArguments{QStringLiteral("-lc"), command};
#endif

    process.start(shellProgram, shellArguments);
    if (!process.waitForStarted()) {
        reportError(trMessage(QStringLiteral("外部コマンドを起動できません: %1"),
                              QStringLiteral("Cannot start external command: %1"))
                        .arg(command));
        return false;
    }

    process.waitForFinished(-1);

    if (process.exitStatus() != QProcess::NormalExit) {
        reportError(trMessage(QStringLiteral("外部コマンドがクラッシュしました: %1"),
                              QStringLiteral("External command crashed: %1"))
                        .arg(command));
        return false;
    }

    if (process.exitCode() != 0) {
        reportError(
            trMessage(QStringLiteral("外部コマンドがステータス %1 で終了しました: %2"),
                      QStringLiteral("External command exited with status %1: %2"))
                .arg(process.exitCode())
                .arg(command));
        return false;
    }

    return true;
}

void ReplSessionImpl::runExternalCommand(const QString& command, const QString& historyEntry)
{
    if (command.isEmpty()) {
        reportError(trMessage(QStringLiteral(".! にはコマンドが必要です。"),
                              QStringLiteral(".! requires a command.")));
        return;
    }

    executeExternalCommand(command);
    addHistoryEntry(historyEntry);
}

void ReplSessionImpl::generateQtFilesAndRun(const QString& historyEntry)
{
    if (visibleBufferHasUnsavedChanges() && !rememberedVisibleBufferFilePath.isEmpty()) {
        standardError << trMessage(
                             QStringLiteral("警告: 編集バッファに未保存の変更があります。.gen の前に .save することを推奨します。"),
                             QStringLiteral("Warning: edit buffer has unsaved changes. Run .save before .gen if generated files depend on it."))
                      << Qt::endl;
    }

    if (!executeExternalCommand(QStringLiteral("run_all"))) {
        addHistoryEntry(historyEntry);
        return;
    }

    runVisibleSourceCodeWithoutAutogen();
    addHistoryEntry(historyEntry);
}

bool ReplSessionImpl::prepareAutogenIfEnabled()
{
    if (!autogenEnabled || !qtGeneratedFilesMayBeNeeded()) {
        return true;
    }

    if (visibleBufferHasUnsavedChanges()) {
        if (rememberedVisibleBufferFilePath.isEmpty()) {
            standardError << trMessage(
                                 QStringLiteral("警告: autogen には保存済みファイルが必要です。.save <file> を実行してください。"),
                                 QStringLiteral("Warning: autogen needs a saved file. Use .save <file> first."))
                          << Qt::endl;
        } else if (confirmYesNo(trMessage(
                       QStringLiteral("警告: autogen の前に編集バッファを %1 に保存しますか? [Y/N] "),
                       QStringLiteral("Warning: save edit buffer to %1 before autogen? [Y/N] "))
                       .arg(rememberedVisibleBufferFilePath))) {
            if (!writeTextFile(rememberedVisibleBufferFilePath, visibleSourceCode())) {
                return false;
            }
            lastSavedVisibleSourceCode = visibleSourceCode();
            rememberTouchedSourceFile(rememberedVisibleBufferFilePath);
            standardOutput << trMessage(QStringLiteral("保存しました: %1"),
                                        QStringLiteral("Saved: %1"))
                                  .arg(QFileInfo(rememberedVisibleBufferFilePath).fileName())
                           << Qt::endl;
        } else {
            return true;
        }
    }

    standardOutput << trMessage(QStringLiteral("autogen: run_all"),
                                QStringLiteral("Autogen: run_all"))
                   << Qt::endl;
    return executeExternalCommand(QStringLiteral("run_all"));
}

bool ReplSessionImpl::qtGeneratedFilesMayBeNeeded() const
{
    const QString sourceCode = visibleSourceCode();
    static const QRegularExpression mocIncludePattern(
        QStringLiteral(R"re(#\s*include\s*[<"][^>"]+\.moc[>"])re"));
    if (sourceCode.contains(QStringLiteral("Q_" "OBJECT"))
        || mocIncludePattern.match(sourceCode).hasMatch()) {
        return true;
    }

    for (const QString& filePath : registeredSourceFilePaths) {
        const QString suffix = QFileInfo(filePath).suffix().toLower();
        if (suffix == QStringLiteral("ui") || suffix == QStringLiteral("qrc")) {
            return true;
        }
        const std::optional<QString> sourceCode = readTextFile(filePath);
        if (sourceCode.has_value()
            && (sourceCode->contains(QStringLiteral("Q_" "OBJECT"))
                || mocIncludePattern.match(*sourceCode).hasMatch())) {
            return true;
        }
    }

    const QDir currentDirectory = QDir::current();
    return !currentDirectory.entryList(QStringList() << QStringLiteral("*.ui")
                                                     << QStringLiteral("*.qrc"),
                                       QDir::Files)
                .isEmpty();
}

void ReplSessionImpl::warnIfQtGeneratedFilesMayBeStale() const
{
    if (qtGeneratedFilesMayBeNeeded()) {
        standardError
            << trMessage(QStringLiteral("警告: Qt 生成物が必要な可能性があります。.gen で moc/uic/rcc を再生成して再起動してください。"),
                         QStringLiteral("Warning: Qt generated files may be needed. Run .gen to regenerate moc/uic/rcc and restart."))
            << Qt::endl;
    }
}

} // namespace icpp
