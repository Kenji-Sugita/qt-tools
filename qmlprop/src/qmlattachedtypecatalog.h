#ifndef QMLATTACHEDTYPECATALOG_H
#define QMLATTACHEDTYPECATALOG_H

#include <QList>
#include <QString>

class QQmlEngine;

struct QmlAttachedTypeProperty
{
    QString name;
    QString typeName;
    bool isReadonly = false;
    bool isPointer = false;
    bool isList = false;
};

struct QmlAttachedTypeParameter
{
    QString name;
    QString typeName;
    bool isPointer = false;
    bool isList = false;
};

struct QmlAttachedTypeMethod
{
    QString name;
    QString returnTypeName;
    QList<QmlAttachedTypeParameter> parameters;
    bool isSignal = false;
};

struct QmlAttachedTypeEnum
{
    QString name;
    QStringList keys;
};

struct QmlAttachedTypeEntry
{
    QString moduleUri;
    QString attachingTypeName;
    QString attachedTypeId;
    QList<QmlAttachedTypeProperty> properties;
    QList<QmlAttachedTypeMethod> methods;
    QList<QmlAttachedTypeEnum> enumerators;
};

class QmlAttachedTypeCatalog
{
public:
    explicit QmlAttachedTypeCatalog(const QQmlEngine& qmlEngine);

    QList<QmlAttachedTypeEntry> collectAttachedTypes() const;

private:
    struct ComponentMetadata;

    static QString normalizeImportPath(const QString& importPath);
    static QString readBalancedBlock(const QString& content, qsizetype openingBraceIndex);

    void collectFromImportPath(const QString& importPath,
                               QList<ComponentMetadata>& componentMetadataList) const;
    void parseQmlTypesFile(const QString& qmlTypesFilePath,
                           QList<ComponentMetadata>& componentMetadataList) const;

    const QQmlEngine& engine;
};

#endif // QMLATTACHEDTYPECATALOG_H
