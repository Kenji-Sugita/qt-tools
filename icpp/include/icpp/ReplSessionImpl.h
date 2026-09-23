#pragma once

#include "icpp/InterpreterProcess.h"
#include "icpp/ReplHelpers.h"

#include <QDir>
#include <QFileInfo>
#include <QList>
#include <QString>
#include <QStringList>
#include <QXmlStreamAttributes>

#include <optional>

class QTextStream;

namespace icpp {

struct GeneratedFileInfo {
    QString kind;
    QString filePath;
    QString sourcePath;
    bool sourceExists = false;
    bool stale = false;
};

class ReplSessionImpl
{
public:
    ReplSessionImpl(Engine engine,
                QString programPath,
                QStringList interpreterArguments,
                HelpLanguage language,
                QTextStream& output,
                QTextStream& error);
    int run();

private:
    QString prompt() const;
    QString usageText() const;
    void showHelp();
    bool showTextInPager(const QString& text, bool forcePager = false);
    QString trMessage(const QString& japanese, const QString& english) const;
    bool showCommandHelpIfRequested(const QString& commandName,
                                    const QString& argument,
                                    const QString& historyEntry);
    bool showCommandHelp(const QString& commandName, const QString& historyEntry);
    QString commandHelpText(const QString& commandName) const;
    void handleCommand(const QString& trimmedLine);
    QString visibleSourceCode() const;
    void replaceVisibleSourceCode(const QString& sourceCode);
    void appendSource(const QString& sourceCode);
    void showVisibleSourceCode(const QString& rangeArgument);
    void discardVisibleSourceCode();
    void runVisibleSourceCode();
    void runVisibleSourceCodeWithoutAutogen();
    bool sourceAppearsIncomplete(const QString& sourceCode, const QString& label);
    void restartInterpreter();
    void resetSession();
    void showRegisteredSourceFiles();
    bool editRegisteredSourceFileOrder();
    void pasteClipboardIntoVisibleBuffer(const QString& historyEntry);
    void copyVisibleBufferToClipboard(const QString& historyEntry);
    void addRegisteredSourceFile(const QString& filePathArgument, const QString& historyEntry);
    void dropRegisteredSourceFile(const QString& filePathOrNumber, const QString& historyEntry);
    std::optional<QString> resolveRegisteredSourceFilePath(const QString& filePathOrNumber,
                                                           const QString& commandName);
    void editSourceFile(const QString& filePathOrNumber, const QString& historyEntry);
    void clearRegisteredSourceFiles();
    void openFileIntoVisibleBuffer(const QString& filePath, const QString& historyEntry);
    void loadAndEvaluateFile(const QString& filePath, const QString& historyEntry);
    void saveVisibleBuffer(const QString& filePathArgument, const QString& historyEntry);
    void openEditor(const QString& target, const QString& historyEntry);
    void openDesigner(const QString& filePathArgument, const QString& historyEntry);
    void openLinguist(const QString& filePathArgument, const QString& historyEntry);
    void openQrc(const QString& argument, const QString& historyEntry);
    void openQrcWithTextEditor(const QString& filePathArgument, const QString& historyEntry);
    void openQrcWithQtCreator(const QString& filePathArgument, const QString& historyEntry);
    QFileInfo validateQrcFilePath(const QString& filePathArgument, const QString& commandName);
    void finishQrcEdit(const QString& historyEntry);
    void ensureMocIncludePlaceholders(const QString& sourceCode, const QDir& baseDirectory);
    bool runEditorForFile(const QString& filePath);
    bool runDesignerForFile(const QString& filePath);
    bool runLinguistForFile(const QString& filePath);
    bool runQtCreatorForFile(const QString& filePath);
    QString determineDesignerCommand() const;
    QString determineLinguistCommand() const;
    QString determineQtCreatorCommand() const;
    QString determineEditorCommand() const;
    void handleTemplateCommand(const QString& argument, const QString& historyEntry);
    void showTemplateHelp(const QString& historyEntry);
    void loadLibrary(const QString& path, const QString& historyEntry);
    void includeHeader(const QString& header, const QString& historyEntry);
    void handleIncludePathCommand(const QString& argument, const QString& historyEntry);
    void showIncludePaths() const;
    bool editIncludePaths();
    void addIncludePath(const QString& path, const QString& historyEntry);
    void sendPragma(const QString& argument, const QString& historyEntry);
    bool executeExternalCommand(const QString& command);
    void runExternalCommand(const QString& command, const QString& historyEntry);
    void generateQtFilesAndRun(const QString& historyEntry);
    bool prepareAutogenIfEnabled();
    bool qtGeneratedFilesMayBeNeeded() const;
    void warnIfQtGeneratedFilesMayBeStale() const;
    void printExpression(const QString& expression, const QString& historyEntry);
    void printExpressionType(const QString& expression, const QString& historyEntry);
    void showDefinitions();
    void showDefinitionsForSource(const QString& sourceCode, const MemberAccessMap& accessMap);
    void evaluateAndAppend(const QString& sourceCode, const QString& historyEntry);
    void showUiInfo(const QString& filePathArgument, const QString& historyEntry);
    UiObjectInfo uiObjectInfoFromAttributes(const QXmlStreamAttributes& attributes) const;
    void showUiObjectSection(const QString& title, const QList<UiObjectInfo>& objects) const;
    void evaluateWithoutAppending(const QString& sourceCode, const QString& historyEntry);
    void previewUiFile(const QString& commandName,
                       const QString& filePathArgument,
                       const QString& historyEntry);
    void inspectTopLevelWidgets(const QString& historyEntry);
    void showWidgets(const QString& argument, const QString& historyEntry);
    void closeAllWidgets(const QString& historyEntry);
    bool requireQtClingCommand(const QString& commandName);
    void showRecentErrors();
    void handleBufferAppendMode(const QString& commandName,
                                const QString& argument,
                                const QString& historyEntry);
    void handleAutogenMode(const QString& argument, const QString& historyEntry);
    void handleQuietMode(const QString& argument, const QString& historyEntry);
    void handleNewFileNameMode(const QString& argument, const QString& historyEntry);
    void handleHelpLanguageMode(const QString& argument, const QString& historyEntry);
    void showBufferAppendMode() const;
    void showAutogenMode() const;
    void showQuietMode() const;
    void showNewFileNameMode() const;
    void showHelpLanguage() const;
    void showStatus() const;
    void showGeneratedFiles(const QString& historyEntry);
    void cleanGeneratedFiles(const QString& historyEntry);
    void showDoctor(const QString& historyEntry);
    void showExamples(const QString& historyEntry);
    void showWhere(const QString& historyEntry);
    void showRunOrder(const QString& historyEntry);
    void showQtStatus(const QString& historyEntry);
    QList<GeneratedFileInfo> generatedFilesInCurrentDirectory() const;
    QString generatedFileStateText(const GeneratedFileInfo& generatedFile) const;
    void rememberTouchedSourceFile(const QString& filePath);
    bool shouldConfirmImplicitSave() const;
    bool confirmYesNo(const QString& promptText);
    void reportError(const QString& errorMessage);
    bool visibleBufferHasUnsavedChanges() const;
    bool confirmQuitIfVisibleBufferHasUnsavedChanges();
    void addHistoryEntry(const QString& historyEntry);

    Engine currentEngine;
    InterpreterProcess interpreter;
    HelpLanguage helpLanguage;
    QTextStream& standardOutput;
    QTextStream& standardError;
    QStringList committedSourceCode;
    QStringList registeredSourceFilePaths;
    QStringList includePathPragmas;
    QStringList touchedSourceFilePaths;
    QStringList recentErrors;
    QString rememberedVisibleBufferFilePath;
    QString lastSavedVisibleSourceCode;
    bool shouldQuit = false;
    bool appendNormalInputToBuffer = false;
    bool autogenEnabled = false;
    bool quietMode = false;
    QString newFileNameMode;
};

} // namespace icpp
