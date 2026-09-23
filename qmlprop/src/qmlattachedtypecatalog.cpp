#include "qmlattachedtypecatalog.h"

#include <QDirIterator>
#include <QFileInfo>
#include <QFile>
#include <QHash>
#include <QRegularExpression>
#include <QQmlEngine>
#include <algorithm>

namespace
{

QString normalizeImportPathValue(const QString& importPath)
{
    if (importPath.startsWith(QStringLiteral("qrc:/"))) {
        return QStringLiteral(":/") + importPath.mid(5);
    }
    return importPath;
}

QString readBalancedBlockValue(const QString& content, qsizetype openingBraceIndex)
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

bool parseBooleanBinding(const QString& block, const QString& bindingName)
{
    const QRegularExpression pattern(
        QStringLiteral(R"qml(\b%1\s*:\s*(true|false))qml").arg(QRegularExpression::escape(bindingName)));
    const QRegularExpressionMatch match = pattern.match(block);
    return match.hasMatch() && match.captured(1) == QStringLiteral("true");
}

QStringList parseExportNames(const QString& componentBlock)
{
    QStringList exportNames;

    static const QRegularExpression exportsPattern(
        QStringLiteral(R"qml(exports\s*:\s*\[(.*?)\])qml"),
        QRegularExpression::DotMatchesEverythingOption);
    static const QRegularExpression stringPattern(QStringLiteral(R"qml("([^"]+)")qml"));

    const QRegularExpressionMatch exportsMatch = exportsPattern.match(componentBlock);
    if (!exportsMatch.hasMatch()) {
        return exportNames;
    }

    const QString exportsList = exportsMatch.captured(1);
    QRegularExpressionMatchIterator stringIterator = stringPattern.globalMatch(exportsList);
    while (stringIterator.hasNext()) {
        exportNames.append(stringIterator.next().captured(1));
    }
    return exportNames;
}

QPair<QString, QString> splitExportName(const QString& exportName)
{
    static const QRegularExpression exportPattern(
        QStringLiteral(R"(^([A-Za-z0-9_.]+)\/([A-Za-z0-9_]+)\s+\d+(?:\.\d+)?$)"));
    const QRegularExpressionMatch match = exportPattern.match(exportName);
    if (!match.hasMatch()) {
        return {};
    }
    return {match.captured(1), match.captured(2)};
}

QString decoratedTypeName(const QString& baseTypeName, bool isList, bool isPointer)
{
    QString typeName = baseTypeName;
    if (typeName.isEmpty()) {
        return QString();
    }

    if (isList) {
        typeName = QStringLiteral("QQmlListProperty<%1>").arg(typeName);
    }
    if (isPointer) {
        typeName += QLatin1Char('*');
    }
    return typeName;
}

QStringList parseEnumKeys(const QString& enumBlock)
{
    QStringList keys;
    const QRegularExpression valuesPattern(
        QStringLiteral(R"qml(values\s*:\s*\{(.*?)\})qml"),
        QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpressionMatch valuesMatch = valuesPattern.match(enumBlock);
    if (!valuesMatch.hasMatch()) {
        return keys;
    }

    const QString valuesBlock = valuesMatch.captured(1);
    const QRegularExpression keyPattern(QStringLiteral(R"qml(\b([A-Za-z_][A-Za-z0-9_]*)\s*:)qml"));
    QRegularExpressionMatchIterator keyIterator = keyPattern.globalMatch(valuesBlock);
    while (keyIterator.hasNext()) {
        keys.append(keyIterator.next().captured(1));
    }
    return keys;
}

} // namespace

struct QmlAttachedTypeCatalog::ComponentMetadata
{
    QString id;
    QString attachedTypeId;
    QList<QPair<QString, QString>> exports;
    QList<QmlAttachedTypeProperty> properties;
    QList<QmlAttachedTypeMethod> methods;
    QList<QmlAttachedTypeEnum> enumerators;
};

QmlAttachedTypeCatalog::QmlAttachedTypeCatalog(const QQmlEngine& qmlEngine)
    : engine(qmlEngine)
{
}

QList<QmlAttachedTypeEntry> QmlAttachedTypeCatalog::collectAttachedTypes() const
{
    QList<ComponentMetadata> componentMetadataList;
    const QStringList importPathList = engine.importPathList();
    for (const QString& importPath : importPathList) {
        collectFromImportPath(importPath, componentMetadataList);
    }

    QHash<QString, ComponentMetadata> metadataById;
    for (const ComponentMetadata& componentMetadata : componentMetadataList) {
        if (!componentMetadata.id.isEmpty() && !metadataById.contains(componentMetadata.id)) {
            metadataById.insert(componentMetadata.id, componentMetadata);
        }
    }

    QList<QmlAttachedTypeEntry> attachedTypeEntries;
    QHash<QString, bool> seenEntries;
    for (const ComponentMetadata& componentMetadata : componentMetadataList) {
        if (componentMetadata.attachedTypeId.isEmpty() || componentMetadata.exports.isEmpty()) {
            continue;
        }

        const ComponentMetadata attachedComponentMetadata = metadataById.value(componentMetadata.attachedTypeId);
        for (const auto& exportedType : componentMetadata.exports) {
            const QString moduleUri = exportedType.first;
            const QString attachingTypeName = exportedType.second;
            const QString entryKey = moduleUri + QStringLiteral("::") + attachingTypeName;
            if (seenEntries.contains(entryKey)) {
                continue;
            }
            seenEntries.insert(entryKey, true);

            QmlAttachedTypeEntry attachedTypeEntry;
            attachedTypeEntry.moduleUri = moduleUri;
            attachedTypeEntry.attachingTypeName = attachingTypeName;
            attachedTypeEntry.attachedTypeId = componentMetadata.attachedTypeId;
            attachedTypeEntry.properties = attachedComponentMetadata.properties;
            attachedTypeEntry.methods = attachedComponentMetadata.methods;
            attachedTypeEntry.enumerators = attachedComponentMetadata.enumerators;
            attachedTypeEntries.append(attachedTypeEntry);
        }
    }

    std::sort(attachedTypeEntries.begin(), attachedTypeEntries.end(), [](const QmlAttachedTypeEntry& left, const QmlAttachedTypeEntry& right) {
        const int moduleComparison = QString::compare(left.moduleUri, right.moduleUri, Qt::CaseInsensitive);
        if (moduleComparison != 0) {
            return moduleComparison < 0;
        }
        return QString::compare(left.attachingTypeName, right.attachingTypeName, Qt::CaseInsensitive) < 0;
    });

    return attachedTypeEntries;
}

QString QmlAttachedTypeCatalog::normalizeImportPath(const QString& importPath)
{
    return normalizeImportPathValue(importPath);
}

QString QmlAttachedTypeCatalog::readBalancedBlock(const QString& content, qsizetype openingBraceIndex)
{
    return readBalancedBlockValue(content, openingBraceIndex);
}

void QmlAttachedTypeCatalog::collectFromImportPath(const QString& importPath,
                                                   QList<ComponentMetadata>& componentMetadataList) const
{
    const QString normalizedImportPath = normalizeImportPath(importPath);
    const QFileInfo importPathInfo(normalizedImportPath);
    if (!importPathInfo.exists() || !importPathInfo.isDir()) {
        return;
    }

    QDirIterator qmlTypesIterator(normalizedImportPath,
                                  QStringList{QStringLiteral("*.qmltypes")},
                                  QDir::Files,
                                  QDirIterator::Subdirectories);
    while (qmlTypesIterator.hasNext()) {
        parseQmlTypesFile(qmlTypesIterator.next(), componentMetadataList);
    }
}

void QmlAttachedTypeCatalog::parseQmlTypesFile(const QString& qmlTypesFilePath,
                                               QList<ComponentMetadata>& componentMetadataList) const
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

        ComponentMetadata componentMetadata;
        componentMetadata.id = parseStringBinding(componentBlock, QStringLiteral("name"));
        componentMetadata.attachedTypeId = parseStringBinding(componentBlock, QStringLiteral("attachedType"));

        const QStringList exportNames = parseExportNames(componentBlock);
        for (const QString& exportName : exportNames) {
            const QPair<QString, QString> exportedType = splitExportName(exportName);
            if (!exportedType.first.isEmpty() && !exportedType.second.isEmpty()) {
                componentMetadata.exports.append(exportedType);
            }
        }

        static const QRegularExpression memberPattern(QStringLiteral(R"(\b(Property|Method|Signal|Enum)\s*\{)"));
        QRegularExpressionMatchIterator memberIterator = memberPattern.globalMatch(componentBlock);
        while (memberIterator.hasNext()) {
            const QRegularExpressionMatch memberMatch = memberIterator.next();
            const QString memberKind = memberMatch.captured(1);
            const qsizetype memberStart = memberMatch.capturedStart();
            const qsizetype memberBraceIndex = componentBlock.indexOf(QLatin1Char('{'), memberStart);
            const QString memberBlock = readBalancedBlock(componentBlock, memberBraceIndex);
            if (memberBlock.isEmpty()) {
                continue;
            }

            if (memberKind == QStringLiteral("Property")) {
                QmlAttachedTypeProperty property;
                property.name = parseStringBinding(memberBlock, QStringLiteral("name"));
                property.typeName = decoratedTypeName(parseStringBinding(memberBlock, QStringLiteral("type")),
                                                      parseBooleanBinding(memberBlock, QStringLiteral("isList")),
                                                      parseBooleanBinding(memberBlock, QStringLiteral("isPointer")));
                property.isReadonly = parseBooleanBinding(memberBlock, QStringLiteral("isReadonly"));
                property.isPointer = parseBooleanBinding(memberBlock, QStringLiteral("isPointer"));
                property.isList = parseBooleanBinding(memberBlock, QStringLiteral("isList"));
                if (!property.name.isEmpty()) {
                    componentMetadata.properties.append(property);
                }
                continue;
            }

            if (memberKind == QStringLiteral("Method") || memberKind == QStringLiteral("Signal")) {
                QmlAttachedTypeMethod method;
                method.name = parseStringBinding(memberBlock, QStringLiteral("name"));
                method.returnTypeName = decoratedTypeName(parseStringBinding(memberBlock, QStringLiteral("type")), false, false);
                method.isSignal = memberKind == QStringLiteral("Signal");

                static const QRegularExpression parameterPattern(QStringLiteral(R"(\bParameter\s*\{)"));
                QRegularExpressionMatchIterator parameterIterator = parameterPattern.globalMatch(memberBlock);
                while (parameterIterator.hasNext()) {
                    const QRegularExpressionMatch parameterMatch = parameterIterator.next();
                    const qsizetype parameterStart = parameterMatch.capturedStart();
                    const qsizetype parameterBraceIndex = memberBlock.indexOf(QLatin1Char('{'), parameterStart);
                    const QString parameterBlock = readBalancedBlock(memberBlock, parameterBraceIndex);
                    if (parameterBlock.isEmpty()) {
                        continue;
                    }

                    QmlAttachedTypeParameter parameter;
                    parameter.name = parseStringBinding(parameterBlock, QStringLiteral("name"));
                    parameter.isPointer = parseBooleanBinding(parameterBlock, QStringLiteral("isPointer"));
                    parameter.isList = parseBooleanBinding(parameterBlock, QStringLiteral("isList"));
                    parameter.typeName = decoratedTypeName(parseStringBinding(parameterBlock, QStringLiteral("type")),
                                                           parameter.isList,
                                                           parameter.isPointer);
                    method.parameters.append(parameter);
                }

                if (!method.name.isEmpty()) {
                    componentMetadata.methods.append(method);
                }
                continue;
            }

            if (memberKind == QStringLiteral("Enum")) {
                QmlAttachedTypeEnum enumerator;
                enumerator.name = parseStringBinding(memberBlock, QStringLiteral("name"));
                enumerator.keys = parseEnumKeys(memberBlock);
                if (!enumerator.name.isEmpty()) {
                    componentMetadata.enumerators.append(enumerator);
                }
            }
        }

        componentMetadataList.append(componentMetadata);
    }
}
