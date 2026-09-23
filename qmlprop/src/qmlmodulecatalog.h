#ifndef QMLMODULECATALOG_H
#define QMLMODULECATALOG_H

#include "importprofile.h"

#include <QList>
#include <QString>

class QmlModuleCatalog
{
public:
    static QList<ImportProfile> profilesForType(const QString& requestedTypeName);
    static QList<ImportProfile> profilesForTypeInModule(const QString& requestedTypeName,
                                                        const QString& moduleUri);

private:
    static QString qualifierFromTypeName(const QString& requestedTypeName);
    static QStringList baseImports();
};

#endif // QMLMODULECATALOG_H
