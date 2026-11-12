#ifndef METAOBJECTPRINTER_H
#define METAOBJECTPRINTER_H

#include "qmlattachedtypecatalog.h"
#include "qmltypenamecatalog.h"

#include <QList>
#include <QString>

class QObject;
class QMetaObject;
class QMetaProperty;
class QQmlEngine;
class QTextStream;
class QVariant;
class QJSValue;

class MetaObjectPrinter
{
public:
    MetaObjectPrinter(bool showAllMembers,
                      bool showAttachedMembers,
                      bool showDeclaringClassNames,
                      QTextStream& outputStream,
                      QQmlEngine& qmlEngine,
                      QList<QmlAttachedTypeEntry> attachedTypeEntries);

    void printTypeReport(const QString& requestedTypeName,
                         const QObject* inspectedObject,
                         const QObject* attachedProbeObject,
                         const QmlTypeNameEntry* typeMetadata) const;

private:
    const QMetaObject* effectiveMetaObject(const QObject* inspectedObject) const;
    const QMetaObject* declaringMetaObjectForProperty(const QMetaObject* metaObject, int propertyIndex) const;
    const QMetaObject* declaringMetaObjectForMethod(const QMetaObject* metaObject, int methodIndex) const;
    const QMetaObject* declaringMetaObjectForEnumerator(const QMetaObject* metaObject, int enumeratorIndex) const;

    void printTypeMetadata(const QmlTypeNameEntry* typeMetadata) const;
    void printTypeHierarchy(const QObject* inspectedObject, const QMetaObject* metaObject) const;
    void printDefaultMembers(const QObject* inspectedObject, const QMetaObject* metaObject) const;
    void printProperties(int indentationLevel, const QObject* inspectedObject, const QMetaObject* metaObject) const;
    void printMethods(int indentationLevel, const QMetaObject* metaObject) const;
    void printEnumerators(int indentationLevel, const QMetaObject* metaObject) const;
    void printObjectPropertyDetails(int indentationLevel, const QObject* propertyObject) const;
    void printQjsValueDetails(int indentationLevel, const QJSValue& scriptValue) const;
    void printPropertyAttributes(const QMetaProperty& metaProperty,
                                 bool isInherited,
                                 bool isOverride,
                                 bool isDefault,
                                 bool isDeferred,
                                 const QString& propertyTypeName) const;
    void printDeclarationSuffix(const QMetaObject* declaringMetaObject) const;
    void printFontDetails(int indentationLevel, const QObject* inspectedObject) const;
    void printAttachedTypes(const QObject* inspectedObject, const QObject* attachedProbeObject) const;
    void printAttachedType(const QObject* inspectedObject, const QObject* attachedProbeObject, const QmlAttachedTypeEntry& attachedTypeEntry) const;
    bool hasVisibleAttachedProperties(const QObject* attachedProbeObject, const QmlAttachedTypeEntry& attachedTypeEntry) const;
    QString indentation(int indentationLevel) const;
    QString formatPropertyValue(const QObject* inspectedObject, const QMetaProperty& metaProperty) const;
    QString formatAttachedPropertyValue(const QmlAttachedTypeProperty& property, const QVariant& value) const;

    bool showAll;
    bool showAttached;
    bool showDeclaredIn;
    QTextStream& output;
    QQmlEngine& engine;
    QList<QmlAttachedTypeEntry> attachedTypes;
};

#endif // METAOBJECTPRINTER_H
