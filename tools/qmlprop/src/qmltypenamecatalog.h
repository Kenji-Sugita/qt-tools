#ifndef QMLTYPENAMECATALOG_H
#define QMLTYPENAMECATALOG_H

#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>

class QQmlEngine;

struct QmlTypeNameEntry
{
    QString moduleUri;
    QString typeName;
    bool isCreatable = true;
    bool isSingleton = false;
    bool isReferenceType = true;
    bool hasAttachedType = false;
    bool isValueType = false;
    bool typeFlagsKnown = true;
};

class QmlTypeNameCatalog
{
public:
    explicit QmlTypeNameCatalog(const QQmlEngine& qmlEngine);

    QList<QmlTypeNameEntry> collectTypeNames(bool includeValueTypes = false) const;
    QStringList collectModuleUris(bool includeValueTypes = true) const;

private:
    static QString normalizeImportPath(const QString& importPath);
    static QString stripComment(const QString& line);
    static bool looksLikeVersion(const QString& token);
    static QString inferredModuleUri(const QString& normalizedImportPath, const QString& moduleDirectoryPath);
    static QString typeKey(const QString& moduleUri, const QString& typeName);

    void collectFromImportPath(const QString& importPath, QHash<QString, QmlTypeNameEntry>& collectedEntries) const;
    void parseQmldirFile(const QString& normalizedImportPath,
                        const QString& qmldirFilePath,
                        QHash<QString, QmlTypeNameEntry>& collectedEntries) const;
    void parseQmlTypesFile(const QString& qmlTypesFilePath,
                           QHash<QString, QmlTypeNameEntry>& collectedEntries) const;
    void addEntry(const QString& moduleUri,
                  const QString& typeName,
                  bool isCreatable,
                  bool isSingleton,
                  bool isReferenceType,
                  bool hasAttachedType,
                  bool isValueType,
                  QHash<QString, QmlTypeNameEntry>& collectedEntries) const;

    const QQmlEngine& engine;
};

#endif // QMLTYPENAMECATALOG_H
