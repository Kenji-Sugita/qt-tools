#include "icpp/ReplSessionImpl.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QLibraryInfo>
#include <QProcess>
#include <QSet>
#include <QStandardPaths>
#include <QTextStream>

#include <algorithm>

namespace icpp {

namespace {

QString firstExistingCandidate(const QDir& directory, const QStringList& candidateNames)
{
    for (const QString& candidateName : candidateNames) {
        const QString candidatePath = directory.absoluteFilePath(candidateName);
        const QFileInfo candidateInfo(candidatePath);
        if (candidateInfo.exists() && candidateInfo.isFile()) {
            return candidateInfo.absoluteFilePath();
        }
    }
    return {};
}

QString generatedBaseName(const QString& completeBaseName, const QString& prefix)
{
    if (completeBaseName.startsWith(prefix)) {
        return completeBaseName.mid(prefix.size());
    }
    return {};
}

bool sourceContains(const QString& filePath, const QString& text)
{
    QFile inputFile(filePath);
    if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    return QString::fromUtf8(inputFile.readAll()).contains(text);
}

QString toolFromCommand(QString command)
{
    QStringList parts = QProcess::splitCommand(command);
    if (parts.isEmpty()) {
        return {};
    }
    return parts.takeFirst();
}

QString findPathTool(const QString& toolName)
{
    return QStandardPaths::findExecutable(toolName);
}

QString commandFirstLine(const QString& program, const QStringList& arguments)
{
    if (program.isEmpty()) {
        return {};
    }

    QProcess process;
    process.start(program, arguments);
    if (!process.waitForStarted(1000)) {
        return {};
    }
    if (!process.waitForFinished(2000)) {
        process.kill();
        process.waitForFinished();
        return {};
    }

    QString output = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
    if (output.isEmpty()) {
        output = QString::fromLocal8Bit(process.readAllStandardError()).trimmed();
    }
    return output.split(QLatin1Char('\n'), Qt::SkipEmptyParts).value(0).trimmed();
}

QString toolVersionLine(const QString& toolName)
{
    const QString toolPath = findPathTool(toolName);
    if (toolPath.isEmpty()) {
        return {};
    }
    return commandFirstLine(toolPath, QStringList{QStringLiteral("-v")});
}

QString qtpathsValue(const QString& option)
{
    const QString qtpaths = findPathTool(QStringLiteral("qtpaths"));
    if (qtpaths.isEmpty()) {
        return {};
    }
    return commandFirstLine(qtpaths, QStringList{option});
}

QString statusMark(bool ok)
{
    return ok ? QStringLiteral("OK") : QStringLiteral("NG");
}

} // namespace

QList<GeneratedFileInfo> ReplSessionImpl::generatedFilesInCurrentDirectory() const
{
    QList<GeneratedFileInfo> result;
    QSet<QString> seenPaths;
    const QDir currentDirectory = QDir::current();
    const QFileInfoList entries = currentDirectory.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

    auto appendGeneratedFile = [&](const QString& kind,
                                   const QFileInfo& generatedInfo,
                                   const QString& sourcePath) {
        const QString generatedPath = generatedInfo.absoluteFilePath();
        if (seenPaths.contains(generatedPath)) {
            return;
        }
        seenPaths.insert(generatedPath);

        GeneratedFileInfo generatedFile;
        generatedFile.kind = kind;
        generatedFile.filePath = generatedPath;
        generatedFile.sourcePath = sourcePath;
        generatedFile.sourceExists = !sourcePath.isEmpty();
        if (generatedFile.sourceExists) {
            const std::optional<QString> generatedText = readTextFile(generatedInfo.absoluteFilePath());
            generatedFile.stale = generatedInfo.size() == 0
                                  || (generatedText.has_value() && generatedText->trimmed().isEmpty())
                                  || generatedInfo.lastModified() < QFileInfo(sourcePath).lastModified();
        }
        result.append(generatedFile);
    };

    for (const QFileInfo& entry : entries) {
        const QString fileName = entry.fileName();
        const QString completeBaseName = entry.completeBaseName();
        const QString suffix = entry.suffix().toLower();

        if (fileName.startsWith(QStringLiteral("moc_")) && suffix == QStringLiteral("cpp")) {
            const QString baseName = generatedBaseName(completeBaseName, QStringLiteral("moc_"));
            const QString sourcePath = firstExistingCandidate(
                currentDirectory,
                QStringList{QStringLiteral("%1.h").arg(baseName),
                            QStringLiteral("%1.hpp").arg(baseName),
                            QStringLiteral("%1.hh").arg(baseName),
                            QStringLiteral("%1.cpp").arg(baseName),
                            QStringLiteral("%1.cc").arg(baseName),
                            QStringLiteral("%1.cxx").arg(baseName)});
            appendGeneratedFile(QStringLiteral("moc"), entry, sourcePath);
            continue;
        }

        if (fileName.startsWith(QStringLiteral("qrc_")) && suffix == QStringLiteral("cpp")) {
            const QString baseName = generatedBaseName(completeBaseName, QStringLiteral("qrc_"));
            const QString sourcePath = firstExistingCandidate(
                currentDirectory,
                QStringList{QStringLiteral("%1.qrc").arg(baseName)});
            appendGeneratedFile(QStringLiteral("rcc"), entry, sourcePath);
            continue;
        }

        if (fileName.startsWith(QStringLiteral("ui_")) && suffix == QStringLiteral("h")) {
            const QString baseName = generatedBaseName(completeBaseName, QStringLiteral("ui_"));
            const QString sourcePath = firstExistingCandidate(
                currentDirectory,
                QStringList{QStringLiteral("%1.ui").arg(baseName)});
            appendGeneratedFile(QStringLiteral("uic"), entry, sourcePath);
            continue;
        }

        if (suffix == QStringLiteral("qm")) {
            const QString sourcePath = firstExistingCandidate(
                currentDirectory,
                QStringList{QStringLiteral("%1.ts").arg(entry.completeBaseName())});
            appendGeneratedFile(QStringLiteral("lrelease"), entry, sourcePath);
        }
    }

    std::sort(result.begin(), result.end(), [](const GeneratedFileInfo& left,
                                               const GeneratedFileInfo& right) {
        if (left.kind != right.kind) {
            return left.kind < right.kind;
        }
        return QFileInfo(left.filePath).fileName() < QFileInfo(right.filePath).fileName();
    });
    return result;
}

QString ReplSessionImpl::generatedFileStateText(const GeneratedFileInfo& generatedFile) const
{
    if (!generatedFile.sourceExists) {
        return trMessage(QStringLiteral("元ファイル不明"),
                         QStringLiteral("source unknown"));
    }
    if (generatedFile.stale) {
        return trMessage(QStringLiteral("古い可能性あり"),
                         QStringLiteral("possibly stale"));
    }
    return trMessage(QStringLiteral("最新"),
                     QStringLiteral("current"));
}

void ReplSessionImpl::showGeneratedFiles(const QString& historyEntry)
{
    const QList<GeneratedFileInfo> generatedFiles = generatedFilesInCurrentDirectory();
    standardOutput << trMessage(QStringLiteral("生成物: %1"),
                                QStringLiteral("generated files: %1"))
                          .arg(QDir::currentPath())
                   << Qt::endl;
    if (generatedFiles.isEmpty()) {
        standardOutput << QStringLiteral("  %1")
                              .arg(trMessage(QStringLiteral("(なし)"),
                                             QStringLiteral("(none)")))
                       << Qt::endl;
        addHistoryEntry(historyEntry);
        return;
    }

    for (const GeneratedFileInfo& generatedFile : generatedFiles) {
        const QString sourceName = generatedFile.sourceExists
                                       ? QFileInfo(generatedFile.sourcePath).fileName()
                                       : trMessage(QStringLiteral("(不明)"),
                                                   QStringLiteral("(unknown)"));
        standardOutput << QStringLiteral("  %1  %2  <- %3  [%4]")
                              .arg(generatedFile.kind, -8)
                              .arg(QFileInfo(generatedFile.filePath).fileName(), -24)
                              .arg(sourceName)
                              .arg(generatedFileStateText(generatedFile))
                       << Qt::endl;
    }
    addHistoryEntry(historyEntry);
}

void ReplSessionImpl::cleanGeneratedFiles(const QString& historyEntry)
{
    const QList<GeneratedFileInfo> generatedFiles = generatedFilesInCurrentDirectory();
    if (generatedFiles.isEmpty()) {
        standardOutput << trMessage(QStringLiteral("削除する生成物はありません。"),
                                    QStringLiteral("No generated files to delete."))
                       << Qt::endl;
        addHistoryEntry(historyEntry);
        return;
    }

    standardOutput << trMessage(QStringLiteral("削除する生成物:"),
                                QStringLiteral("Generated files to delete:"))
                   << Qt::endl;
    for (const GeneratedFileInfo& generatedFile : generatedFiles) {
        standardOutput << QStringLiteral("  %1").arg(QFileInfo(generatedFile.filePath).fileName())
                       << Qt::endl;
    }

    if (!confirmYesNo(trMessage(QStringLiteral("これらの生成物を削除しますか? [Y/N] "),
                                QStringLiteral("Delete these generated files? [Y/N] ")))) {
        standardOutput << trMessage(QStringLiteral("削除を中止しました。"),
                                    QStringLiteral("Delete canceled."))
                       << Qt::endl;
        addHistoryEntry(historyEntry);
        return;
    }

    for (const GeneratedFileInfo& generatedFile : generatedFiles) {
        if (QFile::remove(generatedFile.filePath)) {
            standardOutput << trMessage(QStringLiteral("削除しました: %1"),
                                        QStringLiteral("Deleted: %1"))
                                  .arg(QFileInfo(generatedFile.filePath).fileName())
                           << Qt::endl;
        } else {
            reportError(trMessage(QStringLiteral("削除できません: %1"),
                                  QStringLiteral("Cannot delete: %1"))
                            .arg(generatedFile.filePath));
        }
    }
    addHistoryEntry(historyEntry);
}

void ReplSessionImpl::showDoctor(const QString& historyEntry)
{
    standardOutput << trMessage(QStringLiteral("icpp doctor"),
                                QStringLiteral("icpp doctor"))
                   << Qt::endl;
    interpreter.showArguments();
    standardOutput << trMessage(QStringLiteral("カレントディレクトリ: %1"),
                                QStringLiteral("current directory: %1"))
                          .arg(QDir::currentPath())
                   << Qt::endl;
    standardOutput << Qt::endl;

    standardOutput << trMessage(QStringLiteral("icpp Qt:"),
                                QStringLiteral("icpp Qt:"))
                   << Qt::endl;
    standardOutput << QStringLiteral("  version: %1")
                          .arg(QLibraryInfo::version().toString())
                   << Qt::endl;
    standardOutput << QStringLiteral("  prefix: %1")
                          .arg(QLibraryInfo::path(QLibraryInfo::PrefixPath))
                   << Qt::endl;
    standardOutput << QStringLiteral("  binaries: %1")
                          .arg(QLibraryInfo::path(QLibraryInfo::BinariesPath))
                   << Qt::endl;
    standardOutput << QStringLiteral("  QtUiTools: %1")
                          .arg(qtUiToolsLibraryPath().isEmpty()
                                   ? trMessage(QStringLiteral("(見つかりません)"),
                                               QStringLiteral("(not found)"))
                                   : qtUiToolsLibraryPath())
                   << Qt::endl;
    standardOutput << Qt::endl;

    standardOutput << trMessage(QStringLiteral("active Qt tools:"),
                                QStringLiteral("active Qt tools:"))
                   << Qt::endl;
    standardOutput << QStringLiteral("  qtpaths: %1")
                          .arg(findPathTool(QStringLiteral("qtpaths")).isEmpty()
                                   ? trMessage(QStringLiteral("(見つかりません)"),
                                               QStringLiteral("(not found)"))
                                   : findPathTool(QStringLiteral("qtpaths")))
                   << Qt::endl;
    standardOutput << QStringLiteral("  qtpaths version: %1")
                          .arg(qtpathsValue(QStringLiteral("--qt-version")).isEmpty()
                                   ? trMessage(QStringLiteral("(不明)"), QStringLiteral("(unknown)"))
                                   : qtpathsValue(QStringLiteral("--qt-version")))
                   << Qt::endl;
    standardOutput << QStringLiteral("  qtpaths prefix: %1")
                          .arg(qtpathsValue(QStringLiteral("--install-prefix")).isEmpty()
                                   ? trMessage(QStringLiteral("(不明)"), QStringLiteral("(unknown)"))
                                   : qtpathsValue(QStringLiteral("--install-prefix")))
                   << Qt::endl;
    for (const QString& toolName : {QStringLiteral("moc"), QStringLiteral("uic"),
                                    QStringLiteral("rcc")}) {
        const QString toolPath = findPathTool(toolName);
        standardOutput << QStringLiteral("  %1: %2")
                              .arg(toolName,
                                   toolPath.isEmpty()
                                       ? trMessage(QStringLiteral("(見つかりません)"),
                                                   QStringLiteral("(not found)"))
                                       : QStringLiteral("%1 [%2]")
                                             .arg(toolPath,
                                                  toolVersionLine(toolName).isEmpty()
                                                      ? trMessage(QStringLiteral("version 不明"),
                                                                  QStringLiteral("version unknown"))
                                                      : toolVersionLine(toolName)))
                       << Qt::endl;
    }
    standardOutput << Qt::endl;

    auto printTool = [&](const QString& label, const QString& path) {
        const bool ok = !path.isEmpty() && QFileInfo(path).exists();
        standardOutput << QStringLiteral("  [%1] %2: %3")
                              .arg(statusMark(ok), label, ok ? path : trMessage(QStringLiteral("(未検出)"),
                                                                                 QStringLiteral("(not found)")))
                       << Qt::endl;
    };

    standardOutput << trMessage(QStringLiteral("ツール:"),
                                QStringLiteral("tools:"))
                   << Qt::endl;
    printTool(QStringLiteral("run_all"), QStandardPaths::findExecutable(QStringLiteral("run_all")));
    printTool(QStringLiteral("designer"), toolFromCommand(determineDesignerCommand()));
    printTool(QStringLiteral("linguist"), toolFromCommand(determineLinguistCommand()));
    printTool(QStringLiteral("qtcreator"), toolFromCommand(determineQtCreatorCommand()));
    printTool(QStringLiteral("lrelease"), findPathTool(QStringLiteral("lrelease")));
    printTool(QStringLiteral("lupdate"), findPathTool(QStringLiteral("lupdate")));

    standardOutput << Qt::endl;
    standardOutput << trMessage(QStringLiteral("現在ディレクトリ:"),
                                QStringLiteral("current directory:"))
                   << Qt::endl;
    const QDir currentDirectory = QDir::current();
    const int uiCount = currentDirectory.entryList(QStringList{QStringLiteral("*.ui")}, QDir::Files).size();
    const int qrcCount = currentDirectory.entryList(QStringList{QStringLiteral("*.qrc")}, QDir::Files).size();
    const int tsCount = currentDirectory.entryList(QStringList{QStringLiteral("*.ts")}, QDir::Files).size();
    const int generatedCount = generatedFilesInCurrentDirectory().size();
    standardOutput << QStringLiteral("  .ui: %1, .qrc: %2, .ts: %3, generated: %4")
                          .arg(uiCount)
                          .arg(qrcCount)
                          .arg(tsCount)
                          .arg(generatedCount)
                   << Qt::endl;
    standardOutput << QStringLiteral("  %1: %2")
                          .arg(trMessage(QStringLiteral("登録ファイル数"),
                                         QStringLiteral("registered files")))
                          .arg(registeredSourceFilePaths.size())
                   << Qt::endl;

    addHistoryEntry(historyEntry);
}

void ReplSessionImpl::showExamples(const QString& historyEntry)
{
    const QString text = trMessage(
        QStringLiteral(R"(examples:

  式を試す:
    .p QString("abc").toUpper()

  実ファイルで QWidget を試す:
    .new widget counter
    .add counter.cpp
    .gen
    static auto w = go();
    w->show();
    w->raise();

  Designer UI:
    .new ui form
    .designer form.ui
    .uiinfo form.ui

  resource:
    .new qrc resources
    .qrc resources.qrc
    .gen

  翻訳:
    .! lupdate widget.cpp -ts app_ja.ts
    .linguist app_ja.ts
    .! lrelease app_ja.ts
    .gen

  表示中 widget を調べる:
    .widgets
    .inspect)"),
        QStringLiteral(R"(examples:

  Try an expression:
    .p QString("abc").toUpper()

  Try a QWidget with real files:
    .new widget counter
    .add counter.cpp
    .gen
    static auto w = go();
    w->show();
    w->raise();

  Designer UI:
    .new ui form
    .designer form.ui
    .uiinfo form.ui

  Resource:
    .new qrc resources
    .qrc resources.qrc
    .gen

  Translation:
    .! lupdate widget.cpp -ts app_ja.ts
    .linguist app_ja.ts
    .! lrelease app_ja.ts
    .gen

  Inspect visible widgets:
    .widgets
    .inspect)"))
                             .arg(QString());
    if (!showTextInPager(text, true)) {
        standardOutput << text << Qt::endl;
    }
    addHistoryEntry(historyEntry);
}

void ReplSessionImpl::showWhere(const QString& historyEntry)
{
    const QString sourceCode = visibleSourceCode();
    const int lineCount = sourceCode.isEmpty() ? 0 : sourceCode.count(QLatin1Char('\n')) + 1;
    standardOutput << trMessage(QStringLiteral("where:"),
                                QStringLiteral("where:"))
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("  dir: %1"),
                                QStringLiteral("  dir: %1"))
                          .arg(QDir::currentPath())
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("  save: %1"),
                                QStringLiteral("  save: %1"))
                          .arg(rememberedVisibleBufferFilePath.isEmpty()
                                   ? trMessage(QStringLiteral("(なし)"), QStringLiteral("(none)"))
                                   : rememberedVisibleBufferFilePath)
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("  buffer: %1 行, %2"),
                                QStringLiteral("  buffer: %1 lines, %2"))
                          .arg(lineCount)
                          .arg(visibleBufferHasUnsavedChanges()
                                   ? trMessage(QStringLiteral("変更あり"), QStringLiteral("modified"))
                                   : trMessage(QStringLiteral("変更なし"), QStringLiteral("clean")))
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("  files: %1"),
                                QStringLiteral("  files: %1"))
                          .arg(registeredSourceFilePaths.size())
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("  generated: %1"),
                                QStringLiteral("  generated: %1"))
                          .arg(generatedFilesInCurrentDirectory().size())
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("  autogen: %1"),
                                QStringLiteral("  autogen: %1"))
                          .arg(autogenEnabled ? QStringLiteral("on") : QStringLiteral("off"))
                   << Qt::endl;
    addHistoryEntry(historyEntry);
}

void ReplSessionImpl::showRunOrder(const QString& historyEntry)
{
    standardOutput << trMessage(QStringLiteral("run order:"),
                                QStringLiteral("run order:"))
                   << Qt::endl;
    if (registeredSourceFilePaths.isEmpty()) {
        standardOutput << QStringLiteral("  %1")
                              .arg(trMessage(QStringLiteral("(登録ファイルなし)"),
                                             QStringLiteral("(no registered files)")))
                       << Qt::endl;
    } else {
        for (int index = 0; index < registeredSourceFilePaths.size(); ++index) {
            standardOutput << QStringLiteral("  %1  %2")
                                  .arg(index + 1)
                                  .arg(registeredSourceFilePaths.at(index))
                           << Qt::endl;
        }
    }
    standardOutput << QStringLiteral("  %1")
                          .arg(trMessage(QStringLiteral("順番を変えるには .r edit を使います。"),
                                         QStringLiteral("Use .r edit to reorder files.")))
                   << Qt::endl;
    addHistoryEntry(historyEntry);
}

void ReplSessionImpl::showQtStatus(const QString& historyEntry)
{
    const QDir currentDirectory = QDir::current();
    const int uiCount = currentDirectory.entryList(QStringList{QStringLiteral("*.ui")}, QDir::Files).size();
    const int qrcCount = currentDirectory.entryList(QStringList{QStringLiteral("*.qrc")}, QDir::Files).size();
    const int tsCount = currentDirectory.entryList(QStringList{QStringLiteral("*.ts")}, QDir::Files).size();

    int qObjectSourceCount = 0;
    const QFileInfoList sourceFiles = currentDirectory.entryInfoList(
        QStringList{QStringLiteral("*.h"), QStringLiteral("*.hpp"), QStringLiteral("*.hh"),
                    QStringLiteral("*.cpp"), QStringLiteral("*.cc"), QStringLiteral("*.cxx")},
        QDir::Files);
    for (const QFileInfo& sourceFile : sourceFiles) {
        if (sourceContains(sourceFile.absoluteFilePath(), QStringLiteral("Q_" "OBJECT"))) {
            ++qObjectSourceCount;
        }
    }

    const QList<GeneratedFileInfo> generatedFiles = generatedFilesInCurrentDirectory();
    int staleCount = 0;
    for (const GeneratedFileInfo& generatedFile : generatedFiles) {
        if (generatedFile.stale) {
            ++staleCount;
        }
    }

    standardOutput << trMessage(QStringLiteral("qt:"),
                                QStringLiteral("qt:"))
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("  icpp Qt version: %1"),
                                QStringLiteral("  icpp Qt version: %1"))
                          .arg(QLibraryInfo::version().toString())
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("  icpp Qt prefix: %1"),
                                QStringLiteral("  icpp Qt prefix: %1"))
                          .arg(QLibraryInfo::path(QLibraryInfo::PrefixPath))
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("  active Qt version: %1"),
                                QStringLiteral("  active Qt version: %1"))
                          .arg(qtpathsValue(QStringLiteral("--qt-version")).isEmpty()
                                   ? trMessage(QStringLiteral("(不明)"), QStringLiteral("(unknown)"))
                                   : qtpathsValue(QStringLiteral("--qt-version")))
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("  active Qt prefix: %1"),
                                QStringLiteral("  active Qt prefix: %1"))
                          .arg(qtpathsValue(QStringLiteral("--install-prefix")).isEmpty()
                                   ? trMessage(QStringLiteral("(不明)"), QStringLiteral("(unknown)"))
                                   : qtpathsValue(QStringLiteral("--install-prefix")))
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("  moc: %1"),
                                QStringLiteral("  moc: %1"))
                          .arg(toolVersionLine(QStringLiteral("moc")).isEmpty()
                                   ? trMessage(QStringLiteral("(不明)"), QStringLiteral("(unknown)"))
                                   : toolVersionLine(QStringLiteral("moc")))
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("  uic: %1"),
                                QStringLiteral("  uic: %1"))
                          .arg(toolVersionLine(QStringLiteral("uic")).isEmpty()
                                   ? trMessage(QStringLiteral("(不明)"), QStringLiteral("(unknown)"))
                                   : toolVersionLine(QStringLiteral("uic")))
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("  rcc: %1"),
                                QStringLiteral("  rcc: %1"))
                          .arg(toolVersionLine(QStringLiteral("rcc")).isEmpty()
                                   ? trMessage(QStringLiteral("(不明)"), QStringLiteral("(unknown)"))
                                   : toolVersionLine(QStringLiteral("rcc")))
                   << Qt::endl;
    standardOutput << QStringLiteral("  .ui: %1, .qrc: %2, .ts: %3")
                          .arg(uiCount)
                          .arg(qrcCount)
                          .arg(tsCount)
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("  Q_%1OBJECT source: %2"),
                                QStringLiteral("  Q_%1OBJECT sources: %2"))
                          .arg(QString())
                          .arg(qObjectSourceCount)
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("  generated: %1, stale: %2"),
                                QStringLiteral("  generated: %1, stale: %2"))
                          .arg(generatedFiles.size())
                          .arg(staleCount)
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("  autogen: %1"),
                                QStringLiteral("  autogen: %1"))
                          .arg(autogenEnabled ? QStringLiteral("on") : QStringLiteral("off"))
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("  run_all: %1"),
                                QStringLiteral("  run_all: %1"))
                          .arg(QStandardPaths::findExecutable(QStringLiteral("run_all")).isEmpty()
                                   ? trMessage(QStringLiteral("(見つかりません)"),
                                               QStringLiteral("(not found)"))
                                   : QStandardPaths::findExecutable(QStringLiteral("run_all")))
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("  QtUiTools: %1"),
                                QStringLiteral("  QtUiTools: %1"))
                          .arg(qtUiToolsLibraryPath().isEmpty()
                                   ? trMessage(QStringLiteral("(見つかりません)"),
                                               QStringLiteral("(not found)"))
                                   : qtUiToolsLibraryPath())
                   << Qt::endl;
    addHistoryEntry(historyEntry);
}

} // namespace icpp
