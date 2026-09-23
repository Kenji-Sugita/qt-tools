#include "icpp/ReplHelpers.h"
#include "icpp/LineEdit.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QList>
#include <QCoreApplication>
#include <QLibraryInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTextStream>
#include <QThread>
#include <QXmlStreamReader>

#if defined(Q_OS_WIN)
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <utility>

namespace icpp {

QTextStream& standardOutput()
{
    static QTextStream stream(stdout);
    return stream;
}

QTextStream& standardError()
{
    static QTextStream stream(stderr);
    return stream;
}

HelpLanguage& activeHelpLanguage()
{
    static HelpLanguage language = HelpLanguage::Japanese;
    return language;
}

QString trGlobalMessage(const QString& japanese, const QString& english)
{
    return activeHelpLanguage() == HelpLanguage::Japanese ? japanese : english;
}

bool shouldShowPrompt()
{
#if defined(Q_OS_WIN)
    HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
    if (handle == INVALID_HANDLE_VALUE || handle == nullptr) {
        return false;
    }

    DWORD consoleMode = 0;
    return GetConsoleMode(handle, &consoleMode) != 0;
#else
    return isatty(STDIN_FILENO);
#endif
}

bool shouldUsePager()
{
#if defined(Q_OS_WIN)
    return false;
#else
    return isatty(STDIN_FILENO) && isatty(STDOUT_FILENO);
#endif
}

QString pagerCommand()
{
    if (const char* pager = std::getenv("ICPP_PAGER"); pager != nullptr && *pager != '\0') {
        return QString::fromLocal8Bit(pager);
    }
    if (const char* pager = std::getenv("PAGER"); pager != nullptr && *pager != '\0') {
        return QString::fromLocal8Bit(pager);
    }
    return QStringLiteral("less -R");
}

#if defined(HAVE_READLINE)
const char* const kCommandCompletions[] = {
    ".!",
    ".?",
    ".a",
    ".add",
    ".append",
    ".args",
    ".autogen",
    ".b",
    ".buffer",
    ".clean",
    ".clearfiles",
    ".closeall",
    ".copy",
    ".d",
    ".de",
    ".defs",
    ".designer",
    ".discard",
    ".doctor",
    ".drop",
    ".e",
    ".edit",
    ".errors",
    ".eval",
    ".examples",
    ".files",
    ".gen",
    ".generated",
    ".h",
    ".help",
    ".i",
    ".include",
    ".inspect",
    ".l",
    ".lang",
    ".language",
    ".load",
    ".loadlib",
    ".li",
    ".linguist",
    ".new",
    ".newname",
    ".o",
    ".open",
    ".p",
    ".paste",
    ".preview",
    ".print",
    ".pragma",
    ".ptype",
    ".pv",
    ".q",
    ".quiet",
    ".qrc",
    ".qtc",
    ".quit",
    ".qt",
    ".r",
    ".reset",
    ".restart",
    ".run",
    ".runorder",
    ".s",
    ".save",
    ".sh",
    ".show",
    ".st",
    ".status",
    ".template",
    ".uiinfo",
    ".where",
    ".widgets",
    ".x",
    nullptr,
};

char* commandCompletionGenerator(const char* text, int state)
{
    static int commandIndex = 0;
    static QByteArray prefix;

    if (state == 0) {
        commandIndex = 0;
        prefix = QByteArray(text);
    }

    while (const char* command = kCommandCompletions[commandIndex++]) {
        if (QByteArray(command).startsWith(prefix)) {
            return ::strdup(command);
        }
    }

    return nullptr;
}

char** completeConsoleInput(const char* text, int start, int)
{
    if (start == 0 && text != nullptr && text[0] == '.') {
        rl_attempted_completion_over = 1;
        return rl_completion_matches(text, commandCompletionGenerator);
    }

    return nullptr;
}

void installReadlineCompletion()
{
    static bool installed = false;
    if (installed) {
        return;
    }

    rl_attempted_completion_function = completeConsoleInput;
    installed = true;
}
#endif

std::optional<QString> readConsoleLine(const QString& promptText)
{
#if defined(HAVE_READLINE)
    if (shouldShowPrompt()) {
        installReadlineCompletion();
        const QByteArray promptBytes = promptText.toLocal8Bit();
        char* rawLine = ::readline(promptBytes.constData());
        if (rawLine == nullptr) {
            return std::nullopt;
        }

        const QString line = QString::fromLocal8Bit(rawLine);
        std::free(rawLine);
        return line;
    }
#endif
    if (shouldShowPrompt()) {
        standardOutput() << promptText << Qt::flush;
    }

    static QTextStream standardInput(stdin);
    const QString line = standardInput.readLine();
    if (line.isNull()) {
        return std::nullopt;
    }

    return line;
}

ParsedCommand parseCommand(const QString& trimmedLine)
{
    if (trimmedLine.startsWith(QStringLiteral(".!"))) {
        return {QStringLiteral(".!"), trimmedLine.mid(2).trimmed()};
    }

    const QStringList commandParts =
        trimmedLine.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    if (commandParts.isEmpty()) {
        return {};
    }

    const QString commandName = commandParts.first();
    const int commandEnd = trimmedLine.indexOf(commandName) + commandName.size();
    return {commandName, trimmedLine.mid(commandEnd).trimmed()};
}

QString normalizeSourceCode(const QString& sourceCode)
{
    QString normalized = sourceCode;
    normalized.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    normalized.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    while (normalized.endsWith(QLatin1Char('\n'))) {
        normalized.chop(1);
    }
    return normalized;
}

QString joinSourceCode(const QStringList& blocks)
{
    QStringList nonEmptyBlocks;
    for (const QString& block : blocks) {
        const QString normalized = normalizeSourceCode(block);
        if (!normalized.isEmpty()) {
            nonEmptyBlocks.append(normalized);
        }
    }
    return nonEmptyBlocks.join(QStringLiteral("\n"));
}

std::optional<QString> readTextFile(const QString& filePath)
{
    QFile inputFile(filePath);
    if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        standardError() << trGlobalMessage(QStringLiteral("%1: 開けません。"),
                                           QStringLiteral("%1: Cannot open."))
                               .arg(filePath)
                        << Qt::endl;
        return std::nullopt;
    }

    QTextStream inputStream(&inputFile);
    return inputStream.readAll();
}

bool writeTextFile(const QString& filePath, const QString& sourceCode)
{
    QFile outputFile(filePath);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        standardError() << trGlobalMessage(QStringLiteral("%1: 書き込めません。"),
                                           QStringLiteral("%1: Cannot write."))
                               .arg(filePath)
                        << Qt::endl;
        return false;
    }

    QTextStream outputStream(&outputFile);
    outputStream << sourceCode;
    if (!sourceCode.endsWith(QLatin1Char('\n'))) {
        outputStream << Qt::endl;
    }
    outputStream.flush();
    return outputStream.status() == QTextStream::Ok;
}

QString quoteForPragma(const QString& path)
{
    QString quoted = path;
    quoted.replace(QLatin1Char('\\'), QStringLiteral("\\\\"));
    quoted.replace(QLatin1Char('"'), QStringLiteral("\\\""));
    return quoted;
}

QString quoteForCppString(const QString& text)
{
    QString quoted = text;
    quoted.replace(QLatin1Char('\\'), QStringLiteral("\\\\"));
    quoted.replace(QLatin1Char('"'), QStringLiteral("\\\""));
    quoted.replace(QLatin1Char('\n'), QStringLiteral("\\n"));
    quoted.replace(QLatin1Char('\r'), QStringLiteral("\\r"));
    quoted.replace(QLatin1Char('\t'), QStringLiteral("\\t"));
    return quoted;
}

QString qtUiToolsLibraryPath()
{
    const QDir libraryDirectory(QLibraryInfo::path(QLibraryInfo::LibrariesPath));
#if defined(Q_OS_MACOS)
    const QString frameworkPath =
        libraryDirectory.filePath(QStringLiteral("QtUiTools.framework/QtUiTools"));
    if (QFileInfo::exists(frameworkPath)) {
        return frameworkPath;
    }
#elif defined(Q_OS_WIN)
    const QString dllPath = libraryDirectory.filePath(QStringLiteral("Qt6UiTools.dll"));
    if (QFileInfo::exists(dllPath)) {
        return dllPath;
    }
#else
    const QString soPath = libraryDirectory.filePath(QStringLiteral("libQt6UiTools.so"));
    if (QFileInfo::exists(soPath)) {
        return soPath;
    }
#endif
    return {};
}

QString propertyEditorLibraryPath()
{
    const QString environmentPath =
        QString::fromLocal8Bit(qgetenv("ICPP_PROPERTY_EDITOR_LIB")).trimmed();
    if (!environmentPath.isEmpty()) {
        return environmentPath;
    }

    const QDir applicationDirectory(QCoreApplication::applicationDirPath());
    QStringList relativePaths;
#if defined(Q_OS_MACOS)
    relativePaths << QStringLiteral("lib/property_editor.dylib")
                  << QStringLiteral("../lib/property_editor.dylib")
                  << QStringLiteral("property_editor.dylib");
#else
    relativePaths << QStringLiteral("lib/property_editor.so")
                  << QStringLiteral("lib/libproperty_editor.so")
                  << QStringLiteral("../lib/property_editor.so")
                  << QStringLiteral("../lib/libproperty_editor.so")
                  << QStringLiteral("property_editor.so")
                  << QStringLiteral("libproperty_editor.so");
#endif
    for (const QString& relativePath : relativePaths) {
        const QString candidatePath = applicationDirectory.filePath(relativePath);
        if (QFileInfo::exists(candidatePath)) {
            return QFileInfo(candidatePath).absoluteFilePath();
        }
    }

    return {};
}

QString propertyEditorIncludePath()
{
    const QString environmentPath =
        QString::fromLocal8Bit(qgetenv("ICPP_PROPERTY_EDITOR_INCLUDE")).trimmed();
    if (!environmentPath.isEmpty()) {
        return environmentPath;
    }

    const QDir applicationDirectory(QCoreApplication::applicationDirPath());
    const QStringList relativePaths {
        QStringLiteral("include"),
        QStringLiteral("include/propertyeditor"),
        QStringLiteral("include/propertyeditor/src"),
        QStringLiteral("../include"),
        QStringLiteral("../include/propertyeditor"),
        QStringLiteral("../include/propertyeditor/src"),
        QStringLiteral("propertyeditor"),
        QStringLiteral("propertyeditor/src"),
    };
    for (const QString& relativePath : relativePaths) {
        const QString candidatePath = applicationDirectory.filePath(relativePath);
        if (QFileInfo(candidatePath).isDir()) {
            return QFileInfo(candidatePath).absoluteFilePath();
        }
    }

    return {};
}

QString configuredPath(const char* environmentVariableName, const char* fallbackPath)
{
    const QString environmentPath = QString::fromLocal8Bit(qgetenv(environmentVariableName)).trimmed();
    if (!environmentPath.isEmpty()) {
        return environmentPath;
    }
    return QString::fromLatin1(fallbackPath);
}

bool shouldAutoRegisterEditedFile(const QFileInfo& fileInfo)
{
    return isAddableSourceFile(fileInfo);
}

bool isAddableSourceFile(const QFileInfo& fileInfo)
{
    const QString suffix = fileInfo.suffix().toLower();
    return suffix == QStringLiteral("cpp") || suffix == QStringLiteral("cc")
           || suffix == QStringLiteral("cxx") || suffix == QStringLiteral("c++")
           || suffix == QStringLiteral("c");
}

QStringList missingMocIncludes(const QString& sourceCode, const QDir& baseDirectory)
{
    QStringList missingFilePaths;
    const QRegularExpression includePattern(
        QStringLiteral(R"re(#\s*include\s*"([^"]*(?:moc_[^"/\\]+\.cpp|[^"/\\]+\.moc))")re"));
    QRegularExpressionMatchIterator iterator = includePattern.globalMatch(sourceCode);
    while (iterator.hasNext()) {
        const QRegularExpressionMatch match = iterator.next();
        const QString includePath = match.captured(1);
        const QString absoluteFilePath = baseDirectory.absoluteFilePath(includePath);
        if (!QFileInfo::exists(absoluteFilePath) && !missingFilePaths.contains(absoluteFilePath)) {
            missingFilePaths.append(absoluteFilePath);
        }
    }
    return missingFilePaths;
}

QString stripLineComment(QString line)
{
    const int commentIndex = line.indexOf(QStringLiteral("//"));
    if (commentIndex >= 0) {
        line.truncate(commentIndex);
    }
    return line;
}

QString normalizeDefinitionSignature(QString signature)
{
    signature.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral(" "));
    signature = signature.trimmed();
    const int braceIndex = signature.indexOf(QLatin1Char('{'));
    if (braceIndex >= 0) {
        signature.truncate(braceIndex);
    }
    signature = signature.trimmed();

    const int initializerIndex = signature.indexOf(QRegularExpression(QStringLiteral(R"(\)\s*:)")));
    if (initializerIndex >= 0) {
        signature.truncate(initializerIndex + 1);
    }
    return signature.trimmed();
}

bool isLikelyControlStatement(const QString& signature)
{
    static const QRegularExpression controlPattern(
        QStringLiteral(R"(^\s*(if|for|while|switch|catch|else|do)\b)"));
    return controlPattern.match(signature).hasMatch();
}

QString functionDefinitionDescription(const QString& signature)
{
    const QString normalized = normalizeDefinitionSignature(signature);
    if (normalized.isEmpty() || isLikelyControlStatement(normalized)
        || normalized.startsWith(QLatin1Char('#')) || normalized.endsWith(QLatin1Char(';'))
        || !normalized.contains(QLatin1Char('(')) || !normalized.contains(QLatin1Char(')'))) {
        return {};
    }

    return QStringLiteral("function %1").arg(normalized);
}

QString memberFunctionKeyFromDefinition(const QString& description)
{
    static const QRegularExpression memberPattern(
        QStringLiteral(R"((?:^|\s)([A-Za-z_]\w*)::(~?[A-Za-z_]\w*)\s*\()"));
    const QRegularExpressionMatch match = memberPattern.match(description);
    if (!match.hasMatch()) {
        return {};
    }
    return QStringLiteral("%1::%2").arg(match.captured(1), match.captured(2));
}

QString memberFunctionKeyFromDeclaration(const QString& className, QString declaration)
{
    declaration = normalizeDefinitionSignature(declaration);
    declaration.remove(QRegularExpression(QStringLiteral(R"(\b(explicit|virtual|static|inline|constexpr|friend)\b)")));
    declaration = declaration.trimmed();

    static const QRegularExpression memberPattern(
        QStringLiteral(R"((~?[A-Za-z_]\w*)\s*\([^;{}]*\)\s*(?:const\s*)?(?:noexcept\s*)?(?:override\s*)?(?:final\s*)?(?:=\s*(?:0|default|delete)\s*)?;?$)"));
    const QRegularExpressionMatch match = memberPattern.match(declaration);
    if (!match.hasMatch()) {
        return {};
    }

    return QStringLiteral("%1::%2").arg(className, match.captured(1));
}

QString typeDefinitionDescription(const QString& line, const QString& nextNonEmptyLine)
{
    const QString cleanedLine = stripLineComment(line).trimmed();
    const bool hasOpeningBrace = cleanedLine.contains(QLatin1Char('{'))
                                 || nextNonEmptyLine.trimmed().startsWith(QLatin1Char('{'));
    if (!hasOpeningBrace) {
        return {};
    }

    static const QRegularExpression typePattern(
        QStringLiteral(R"(^\s*(?:template\s*<[^>]+>\s*)?(class|struct)\s+([A-Za-z_]\w*)\b)"));
    const QRegularExpressionMatch typeMatch = typePattern.match(cleanedLine);
    if (typeMatch.hasMatch()) {
        return QStringLiteral("%1 %2").arg(typeMatch.captured(1), typeMatch.captured(2));
    }

    static const QRegularExpression enumPattern(
        QStringLiteral(R"(^\s*enum\s+(class\s+)?([A-Za-z_]\w*)\b)"));
    const QRegularExpressionMatch enumMatch = enumPattern.match(cleanedLine);
    if (enumMatch.hasMatch()) {
        return QStringLiteral("enum %1").arg(enumMatch.captured(2));
    }

    return {};
}

QString nextNonEmptyLine(const QStringList& lines, int startIndex)
{
    for (int index = startIndex; index < lines.size(); ++index) {
        const QString cleanedLine = stripLineComment(lines.at(index)).trimmed();
        if (!cleanedLine.isEmpty()) {
            return cleanedLine;
        }
    }
    return {};
}

QString appendSignatureLine(const QString& signature, const QString& line)
{
    const QString cleanedLine = stripLineComment(line).trimmed();
    if (cleanedLine.isEmpty()) {
        return signature;
    }
    if (signature.isEmpty()) {
        return cleanedLine;
    }
    return QStringLiteral("%1 %2").arg(signature, cleanedLine);
}

QStringList sourceLines(const QString& sourceCode)
{
    if (sourceCode.isEmpty()) {
        return {};
    }
    return normalizeSourceCode(sourceCode).split(QLatin1Char('\n'));
}

int braceDeltaOutsideQuotedText(const QString& line);

void collectMemberAccessFromSource(const QString& sourceCode, MemberAccessMap& accessMap)
{
    const QStringList lines = sourceLines(sourceCode);
    QString currentClassName;
    QString currentAccess;
    int classBraceDepth = 0;
    QString pendingDeclaration;

    for (const QString& rawLine : lines) {
        const QString line = stripLineComment(rawLine);
        const QString trimmedLine = line.trimmed();
        if (trimmedLine.isEmpty()) {
            continue;
        }

        if (currentClassName.isEmpty()) {
            static const QRegularExpression classPattern(
                QStringLiteral(R"(^\s*(class|struct)\s+([A-Za-z_]\w*)\b[^;{]*\{?)"));
            const QRegularExpressionMatch match = classPattern.match(trimmedLine);
            if (!match.hasMatch() || !trimmedLine.contains(QLatin1Char('{'))) {
                continue;
            }

            currentClassName = match.captured(2);
            currentAccess = match.captured(1) == QStringLiteral("struct")
                                ? QStringLiteral("public")
                                : QStringLiteral("private");
            classBraceDepth = braceDeltaOutsideQuotedText(rawLine);
            pendingDeclaration.clear();
            continue;
        }

        if (classBraceDepth == 1) {
            static const QRegularExpression accessPattern(
                QStringLiteral(R"(^\s*(public|protected|private)(?:\s+slots)?\s*:\s*$)"));
            const QRegularExpressionMatch accessMatch = accessPattern.match(trimmedLine);
            if (accessMatch.hasMatch()) {
                currentAccess = accessMatch.captured(1);
                classBraceDepth += braceDeltaOutsideQuotedText(rawLine);
                continue;
            }
            if (trimmedLine == QStringLiteral("signals:")) {
                currentAccess = QStringLiteral("public");
                classBraceDepth += braceDeltaOutsideQuotedText(rawLine);
                continue;
            }

            if (!trimmedLine.startsWith(QLatin1Char('#')) && trimmedLine.contains(QLatin1Char('('))) {
                pendingDeclaration = appendSignatureLine(pendingDeclaration, line);
                if (trimmedLine.contains(QLatin1Char(';'))) {
                    const QString key =
                        memberFunctionKeyFromDeclaration(currentClassName, pendingDeclaration);
                    if (!key.isEmpty()) {
                        accessMap.insert(key, currentAccess);
                    }
                    pendingDeclaration.clear();
                }
            } else if (!pendingDeclaration.isEmpty()) {
                pendingDeclaration = appendSignatureLine(pendingDeclaration, line);
                if (trimmedLine.contains(QLatin1Char(';'))) {
                    const QString key =
                        memberFunctionKeyFromDeclaration(currentClassName, pendingDeclaration);
                    if (!key.isEmpty()) {
                        accessMap.insert(key, currentAccess);
                    }
                    pendingDeclaration.clear();
                }
            }
        }

        classBraceDepth += braceDeltaOutsideQuotedText(rawLine);
        if (classBraceDepth <= 0) {
            currentClassName.clear();
            currentAccess.clear();
            classBraceDepth = 0;
            pendingDeclaration.clear();
        }
    }
}

QStringList localIncludeFilePaths(const QString& sourceCode, const QDir& baseDirectory)
{
    QStringList filePaths;
    static const QRegularExpression includePattern(
        QStringLiteral(R"re(^\s*#\s*include\s*"([^"]+)")re"));
    const QStringList lines = sourceLines(sourceCode);
    for (const QString& line : lines) {
        const QRegularExpressionMatch match = includePattern.match(line);
        if (!match.hasMatch()) {
            continue;
        }
        const QString filePath = QFileInfo(baseDirectory.filePath(match.captured(1))).absoluteFilePath();
        if (QFileInfo::exists(filePath) && !filePaths.contains(filePath)) {
            filePaths.append(filePath);
        }
    }
    return filePaths;
}

MemberAccessMap memberAccessMapForSource(const QString& sourceCode, const QDir& baseDirectory)
{
    MemberAccessMap accessMap;
    collectMemberAccessFromSource(sourceCode, accessMap);

    for (const QString& includeFilePath : localIncludeFilePaths(sourceCode, baseDirectory)) {
        const std::optional<QString> includeSourceCode = readTextFile(includeFilePath);
        if (includeSourceCode.has_value()) {
            collectMemberAccessFromSource(*includeSourceCode, accessMap);
        }
    }

    return accessMap;
}

int braceDeltaOutsideQuotedText(const QString& line)
{
    int delta = 0;
    bool inString = false;
    bool inCharacter = false;
    bool escaped = false;

    for (int index = 0; index < line.size(); ++index) {
        const QChar current = line.at(index);
        const QChar next = index + 1 < line.size() ? line.at(index + 1) : QChar();

        if (!inString && !inCharacter && current == QLatin1Char('/') && next == QLatin1Char('/')) {
            break;
        }

        if (escaped) {
            escaped = false;
            continue;
        }

        if ((inString || inCharacter) && current == QLatin1Char('\\')) {
            escaped = true;
            continue;
        }

        if (!inCharacter && current == QLatin1Char('"')) {
            inString = !inString;
            continue;
        }

        if (!inString && current == QLatin1Char('\'')) {
            inCharacter = !inCharacter;
            continue;
        }

        if (inString || inCharacter) {
            continue;
        }

        if (current == QLatin1Char('{')) {
            ++delta;
        } else if (current == QLatin1Char('}')) {
            --delta;
        }
    }

    return delta;
}

QList<SourceDefinition> findSourceDefinitions(const QString& sourceCode)
{
    QList<SourceDefinition> definitions;
    const QStringList lines = sourceLines(sourceCode);
    QString pendingSignature;
    int pendingStartLine = 0;
    int braceDepth = 0;

    for (int index = 0; index < lines.size(); ++index) {
        const QString line = stripLineComment(lines.at(index));
        const QString trimmedLine = line.trimmed();
        if (trimmedLine.isEmpty()) {
            braceDepth = qMax(0, braceDepth + braceDeltaOutsideQuotedText(lines.at(index)));
            continue;
        }

        if (braceDepth == 0) {
            const QString typeDescription = typeDefinitionDescription(line, nextNonEmptyLine(lines, index + 1));
            if (!typeDescription.isEmpty()) {
                definitions.append(SourceDefinition{index + 1, typeDescription});
            }

            if (pendingSignature.isEmpty()) {
                if (trimmedLine.startsWith(QLatin1Char('#')) || trimmedLine.endsWith(QLatin1Char(';'))
                    || !trimmedLine.contains(QLatin1Char('('))) {
                    braceDepth = qMax(0, braceDepth + braceDeltaOutsideQuotedText(lines.at(index)));
                    continue;
                }
                pendingSignature = appendSignatureLine({}, line);
                pendingStartLine = index + 1;
            } else {
                pendingSignature = appendSignatureLine(pendingSignature, line);
            }

            if (pendingSignature.endsWith(QLatin1Char(';'))) {
                pendingSignature.clear();
                pendingStartLine = 0;
            }

            if (pendingSignature.contains(QLatin1Char('{'))) {
                const QString description = functionDefinitionDescription(pendingSignature);
                if (!description.isEmpty()) {
                    definitions.append(SourceDefinition{pendingStartLine, description});
                }
                pendingSignature.clear();
                pendingStartLine = 0;
            }
        } else if (!pendingSignature.isEmpty()) {
            pendingSignature.clear();
            pendingStartLine = 0;
        }

        braceDepth = qMax(0, braceDepth + braceDeltaOutsideQuotedText(lines.at(index)));
    }

    return definitions;
}

QString definitionDescriptionWithAccess(const SourceDefinition& definition,
                                        const MemberAccessMap& accessMap)
{
    const QString key = memberFunctionKeyFromDefinition(definition.description);
    if (key.isEmpty() || !accessMap.contains(key)) {
        return definition.description;
    }
    return QStringLiteral("%1 %2").arg(accessMap.value(key), definition.description);
}

std::optional<ShowRange> parseShowRange(const QString& rangeArgument, int availableLineCount)
{
    if (availableLineCount <= 0) {
        return ShowRange{1, 0};
    }

    const QString trimmed = rangeArgument.trimmed();
    if (trimmed.isEmpty()) {
        return ShowRange{1, availableLineCount};
    }

    const QRegularExpression pattern(QStringLiteral(R"(^(\d+)(?::(\d+))?$)"));
    const QRegularExpressionMatch match = pattern.match(trimmed);
    if (!match.hasMatch()) {
        return std::nullopt;
    }

    const int startLine = match.captured(1).toInt();
    const int endLine = match.captured(2).isEmpty() ? startLine : match.captured(2).toInt();
    if (startLine <= 0 || endLine < startLine) {
        return std::nullopt;
    }

    return ShowRange{qMin(startLine, availableLineCount), qMin(endLine, availableLineCount)};
}

QString findProgramInPath(const QString& programName)
{
    const QString foundPath = QStandardPaths::findExecutable(programName);
    if (!foundPath.isEmpty()) {
        return foundPath;
    }
    return {};
}

std::optional<ClipboardCommand> clipboardCommandFromEnvironment(const char* environmentVariable)
{
    const QString commandText = qEnvironmentVariable(environmentVariable).trimmed();
    if (commandText.isEmpty()) {
        return std::nullopt;
    }

    QStringList commandParts = QProcess::splitCommand(commandText);
    if (commandParts.isEmpty()) {
        return std::nullopt;
    }

    ClipboardCommand command;
    command.program = commandParts.takeFirst();
    command.arguments = commandParts;
    return command;
}

std::optional<ClipboardCommand> findClipboardReadCommand()
{
    if (const std::optional<ClipboardCommand> command =
            clipboardCommandFromEnvironment("ICPP_CLIPBOARD_PASTE")) {
        return command;
    }

    struct Candidate {
        const char* program;
        QStringList arguments;
    };
    const Candidate candidates[] = {
        {"pbpaste", {}},
        {"wl-paste", {}},
        {"xclip", {QStringLiteral("-selection"), QStringLiteral("clipboard"), QStringLiteral("-o")}},
        {"xsel", {QStringLiteral("--clipboard"), QStringLiteral("--output")}},
    };

    for (const Candidate& candidate : candidates) {
        const QString programPath = findProgramInPath(QString::fromLatin1(candidate.program));
        if (!programPath.isEmpty()) {
            return ClipboardCommand{programPath, candidate.arguments};
        }
    }
    return std::nullopt;
}

std::optional<ClipboardCommand> findClipboardWriteCommand()
{
    if (const std::optional<ClipboardCommand> command =
            clipboardCommandFromEnvironment("ICPP_CLIPBOARD_COPY")) {
        return command;
    }

    struct Candidate {
        const char* program;
        QStringList arguments;
    };
    const Candidate candidates[] = {
        {"pbcopy", {}},
        {"wl-copy", {}},
        {"xclip", {QStringLiteral("-selection"), QStringLiteral("clipboard")}},
        {"xsel", {QStringLiteral("--clipboard"), QStringLiteral("--input")}},
    };

    for (const Candidate& candidate : candidates) {
        const QString programPath = findProgramInPath(QString::fromLatin1(candidate.program));
        if (!programPath.isEmpty()) {
            return ClipboardCommand{programPath, candidate.arguments};
        }
    }
    return std::nullopt;
}

QString resolveInterpreterProgramImpl(Engine engine,
                                      const QString& commandLinePath,
                                      HelpLanguage helpLanguage,
                                      QTextStream& errorStream)
{
    activeHelpLanguage() = helpLanguage;

    if (!commandLinePath.trimmed().isEmpty()) {
        return commandLinePath;
    }

    const QByteArray envName = engine == Engine::Cling ? QByteArray("ICPP_CLING")
                                                       : QByteArray("ICPP_QTCLING");
    const QString environmentPath = QString::fromLocal8Bit(qgetenv(envName.constData()));
    if (!environmentPath.trimmed().isEmpty()) {
        return environmentPath;
    }

    const QString programName = engineName(engine);
    const QString pathProgram = findProgramInPath(programName);
    if (!pathProgram.isEmpty()) {
        return pathProgram;
    }

    const QString knownPath = engine == Engine::Cling ? QString::fromLatin1(kDefaultClingPath)
                                                      : QString::fromLatin1(kDefaultQtClingPath);
    if (QFileInfo::exists(knownPath)) {
        return knownPath;
    }

    errorStream << trGlobalMessage(QStringLiteral("%1 が見つかりません。--%1 <path> または ICPP_%2 を指定してください。"),
                                   QStringLiteral("Cannot find %1. Use --%1 <path> or ICPP_%2."))
                       .arg(programName, programName.toUpper())
                << Qt::endl;
    return {};
}


} // namespace icpp
