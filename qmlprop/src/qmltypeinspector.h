#ifndef QMLTYPEINSPECTOR_H
#define QMLTYPEINSPECTOR_H

#include "importprofile.h"
#include "qmlattachedtypecatalog.h"

#include <QQmlError>
#include <QString>
#include <QStringList>

#include <memory>

class QObject;
class QQmlEngine;

struct QmlTypeInspectionAttempt
{
    QString profileDescription;
    QStringList moduleUris;
    QStringList importStatements;
    QString qmlSource;
    QList<QQmlError> errors;
};

struct QmlTypeInspectionResult
{
    bool isSuccessful = false;
    QString requestedTypeName;
    QString qmlSource;
    QString selectedProfileDescription;
    QStringList selectedModuleUris;
    QList<QQmlError> errors;
    QList<QmlTypeInspectionAttempt> failedAttempts;
    std::unique_ptr<QObject> createdRootObject;
    QObject* inspectedObject = nullptr;
    QObject* attachedProbeObject = nullptr;
};

class QmlTypeInspector
{
public:
    explicit QmlTypeInspector(QQmlEngine& qmlEngine);

    QmlTypeInspectionResult inspectType(const QString& requestedTypeName,
                                        const QList<QmlAttachedTypeEntry>& attachedTypeEntries,
                                        int debugLevel) const;
    QmlTypeInspectionResult inspectTypeWithProfiles(const QString& requestedTypeName,
                                                    const QList<QmlAttachedTypeEntry>& attachedTypeEntries,
                                                    const QList<ImportProfile>& importProfiles,
                                                    int debugLevel) const;

private:
    QString buildQmlSource(const QString& requestedTypeName,
                           const ImportProfile& importProfile,
                           const QList<QmlAttachedTypeEntry>& attachedTypeEntries) const;
    QString initializationBodyForType(const QString& requestedTypeName) const;
    QObject* extractInspectedObject(QObject* rootObject, const QString& requestedTypeName) const;
    QObject* extractAttachedProbeObject(QObject* rootObject) const;

    QQmlEngine& engine;
};

#endif // QMLTYPEINSPECTOR_H
