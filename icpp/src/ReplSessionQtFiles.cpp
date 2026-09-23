#include "icpp/ReplSessionImpl.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QLibraryInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QStringList>
#include <QTemporaryFile>
#include <QTextStream>
#include <QXmlStreamReader>

#include <algorithm>
#include <cstdlib>
#include <optional>
#include <utility>

namespace icpp {

namespace {

QString xmlEscaped(QString text)
{
    text.replace(QLatin1Char('&'), QStringLiteral("&amp;"));
    text.replace(QLatin1Char('<'), QStringLiteral("&lt;"));
    text.replace(QLatin1Char('>'), QStringLiteral("&gt;"));
    text.replace(QLatin1Char('"'), QStringLiteral("&quot;"));
    text.replace(QLatin1Char('\''), QStringLiteral("&apos;"));
    return text;
}

QString sanitizedUiFileStem(QString text, const QString& newFileNameMode)
{
    text = text.trimmed();
    text.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9_]+")), QStringLiteral("_"));
    text.replace(QRegularExpression(QStringLiteral("_+")), QStringLiteral("_"));
    text.remove(QRegularExpression(QStringLiteral("^_+|_+$")));
    if (text.isEmpty()) {
        return QStringLiteral("form");
    }
    if (newFileNameMode == QStringLiteral("asis")) {
        return text;
    }
    return text.toLower();
}

QString splitKnownUiWords(QString text)
{
    static const QStringList knownWords = {
        QStringLiteral("button"),
        QStringLiteral("widget"),
        QStringLiteral("window"),
        QStringLiteral("dialog"),
        QStringLiteral("label"),
        QStringLiteral("slider"),
        QStringLiteral("edit"),
        QStringLiteral("view"),
        QStringLiteral("box"),
        QStringLiteral("list"),
        QStringLiteral("table"),
        QStringLiteral("tree"),
        QStringLiteral("combo"),
        QStringLiteral("spin"),
        QStringLiteral("text"),
    };

    QStringList parts;
    for (const QString& word : knownWords) {
        const QString suffix = word + QStringLiteral("form");
        if (text.endsWith(suffix) && text.size() >= suffix.size()) {
            parts.prepend(QStringLiteral("form"));
            parts.prepend(word);
            text.chop(suffix.size());
            break;
        }
    }

    while (!text.isEmpty()) {
        QString matchedWord;
        for (const QString& word : knownWords) {
            if (text.endsWith(word) && text.size() > word.size()) {
                matchedWord = word;
                break;
            }
        }
        if (matchedWord.isEmpty()) {
            parts.prepend(text);
            break;
        }
        parts.prepend(matchedWord);
        text.chop(matchedWord.size());
    }
    return parts.join(QLatin1Char('_'));
}

QString uiClassNameFromText(QString text)
{
    text = text.trimmed();
    if (text.isEmpty()) {
        text = QStringLiteral("Form");
    }

    QString className;
    bool capitalizeNext = true;
    for (const QChar character : text) {
        if (character.isLetterOrNumber()) {
            QString characterText(character);
            if (className.isEmpty() && character.isDigit()) {
                className.append(QLatin1Char('_'));
            }
            className.append(capitalizeNext ? characterText.toUpper() : characterText);
            capitalizeNext = false;
        } else {
            capitalizeNext = true;
        }
    }

    return className.isEmpty() ? QStringLiteral("Form") : className;
}

bool containsUppercaseLetter(const QString& text)
{
    for (const QChar character : text) {
        if (character.isUpper()) {
            return true;
        }
    }
    return false;
}

QString normalizedUiClassStem(QString baseName)
{
    baseName = baseName.trimmed();
    if (baseName.contains(QRegularExpression(QStringLiteral("[^A-Za-z0-9]")))) {
        return baseName;
    }
    return splitKnownUiWords(baseName.toLower());
}

QString uiClassNameFromLowerFileName(const QFileInfo& fileInfo)
{
    const QString baseName = normalizedUiClassStem(fileInfo.completeBaseName());
    return uiClassNameFromText(baseName);
}

QString uiClassNameFromFormName(const QString& formName)
{
    if (containsUppercaseLetter(formName)) {
        return uiClassNameFromText(formName);
    }
    return uiClassNameFromText(normalizedUiClassStem(formName));
}

QFileInfo designerFileInfoForArgument(const QString& filePathArgument,
                                      const QString& newFileNameMode)
{
    const QFileInfo rawInfo(filePathArgument);
    if (!rawInfo.suffix().isEmpty()
        && (rawInfo.suffix().toLower() != QStringLiteral("ui") || rawInfo.exists())) {
        return rawInfo;
    }

    const QString parentPath = rawInfo.path() == QStringLiteral(".") ? QString() : rawInfo.path();
    const QString stem = sanitizedUiFileStem(rawInfo.completeBaseName(), newFileNameMode);
    const QString fileName = stem + QStringLiteral(".ui");
    const QString filePath = parentPath.isEmpty() ? fileName : QDir(parentPath).filePath(fileName);
    return QFileInfo(filePath);
}

QString defaultUiFileSource(const QString& className, const QString& title)
{
    return QStringLiteral(R"(<?xml version="1.0" encoding="UTF-8"?>
<ui version="4.0">
 <class>%1</class>
 <widget class="QWidget" name="%1">
  <property name="geometry">
   <rect>
    <x>0</x>
    <y>0</y>
    <width>400</width>
    <height>300</height>
   </rect>
  </property>
  <property name="windowTitle">
   <string>%2</string>
  </property>
 </widget>
 <resources/>
 <connections/>
</ui>
)")
        .arg(xmlEscaped(className), xmlEscaped(title));
}

QString designerClassNameForArgument(const QString& filePathArgument, const QFileInfo& fileInfo)
{
    const QFileInfo rawInfo(filePathArgument);
    if (rawInfo.suffix().isEmpty() || rawInfo.suffix().toLower() == QStringLiteral("ui")) {
        return uiClassNameFromFormName(rawInfo.completeBaseName());
    }
    return uiClassNameFromLowerFileName(fileInfo);
}

#if defined(Q_OS_MACOS)
std::optional<QString> macApplicationBundleForExecutable(const QString& program)
{
    const QString marker = QStringLiteral(".app/Contents/MacOS/");
    const int markerIndex = program.indexOf(marker);
    if (markerIndex < 0) {
        return std::nullopt;
    }

    const QString appBundlePath = program.left(markerIndex + int(QStringLiteral(".app").size()));
    if (!QFileInfo(appBundlePath).isDir()) {
        return std::nullopt;
    }
    return appBundlePath;
}

bool isMacApplicationRunning(const QString& executableName)
{
    QProcess process;
    process.start(QStringLiteral("/usr/bin/pgrep"), QStringList{QStringLiteral("-x"), executableName});
    if (!process.waitForFinished(1000)) {
        process.kill();
        process.waitForFinished(1000);
        return false;
    }
    return process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
}

bool startMacApplicationForFile(const QString& appBundlePath,
                                const QString& executableName,
                                const QString& filePath)
{
    if (isMacApplicationRunning(executableName)) {
        return QProcess::startDetached(QStringLiteral("/usr/bin/open"),
                                       QStringList{QStringLiteral("-a"), appBundlePath, filePath});
    }

    return QProcess::startDetached(QStringLiteral("/usr/bin/open"),
                                   QStringList{appBundlePath,
                                               QStringLiteral("--args"),
                                               filePath});
}
#endif

} // namespace

void ReplSessionImpl::openDesigner(const QString& filePathArgument, const QString& historyEntry)
{
    const QString filePath = filePathArgument.trimmed();
    if (filePath.isEmpty()) {
        reportError(trMessage(QStringLiteral(".designer には .ui ファイルパスが必要です。"),
                              QStringLiteral(".designer requires a .ui file path.")));
        return;
    }

    const QFileInfo fileInfo = designerFileInfoForArgument(filePath, newFileNameMode);
    if (fileInfo.suffix().toLower() != QStringLiteral("ui")) {
        reportError(trMessage(QStringLiteral("%1: .ui ファイルではありません。"),
                              QStringLiteral("%1: Not a .ui file."))
                        .arg(filePath));
        return;
    }
    if (fileInfo.exists() && !fileInfo.isFile()) {
        reportError(trMessage(QStringLiteral("%1: 通常ファイルではありません。"),
                              QStringLiteral("%1: Not a regular file."))
                        .arg(filePath));
        return;
    }
    if (!fileInfo.exists()) {
        const QDir parentDirectory = fileInfo.absoluteDir();
        if (!parentDirectory.exists()) {
            reportError(trMessage(QStringLiteral("%1: 親ディレクトリが見つかりません。"),
                                  QStringLiteral("%1: Parent directory not found."))
                            .arg(filePath));
            return;
        }
        const QString className = designerClassNameForArgument(filePath, fileInfo);
        const QString title = className;
        standardOutput << trMessage(QStringLiteral("作成する .ui ファイル: %1"),
                                    QStringLiteral("UI file to create: %1"))
                              .arg(fileInfo.filePath())
                       << Qt::endl;
        standardOutput << trMessage(QStringLiteral("form class: %1"),
                                    QStringLiteral("form class: %1"))
                              .arg(className)
                       << Qt::endl;
        if (!confirmYesNo(trMessage(QStringLiteral("続けますか? [Y/N] "),
                                    QStringLiteral("Continue? [Y/N] ")))) {
            addHistoryEntry(historyEntry);
            return;
        }
        if (!writeTextFile(fileInfo.absoluteFilePath(), defaultUiFileSource(className, title))) {
            return;
        }
        standardOutput << trMessage(QStringLiteral(".ui ファイルを作成しました: %1"),
                                    QStringLiteral("Created .ui file: %1"))
                              .arg(fileInfo.fileName())
                       << Qt::endl;
    }

    const QString absoluteFilePath = fileInfo.absoluteFilePath();
    if (!runDesignerForFile(absoluteFilePath)) {
        return;
    }

    rememberTouchedSourceFile(absoluteFilePath);
    standardOutput << trMessage(QStringLiteral("Designer で開きました: %1"),
                                QStringLiteral("Opened with Designer: %1"))
                          .arg(fileInfo.fileName())
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("Designer で保存した後、.gen で生成物を更新してください。"),
                                QStringLiteral("After saving in Designer, run .gen to update generated files."))
                   << Qt::endl;
    addHistoryEntry(historyEntry);
}

void ReplSessionImpl::openLinguist(const QString& filePathArgument, const QString& historyEntry)
{
    const QString filePath = filePathArgument.trimmed();
    if (filePath.isEmpty()) {
        reportError(trMessage(QStringLiteral(".linguist には .ts ファイルパスが必要です。"),
                              QStringLiteral(".linguist requires a .ts file path.")));
        return;
    }

    const QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        reportError(trMessage(QStringLiteral("%1: .ts ファイルが見つかりません。"),
                              QStringLiteral("%1: .ts file not found."))
                        .arg(filePath));
        return;
    }
    if (fileInfo.suffix().toLower() != QStringLiteral("ts")) {
        reportError(trMessage(QStringLiteral("%1: .ts ファイルではありません。"),
                              QStringLiteral("%1: Not a .ts file."))
                        .arg(filePath));
        return;
    }

    const QString absoluteFilePath = fileInfo.absoluteFilePath();
    if (!runLinguistForFile(absoluteFilePath)) {
        return;
    }

    rememberTouchedSourceFile(absoluteFilePath);
    standardOutput << trMessage(QStringLiteral("Linguist で開きました: %1"),
                                QStringLiteral("Opened with Linguist: %1"))
                          .arg(fileInfo.fileName())
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("Linguist で保存した後、.! lrelease %1 で .qm を更新してください。"),
                                QStringLiteral("After saving in Linguist, run .! lrelease %1 to update the .qm file."))
                          .arg(filePath)
                   << Qt::endl;
    addHistoryEntry(historyEntry);
}

void ReplSessionImpl::openQrc(const QString& argument, const QString& historyEntry)
{
    QStringList parts = QProcess::splitCommand(argument);
    if (parts.isEmpty()) {
        reportError(trMessage(QStringLiteral(".qrc には .qrc ファイルパスが必要です。"),
                              QStringLiteral(".qrc requires a .qrc file path.")));
        return;
    }

    QString mode = QStringLiteral("text");
    if (parts.first() == QStringLiteral("text") || parts.first() == QStringLiteral("editor")
        || parts.first() == QStringLiteral("creator") || parts.first() == QStringLiteral("qtcreator")) {
        mode = parts.takeFirst();
        if (mode == QStringLiteral("editor")) {
            mode = QStringLiteral("text");
        }
        if (mode == QStringLiteral("qtcreator")) {
            mode = QStringLiteral("creator");
        }
    }

    const QString filePath = parts.join(QLatin1Char(' '));
    if (mode == QStringLiteral("creator")) {
        openQrcWithQtCreator(filePath, historyEntry);
        return;
    }

    openQrcWithTextEditor(filePath, historyEntry);
}

void ReplSessionImpl::openQrcWithTextEditor(const QString& filePathArgument, const QString& historyEntry)
{
    const QFileInfo fileInfo = validateQrcFilePath(filePathArgument, QStringLiteral(".qrc"));
    if (!fileInfo.exists()) {
        return;
    }

    const QString absoluteFilePath = fileInfo.absoluteFilePath();
    if (!runEditorForFile(absoluteFilePath)) {
        return;
    }

    rememberTouchedSourceFile(absoluteFilePath);
    standardOutput << trMessage(QStringLiteral("テキストエディタで編集しました: %1"),
                                QStringLiteral("Edited with text editor: %1"))
                          .arg(fileInfo.fileName())
                   << Qt::endl;
    finishQrcEdit(historyEntry);
}

void ReplSessionImpl::openQrcWithQtCreator(const QString& filePathArgument, const QString& historyEntry)
{
    const QFileInfo fileInfo = validateQrcFilePath(filePathArgument, QStringLiteral(".qtc"));
    if (!fileInfo.exists()) {
        return;
    }

    const QString absoluteFilePath = fileInfo.absoluteFilePath();
    if (!runQtCreatorForFile(absoluteFilePath)) {
        return;
    }

    rememberTouchedSourceFile(absoluteFilePath);
    standardOutput << trMessage(QStringLiteral("Qt Creator で編集しました: %1"),
                                QStringLiteral("Edited with Qt Creator: %1"))
                          .arg(fileInfo.fileName())
                   << Qt::endl;
    finishQrcEdit(historyEntry);
}

QFileInfo ReplSessionImpl::validateQrcFilePath(const QString& filePathArgument, const QString& commandName)
{
    const QString filePath = filePathArgument.trimmed();
    if (filePath.isEmpty()) {
        reportError(trMessage(QStringLiteral("%1 には .qrc ファイルパスが必要です。"),
                              QStringLiteral("%1 requires a .qrc file path."))
                        .arg(commandName));
        return {};
    }

    const QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        reportError(trMessage(QStringLiteral("%1: .qrc ファイルが見つかりません。"),
                              QStringLiteral("%1: .qrc file not found."))
                        .arg(filePath));
        return {};
    }
    if (fileInfo.suffix().toLower() != QStringLiteral("qrc")) {
        reportError(trMessage(QStringLiteral("%1: .qrc ファイルではありません。"),
                              QStringLiteral("%1: Not a .qrc file."))
                        .arg(filePath));
        return {};
    }

    return fileInfo;
}

void ReplSessionImpl::finishQrcEdit(const QString& historyEntry)
{
    if (autogenEnabled) {
        generateQtFilesAndRun(historyEntry);
    } else {
        standardOutput << trMessage(QStringLiteral(".qrc を変更した場合は .gen で生成物を更新してください。"),
                                    QStringLiteral("If the .qrc file changed, run .gen to update generated files."))
                       << Qt::endl;
        addHistoryEntry(historyEntry);
    }
}

bool ReplSessionImpl::runDesignerForFile(const QString& filePath)
{
    const QString designerCommand = determineDesignerCommand();
    QStringList designerCommandParts = QProcess::splitCommand(designerCommand);
    if (designerCommandParts.isEmpty()) {
        reportError(trMessage(QStringLiteral("Designer コマンドを決定できません。ICPP_DESIGNER を設定してください。"),
                              QStringLiteral("Cannot determine the Designer command. Set ICPP_DESIGNER.")));
        return false;
    }

    const QString designerProgram = designerCommandParts.takeFirst();
    designerCommandParts.append(filePath);

#if defined(Q_OS_MACOS)
    if (const std::optional<QString> appBundlePath = macApplicationBundleForExecutable(designerProgram)) {
        if (!startMacApplicationForFile(*appBundlePath, QFileInfo(designerProgram).fileName(), filePath)) {
            reportError(trMessage(QStringLiteral("Designer を起動できません: %1"),
                                  QStringLiteral("Cannot start Designer: %1"))
                            .arg(*appBundlePath));
            return false;
        }
        return true;
    }
#endif

    if (!QProcess::startDetached(designerProgram, designerCommandParts)) {
        reportError(trMessage(QStringLiteral("Designer を起動できません: %1"),
                              QStringLiteral("Cannot start Designer: %1"))
                        .arg(designerProgram));
        return false;
    }
    return true;
}

bool ReplSessionImpl::runLinguistForFile(const QString& filePath)
{
    const QString linguistCommand = determineLinguistCommand();
    QStringList linguistCommandParts = QProcess::splitCommand(linguistCommand);
    if (linguistCommandParts.isEmpty()) {
        reportError(trMessage(QStringLiteral("Linguist コマンドを決定できません。ICPP_LINGUIST を設定してください。"),
                              QStringLiteral("Cannot determine the Linguist command. Set ICPP_LINGUIST.")));
        return false;
    }

    const QString linguistProgram = linguistCommandParts.takeFirst();
    linguistCommandParts.append(filePath);

    if (!QProcess::startDetached(linguistProgram, linguistCommandParts)) {
        reportError(trMessage(QStringLiteral("Linguist を起動できません: %1"),
                              QStringLiteral("Cannot start Linguist: %1"))
                        .arg(linguistProgram));
        return false;
    }
    return true;
}

bool ReplSessionImpl::runQtCreatorForFile(const QString& filePath)
{
    const QString qtCreatorCommand = determineQtCreatorCommand();
    QStringList qtCreatorCommandParts = QProcess::splitCommand(qtCreatorCommand);
    if (qtCreatorCommandParts.isEmpty()) {
        reportError(trMessage(QStringLiteral("Qt Creator コマンドを決定できません。ICPP_QTCREATOR を設定してください。"),
                              QStringLiteral("Cannot determine the Qt Creator command. Set ICPP_QTCREATOR.")));
        return false;
    }

    const QString qtCreatorProgram = qtCreatorCommandParts.takeFirst();
    qtCreatorCommandParts.append(filePath);

    QProcess qtCreatorProcess;
    qtCreatorProcess.setInputChannelMode(QProcess::ForwardedInputChannel);
    qtCreatorProcess.setProcessChannelMode(QProcess::ForwardedChannels);
    qtCreatorProcess.start(qtCreatorProgram, qtCreatorCommandParts);

    if (!qtCreatorProcess.waitForStarted()) {
        reportError(trMessage(QStringLiteral("Qt Creator を起動できません: %1"),
                              QStringLiteral("Cannot start Qt Creator: %1"))
                        .arg(qtCreatorProgram));
        return false;
    }
    qtCreatorProcess.waitForFinished(-1);
    if (qtCreatorProcess.exitStatus() != QProcess::NormalExit) {
        reportError(trMessage(QStringLiteral("Qt Creator がクラッシュしました: %1"),
                              QStringLiteral("Qt Creator crashed: %1"))
                        .arg(qtCreatorProgram));
        return false;
    }
    if (qtCreatorProcess.exitCode() != 0) {
        reportError(trMessage(QStringLiteral("Qt Creator がステータス %1 で終了しました: %2"),
                              QStringLiteral("Qt Creator exited with status %1: %2"))
                        .arg(qtCreatorProcess.exitCode())
                        .arg(qtCreatorProgram));
        return false;
    }
    return true;
}

QString ReplSessionImpl::determineDesignerCommand() const
{
    QString designerCommand = qEnvironmentVariable("ICPP_DESIGNER");
    if (!designerCommand.trimmed().isEmpty()) {
        return designerCommand;
    }

    const QDir qtBinaryDirectory(QLibraryInfo::path(QLibraryInfo::BinariesPath));
#if defined(Q_OS_MACOS)
    const QString qtDesignerApp =
        qtBinaryDirectory.filePath(QStringLiteral("Designer.app/Contents/MacOS/Designer"));
    if (QFileInfo(qtDesignerApp).isExecutable()) {
        return qtDesignerApp;
    }
#endif

    const QString qtDesigner = qtBinaryDirectory.filePath(QStringLiteral("designer"));
    if (QFileInfo(qtDesigner).isExecutable()) {
        return qtDesigner;
    }

    const QString pathDesigner = QStandardPaths::findExecutable(QStringLiteral("designer"));
    if (!pathDesigner.isEmpty()) {
        return pathDesigner;
    }

    return {};
}

QString ReplSessionImpl::determineLinguistCommand() const
{
    QString linguistCommand = qEnvironmentVariable("ICPP_LINGUIST");
    if (!linguistCommand.trimmed().isEmpty()) {
        return linguistCommand;
    }

    const QDir qtBinaryDirectory(QLibraryInfo::path(QLibraryInfo::BinariesPath));
#if defined(Q_OS_MACOS)
    const QString qtLinguistApp =
        qtBinaryDirectory.filePath(QStringLiteral("Linguist.app/Contents/MacOS/Linguist"));
    if (QFileInfo(qtLinguistApp).isExecutable()) {
        return qtLinguistApp;
    }
#endif

    const QString qtLinguist = qtBinaryDirectory.filePath(QStringLiteral("linguist"));
    if (QFileInfo(qtLinguist).isExecutable()) {
        return qtLinguist;
    }

    const QString pathLinguist = QStandardPaths::findExecutable(QStringLiteral("linguist"));
    if (!pathLinguist.isEmpty()) {
        return pathLinguist;
    }

    return {};
}

QString ReplSessionImpl::determineQtCreatorCommand() const
{
    QString qtCreatorCommand = qEnvironmentVariable("ICPP_QTCREATOR");
    if (!qtCreatorCommand.trimmed().isEmpty()) {
        return qtCreatorCommand;
    }

#if defined(Q_OS_MACOS)
    const QString qtCreatorApp =
        QDir(QLibraryInfo::path(QLibraryInfo::PrefixPath))
            .filePath(QStringLiteral("../../Qt Creator.app/Contents/MacOS/Qt Creator"));
    const QString cleanQtCreatorApp = QDir::cleanPath(qtCreatorApp);
    if (QFileInfo(cleanQtCreatorApp).isExecutable()) {
        return QStringLiteral("\"%1\" -client -block").arg(cleanQtCreatorApp);
    }

    const QString applicationsQtCreator =
        QStringLiteral("/Applications/Qt Creator.app/Contents/MacOS/Qt Creator");
    if (QFileInfo(applicationsQtCreator).isExecutable()) {
        return QStringLiteral("\"%1\" -client -block").arg(applicationsQtCreator);
    }
#endif

    const QString pathQtCreator = QStandardPaths::findExecutable(QStringLiteral("qtcreator"));
    if (!pathQtCreator.isEmpty()) {
        return QStringLiteral("\"%1\" -client -block").arg(pathQtCreator);
    }

    return {};
}

void ReplSessionImpl::showUiInfo(const QString& filePathArgument, const QString& historyEntry)
{
    const QString filePath = filePathArgument.trimmed();
    if (filePath.isEmpty()) {
        reportError(trMessage(QStringLiteral(".uiinfo には .ui ファイルパスが必要です。"),
                              QStringLiteral(".uiinfo requires a .ui file path.")));
        return;
    }

    QFile inputFile(filePath);
    if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        reportError(trMessage(QStringLiteral("%1: .ui ファイルを開けません。"),
                              QStringLiteral("%1: Cannot open .ui file."))
                        .arg(filePath));
        return;
    }

    QString uiClassName;
    QString baseWidgetType;
    QList<UiObjectInfo> widgets;
    QList<UiObjectInfo> layouts;
    QList<UiObjectInfo> actions;
    QList<UiObjectInfo> spacers;

    QXmlStreamReader reader(&inputFile);
    while (!reader.atEnd()) {
        reader.readNext();
        if (!reader.isStartElement()) {
            continue;
        }

        const QStringView elementName = reader.name();
        if (elementName == QStringLiteral("class")) {
            uiClassName = reader.readElementText(QXmlStreamReader::SkipChildElements).trimmed();
            continue;
        }

        if (elementName == QStringLiteral("widget")) {
            const UiObjectInfo objectInfo = uiObjectInfoFromAttributes(reader.attributes());
            if (baseWidgetType.isEmpty()) {
                baseWidgetType = objectInfo.typeName;
            }
            widgets.append(objectInfo);
            continue;
        }

        if (elementName == QStringLiteral("layout")) {
            layouts.append(uiObjectInfoFromAttributes(reader.attributes()));
            continue;
        }

        if (elementName == QStringLiteral("action")) {
            UiObjectInfo actionInfo = uiObjectInfoFromAttributes(reader.attributes());
            if (actionInfo.typeName.isEmpty()) {
                actionInfo.typeName = QStringLiteral("QAction");
            }
            actions.append(actionInfo);
            continue;
        }

        if (elementName == QStringLiteral("spacer")) {
            UiObjectInfo spacerInfo = uiObjectInfoFromAttributes(reader.attributes());
            if (spacerInfo.typeName.isEmpty()) {
                spacerInfo.typeName = QStringLiteral("spacer");
            }
            spacers.append(spacerInfo);
        }
    }

    if (reader.hasError()) {
        reportError(trMessage(QStringLiteral("%1: .ui XML の読み取りに失敗しました: %2"),
                              QStringLiteral("%1: Failed to read .ui XML: %2"))
                        .arg(filePath, reader.errorString()));
        return;
    }

    standardOutput << trMessage(QStringLiteral("class: %1"), QStringLiteral("class: %1"))
                          .arg(uiClassName.isEmpty() ? trMessage(QStringLiteral("(未指定)"),
                                                                  QStringLiteral("(not specified)"))
                                                     : uiClassName)
                   << Qt::endl;
    standardOutput << trMessage(QStringLiteral("base: %1"), QStringLiteral("base: %1"))
                          .arg(baseWidgetType.isEmpty() ? trMessage(QStringLiteral("(未検出)"),
                                                                    QStringLiteral("(not detected)"))
                                                        : baseWidgetType)
                   << Qt::endl;
    standardOutput << Qt::endl;
    showUiObjectSection(trMessage(QStringLiteral("widgets:"), QStringLiteral("widgets:")), widgets);
    showUiObjectSection(trMessage(QStringLiteral("layouts:"), QStringLiteral("layouts:")), layouts);
    showUiObjectSection(trMessage(QStringLiteral("actions:"), QStringLiteral("actions:")), actions);
    showUiObjectSection(trMessage(QStringLiteral("spacers:"), QStringLiteral("spacers:")), spacers);
    addHistoryEntry(historyEntry);
}

UiObjectInfo ReplSessionImpl::uiObjectInfoFromAttributes(const QXmlStreamAttributes& attributes) const
{
    UiObjectInfo objectInfo;
    objectInfo.typeName = attributes.value(QStringLiteral("class")).toString();
    objectInfo.objectName = attributes.value(QStringLiteral("name")).toString();
    return objectInfo;
}

void ReplSessionImpl::showUiObjectSection(const QString& title, const QList<UiObjectInfo>& objects) const
{
    standardOutput << title << Qt::endl;
    if (objects.isEmpty()) {
        standardOutput << QStringLiteral("  %1")
                              .arg(trMessage(QStringLiteral("(なし)"), QStringLiteral("(none)")))
                       << Qt::endl;
        return;
    }

    int typeWidth = 0;
    for (const UiObjectInfo& objectInfo : objects) {
        typeWidth = std::max(typeWidth, static_cast<int>(objectInfo.typeName.size()));
    }

    for (const UiObjectInfo& objectInfo : objects) {
        standardOutput << QStringLiteral("  %1  %2")
                              .arg(objectInfo.typeName, -typeWidth)
                              .arg(objectInfo.objectName.isEmpty()
                                       ? trMessage(QStringLiteral("(名前なし)"),
                                                   QStringLiteral("(unnamed)"))
                                       : objectInfo.objectName)
                       << Qt::endl;
    }
    standardOutput << Qt::endl;
}

} // namespace icpp
