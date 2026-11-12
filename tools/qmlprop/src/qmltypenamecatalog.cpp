#include "qmltypenamecatalog.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QRegularExpression>
#include <QQmlEngine>
#include <algorithm>

namespace
{

QString readBalancedBlock(const QString& content, qsizetype openingBraceIndex)
{
    if (openingBraceIndex < 0 || openingBraceIndex >= content.size() || content.at(openingBraceIndex) != QLatin1Char('{')) {
        return QString();
    }

    qsizetype depth = 1;
    bool isInsideString = false;
    bool isEscaping = false;
    for (qsizetype index = openingBraceIndex + 1; index < content.size(); ++index) {
        const QChar character = content.at(index);
        if (isInsideString) {
            if (isEscaping) {
                isEscaping = false;
                continue;
            }
            if (character == QLatin1Char('\\')) {
                isEscaping = true;
                continue;
            }
            if (character == QLatin1Char('"')) {
                isInsideString = false;
            }
            continue;
        }

        if (character == QLatin1Char('"')) {
            isInsideString = true;
            continue;
        }

        if (character == QLatin1Char('{')) {
            ++depth;
            continue;
        }

        if (character == QLatin1Char('}')) {
            --depth;
            if (depth == 0) {
                return content.mid(openingBraceIndex + 1, index - openingBraceIndex - 1);
            }
        }
    }

    return QString();
}

QString parseStringBinding(const QString& block, const QString& bindingName)
{
    const QRegularExpression pattern(
        QStringLiteral(R"qml(\b%1\s*:\s*"([^"]*)")qml").arg(QRegularExpression::escape(bindingName)));
    const QRegularExpressionMatch match = pattern.match(block);
    return match.hasMatch() ? match.captured(1) : QString();
}

bool parseBooleanBinding(const QString& block, const QString& bindingName, bool defaultValue)
{
    const QRegularExpression pattern(
        QStringLiteral(R"qml(\b%1\s*:\s*(true|false))qml").arg(QRegularExpression::escape(bindingName)));
    const QRegularExpressionMatch match = pattern.match(block);
    if (!match.hasMatch()) {
        return defaultValue;
    }
    return match.captured(1) == QStringLiteral("true");
}

bool isReferenceTypeComponent(const QString& componentBlock)
{
    const QString accessSemantics = parseStringBinding(componentBlock, QStringLiteral("accessSemantics"));
    if (accessSemantics.isEmpty()) {
        return true;
    }

    return QString::compare(accessSemantics, QStringLiteral("reference"), Qt::CaseInsensitive) == 0;
}

QList<QPair<QString, QString>> exportedTypesFromComponent(const QString& componentBlock)
{
    QList<QPair<QString, QString>> exportedTypes;

    static const QRegularExpression exportsPattern(
        QStringLiteral(R"qml(exports\s*:\s*\[(.*?)\])qml"),
        QRegularExpression::DotMatchesEverythingOption);
    static const QRegularExpression stringPattern(QStringLiteral(R"qml("([^"]+)")qml"));
    static const QRegularExpression exportPattern(
        QStringLiteral(R"(^([A-Za-z0-9_.]+)\/([A-Za-z0-9_]+)\s+\d+(?:\.\d+)?$)"));

    const QRegularExpressionMatch exportsMatch = exportsPattern.match(componentBlock);
    if (!exportsMatch.hasMatch()) {
        return exportedTypes;
    }

    const QString exportsList = exportsMatch.captured(1);
    QRegularExpressionMatchIterator stringIterator = stringPattern.globalMatch(exportsList);
    while (stringIterator.hasNext()) {
        const QRegularExpressionMatch stringMatch = stringIterator.next();
        const QString exportedName = stringMatch.captured(1);
        const QRegularExpressionMatch exportMatch = exportPattern.match(exportedName);
        if (!exportMatch.hasMatch()) {
            continue;
        }

        exportedTypes.append({exportMatch.captured(1), exportMatch.captured(2)});
    }

    return exportedTypes;
}

} // namespace

QmlTypeNameCatalog::QmlTypeNameCatalog(const QQmlEngine& qmlEngine)
    : engine(qmlEngine)
{
}

QList<QmlTypeNameEntry> QmlTypeNameCatalog::collectTypeNames(bool includeValueTypes) const
{
    QHash<QString, QmlTypeNameEntry> collectedEntries;
    const QStringList importPathList = engine.importPathList();
    for (const QString& importPath : importPathList) {
        collectFromImportPath(importPath, collectedEntries);
    }

    QList<QmlTypeNameEntry> typeNames;
    for (const QmlTypeNameEntry& entry : collectedEntries.values()) {
        if (!includeValueTypes && entry.isValueType) {
            continue;
        }
        typeNames.append(entry);
    }
    std::sort(typeNames.begin(), typeNames.end(), [](const QmlTypeNameEntry& left, const QmlTypeNameEntry& right) {
        const int moduleComparison = QString::compare(left.moduleUri, right.moduleUri, Qt::CaseInsensitive);
        if (moduleComparison != 0) {
            return moduleComparison < 0;
        }
        return QString::compare(left.typeName, right.typeName, Qt::CaseInsensitive) < 0;
    });
    return typeNames;
}

QStringList QmlTypeNameCatalog::collectModuleUris(bool includeValueTypes) const
{
    const QList<QmlTypeNameEntry> typeNames = collectTypeNames(includeValueTypes);

    QStringList moduleUris;
    moduleUris.reserve(typeNames.size());
    for (const QmlTypeNameEntry& entry : typeNames) {
        if (!entry.moduleUri.isEmpty() && !moduleUris.contains(entry.moduleUri)) {
            moduleUris.append(entry.moduleUri);
        }
    }

    std::sort(moduleUris.begin(), moduleUris.end(), [](const QString& left, const QString& right) {
        return QString::compare(left, right, Qt::CaseInsensitive) < 0;
    });
    return moduleUris;
}

QString QmlTypeNameCatalog::normalizeImportPath(const QString& importPath)
{
    if (importPath.startsWith(QStringLiteral("qrc:/"))) {
        return QStringLiteral(":/") + importPath.mid(5);
    }
    return importPath;
}

QString QmlTypeNameCatalog::stripComment(const QString& line)
{
    const qsizetype commentIndex = line.indexOf(QLatin1Char('#'));
    if (commentIndex < 0) {
        return line.trimmed();
    }
    return line.left(commentIndex).trimmed();
}

bool QmlTypeNameCatalog::looksLikeVersion(const QString& token)
{
    static const QRegularExpression versionPattern(QStringLiteral(R"(^\d+(?:\.\d+)?$)"));
    return versionPattern.match(token).hasMatch();
}

QString QmlTypeNameCatalog::inferredModuleUri(const QString& normalizedImportPath, const QString& moduleDirectoryPath)
{
    QDir importDirectory(normalizedImportPath);
    const QString relativePath = importDirectory.relativeFilePath(moduleDirectoryPath);
    if (relativePath.isEmpty() || relativePath == QStringLiteral(".")) {
        return QString();
    }

    QString moduleUri = QDir::fromNativeSeparators(relativePath);
    moduleUri.replace(QLatin1Char('/'), QLatin1Char('.'));
    return moduleUri;
}

QString QmlTypeNameCatalog::typeKey(const QString& moduleUri, const QString& typeName)
{
    return moduleUri + QStringLiteral("::") + typeName;
}

void QmlTypeNameCatalog::collectFromImportPath(const QString& importPath,
                                               QHash<QString, QmlTypeNameEntry>& collectedEntries) const
{
    const QString normalizedImportPath = normalizeImportPath(importPath);
    const QFileInfo importPathInfo(normalizedImportPath);
    if (!importPathInfo.exists() || !importPathInfo.isDir()) {
        return;
    }

    QDirIterator qmldirIterator(normalizedImportPath,
                                QStringList{QStringLiteral("qmldir")},
                                QDir::Files,
                                QDirIterator::Subdirectories);
    while (qmldirIterator.hasNext()) {
        parseQmldirFile(normalizedImportPath, qmldirIterator.next(), collectedEntries);
    }

    QDirIterator qmlTypesIterator(normalizedImportPath,
                                  QStringList{QStringLiteral("*.qmltypes")},
                                  QDir::Files,
                                  QDirIterator::Subdirectories);
    while (qmlTypesIterator.hasNext()) {
        parseQmlTypesFile(qmlTypesIterator.next(), collectedEntries);
    }
}

void QmlTypeNameCatalog::parseQmldirFile(const QString& normalizedImportPath,
                                         const QString& qmldirFilePath,
                                         QHash<QString, QmlTypeNameEntry>& collectedEntries) const
{
    QFile qmldirFile(qmldirFilePath);
    if (!qmldirFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    const QFileInfo qmldirFileInfo(qmldirFilePath);
    QString moduleUri = inferredModuleUri(normalizedImportPath, qmldirFileInfo.absolutePath());

    while (!qmldirFile.atEnd()) {
        const QString line = stripComment(QString::fromUtf8(qmldirFile.readLine()));
        if (line.isEmpty()) {
            continue;
        }

        const QStringList tokens = line.simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (tokens.isEmpty()) {
            continue;
        }

        if (tokens.constFirst() == QStringLiteral("module") && tokens.size() >= 2) {
            moduleUri = tokens.at(1);
            continue;
        }

        if (tokens.constFirst() == QStringLiteral("internal") ||
            tokens.constFirst() == QStringLiteral("plugin") ||
            tokens.constFirst() == QStringLiteral("classname") ||
            tokens.constFirst() == QStringLiteral("typeinfo") ||
            tokens.constFirst() == QStringLiteral("depends") ||
            tokens.constFirst() == QStringLiteral("import") ||
            tokens.constFirst() == QStringLiteral("optional") ||
            tokens.constFirst() == QStringLiteral("prefer") ||
            tokens.constFirst() == QStringLiteral("designersupported")) {
            continue;
        }

        int firstTokenIndex = 0;
        bool isSingleton = false;
        if (tokens.constFirst() == QStringLiteral("singleton")) {
            firstTokenIndex = 1;
            isSingleton = true;
        }

        if (tokens.size() - firstTokenIndex < 3) {
            continue;
        }

        const QString& typeName = tokens.at(firstTokenIndex);
        const QString& initialVersion = tokens.at(firstTokenIndex + 1);
        if (!looksLikeVersion(initialVersion)) {
            continue;
        }

        if (typeName.isEmpty() || !typeName.at(0).isUpper()) {
            continue;
        }

        addEntry(moduleUri, typeName, true, isSingleton, true, false, false, collectedEntries);
    }
}

void QmlTypeNameCatalog::parseQmlTypesFile(const QString& qmlTypesFilePath,
                                           QHash<QString, QmlTypeNameEntry>& collectedEntries) const
{
    QFile qmlTypesFile(qmlTypesFilePath);
    if (!qmlTypesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    const QString content = QString::fromUtf8(qmlTypesFile.readAll());
    static const QRegularExpression componentPattern(QStringLiteral(R"(\bComponent\s*\{)"));

    QRegularExpressionMatchIterator componentIterator = componentPattern.globalMatch(content);
    while (componentIterator.hasNext()) {
        const QRegularExpressionMatch componentMatch = componentIterator.next();
        const qsizetype componentStart = componentMatch.capturedStart();
        const qsizetype openingBraceIndex = content.indexOf(QLatin1Char('{'), componentStart);
        const QString componentBlock = readBalancedBlock(content, openingBraceIndex);
        if (componentBlock.isEmpty()) {
            continue;
        }

        const bool isReferenceType = isReferenceTypeComponent(componentBlock);
        const bool isValueType = !isReferenceType;

        const bool isCreatable = parseBooleanBinding(componentBlock, QStringLiteral("isCreatable"), true);
        const bool isSingleton = parseBooleanBinding(componentBlock, QStringLiteral("isSingleton"), false);
        const bool hasAttachedType = !parseStringBinding(componentBlock, QStringLiteral("attachedType")).isEmpty();

        const QList<QPair<QString, QString>> exportedTypes = exportedTypesFromComponent(componentBlock);
        for (const auto& exportedType : exportedTypes) {
            const QString& moduleUri = exportedType.first;
            const QString& typeName = exportedType.second;
            if (typeName.isEmpty() || !typeName.at(0).isUpper()) {
                continue;
            }
            addEntry(moduleUri, typeName, isCreatable, isSingleton, isReferenceType, hasAttachedType, isValueType, collectedEntries);
        }
    }
}

void QmlTypeNameCatalog::addEntry(const QString& moduleUri,
                                  const QString& typeName,
                                  bool isCreatable,
                                  bool isSingleton,
                                  bool isReferenceType,
                                  bool hasAttachedType,
                                  bool isValueType,
                                  QHash<QString, QmlTypeNameEntry>& collectedEntries) const
{
    if (moduleUri.isEmpty() || typeName.isEmpty()) {
        return;
    }

    const QString key = typeKey(moduleUri, typeName);
    QmlTypeNameEntry& entry = collectedEntries[key];
    entry.moduleUri = moduleUri;
    entry.typeName = typeName;
    entry.isCreatable = entry.isCreatable && isCreatable;
    entry.isSingleton = entry.isSingleton || isSingleton;
    entry.isReferenceType = entry.isReferenceType && isReferenceType;
    entry.hasAttachedType = entry.hasAttachedType || hasAttachedType;
    entry.isValueType = entry.isValueType || isValueType;
}
