#include "commandlineoptions.h"
#include "metaobjectprinter.h"
#include "qmlattachedtypecatalog.h"
#include "qmlmodulecatalog.h"
#include "qmltypenamecatalog.h"
#include "qmltypeinspector.h"

#include <QGuiApplication>
#include <QMetaEnum>
#include <QMetaMethod>
#include <QMetaObject>
#include <QMetaProperty>
#include <QProcess>
#include <QQmlEngine>
#include <QQmlProperty>
#include <QRegularExpression>
#include <QTextStream>
#include <algorithm>
#include <utility>
#include <vector>

#if defined(Q_OS_UNIX)
#include <unistd.h>
#endif

namespace
{

struct FailureDiagnosis
{
    QString reason = QStringLiteral("unknown");
    QString details;
};

struct PagerCommand
{
    QString program;
    QStringList arguments;
};

struct ModuleIndexRow
{
    QString typeName;
    QString baseName = QStringLiteral("-");
    QString flags = QStringLiteral("-");
    QString propertyCount = QStringLiteral("-");
    QString methodCount = QStringLiteral("-");
    QString enumeratorCount = QStringLiteral("-");
    QString defaultProperty = QStringLiteral("-");
    QString status = QStringLiteral("failed");
};

bool standardOutputIsTerminal()
{
#if defined(Q_OS_UNIX)
    return ::isatty(STDOUT_FILENO) != 0;
#else
    return false;
#endif
}

PagerCommand pagerCommand()
{
    const QString pagerEnvironmentValue = qEnvironmentVariable("PAGER").trimmed();
    if (!pagerEnvironmentValue.isEmpty()) {
        const QStringList pagerTokens = QProcess::splitCommand(pagerEnvironmentValue);
        if (!pagerTokens.isEmpty()) {
            PagerCommand command;
            command.program = pagerTokens.constFirst();
            command.arguments = pagerTokens.mid(1);
            return command;
        }
    }

    PagerCommand command;
    command.program = QStringLiteral("less");
    command.arguments = {QStringLiteral("-FRX")};
    return command;
}

bool writeThroughPager(const QString& text, QTextStream& standardError)
{
    const PagerCommand command = pagerCommand();
    if (command.program.isEmpty()) {
        return false;
    }

    QProcess pagerProcess;
    pagerProcess.setProcessChannelMode(QProcess::ForwardedChannels);
    pagerProcess.start(command.program, command.arguments, QIODevice::ReadWrite);
    if (!pagerProcess.waitForStarted()) {
        PagerCommand fallbackCommand;
        fallbackCommand.program = QStringLiteral("more");
        QProcess fallbackPagerProcess;
        fallbackPagerProcess.setProcessChannelMode(QProcess::ForwardedChannels);
        fallbackPagerProcess.start(fallbackCommand.program, fallbackCommand.arguments, QIODevice::ReadWrite);
        if (!fallbackPagerProcess.waitForStarted()) {
            standardError << "Warning: could not start pager '" << command.program
                          << "' or fallback pager 'more'. Writing directly to standard output." << Qt::endl;
            return false;
        }

        fallbackPagerProcess.write(text.toUtf8());
        fallbackPagerProcess.closeWriteChannel();
        fallbackPagerProcess.waitForFinished(-1);
        return true;
    }

    pagerProcess.write(text.toUtf8());
    pagerProcess.closeWriteChannel();
    pagerProcess.waitForFinished(-1);
    return true;
}

void writeBufferedOutput(const QString& text, const CommandLineOptions& commandLineOptions, QTextStream& standardOutput, QTextStream& standardError)
{
    if (text.isEmpty()) {
        return;
    }

    if (!commandLineOptions.usePager || !standardOutputIsTerminal()) {
        standardOutput << text;
        standardOutput.flush();
        return;
    }

    if (writeThroughPager(text, standardError)) {
        return;
    }

    standardOutput << text;
    standardOutput.flush();
}

QStringList typeFlags(const QmlTypeNameEntry& entry)
{
    if (!entry.typeFlagsKnown) {
        return {};
    }

    QStringList flags;
    flags.append(entry.isCreatable ? QStringLiteral("creatable") : QStringLiteral("uncreatable"));
    if (entry.isSingleton) {
        flags.append(QStringLiteral("singleton"));
    }
    if (entry.hasAttachedType) {
        flags.append(QStringLiteral("attached provider"));
    }
    if (entry.isValueType) {
        flags.append(QStringLiteral("value type"));
    }
    return flags;
}

const QMetaObject* effectiveMetaObject(const QObject* inspectedObject)
{
    const QMetaObject* metaObject = inspectedObject ? inspectedObject->metaObject() : nullptr;
    while (metaObject && metaObject->superClass()) {
        const QByteArray className(metaObject->className());
        if (!className.contains("_QMLTYPE_") && !className.contains("_QML_")) {
            break;
        }
        metaObject = metaObject->superClass();
    }
    return metaObject;
}

QString defaultPropertyName(const QObject* inspectedObject, QQmlEngine& qmlEngine)
{
    const QQmlProperty defaultQmlProperty(const_cast<QObject*>(inspectedObject), &qmlEngine);
    if (!defaultQmlProperty.isValid()) {
        return QString();
    }
    return defaultQmlProperty.name();
}

int ownVisibleMethodCount(const QMetaObject* metaObject)
{
    if (!metaObject) {
        return 0;
    }

    int count = 0;
    for (int methodIndex = metaObject->methodOffset(); methodIndex < metaObject->methodCount(); ++methodIndex) {
        const QMetaMethod metaMethod = metaObject->method(methodIndex);
        if (metaMethod.access() != QMetaMethod::Private) {
            ++count;
        }
    }
    return count;
}

QString paddedCell(const QString& text, qsizetype width)
{
    return text.left(width).leftJustified(width, QLatin1Char(' '));
}

QString unqualifiedTypeName(const QString& requestedTypeName)
{
    return requestedTypeName.section(QLatin1Char('.'), -1);
}

QStringList baseModuleUris();

const QmlTypeNameEntry* resolveTypeMetadata(const QList<QmlTypeNameEntry>& typeNameEntries,
                                            const QString& requestedTypeName,
                                            const QStringList& importedModuleUris,
                                            bool allowNameOnlyFallback = true)
{
    const QString requestedElementName = unqualifiedTypeName(requestedTypeName);

    for (const QString& importedModuleUri : importedModuleUris) {
        for (const QmlTypeNameEntry& entry : typeNameEntries) {
            if (entry.typeName == requestedElementName && entry.moduleUri == importedModuleUri) {
                return &entry;
            }
        }
    }

    if (!allowNameOnlyFallback) {
        return nullptr;
    }

    for (const QmlTypeNameEntry& entry : typeNameEntries) {
        if (entry.typeName == requestedElementName) {
            return &entry;
        }
    }

    return nullptr;
}

QList<const QmlTypeNameEntry*> matchingTypeMetadataEntries(const QList<QmlTypeNameEntry>& typeNameEntries,
                                                           const QString& requestedTypeName)
{
    QList<const QmlTypeNameEntry*> matchingEntries;
    const QString requestedElementName = unqualifiedTypeName(requestedTypeName);
    for (const QmlTypeNameEntry& entry : typeNameEntries) {
        if (entry.typeName == requestedElementName) {
            matchingEntries.append(&entry);
        }
    }
    return matchingEntries;
}

QString importedModuleUri(const QString& importStatement)
{
    if (!importStatement.startsWith(QStringLiteral("import "))) {
        return QString();
    }

    QString remainder = importStatement.mid(QStringLiteral("import ").size()).trimmed();
    const qsizetype asIndex = remainder.indexOf(QStringLiteral(" as "));
    if (asIndex >= 0) {
        remainder = remainder.left(asIndex).trimmed();
    }

    return remainder.section(QLatin1Char(' '), 0, 0).trimmed();
}

QString primaryModuleUri(const ImportProfile& importProfile)
{
    const QStringList implicitBaseModuleUris = baseModuleUris();

    QString selectedModuleUri;
    for (const QString& importStatement : importProfile.importStatements) {
        const QString moduleUri = importedModuleUri(importStatement);
        if (moduleUri.isEmpty() || implicitBaseModuleUris.contains(moduleUri)) {
            continue;
        }
        selectedModuleUri = moduleUri;
    }
    return selectedModuleUri;
}

QList<QmlTypeNameEntry> uniqueMatchingTypeEntries(const QList<QmlTypeNameEntry>& typeNameEntries,
                                                  const QString& requestedTypeName)
{
    QHash<QString, QmlTypeNameEntry> uniqueEntries;
    const QString requestedElementName = unqualifiedTypeName(requestedTypeName);
    for (const QmlTypeNameEntry& entry : typeNameEntries) {
        if (entry.typeName != requestedElementName) {
            continue;
        }
        const QString key = entry.moduleUri + QStringLiteral("::") + entry.typeName;
        if (!uniqueEntries.contains(key)) {
            uniqueEntries.insert(key, entry);
        }
    }

    QList<QmlTypeNameEntry> matchingEntries = uniqueEntries.values();
    std::sort(matchingEntries.begin(), matchingEntries.end(), [](const QmlTypeNameEntry& left, const QmlTypeNameEntry& right) {
        const int moduleComparison = QString::compare(left.moduleUri, right.moduleUri, Qt::CaseInsensitive);
        if (moduleComparison != 0) {
            return moduleComparison < 0;
        }
        return QString::compare(left.typeName, right.typeName, Qt::CaseInsensitive) < 0;
    });
    return matchingEntries;
}

QStringList uniqueSortedStrings(QStringList values)
{
    std::sort(values.begin(), values.end(), [](const QString& left, const QString& right) {
        return QString::compare(left, right, Qt::CaseInsensitive) < 0;
    });
    values.erase(std::unique(values.begin(), values.end(), [](const QString& left, const QString& right) {
        return QString::compare(left, right, Qt::CaseInsensitive) == 0;
    }), values.end());
    return values;
}

QStringList candidateModuleUrisForAllMatches(const QList<QmlTypeNameEntry>& typeNameEntries,
                                             const QString& requestedTypeName)
{
    QStringList moduleUris;
    const QList<QmlTypeNameEntry> matchingEntries = uniqueMatchingTypeEntries(typeNameEntries, requestedTypeName);
    for (const QmlTypeNameEntry& entry : matchingEntries) {
        if (!entry.moduleUri.isEmpty()) {
            moduleUris.append(entry.moduleUri);
        }
    }

    const QList<ImportProfile> importProfiles = QmlModuleCatalog::profilesForType(requestedTypeName);
    for (const ImportProfile& importProfile : importProfiles) {
        const QString moduleUri = primaryModuleUri(importProfile);
        if (!moduleUri.isEmpty() && !moduleUris.contains(moduleUri)) {
            moduleUris.append(moduleUri);
        }
    }

    return uniqueSortedStrings(moduleUris);
}

QStringList candidateModuleUris(const QList<const QmlTypeNameEntry*>& matchingEntries)
{
    QStringList moduleUris;
    for (const QmlTypeNameEntry* entry : matchingEntries) {
        if (!entry || entry->moduleUri.isEmpty()) {
            continue;
        }
        moduleUris.append(entry->moduleUri);
    }
    return uniqueSortedStrings(moduleUris);
}

QStringList attemptedModuleUris(const QmlTypeInspectionResult& inspectionResult)
{
    QStringList moduleUris;
    for (const QmlTypeInspectionAttempt& failedAttempt : inspectionResult.failedAttempts) {
        moduleUris.append(failedAttempt.moduleUris);
    }
    return uniqueSortedStrings(moduleUris);
}

QStringList baseModuleUris()
{
    return {
        QStringLiteral("QtCore"),
        QStringLiteral("QtQml"),
        QStringLiteral("QtQuick")
    };
}

QString preferredSelectedModuleUri(const QmlTypeInspectionResult& inspectionResult)
{
    const QStringList implicitBaseModuleUris = baseModuleUris();
    for (auto moduleUriIterator = inspectionResult.selectedModuleUris.crbegin();
         moduleUriIterator != inspectionResult.selectedModuleUris.crend();
         ++moduleUriIterator) {
        if (!implicitBaseModuleUris.contains(*moduleUriIterator)) {
            return *moduleUriIterator;
        }
    }

    if (!inspectionResult.selectedModuleUris.isEmpty()) {
        return inspectionResult.selectedModuleUris.constLast();
    }

    return QString();
}

QStringList allErrorTexts(const QmlTypeInspectionResult& inspectionResult)
{
    QStringList errorTexts;
    for (const QmlTypeInspectionAttempt& failedAttempt : inspectionResult.failedAttempts) {
        for (const QQmlError& error : failedAttempt.errors) {
            errorTexts.append(error.toString());
        }
    }
    if (errorTexts.isEmpty()) {
        for (const QQmlError& error : inspectionResult.errors) {
            errorTexts.append(error.toString());
        }
    }
    return errorTexts;
}

QString firstMatchingError(const QStringList& errorTexts, const QStringList& needles)
{
    for (const QString& errorText : errorTexts) {
        const QString loweredErrorText = errorText.toLower();
        for (const QString& needle : needles) {
            if (loweredErrorText.contains(needle)) {
                return errorText;
            }
        }
    }
    return QString();
}

QString firstCapturedGroup(const QStringList& errorTexts, const QRegularExpression& regularExpression)
{
    for (const QString& errorText : errorTexts) {
        const QRegularExpressionMatch match = regularExpression.match(errorText);
        if (match.hasMatch()) {
            return match.captured(1);
        }
    }
    return QString();
}

void printTypeNameList(const QList<QmlTypeNameEntry>& typeNameEntries, QTextStream& outputStream)
{
    qsizetype longestModuleNameLength = 0;
    qsizetype longestTypeNameLength = 0;
    for (const QmlTypeNameEntry& entry : typeNameEntries) {
        longestModuleNameLength = std::max(longestModuleNameLength, entry.moduleUri.size());
        longestTypeNameLength = std::max(longestTypeNameLength, entry.typeName.size());
    }

    outputStream << "Found " << typeNameEntries.size() << " QML type names." << Qt::endl;
    for (const QmlTypeNameEntry& entry : typeNameEntries) {
        outputStream << entry.moduleUri.leftJustified(longestModuleNameLength, QLatin1Char(' '))
                     << " : "
                     << entry.typeName.leftJustified(longestTypeNameLength, QLatin1Char(' '));

        const QStringList flags = typeFlags(entry);
        if (!flags.isEmpty()) {
            outputStream << " [" << flags.join(QStringLiteral(", ")) << ']';
        }
        outputStream << Qt::endl;
    }
}

void printModuleUriList(const QStringList& moduleUris, QTextStream& outputStream)
{
    outputStream << "Found " << moduleUris.size() << " QML modules." << Qt::endl;
    for (const QString& moduleUri : moduleUris) {
        outputStream << moduleUri << Qt::endl;
    }
}

int printModuleIndex(const QString& moduleUri,
                     const QList<QmlTypeNameEntry>& typeNameEntries,
                     QmlTypeInspector& qmlTypeInspector,
                     QQmlEngine& qmlEngine,
                     QTextStream& outputStream)
{
    QList<QmlTypeNameEntry> moduleTypeEntries;
    for (const QmlTypeNameEntry& entry : typeNameEntries) {
        if (entry.moduleUri == moduleUri) {
            moduleTypeEntries.append(entry);
        }
    }

    if (moduleTypeEntries.isEmpty()) {
        outputStream << "No types were found for module: " << moduleUri << Qt::endl;
        return 1;
    }

    std::sort(moduleTypeEntries.begin(), moduleTypeEntries.end(), [](const QmlTypeNameEntry& left, const QmlTypeNameEntry& right) {
        return QString::compare(left.typeName, right.typeName, Qt::CaseInsensitive) < 0;
    });

    QList<ModuleIndexRow> rows;
    rows.reserve(moduleTypeEntries.size());
    for (const QmlTypeNameEntry& entry : moduleTypeEntries) {
        ModuleIndexRow row;
        row.typeName = entry.typeName;

        const QStringList flags = typeFlags(entry);
        if (!flags.isEmpty()) {
            row.flags = flags.join(QStringLiteral(", "));
        }

        const QList<ImportProfile> importProfiles = QmlModuleCatalog::profilesForTypeInModule(entry.typeName, moduleUri);
        if (importProfiles.isEmpty()) {
            row.status = QStringLiteral("no profile");
            rows.append(row);
            continue;
        }

        const QmlTypeInspectionResult inspectionResult = qmlTypeInspector.inspectTypeWithProfiles(entry.typeName,
                                                                                                   {},
                                                                                                   importProfiles,
                                                                                                   0);
        if (!inspectionResult.isSuccessful || !inspectionResult.inspectedObject) {
            row.status = QStringLiteral("failed");
            rows.append(row);
            continue;
        }

        const QMetaObject* metaObject = effectiveMetaObject(inspectionResult.inspectedObject);
        if (metaObject) {
            if (const QMetaObject* superClass = metaObject->superClass()) {
                row.baseName = QString::fromLatin1(superClass->className());
            }
            row.propertyCount = QString::number(metaObject->propertyCount() - metaObject->propertyOffset());
            row.methodCount = QString::number(ownVisibleMethodCount(metaObject));
            row.enumeratorCount = QString::number(metaObject->enumeratorCount() - metaObject->enumeratorOffset());
        } else {
            row.baseName = QStringLiteral("<unknown>");
            row.propertyCount = QStringLiteral("0");
            row.methodCount = QStringLiteral("0");
            row.enumeratorCount = QStringLiteral("0");
        }

        const QString currentDefaultPropertyName = defaultPropertyName(inspectionResult.inspectedObject, qmlEngine);
        if (!currentDefaultPropertyName.isEmpty()) {
            row.defaultProperty = currentDefaultPropertyName;
        }

        row.status = QStringLiteral("ok");
        rows.append(row);
    }

    qsizetype typeWidth = QStringLiteral("Type").size();
    qsizetype baseWidth = QStringLiteral("Base").size();
    qsizetype flagsWidth = QStringLiteral("Flags").size();
    qsizetype propsWidth = QStringLiteral("Props").size();
    qsizetype methodsWidth = QStringLiteral("Methods").size();
    qsizetype enumsWidth = QStringLiteral("Enums").size();
    qsizetype defaultWidth = QStringLiteral("Default").size();
    qsizetype statusWidth = QStringLiteral("Status").size();
    for (const ModuleIndexRow& row : rows) {
        typeWidth = std::max(typeWidth, row.typeName.size());
        baseWidth = std::max(baseWidth, row.baseName.size());
        flagsWidth = std::max(flagsWidth, row.flags.size());
        propsWidth = std::max(propsWidth, row.propertyCount.size());
        methodsWidth = std::max(methodsWidth, row.methodCount.size());
        enumsWidth = std::max(enumsWidth, row.enumeratorCount.size());
        defaultWidth = std::max(defaultWidth, row.defaultProperty.size());
        statusWidth = std::max(statusWidth, row.status.size());
    }

    outputStream << "Module: " << moduleUri << Qt::endl;
    outputStream << "Found " << rows.size() << " types." << Qt::endl << Qt::endl;
    outputStream << paddedCell(QStringLiteral("Type"), typeWidth) << "  "
                 << paddedCell(QStringLiteral("Base"), baseWidth) << "  "
                 << paddedCell(QStringLiteral("Flags"), flagsWidth) << "  "
                 << paddedCell(QStringLiteral("Props"), propsWidth) << "  "
                 << paddedCell(QStringLiteral("Methods"), methodsWidth) << "  "
                 << paddedCell(QStringLiteral("Enums"), enumsWidth) << "  "
                 << paddedCell(QStringLiteral("Default"), defaultWidth) << "  "
                 << paddedCell(QStringLiteral("Status"), statusWidth) << Qt::endl;
    for (const ModuleIndexRow& row : rows) {
        outputStream << paddedCell(row.typeName, typeWidth) << "  "
                     << paddedCell(row.baseName, baseWidth) << "  "
                     << paddedCell(row.flags, flagsWidth) << "  "
                     << paddedCell(row.propertyCount, propsWidth) << "  "
                     << paddedCell(row.methodCount, methodsWidth) << "  "
                     << paddedCell(row.enumeratorCount, enumsWidth) << "  "
                     << paddedCell(row.defaultProperty, defaultWidth) << "  "
                     << paddedCell(row.status, statusWidth) << Qt::endl;
    }

    return 0;
}

FailureDiagnosis diagnoseInstantiationFailure(const QList<QmlTypeNameEntry>& typeNameEntries,
                                              const QmlTypeInspectionResult& inspectionResult,
                                              const QString& requestedTypeName)
{
    FailureDiagnosis diagnosis;
    const QList<const QmlTypeNameEntry*> matchingEntries = matchingTypeMetadataEntries(typeNameEntries, requestedTypeName);
    const QmlTypeNameEntry* selectedMetadata = resolveTypeMetadata(typeNameEntries,
                                                                   requestedTypeName,
                                                                   attemptedModuleUris(inspectionResult));
    const QStringList errorTexts = allErrorTexts(inspectionResult);

    const QString requiredPropertyName = firstCapturedGroup(
        errorTexts,
        QRegularExpression(QStringLiteral("(?:missing a required property|required property)\\s+\"?([A-Za-z_][A-Za-z0-9_]*)\"?"),
                           QRegularExpression::CaseInsensitiveOption));
    if (!requiredPropertyName.isEmpty()) {
        diagnosis.reason = QStringLiteral("required property not set");
        diagnosis.details = QStringLiteral("Required property '%1' is not initialized.").arg(requiredPropertyName);
        return diagnosis;
    }

    const QString requiredPropertyError = firstMatchingError(
        errorTexts,
        {QStringLiteral("required property"), QStringLiteral("missing a required property")});
    if (!requiredPropertyError.isEmpty()) {
        diagnosis.reason = QStringLiteral("required property not set");
        diagnosis.details = requiredPropertyError;
        return diagnosis;
    }

    if (selectedMetadata && selectedMetadata->isValueType) {
        diagnosis.reason = QStringLiteral("value type");
        diagnosis.details = QStringLiteral("%1 is registered as a value type in module %2 and cannot be instantiated as a QObject-based QML object.")
                                .arg(unqualifiedTypeName(requestedTypeName), selectedMetadata->moduleUri);
        return diagnosis;
    }

    const QString valueTypeError = firstMatchingError(
        errorTexts,
        {QStringLiteral("value type"), QStringLiteral("not a reference type")});
    if (!valueTypeError.isEmpty()) {
        diagnosis.reason = QStringLiteral("value type");
        diagnosis.details = valueTypeError;
        return diagnosis;
    }

    const QString abstractTypeError = firstMatchingError(
        errorTexts,
        {QStringLiteral("abstract"), QStringLiteral("is abstract")});
    if (!abstractTypeError.isEmpty()) {
        diagnosis.reason = QStringLiteral("abstract type");
        diagnosis.details = abstractTypeError;
        return diagnosis;
    }

    const QString attachedOnlyError = firstMatchingError(
        errorTexts,
        {QStringLiteral("attached properties"), QStringLiteral("attached property"), QStringLiteral("attached type")});
    if (!attachedOnlyError.isEmpty()) {
        diagnosis.reason = QStringLiteral("attached-only type");
        diagnosis.details = attachedOnlyError;
        return diagnosis;
    }

    const QStringList availableModuleUris = candidateModuleUris(matchingEntries);
    const QStringList importedModuleUris = attemptedModuleUris(inspectionResult);
    bool anyCandidateModuleWasTried = false;
    for (const QString& moduleUri : availableModuleUris) {
        if (importedModuleUris.contains(moduleUri)) {
            anyCandidateModuleWasTried = true;
            break;
        }
    }

    const QString missingTypeError = firstMatchingError(
        errorTexts,
        {QStringLiteral("is not a type"),
         QStringLiteral("unknown component"),
         QStringLiteral("does not name a type"),
         QStringLiteral("is unavailable")});
    if (!availableModuleUris.isEmpty() && !anyCandidateModuleWasTried && !missingTypeError.isEmpty()) {
        diagnosis.reason = QStringLiteral("module import missing");
        diagnosis.details = QStringLiteral("%1 exists in module(s): %2. None of the attempted profiles imported those module(s).")
                                .arg(unqualifiedTypeName(requestedTypeName), availableModuleUris.join(QStringLiteral(", ")));
        return diagnosis;
    }

    const QString dependencyError = firstMatchingError(
        errorTexts,
        {QStringLiteral("is not installed"),
         QStringLiteral("plugin cannot be loaded"),
         QStringLiteral("dependency"),
         QStringLiteral("depends on")});
    if (!dependencyError.isEmpty()) {
        diagnosis.reason = QStringLiteral("dependent type missing");
        diagnosis.details = dependencyError;
        return diagnosis;
    }

    if (selectedMetadata && !selectedMetadata->isCreatable) {
        diagnosis.reason = QStringLiteral("uncreatable type");
        diagnosis.details = QStringLiteral("%1 is registered in module %2, but its type metadata marks it as uncreatable.")
                                .arg(unqualifiedTypeName(requestedTypeName), selectedMetadata->moduleUri);
        return diagnosis;
    }

    if (!missingTypeError.isEmpty() && !availableModuleUris.isEmpty()) {
        diagnosis.reason = QStringLiteral("module import missing");
        diagnosis.details = QStringLiteral("%1 exists in module(s): %2. The attempted profiles still reported that the name was unavailable.")
                                .arg(unqualifiedTypeName(requestedTypeName), availableModuleUris.join(QStringLiteral(", ")));
        return diagnosis;
    }

    if (!errorTexts.isEmpty()) {
        diagnosis.details = errorTexts.constFirst();
    }
    return diagnosis;
}

QmlTypeNameEntry makeSelectedProfileTypeMetadata(const QList<QmlTypeNameEntry>& typeNameEntries,
                                                 const QString& requestedTypeName,
                                                 const QmlTypeInspectionResult& inspectionResult)
{
    const QString selectedModuleUri = preferredSelectedModuleUri(inspectionResult);

    QmlTypeNameEntry selectedProfileTypeMetadata;
    selectedProfileTypeMetadata.moduleUri = selectedModuleUri;
    selectedProfileTypeMetadata.typeName = unqualifiedTypeName(requestedTypeName);
    selectedProfileTypeMetadata.isCreatable = true;
    selectedProfileTypeMetadata.typeFlagsKnown = false;

    for (const QmlTypeNameEntry& entry : typeNameEntries) {
        if (entry.typeName != selectedProfileTypeMetadata.typeName || entry.moduleUri != selectedModuleUri) {
            continue;
        }

        selectedProfileTypeMetadata.isCreatable = entry.isCreatable;
        selectedProfileTypeMetadata.isSingleton = entry.isSingleton;
        selectedProfileTypeMetadata.hasAttachedType = entry.hasAttachedType;
        selectedProfileTypeMetadata.isValueType = entry.isValueType;
        selectedProfileTypeMetadata.typeFlagsKnown = entry.typeFlagsKnown;
        break;
    }

    return selectedProfileTypeMetadata;
}

const QmlTypeNameEntry* resolveInspectionDisplayMetadata(const QList<QmlTypeNameEntry>& typeNameEntries,
                                                         const QString& requestedTypeName,
                                                         const QmlTypeInspectionResult& inspectionResult,
                                                         QmlTypeNameEntry& selectedProfileTypeMetadata)
{
    const QmlTypeNameEntry* resolvedTypeMetadata = resolveTypeMetadata(typeNameEntries,
                                                                       requestedTypeName,
                                                                       inspectionResult.selectedModuleUris,
                                                                       false);
    if (resolvedTypeMetadata) {
        return resolvedTypeMetadata;
    }

    if (preferredSelectedModuleUri(inspectionResult).isEmpty()) {
        return nullptr;
    }

    selectedProfileTypeMetadata = makeSelectedProfileTypeMetadata(typeNameEntries,
                                                                  requestedTypeName,
                                                                  inspectionResult);
    return &selectedProfileTypeMetadata;
}

void printFailureDebugReport(const QmlTypeInspectionResult& inspectionResult, QTextStream& standardError)
{
    standardError << "Attempted profiles:" << Qt::endl;
    for (const QmlTypeInspectionAttempt& failedAttempt : inspectionResult.failedAttempts) {
        standardError << "    " << failedAttempt.profileDescription << Qt::endl;
        if (!failedAttempt.importStatements.isEmpty()) {
            standardError << "        Imports:" << Qt::endl;
            for (const QString& importStatement : failedAttempt.importStatements) {
                standardError << "            " << importStatement << Qt::endl;
            }
        }
        if (!failedAttempt.errors.isEmpty()) {
            standardError << "        Errors:" << Qt::endl;
            for (const QQmlError& error : failedAttempt.errors) {
                standardError << "            " << error.toString() << Qt::endl;
            }
        }
    }
}

struct AllMatchesDisplayItem
{
    enum class Kind
    {
        NoImportProfile,
        SuccessfulInspection
    };

    Kind kind = Kind::NoImportProfile;
    QmlTypeNameEntry displayEntry;
    QmlTypeInspectionResult inspectionResult;
};

void printMatchSectionHeader(const QmlTypeNameEntry& entry,
                           int matchIndex,
                           int totalMatchCount,
                           QTextStream& outputStream)
{
    outputStream << "== Match " << matchIndex << "/" << totalMatchCount << ": ";
    if (!entry.moduleUri.isEmpty()) {
        outputStream << entry.moduleUri << "::";
    }
    outputStream << entry.typeName;

    const QStringList flags = typeFlags(entry);
    if (!flags.isEmpty()) {
        outputStream << " [" << flags.join(QStringLiteral(", ")) << "]";
    }
    outputStream << " ==" << Qt::endl;
}

int printAllMatchingTypeReports(const CommandLineOptions& commandLineOptions,
                                const QList<QmlTypeNameEntry>& allTypeNameEntries,
                                const QList<QmlAttachedTypeEntry>& attachedTypeEntries,
                                QmlTypeInspector& qmlTypeInspector,
                                MetaObjectPrinter& metaObjectPrinter,
                                QTextStream& standardOutput,
                                QTextStream& standardError)
{
    const QStringList candidateModuleUris = candidateModuleUrisForAllMatches(allTypeNameEntries,
                                                                             commandLineOptions.requestedTypeName);
    if (candidateModuleUris.isEmpty()) {
        standardError << commandLineOptions.requestedTypeName << " was not found in the type catalog." << Qt::endl;
        return 1;
    }

    bool anySuccessfulMatch = false;
    std::vector<AllMatchesDisplayItem> displayItems;
    displayItems.reserve(static_cast<std::size_t>(candidateModuleUris.size()));

    for (const QString& candidateModuleUri : candidateModuleUris) {
        const QList<ImportProfile> importProfiles = QmlModuleCatalog::profilesForTypeInModule(commandLineOptions.requestedTypeName,
                                                                                               candidateModuleUri);

        if (importProfiles.isEmpty()) {
            const QmlTypeNameEntry* catalogEntry = resolveTypeMetadata(allTypeNameEntries,
                                                                       commandLineOptions.requestedTypeName,
                                                                       QStringList{candidateModuleUri},
                                                                       false);
            QmlTypeNameEntry displayEntry;
            if (catalogEntry) {
                displayEntry = *catalogEntry;
            } else {
                displayEntry.moduleUri = candidateModuleUri;
                displayEntry.typeName = unqualifiedTypeName(commandLineOptions.requestedTypeName);
                displayEntry.typeFlagsKnown = false;
            }

            AllMatchesDisplayItem displayItem;
            displayItem.kind = AllMatchesDisplayItem::Kind::NoImportProfile;
            displayItem.displayEntry = displayEntry;
            displayItems.push_back(std::move(displayItem));
            continue;
        }

        QmlTypeInspectionResult inspectionResult = qmlTypeInspector.inspectTypeWithProfiles(commandLineOptions.requestedTypeName,
                                                                                             attachedTypeEntries,
                                                                                             importProfiles,
                                                                                             commandLineOptions.debugLevel);
        if (!inspectionResult.isSuccessful || !inspectionResult.inspectedObject) {
            continue;
        }

        QmlTypeNameEntry selectedProfileTypeMetadata;
        const QmlTypeNameEntry* resolvedTypeMetadata = resolveInspectionDisplayMetadata(allTypeNameEntries,
                                                                                        commandLineOptions.requestedTypeName,
                                                                                        inspectionResult,
                                                                                        selectedProfileTypeMetadata);
        if (!resolvedTypeMetadata || resolvedTypeMetadata->moduleUri != candidateModuleUri) {
            continue;
        }

        anySuccessfulMatch = true;

        AllMatchesDisplayItem displayItem;
        displayItem.kind = AllMatchesDisplayItem::Kind::SuccessfulInspection;
        displayItem.displayEntry = *resolvedTypeMetadata;
        displayItem.inspectionResult = std::move(inspectionResult);
        displayItems.push_back(std::move(displayItem));
    }

    for (std::size_t displayIndex = 0; displayIndex < displayItems.size(); ++displayIndex) {
        const AllMatchesDisplayItem& displayItem = displayItems.at(displayIndex);

        if (displayIndex > 0) {
            standardOutput << Qt::endl;
        }
        printMatchSectionHeader(displayItem.displayEntry,
                                static_cast<int>(displayIndex) + 1,
                                static_cast<int>(displayItems.size()),
                                standardOutput);

        if (displayItem.kind == AllMatchesDisplayItem::Kind::NoImportProfile) {
            standardOutput << "No import profile is available for module: " << displayItem.displayEntry.moduleUri << Qt::endl;
            continue;
        }

        const QmlTypeInspectionResult& inspectionResult = displayItem.inspectionResult;
        if (commandLineOptions.debugLevel > 0) {
            standardOutput << "Selected profile: " << inspectionResult.selectedProfileDescription << Qt::endl;
            standardOutput << inspectionResult.qmlSource << Qt::endl << Qt::endl;
        }

        metaObjectPrinter.printTypeReport(commandLineOptions.requestedTypeName,
                                          inspectionResult.inspectedObject,
                                          inspectionResult.attachedProbeObject,
                                          &displayItem.displayEntry);
    }

    return anySuccessfulMatch ? 0 : 1;
}
} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication::setApplicationName(QStringLiteral("qmlprop"));
    QCoreApplication::setApplicationVersion(QStringLiteral("2.3.0"));
    QGuiApplication application(argc, argv);

    const CommandLineOptions commandLineOptions = parseCommandLineArguments(application);

    QQmlEngine qmlEngine;
    QTextStream standardOutput(stdout);
    QTextStream standardError(stderr);
    QString bufferedStandardOutputText;
    QTextStream bufferedStandardOutput(&bufferedStandardOutputText);

    const QmlTypeNameCatalog qmlTypeNameCatalog(qmlEngine);
    const QList<QmlTypeNameEntry> typeNameEntries = qmlTypeNameCatalog.collectTypeNames();
    const QList<QmlTypeNameEntry> allTypeNameEntries = qmlTypeNameCatalog.collectTypeNames(true);

    if (commandLineOptions.listTypes) {
        printTypeNameList(typeNameEntries, bufferedStandardOutput);
        bufferedStandardOutput.flush();
        writeBufferedOutput(bufferedStandardOutputText, commandLineOptions, standardOutput, standardError);
        return 0;
    }

    if (commandLineOptions.listModules) {
        printModuleUriList(qmlTypeNameCatalog.collectModuleUris(), bufferedStandardOutput);
        bufferedStandardOutput.flush();
        writeBufferedOutput(bufferedStandardOutputText, commandLineOptions, standardOutput, standardError);
        return 0;
    }

    QList<QmlAttachedTypeEntry> attachedTypeEntries;
    if (commandLineOptions.showAttached) {
        const QmlAttachedTypeCatalog qmlAttachedTypeCatalog(qmlEngine);
        attachedTypeEntries = qmlAttachedTypeCatalog.collectAttachedTypes();
    }

    QmlTypeInspector qmlTypeInspector(qmlEngine);
    MetaObjectPrinter metaObjectPrinter(commandLineOptions.showAll,
                                        commandLineOptions.showAttached,
                                        commandLineOptions.showDeclaredIn,
                                        bufferedStandardOutput,
                                        qmlEngine,
                                        attachedTypeEntries);

    if (commandLineOptions.moduleIndex) {
        const int exitCode = printModuleIndex(commandLineOptions.requestedModuleUri,
                                              typeNameEntries,
                                              qmlTypeInspector,
                                              qmlEngine,
                                              bufferedStandardOutput);
        bufferedStandardOutput.flush();
        writeBufferedOutput(bufferedStandardOutputText, commandLineOptions, standardOutput, standardError);
        return exitCode;
    }

    if (commandLineOptions.showAllMatches) {
        const int exitCode = printAllMatchingTypeReports(commandLineOptions,
                                                         allTypeNameEntries,
                                                         attachedTypeEntries,
                                                         qmlTypeInspector,
                                                         metaObjectPrinter,
                                                         bufferedStandardOutput,
                                                         standardError);
        bufferedStandardOutput.flush();
        writeBufferedOutput(bufferedStandardOutputText, commandLineOptions, standardOutput, standardError);
        return exitCode;
    }

    const QmlTypeInspectionResult inspectionResult = qmlTypeInspector.inspectType(commandLineOptions.requestedTypeName,
                                                                                  attachedTypeEntries,
                                                                                  commandLineOptions.debugLevel);

    if (!inspectionResult.isSuccessful || !inspectionResult.inspectedObject) {
        const FailureDiagnosis failureDiagnosis = diagnoseInstantiationFailure(allTypeNameEntries,
                                                                              inspectionResult,
                                                                              commandLineOptions.requestedTypeName);
        standardError << commandLineOptions.requestedTypeName << " could not be instantiated." << Qt::endl;
        standardError << "Reason: " << failureDiagnosis.reason << Qt::endl;
        if (!failureDiagnosis.details.isEmpty()) {
            standardError << "Details: " << failureDiagnosis.details << Qt::endl;
        }
        if (commandLineOptions.debugLevel > 0) {
            printFailureDebugReport(inspectionResult, standardError);
        }
        return 1;
    }

    if (commandLineOptions.debugLevel > 0) {
        bufferedStandardOutput << "Selected profile: " << inspectionResult.selectedProfileDescription << Qt::endl;
        bufferedStandardOutput << inspectionResult.qmlSource << Qt::endl << Qt::endl;
    }

    const QmlTypeNameEntry* resolvedTypeMetadata = resolveTypeMetadata(typeNameEntries,
                                                                       commandLineOptions.requestedTypeName,
                                                                       inspectionResult.selectedModuleUris,
                                                                       false);
    QmlTypeNameEntry selectedProfileTypeMetadata;
    if (!resolvedTypeMetadata && !preferredSelectedModuleUri(inspectionResult).isEmpty()) {
        selectedProfileTypeMetadata = makeSelectedProfileTypeMetadata(typeNameEntries,
                                                                      commandLineOptions.requestedTypeName,
                                                                      inspectionResult);
        resolvedTypeMetadata = &selectedProfileTypeMetadata;
    }

    metaObjectPrinter.printTypeReport(commandLineOptions.requestedTypeName,
                                      inspectionResult.inspectedObject,
                                      inspectionResult.attachedProbeObject,
                                      resolvedTypeMetadata);
    bufferedStandardOutput.flush();
    writeBufferedOutput(bufferedStandardOutputText, commandLineOptions, standardOutput, standardError);
    return 0;
}
