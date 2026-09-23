#include "qmltypeinspector.h"

#include "qmlmodulecatalog.h"

#include <QDebug>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QStringList>
#include <QVariant>

namespace
{

QString sanitizeProbeIdentifier(const QString& text)
{
    QString identifier;
    identifier.reserve(text.size());
    for (const QChar character : text) {
        if (character.isLetterOrNumber()) {
            identifier.append(character);
        } else {
            identifier.append(QLatin1Char('_'));
        }
    }
    return identifier;
}

QString probePropertyName(const QmlAttachedTypeEntry& attachedTypeEntry, const QmlAttachedTypeProperty& property)
{
    return QStringLiteral("__qmlprop_attached_%1_%2_%3")
        .arg(sanitizeProbeIdentifier(attachedTypeEntry.moduleUri),
             sanitizeProbeIdentifier(attachedTypeEntry.attachingTypeName),
             sanitizeProbeIdentifier(property.name));
}

QStringList importedModuleUris(const ImportProfile& importProfile)
{
    QStringList moduleUris;
    for (const QString& importStatement : importProfile.importStatements) {
        if (!importStatement.startsWith(QStringLiteral("import "))) {
            continue;
        }

        QString remainder = importStatement.mid(QStringLiteral("import ").size()).trimmed();
        const qsizetype asIndex = remainder.indexOf(QStringLiteral(" as "));
        if (asIndex >= 0) {
            remainder = remainder.left(asIndex).trimmed();
        }

        const QString moduleUri = remainder.section(QLatin1Char(' '), 0, 0).trimmed();
        if (!moduleUri.isEmpty() && !moduleUris.contains(moduleUri)) {
            moduleUris.append(moduleUri);
        }
    }
    return moduleUris;
}

} // namespace

QmlTypeInspector::QmlTypeInspector(QQmlEngine& qmlEngine)
    : engine(qmlEngine)
{
}

QmlTypeInspectionResult QmlTypeInspector::inspectType(const QString& requestedTypeName,
                                                      const QList<QmlAttachedTypeEntry>& attachedTypeEntries,
                                                      int debugLevel) const
{
    return inspectTypeWithProfiles(requestedTypeName,
                                   attachedTypeEntries,
                                   QmlModuleCatalog::profilesForType(requestedTypeName),
                                   debugLevel);
}

QmlTypeInspectionResult QmlTypeInspector::inspectTypeWithProfiles(const QString& requestedTypeName,
                                                                  const QList<QmlAttachedTypeEntry>& attachedTypeEntries,
                                                                  const QList<ImportProfile>& importProfiles,
                                                                  int debugLevel) const
{
    QmlTypeInspectionResult inspectionResult;
    inspectionResult.requestedTypeName = requestedTypeName;

    for (const ImportProfile& importProfile : importProfiles) {
        QmlTypeInspectionAttempt inspectionAttempt;
        inspectionAttempt.profileDescription = importProfile.description;
        inspectionAttempt.importStatements = importProfile.importStatements;
        inspectionAttempt.moduleUris = importedModuleUris(importProfile);
        inspectionAttempt.qmlSource = buildQmlSource(requestedTypeName, importProfile, attachedTypeEntries);

        QQmlComponent qmlComponent(&engine);
        qmlComponent.setData(inspectionAttempt.qmlSource.toUtf8(), QUrl(QStringLiteral("inmemory:qmlprop.qml")));

        if (!qmlComponent.isReady()) {
            inspectionAttempt.errors = qmlComponent.errors();
            inspectionResult.errors = inspectionAttempt.errors;
            inspectionResult.failedAttempts.append(inspectionAttempt);
            if (debugLevel > 1) {
                qDebug().noquote() << "Profile failed:" << importProfile.description;
                for (const QQmlError& error : inspectionAttempt.errors) {
                    qDebug().noquote() << "  " << error.toString();
                }
            }
            continue;
        }

        std::unique_ptr<QObject> createdRootObject(qmlComponent.create());
        if (!createdRootObject) {
            inspectionAttempt.errors = qmlComponent.errors();
            inspectionResult.errors = inspectionAttempt.errors;
            inspectionResult.failedAttempts.append(inspectionAttempt);
            if (debugLevel > 1) {
                qDebug().noquote() << "Profile created no object:" << importProfile.description;
                for (const QQmlError& error : inspectionAttempt.errors) {
                    qDebug().noquote() << "  " << error.toString();
                }
            }
            continue;
        }

        QObject* inspectedObject = extractInspectedObject(createdRootObject.get(), requestedTypeName);
        if (!inspectedObject) {
            inspectionAttempt.errors = qmlComponent.errors();
            inspectionResult.errors = inspectionAttempt.errors;
            inspectionResult.failedAttempts.append(inspectionAttempt);
            if (debugLevel > 1) {
                qDebug().noquote() << "Inspected object not found for profile:" << importProfile.description;
                for (const QQmlError& error : inspectionAttempt.errors) {
                    qDebug().noquote() << "  " << error.toString();
                }
            }
            continue;
        }

        inspectionResult.isSuccessful = true;
        inspectionResult.qmlSource = inspectionAttempt.qmlSource;
        inspectionResult.selectedProfileDescription = importProfile.description;
        inspectionResult.selectedModuleUris = inspectionAttempt.moduleUris;
        inspectionResult.createdRootObject = std::move(createdRootObject);
        inspectionResult.inspectedObject = inspectedObject;
        inspectionResult.attachedProbeObject = extractAttachedProbeObject(inspectionResult.createdRootObject.get());
        inspectionResult.errors.clear();
        return inspectionResult;
    }

    inspectionResult.qmlSource.clear();
    inspectionResult.selectedProfileDescription.clear();
    inspectionResult.selectedModuleUris.clear();
    return inspectionResult;
}

QString QmlTypeInspector::buildQmlSource(const QString& requestedTypeName,
                                         const ImportProfile& importProfile,
                                         const QList<QmlAttachedTypeEntry>& attachedTypeEntries) const
{
    const QString unqualifiedTypeName = requestedTypeName.section(QLatin1Char('.'), -1);
    QStringList qmlLines = importProfile.importStatements;
    qmlLines << QString();
    qmlLines << QStringLiteral("Item {");

    if (unqualifiedTypeName == QStringLiteral("Component")) {
        qmlLines << QStringLiteral("    Component {");
        qmlLines << QStringLiteral("        id: _");

        const QString initializationBody = initializationBodyForType(requestedTypeName);
        if (!initializationBody.isEmpty()) {
            const QStringList initializationLines = initializationBody.split(QLatin1Char('\n'));
            for (const QString& initializationLine : initializationLines) {
                qmlLines << QStringLiteral("        ") + initializationLine;
            }
        }

        qmlLines << QStringLiteral("    }");
        qmlLines << QStringLiteral("}");
        return qmlLines.join(QLatin1Char('\n'));
    }

    qmlLines << QStringLiteral("    %1 {").arg(requestedTypeName);
    qmlLines << QStringLiteral("        id: inspectedObject");
    qmlLines << QStringLiteral("        objectName: \"inspectedObject\"");

    const QString initializationBody = initializationBodyForType(requestedTypeName);
    if (!initializationBody.isEmpty()) {
        const QStringList initializationLines = initializationBody.split(QLatin1Char('\n'));
        for (const QString& initializationLine : initializationLines) {
            qmlLines << QStringLiteral("        ") + initializationLine;
        }
    }

    qmlLines << QStringLiteral("    }");
    qmlLines << QStringLiteral("    QtObject {");
    qmlLines << QStringLiteral("        objectName: \"attachedProbe\"");

    const QStringList moduleUris = importedModuleUris(importProfile);
    for (const QmlAttachedTypeEntry& attachedTypeEntry : attachedTypeEntries) {
        if (!moduleUris.contains(attachedTypeEntry.moduleUri)) {
            continue;
        }

        for (const QmlAttachedTypeProperty& property : attachedTypeEntry.properties) {
            qmlLines << QStringLiteral("        property var %1: inspectedObject.%2.%3")
                            .arg(probePropertyName(attachedTypeEntry, property),
                                 attachedTypeEntry.attachingTypeName,
                                 property.name);
        }
    }

    qmlLines << QStringLiteral("    }");
    qmlLines << QStringLiteral("}");
    return qmlLines.join(QLatin1Char('\n'));
}

QString QmlTypeInspector::initializationBodyForType(const QString& requestedTypeName) const
{
    const QString unqualifiedTypeName = requestedTypeName.section(u'.', -1);
    if (unqualifiedTypeName == QStringLiteral("Component")) {
        return QStringLiteral("Item {}");
    }

    return QString();
}

QObject* QmlTypeInspector::extractInspectedObject(QObject* rootObject, const QString& requestedTypeName) const
{
    if (!rootObject) {
        return nullptr;
    }

    QObject* inspectedObject = rootObject->findChild<QObject*>(QStringLiteral("inspectedObject"), Qt::FindChildrenRecursively);
    if (inspectedObject) {
        return inspectedObject;
    }

    const QString unqualifiedTypeName = requestedTypeName.section(QLatin1Char('.'), -1);
    if (unqualifiedTypeName == QStringLiteral("Component")) {
        const QObjectList childObjects = rootObject->children();
        for (QObject* childObject : childObjects) {
            if (!childObject) {
                continue;
            }
            return childObject;
        }
    }

    return nullptr;
}

QObject* QmlTypeInspector::extractAttachedProbeObject(QObject* rootObject) const
{
    if (!rootObject) {
        return nullptr;
    }

    return rootObject->findChild<QObject*>(QStringLiteral("attachedProbe"), Qt::FindChildrenRecursively);
}
