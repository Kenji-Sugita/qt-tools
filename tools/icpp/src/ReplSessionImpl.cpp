#include "icpp/ReplSessionImpl.h"

#include <QProcess>
#include <QSettings>
#include <QTemporaryFile>
#include <QTextStream>

#include <optional>
#include <utility>

namespace icpp {

namespace {

QString normalizedNewFileNameMode(QString mode)
{
    mode = mode.trimmed().toLower();
    if (mode == QStringLiteral("asis")) {
        return mode;
    }
    return QStringLiteral("lower");
}

} // namespace

ReplSessionImpl::ReplSessionImpl(Engine engine,
                                 QString programPath,
                                 QStringList interpreterArguments,
                                 HelpLanguage language,
                                 QTextStream& output,
                                 QTextStream& error)
    : currentEngine(engine)
    , interpreter(engine, std::move(programPath), std::move(interpreterArguments), language, output, error)
    , helpLanguage(language)
    , standardOutput(output)
    , standardError(error)
    , quietMode(QSettings(QStringLiteral("icpp"), QStringLiteral("icpp"))
                    .value(QStringLiteral("quiet"), false)
                    .toBool())
    , newFileNameMode(normalizedNewFileNameMode(
          QSettings(QStringLiteral("icpp"), QStringLiteral("icpp"))
              .value(QStringLiteral("newFileName"), QStringLiteral("lower"))
              .toString()))
{
}

int ReplSessionImpl::run()
{
    if (!interpreter.start()) {
        return 1;
    }

    while (!shouldQuit) {
        const std::optional<QString> line = readConsoleLine(prompt());
        if (!line.has_value()) {
            break;
        }

        const QString trimmedLine = line->trimmed();
        if (trimmedLine.isEmpty()) {
            continue;
        }

        if (trimmedLine.startsWith(QLatin1Char('.'))) {
            handleCommand(trimmedLine);
        } else {
            if (appendNormalInputToBuffer) {
                appendSource(*line);
            }
            interpreter.sendSource(*line);
            addHistoryEntry(*line);
        }
    }

    return 0;
}

QString ReplSessionImpl::prompt() const
{
    if (appendNormalInputToBuffer) {
        return QStringLiteral("icpp[%1 +b]> ").arg(engineName(currentEngine));
    }
    return QStringLiteral("icpp[%1]> ").arg(engineName(currentEngine));
}

QString ReplSessionImpl::usageText() const
{
    return QString::fromUtf8(helpLanguage == HelpLanguage::Japanese ? kUsageTextJapanese
                                                                    : kUsageTextEnglish);
}

void ReplSessionImpl::showHelp()
{
    const QString text = usageText();
    if (!showTextInPager(text, true)) {
        standardOutput << text << Qt::endl;
    }
}

bool ReplSessionImpl::showTextInPager(const QString& text, bool forcePager)
{
    if (!shouldUsePager()) {
        return false;
    }
    if (!forcePager && text.count(QLatin1Char('\n')) < 24) {
        return false;
    }

    QStringList pagerParts = QProcess::splitCommand(pagerCommand());
    if (pagerParts.isEmpty()) {
        return false;
    }

    QTemporaryFile helpFile(QDir::tempPath() + QStringLiteral("/icpp_help_XXXXXX.txt"));
    if (!helpFile.open()) {
        return false;
    }

    QTextStream helpOutput(&helpFile);
    helpOutput << text << Qt::endl;
    helpOutput.flush();
    const QString helpFilePath = helpFile.fileName();
    helpFile.close();

    const QString pagerProgram = pagerParts.takeFirst();
    pagerParts.append(helpFilePath);

    QProcess pagerProcess;
    pagerProcess.setInputChannelMode(QProcess::ForwardedInputChannel);
    pagerProcess.setProcessChannelMode(QProcess::ForwardedChannels);
    pagerProcess.start(pagerProgram, pagerParts);
    if (!pagerProcess.waitForStarted()) {
        return false;
    }
    pagerProcess.waitForFinished(-1);
    return pagerProcess.exitStatus() == QProcess::NormalExit;
}

QString ReplSessionImpl::trMessage(const QString& japanese, const QString& english) const
{
    return helpLanguage == HelpLanguage::Japanese ? japanese : english;
}

void ReplSessionImpl::handleCommand(const QString& trimmedLine)
{
    const ParsedCommand parsedCommand = parseCommand(trimmedLine);
    const QString& commandName = parsedCommand.commandName;

    if (commandName == QStringLiteral(".help") || commandName == QStringLiteral(".h")
        || commandName == QStringLiteral(".?")) {
        if (!parsedCommand.argument.isEmpty()) {
            const QString helpTarget = parsedCommand.argument.trimmed();
            const QString normalizedHelpTarget = helpTarget.toLower();
            const QString commandHelpTarget =
                normalizedHelpTarget == QStringLiteral("-h")
                    || normalizedHelpTarget == QStringLiteral("--help")
                    || normalizedHelpTarget == QStringLiteral("help")
                ? QStringLiteral(".help")
                : helpTarget;
            if (!showCommandHelp(commandHelpTarget, trimmedLine)) {
                reportError(trMessage(QStringLiteral("ヘルプが見つかりません: %1"),
                                      QStringLiteral("No help for: %1"))
                                .arg(parsedCommand.argument));
            }
            return;
        }
        showHelp();
        addHistoryEntry(trimmedLine);
        return;
    }

    if (showCommandHelpIfRequested(commandName, parsedCommand.argument, trimmedLine)) {
        return;
    }

    if (commandName == QStringLiteral(".quit") || commandName == QStringLiteral(".q")) {
        shouldQuit = confirmQuitIfVisibleBufferHasUnsavedChanges();
        return;
    }

    if (commandName == QStringLiteral(".args")) {
        interpreter.showArguments();
        addHistoryEntry(trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".status") || commandName == QStringLiteral(".st")) {
        showStatus();
        addHistoryEntry(trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".errors")) {
        showRecentErrors();
        addHistoryEntry(trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".doctor")) {
        showDoctor(trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".generated")) {
        showGeneratedFiles(trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".examples")) {
        showExamples(trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".where")) {
        showWhere(trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".runorder")) {
        showRunOrder(trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".qt")) {
        showQtStatus(trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".clean")) {
        cleanGeneratedFiles(trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".buffer") || commandName == QStringLiteral(".b")) {
        handleBufferAppendMode(commandName, parsedCommand.argument, trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".autogen")) {
        handleAutogenMode(parsedCommand.argument, trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".quiet")) {
        handleQuietMode(parsedCommand.argument, trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".newname")) {
        handleNewFileNameMode(parsedCommand.argument, trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".lang") || commandName == QStringLiteral(".language")) {
        handleHelpLanguageMode(parsedCommand.argument, trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".show") || commandName == QStringLiteral(".sh")) {
        showVisibleSourceCode(parsedCommand.argument);
        addHistoryEntry(trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".files")) {
        showRegisteredSourceFiles();
        addHistoryEntry(trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".paste")) {
        pasteClipboardIntoVisibleBuffer(trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".copy")) {
        copyVisibleBufferToClipboard(trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".add")) {
        addRegisteredSourceFile(parsedCommand.argument, trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".drop")) {
        dropRegisteredSourceFile(parsedCommand.argument, trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".clearfiles")) {
        clearRegisteredSourceFiles();
        addHistoryEntry(trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".discard") || commandName == QStringLiteral(".d")) {
        discardVisibleSourceCode();
        addHistoryEntry(trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".run") || commandName == QStringLiteral(".r")) {
        if (parsedCommand.argument == QStringLiteral("edit")) {
            if (!editRegisteredSourceFileOrder()) {
                return;
            }
        } else if (!parsedCommand.argument.isEmpty()) {
            reportError(trMessage(QStringLiteral("%1 の引数は edit のみ対応しています。"),
                                  QStringLiteral("%1 only supports the edit argument."))
                            .arg(commandName));
            return;
        }
        runVisibleSourceCode();
        addHistoryEntry(trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".gen")) {
        generateQtFilesAndRun(trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".restart")) {
        restartInterpreter();
        addHistoryEntry(trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".reset")) {
        resetSession();
        addHistoryEntry(trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".open") || commandName == QStringLiteral(".o")) {
        openFileIntoVisibleBuffer(parsedCommand.argument, trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".load") || commandName == QStringLiteral(".l")) {
        loadAndEvaluateFile(parsedCommand.argument, trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".save") || commandName == QStringLiteral(".s")) {
        saveVisibleBuffer(parsedCommand.argument, trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".edit") || commandName == QStringLiteral(".e")) {
        openEditor(parsedCommand.argument, trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".designer") || commandName == QStringLiteral(".de")) {
        openDesigner(parsedCommand.argument, trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".linguist") || commandName == QStringLiteral(".li")) {
        openLinguist(parsedCommand.argument, trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".qrc")) {
        openQrc(parsedCommand.argument, trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".qtc")) {
        openQrcWithQtCreator(parsedCommand.argument, trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".template") || commandName == QStringLiteral(".new")) {
        handleTemplateCommand(parsedCommand.argument, trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".print") || commandName == QStringLiteral(".p")) {
        printExpression(parsedCommand.argument, trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".ptype")) {
        printExpressionType(parsedCommand.argument, trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".defs")) {
        showDefinitions();
        addHistoryEntry(trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".uiinfo")) {
        showUiInfo(parsedCommand.argument, trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".preview") || commandName == QStringLiteral(".pv")) {
        previewUiFile(commandName, parsedCommand.argument, trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".inspect")) {
        inspectTopLevelWidgets(trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".eval") || commandName == QStringLiteral(".x")) {
        evaluateWithoutAppending(parsedCommand.argument, trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".append") || commandName == QStringLiteral(".a")) {
        evaluateAndAppend(parsedCommand.argument, trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".widgets")) {
        showWidgets(parsedCommand.argument, trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".closeall")) {
        closeAllWidgets(trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".loadlib")) {
        loadLibrary(parsedCommand.argument, trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".i")) {
        handleIncludePathCommand(parsedCommand.argument, trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".!")) {
        runExternalCommand(parsedCommand.argument, trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".include")) {
        includeHeader(parsedCommand.argument, trimmedLine);
        return;
    }

    if (commandName == QStringLiteral(".pragma")) {
        sendPragma(parsedCommand.argument, trimmedLine);
        return;
    }

    reportError(trMessage(QStringLiteral("不明なコマンド: %1"),
                          QStringLiteral("Unknown command: %1"))
                    .arg(commandName));
}

} // namespace icpp
