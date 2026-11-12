#include "metaobjectprinter.h"

#include <QColor>
#include <QFont>
#include <QHash>
#include <QJSValue>
#include <QJSValueIterator>
#include <QMetaClassInfo>
#include <QMetaEnum>
#include <QMetaMethod>
#include <QMetaObject>
#include <QMetaProperty>
#include <QQmlEngine>
#include <QQmlProperty>
#include <QQuickItem>
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QStringList>
#include <QTextStream>
#include <QUrl>
#include <QVariant>
#include <QtGlobal>
#include <utility>

namespace {

QString safeTypeName(const QQmlProperty& qmlProperty, const QMetaProperty& metaProperty)
{
    if (const char* qmlPropertyTypeName = qmlProperty.propertyTypeName()) {
        return QString::fromLatin1(qmlPropertyTypeName);
    }

    if (const char* metaPropertyTypeName = metaProperty.typeName()) {
        return QString::fromLatin1(metaPropertyTypeName);
    }

    if (const char* metaTypeName = metaProperty.metaType().name()) {
        return QString::fromLatin1(metaTypeName);
    }

    return QStringLiteral("<unknown>");
}

QString formatBoolean(bool value)
{
    return value ? QStringLiteral("true") : QStringLiteral("false");
}

QString formatInteger(qint64 value)
{
    return QStringLiteral("%1 (#%2)").arg(QString::number(value), QString::number(static_cast<qulonglong>(value), 16));
}

QString formatUnsignedInteger(qulonglong value)
{
    return QStringLiteral("%1 (#%2)").arg(QString::number(value), QString::number(value, 16));
}

QString enumValueToString(const QMetaProperty& metaProperty, const QVariant& value)
{
    if (!metaProperty.isEnumType()) {
        return QString();
    }

    const QMetaEnum metaEnum = metaProperty.enumerator();
    if (!metaEnum.isValid()) {
        return QString();
    }

    if (metaEnum.isFlag()) {
        const QByteArray keys = metaEnum.valueToKeys(value.toInt());
        return keys.isEmpty() ? QString() : QString::fromLatin1(keys);
    }

    const char* key = metaEnum.valueToKey(value.toInt());
    if (!key) {
        return QString();
    }

    return QString::fromLatin1(key);
}

QStringList deferredPropertyNames(const QMetaObject* metaObject)
{
    QStringList names;
    for (int classInfoIndex = 0; classInfoIndex < metaObject->classInfoCount(); ++classInfoIndex) {
        const QMetaClassInfo classInfo = metaObject->classInfo(classInfoIndex);
        if (QString::fromLatin1(classInfo.name()) == QStringLiteral("DeferredPropertyNames")) {
            names = QString::fromUtf8(classInfo.value()).split(u',', Qt::SkipEmptyParts);
            names.replaceInStrings(QStringLiteral(" "), QString());
            break;
        }
    }
    return names;
}

QString defaultMethodSignature(const QMetaObject* metaObject)
{
    for (int classInfoIndex = 0; classInfoIndex < metaObject->classInfoCount(); ++classInfoIndex) {
        const QMetaClassInfo classInfo = metaObject->classInfo(classInfoIndex);
        if (QString::fromLatin1(classInfo.name()) == QStringLiteral("DefaultMethod")) {
            return QString::fromLatin1(classInfo.value());
        }
    }
    return QString();
}

QString defaultPropertyName(const QObject* inspectedObject, QQmlEngine& qmlEngine)
{
    const QQmlProperty defaultQmlProperty(const_cast<QObject*>(inspectedObject), &qmlEngine);
    if (!defaultQmlProperty.isValid()) {
        return QString();
    }
    return defaultQmlProperty.name();
}

QString methodSignature(const QMetaMethod& metaMethod)
{
    QString signature = QString::fromLatin1(metaMethod.name()) + QStringLiteral("(");
    const QList<QByteArray> parameterTypes = metaMethod.parameterTypes();
    const QList<QByteArray> parameterNames = metaMethod.parameterNames();
    for (int parameterIndex = 0; parameterIndex < parameterTypes.size(); ++parameterIndex) {
        signature += QString::fromLatin1(parameterTypes.at(parameterIndex));
        if (!parameterNames.at(parameterIndex).isEmpty()) {
            signature += QString::fromLatin1(parameterNames.at(parameterIndex));
        }
        if (parameterIndex + 1 < parameterTypes.size()) {
            signature += QStringLiteral(",");
        }
    }
    signature += QStringLiteral(")");
    return signature;
}

bool isObjectPointerTypeName(const QString& propertyTypeName)
{
    return propertyTypeName.endsWith(QLatin1Char('*'));
}

bool isListPropertyTypeName(const QString& propertyTypeName)
{
    return propertyTypeName.startsWith(QStringLiteral("QQmlListProperty"));
}

QObject* objectPointerValue(const QVariant& value)
{
    if (!value.isValid() || !value.canConvert<QObject*>()) {
        return nullptr;
    }
    return value.value<QObject*>();
}

bool shouldExpandObjectProperty(const QString& propertyTypeName, const QObject* objectValue)
{
    Q_UNUSED(objectValue);
    return propertyTypeName == QStringLiteral("QQuickPen*");
}

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

QString attachedProbePropertyName(const QmlAttachedTypeEntry& attachedTypeEntry,
                                  const QmlAttachedTypeProperty& property)
{
    return QStringLiteral("__qmlprop_attached_%1_%2_%3")
        .arg(sanitizeProbeIdentifier(attachedTypeEntry.moduleUri),
             sanitizeProbeIdentifier(attachedTypeEntry.attachingTypeName),
             sanitizeProbeIdentifier(property.name));
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
    return flags;
}

} // namespace

MetaObjectPrinter::MetaObjectPrinter(bool showAllMembers,
                                     bool showAttachedMembers,
                                     bool showDeclaringClassNames,
                                     QTextStream& outputStream,
                                     QQmlEngine& qmlEngine,
                                     QList<QmlAttachedTypeEntry> attachedTypeEntries)
    : showAll(showAllMembers)
    , showAttached(showAttachedMembers)
    , showDeclaredIn(showDeclaringClassNames)
    , output(outputStream)
    , engine(qmlEngine)
    , attachedTypes(std::move(attachedTypeEntries))
{
}

const QMetaObject* MetaObjectPrinter::effectiveMetaObject(const QObject* inspectedObject) const
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

const QMetaObject* MetaObjectPrinter::declaringMetaObjectForProperty(const QMetaObject* metaObject, int propertyIndex) const
{
    for (const QMetaObject* currentMetaObject = metaObject;
         currentMetaObject;
         currentMetaObject = currentMetaObject->superClass()) {
        if (propertyIndex >= currentMetaObject->propertyOffset() && propertyIndex < currentMetaObject->propertyCount()) {
            return currentMetaObject;
        }
    }
    return metaObject;
}

const QMetaObject* MetaObjectPrinter::declaringMetaObjectForMethod(const QMetaObject* metaObject, int methodIndex) const
{
    for (const QMetaObject* currentMetaObject = metaObject;
         currentMetaObject;
         currentMetaObject = currentMetaObject->superClass()) {
        if (methodIndex >= currentMetaObject->methodOffset() && methodIndex < currentMetaObject->methodCount()) {
            return currentMetaObject;
        }
    }
    return metaObject;
}

const QMetaObject* MetaObjectPrinter::declaringMetaObjectForEnumerator(const QMetaObject* metaObject, int enumeratorIndex) const
{
    for (const QMetaObject* currentMetaObject = metaObject;
         currentMetaObject;
         currentMetaObject = currentMetaObject->superClass()) {
        if (enumeratorIndex >= currentMetaObject->enumeratorOffset() && enumeratorIndex < currentMetaObject->enumeratorCount()) {
            return currentMetaObject;
        }
    }
    return metaObject;
}

void MetaObjectPrinter::printTypeReport(const QString& requestedTypeName,
                                        const QObject* inspectedObject,
                                        const QObject* attachedProbeObject,
                                        const QmlTypeNameEntry* typeMetadata) const
{
    const QMetaObject* metaObject = effectiveMetaObject(inspectedObject);

    output << "Qt " << qVersion() << Qt::endl;
    output << "Type: " << requestedTypeName << Qt::endl;
    printTypeMetadata(typeMetadata);
    printTypeHierarchy(inspectedObject, metaObject);
    printDefaultMembers(inspectedObject, metaObject);
    printProperties(0, inspectedObject, metaObject);
    if (showAttached) {
        printAttachedTypes(inspectedObject, attachedProbeObject);
    }
    printMethods(0, metaObject);
    printEnumerators(0, metaObject);
}

void MetaObjectPrinter::printTypeMetadata(const QmlTypeNameEntry* typeMetadata) const
{
    if (!typeMetadata) {
        return;
    }

    if (!typeMetadata->moduleUri.isEmpty()) {
        output << "Module: " << typeMetadata->moduleUri << Qt::endl;
    }

    const QStringList flags = typeFlags(*typeMetadata);
    if (!flags.isEmpty()) {
        output << "Type flags: " << flags.join(QStringLiteral(", ")) << Qt::endl;
    }
}

void MetaObjectPrinter::printTypeHierarchy(const QObject* inspectedObject, const QMetaObject* metaObject) const
{
    if (!metaObject) {
        output << "Class: <unknown>" << Qt::endl;
        return;
    }

    output << "Class: " << metaObject->className();
    for (const QMetaObject* superClass = metaObject->superClass(); superClass; superClass = superClass->superClass()) {
        output << " -> " << superClass->className();
        if (QString::fromLatin1(superClass->className()) == QStringLiteral("QQuickItem")) {
            if (const QQuickItem* quickItem = qobject_cast<const QQuickItem*>(inspectedObject)) {
                if (quickItem->isFocusScope()) {
                    output << " (FocusScope)";
                }
            }
        }
    }
    output << Qt::endl;
}

void MetaObjectPrinter::printDefaultMembers(const QObject* inspectedObject, const QMetaObject* metaObject) const
{
    if (!inspectedObject || !metaObject) {
        return;
    }

    const QString currentDefaultPropertyName = defaultPropertyName(inspectedObject, engine);
    if (!currentDefaultPropertyName.isEmpty()) {
        const int propertyIndex = metaObject->indexOfProperty(currentDefaultPropertyName.toUtf8().constData());
        output << "Default property: " << currentDefaultPropertyName;
        if (propertyIndex >= 0) {
            printDeclarationSuffix(declaringMetaObjectForProperty(metaObject, propertyIndex));
        }
        output << Qt::endl;
    }

    const QString currentDefaultMethodSignature = defaultMethodSignature(metaObject);
    if (!currentDefaultMethodSignature.isEmpty()) {
        output << "Default method: " << currentDefaultMethodSignature;
        const int methodIndex = metaObject->indexOfMethod(currentDefaultMethodSignature.toUtf8().constData());
        if (methodIndex >= 0) {
            printDeclarationSuffix(declaringMetaObjectForMethod(metaObject, methodIndex));
        }
        output << Qt::endl;
    }
}

void MetaObjectPrinter::printProperties(int indentationLevel, const QObject* inspectedObject, const QMetaObject* metaObject) const
{
    if (!metaObject) {
        return;
    }

    const int propertyCount = metaObject->propertyCount();
    const int propertyOffset = metaObject->propertyOffset();
    if (propertyCount == 0 || (!showAll && propertyCount <= propertyOffset)) {
        return;
    }

    if (indentationLevel == 0) {
        output << Qt::endl << "Properties:" << Qt::endl;
    }

    const QStringList deferredNames = deferredPropertyNames(metaObject);
    const QString currentDefaultPropertyName = defaultPropertyName(inspectedObject, engine);
    QHash<QString, int> propertyOccurrenceCounts;
    QHash<QString, int> lastPropertyIndexByName;
    for (int propertyIndex = 0; propertyIndex < propertyCount; ++propertyIndex) {
        const QString propertyName = QString::fromLatin1(metaObject->property(propertyIndex).name());
        propertyOccurrenceCounts[propertyName] += 1;
        lastPropertyIndexByName[propertyName] = propertyIndex;
    }

    for (int propertyIndex = 0; propertyIndex < propertyCount; ++propertyIndex) {
        if (!showAll && propertyIndex < propertyOffset) {
            continue;
        }

        const QMetaProperty metaProperty = metaObject->property(propertyIndex);
        const QQmlProperty qmlProperty(const_cast<QObject*>(inspectedObject), metaProperty.name(), &engine);
        const QString currentPropertyName = qmlProperty.name();
        const QString currentPropertyTypeName = safeTypeName(qmlProperty, metaProperty);
        const QVariant propertyValue = qmlProperty.read();
        const QObject* objectValue = objectPointerValue(propertyValue);
        const bool isInherited = propertyIndex < propertyOffset;
        const bool isOverride = propertyOccurrenceCounts.value(currentPropertyName) > 1
            && lastPropertyIndexByName.value(currentPropertyName) == propertyIndex;
        const bool isDefault = currentPropertyName == currentDefaultPropertyName;
        const bool isDeferred = deferredNames.contains(currentPropertyName);
        const QMetaObject* declaringMetaObject = declaringMetaObjectForProperty(metaObject, propertyIndex);
        const QString formattedValue = formatPropertyValue(inspectedObject, metaProperty);
        const bool isListProperty = isListPropertyTypeName(currentPropertyTypeName);

        output << indentation(indentationLevel + 1) << currentPropertyName << ": " << currentPropertyTypeName;

        if (!formattedValue.isEmpty() && !(isListProperty && formattedValue == QStringLiteral("list"))) {
            output << " = " << formattedValue;
        } else if (isObjectPointerTypeName(currentPropertyTypeName)) {
            output << " = null";
        } else if (!isListProperty) {
            output << " --";
        }

        printPropertyAttributes(metaProperty,
                                isInherited,
                                isOverride,
                                isDefault,
                                isDeferred,
                                currentPropertyTypeName);
        printDeclarationSuffix(declaringMetaObject);
        output << Qt::endl;

        if (currentPropertyTypeName == QStringLiteral("QJSValue") && propertyValue.isValid()) {
            const QJSValue scriptValue = propertyValue.value<QJSValue>();
            if (scriptValue.isObject()) {
                printQjsValueDetails(indentationLevel + 1, scriptValue);
                continue;
            }
        }

        if (currentPropertyTypeName == QStringLiteral("QFont")) {
            printFontDetails(indentationLevel + 1, inspectedObject);
            continue;
        }

        if (shouldExpandObjectProperty(currentPropertyTypeName, objectValue) && objectValue) {
            printObjectPropertyDetails(indentationLevel + 1, objectValue);
        }
    }
}

void MetaObjectPrinter::printAttachedTypes(const QObject* inspectedObject, const QObject* attachedProbeObject) const
{
    Q_UNUSED(inspectedObject);

    QList<QmlAttachedTypeEntry> visibleAttachedTypes;
    for (const QmlAttachedTypeEntry& attachedTypeEntry : attachedTypes) {
        if (hasVisibleAttachedProperties(attachedProbeObject, attachedTypeEntry)) {
            visibleAttachedTypes.append(attachedTypeEntry);
        }
    }

    if (visibleAttachedTypes.isEmpty()) {
        return;
    }

    output << Qt::endl << "Attached:" << Qt::endl;
    for (const QmlAttachedTypeEntry& attachedTypeEntry : visibleAttachedTypes) {
        printAttachedType(inspectedObject, attachedProbeObject, attachedTypeEntry);
    }
}

void MetaObjectPrinter::printAttachedType(const QObject* inspectedObject,
                                          const QObject* attachedProbeObject,
                                          const QmlAttachedTypeEntry& attachedTypeEntry) const
{
    Q_UNUSED(inspectedObject);

    output << indentation(1) << attachedTypeEntry.attachingTypeName << ':' << Qt::endl;

    for (const QmlAttachedTypeProperty& property : attachedTypeEntry.properties) {
        const QString probeName = attachedProbePropertyName(attachedTypeEntry, property);
        const QVariant value = attachedProbeObject ? attachedProbeObject->property(probeName.toUtf8().constData()) : QVariant();
        if (!value.isValid()) {
            continue;
        }

        output << indentation(2) << property.name << ": " << property.typeName;
        const QString formattedValue = formatAttachedPropertyValue(property, value);
        const bool isListProperty = property.isList || property.typeName.startsWith(QStringLiteral("QQmlListProperty"));
        if (!formattedValue.isEmpty() && !(isListProperty && formattedValue == QStringLiteral("list"))) {
            output << " = " << formattedValue;
        } else if (property.typeName.endsWith(QLatin1Char('*'))) {
            output << " = null";
        } else if (!isListProperty) {
            output << " --";
        }

        if (property.isReadonly) {
            output << " readonly";
        }
        if (property.isList) {
            output << " list";
        }
        if (property.isPointer) {
            output << " object";
        }
        output << Qt::endl;
    }

    for (const QmlAttachedTypeMethod& method : attachedTypeEntry.methods) {
        output << indentation(2) << "public " << (method.returnTypeName.isEmpty() ? QStringLiteral("void") : method.returnTypeName)
               << ' ' << method.name << '(';
        for (int parameterIndex = 0; parameterIndex < method.parameters.size(); ++parameterIndex) {
            const QmlAttachedTypeParameter& parameter = method.parameters.at(parameterIndex);
            output << parameter.typeName;
            if (!parameter.name.isEmpty()) {
                output << ' ' << parameter.name;
            }
            if (parameterIndex + 1 < method.parameters.size()) {
                output << ", ";
            }
        }
        output << ')';
        output << (method.isSignal ? " signal" : " method") << Qt::endl;
    }

    for (const QmlAttachedTypeEnum& enumerator : attachedTypeEntry.enumerators) {
        output << indentation(2) << enumerator.name << Qt::endl;
        for (const QString& key : enumerator.keys) {
            output << indentation(3) << key << Qt::endl;
        }
    }
}

bool MetaObjectPrinter::hasVisibleAttachedProperties(const QObject* attachedProbeObject, const QmlAttachedTypeEntry& attachedTypeEntry) const
{
    if (!attachedProbeObject) {
        return false;
    }

    for (const QmlAttachedTypeProperty& property : attachedTypeEntry.properties) {
        const QString probeName = attachedProbePropertyName(attachedTypeEntry, property);
        if (attachedProbeObject->property(probeName.toUtf8().constData()).isValid()) {
            return true;
        }
    }
    return false;
}

void MetaObjectPrinter::printMethods(int indentationLevel, const QMetaObject* metaObject) const
{
    if (!metaObject) {
        return;
    }

    const int methodCount = metaObject->methodCount();
    const int methodOffset = metaObject->methodOffset();
    if (methodCount == 0 || (!showAll && methodCount <= methodOffset)) {
        return;
    }

    if (indentationLevel == 0) {
        output << Qt::endl << "Methods:" << Qt::endl;
    }

    const QString currentDefaultMethodSignature = defaultMethodSignature(metaObject);

    for (int methodIndex = 0; methodIndex < methodCount; ++methodIndex) {
        if (!showAll && methodIndex < methodOffset) {
            continue;
        }

        const QMetaMethod metaMethod = metaObject->method(methodIndex);
        if (metaMethod.access() == QMetaMethod::Private) {
            continue;
        }

        output << indentation(indentationLevel + 1);
        if (metaMethod.access() == QMetaMethod::Protected) {
            output << "protected ";
        } else {
            output << "public ";
        }

        const QByteArray returnTypeName = metaMethod.typeName() ? QByteArray(metaMethod.typeName()) : QByteArrayLiteral("void");
        output << returnTypeName << ' ' << metaMethod.name() << '(';

        const QList<QByteArray> parameterTypes = metaMethod.parameterTypes();
        const QList<QByteArray> parameterNames = metaMethod.parameterNames();
        for (int parameterIndex = 0; parameterIndex < parameterTypes.size(); ++parameterIndex) {
            output << parameterTypes.at(parameterIndex);
            if (!parameterNames.at(parameterIndex).isEmpty()) {
                output << ' ' << parameterNames.at(parameterIndex);
            }
            if (parameterIndex + 1 < parameterTypes.size()) {
                output << ", ";
            }
        }
        output << ')';

        switch (metaMethod.methodType()) {
        case QMetaMethod::Method:
            output << " method";
            break;
        case QMetaMethod::Signal:
            output << " signal";
            break;
        case QMetaMethod::Slot:
            output << " slot";
            break;
        default:
            break;
        }

        if (methodIndex < methodOffset) {
            output << " inherited";
        }
        if (methodSignature(metaMethod) == currentDefaultMethodSignature) {
            output << " default";
        }
        printDeclarationSuffix(declaringMetaObjectForMethod(metaObject, methodIndex));
        output << Qt::endl;
    }
}

void MetaObjectPrinter::printEnumerators(int indentationLevel, const QMetaObject* metaObject) const
{
    if (!metaObject) {
        return;
    }

    const int enumeratorCount = metaObject->enumeratorCount();
    const int enumeratorOffset = metaObject->enumeratorOffset();
    if (enumeratorCount == 0 || (!showAll && enumeratorCount <= enumeratorOffset)) {
        return;
    }

    if (indentationLevel == 0) {
        output << Qt::endl << "Enumerators:" << Qt::endl;
    }

    for (int enumeratorIndex = 0; enumeratorIndex < enumeratorCount; ++enumeratorIndex) {
        if (!showAll && enumeratorIndex < enumeratorOffset) {
            continue;
        }

        const QMetaEnum metaEnumerator = metaObject->enumerator(enumeratorIndex);
        output << indentation(indentationLevel + 1) << metaEnumerator.name();
        if (enumeratorIndex < enumeratorOffset) {
            output << " inherited";
        }
        printDeclarationSuffix(declaringMetaObjectForEnumerator(metaObject, enumeratorIndex));
        output << Qt::endl;

        for (int keyIndex = 0; keyIndex < metaEnumerator.keyCount(); ++keyIndex) {
            output << indentation(indentationLevel + 2)
                   << metaEnumerator.key(keyIndex)
                   << " = "
                   << metaEnumerator.value(keyIndex)
                   << Qt::endl;
        }
    }
}

void MetaObjectPrinter::printObjectPropertyDetails(int indentationLevel, const QObject* propertyObject) const
{
    if (!propertyObject) {
        return;
    }

    const QMetaObject* propertyMetaObject = effectiveMetaObject(propertyObject);
    printDefaultMembers(propertyObject, propertyMetaObject);
    printProperties(indentationLevel, propertyObject, propertyMetaObject);
    printMethods(indentationLevel, propertyMetaObject);
    printEnumerators(indentationLevel, propertyMetaObject);
}

void MetaObjectPrinter::printQjsValueDetails(int indentationLevel, const QJSValue& scriptValue) const
{
    QJSValueIterator iterator(scriptValue);
    while (iterator.hasNext()) {
        iterator.next();
        output << indentation(indentationLevel + 2)
               << iterator.name()
               << ": "
               << iterator.value().toString()
               << Qt::endl;
    }
}

void MetaObjectPrinter::printPropertyAttributes(const QMetaProperty& metaProperty,
                                                bool isInherited,
                                                bool isOverride,
                                                bool isDefault,
                                                bool isDeferred,
                                                const QString& propertyTypeName) const
{
    if (metaProperty.isReadable() && !metaProperty.isWritable()) {
        output << " readonly";
    }
    if (metaProperty.isFinal()) {
        output << " final";
    }
    if (metaProperty.isConstant()) {
        output << " constant";
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (metaProperty.isRequired()) {
        output << " required";
    }
    if (metaProperty.isBindable()) {
        output << " bindable";
    }
#endif
    if (showAll && isInherited) {
        output << " inherited";
    }
    if (isOverride) {
        output << " override";
    }
    if (isDefault) {
        output << " default";
    }
    if (isDeferred) {
        output << " deferred";
    }
    if (isListPropertyTypeName(propertyTypeName)) {
        output << " list";
    }
    if (isObjectPointerTypeName(propertyTypeName)) {
        output << " object";
    }
}

void MetaObjectPrinter::printDeclarationSuffix(const QMetaObject* declaringMetaObject) const
{
    if (!showDeclaredIn || !declaringMetaObject) {
        return;
    }
    output << " [declared in " << declaringMetaObject->className() << ']';
}

void MetaObjectPrinter::printFontDetails(int indentationLevel, const QObject* inspectedObject) const
{
    static const QStringList fontPropertyNames = {
        QStringLiteral("font.family"),
        QStringLiteral("font.styleName"),
        QStringLiteral("font.bold"),
        QStringLiteral("font.weight"),
        QStringLiteral("font.italic"),
        QStringLiteral("font.underline"),
        QStringLiteral("font.overline"),
        QStringLiteral("font.strikeout"),
        QStringLiteral("font.pointSize"),
        QStringLiteral("font.pixelSize"),
        QStringLiteral("font.capitalization"),
        QStringLiteral("font.letterSpacing"),
        QStringLiteral("font.wordSpacing"),
        QStringLiteral("font.hintingPreference"),
        QStringLiteral("font.kerning"),
        QStringLiteral("font.preferShaping")
    };

    for (const QString& fontPropertyName : fontPropertyNames) {
        const QQmlProperty fontProperty(const_cast<QObject*>(inspectedObject), fontPropertyName, &engine);
        output << indentation(indentationLevel + 1)
               << fontPropertyName.mid(QStringLiteral("font.").size())
               << ": "
               << fontProperty.read().toString()
               << Qt::endl;
    }
}

QString MetaObjectPrinter::formatAttachedPropertyValue(const QmlAttachedTypeProperty& property,
                                                       const QVariant& value) const
{
    if (!value.isValid()) {
        return QString();
    }

    const QString typeName = property.typeName;
    if (typeName == QStringLiteral("int")) {
        return formatInteger(value.toInt());
    }
    if (typeName == QStringLiteral("uint")) {
        return formatUnsignedInteger(value.toUInt());
    }
    if (typeName == QStringLiteral("double") || typeName == QStringLiteral("float") || typeName == QStringLiteral("qreal")) {
        return QString::number(value.toDouble());
    }
    if (typeName == QStringLiteral("bool")) {
        return formatBoolean(value.toBool());
    }
    if (typeName == QStringLiteral("QString")) {
        return QStringLiteral("\"%1\"").arg(value.toString());
    }
    if (typeName == QStringLiteral("QColor")) {
        return QStringLiteral("\"%1\"").arg(value.value<QColor>().name(QColor::HexArgb));
    }
    if (typeName == QStringLiteral("QJSValue")) {
        const QJSValue scriptValue = value.value<QJSValue>();
        return scriptValue.isObject() ? QStringLiteral("Object") : scriptValue.toString();
    }
    if (typeName == QStringLiteral("QUrl")) {
        return QStringLiteral("\"%1\"").arg(value.toUrl().toString());
    }
    if (typeName == QStringLiteral("Qt::Orientation")) {
        return value.value<Qt::Orientation>() == Qt::Horizontal
            ? QStringLiteral("Qt::Horizontal")
            : QStringLiteral("Qt::Vertical");
    }

    if (QObject* objectValue = objectPointerValue(value)) {
        if (const QMetaObject* objectMetaObject = effectiveMetaObject(objectValue)) {
            return QString::fromLatin1(objectMetaObject->className());
        }
        return QStringLiteral("QObject");
    }
    if (typeName.startsWith(QStringLiteral("QQmlListProperty"))) {
        return QStringLiteral("list");
    }

    return value.toString();
}

QString MetaObjectPrinter::indentation(int indentationLevel) const
{
    return QString(indentationLevel * 4, u' ');
}

QString MetaObjectPrinter::formatPropertyValue(const QObject* inspectedObject, const QMetaProperty& metaProperty) const
{
    const QQmlProperty qmlProperty(const_cast<QObject*>(inspectedObject), metaProperty.name(), &engine);
    const QVariant value = qmlProperty.read();
    const QString currentPropertyTypeName = safeTypeName(qmlProperty, metaProperty);

    if (!value.isValid()) {
        return QString();
    }

    const QString enumString = enumValueToString(metaProperty, value);
    if (!enumString.isEmpty()) {
        return enumString;
    }

    if (currentPropertyTypeName == QStringLiteral("int")) {
        return formatInteger(value.toInt());
    }
    if (currentPropertyTypeName == QStringLiteral("uint")) {
        return formatUnsignedInteger(value.toUInt());
    }
    if (currentPropertyTypeName == QStringLiteral("double") || currentPropertyTypeName == QStringLiteral("float") || currentPropertyTypeName == QStringLiteral("qreal")) {
        return QString::number(value.toDouble());
    }
    if (currentPropertyTypeName == QStringLiteral("bool")) {
        return formatBoolean(value.toBool());
    }
    if (currentPropertyTypeName == QStringLiteral("QString")) {
        return QStringLiteral("\"%1\"").arg(value.toString());
    }
    if (currentPropertyTypeName == QStringLiteral("QColor")) {
        return QStringLiteral("\"%1\"").arg(value.value<QColor>().name(QColor::HexArgb));
    }
    if (currentPropertyTypeName == QStringLiteral("QFont")) {
        const QFont font = value.value<QFont>();
        return QStringLiteral("QFont(%1, %2)").arg(font.family(), QString::number(font.pointSize()));
    }
    if (currentPropertyTypeName == QStringLiteral("QJSValue")) {
        const QJSValue scriptValue = value.value<QJSValue>();
        if (!scriptValue.isObject()) {
            return scriptValue.toString();
        }
        return QStringLiteral("Object");
    }
    if (currentPropertyTypeName == QStringLiteral("QRectF")) {
        const QRectF rectangle = value.toRectF();
        return rectangle.isValid()
            ? QStringLiteral("%1,%2 %3x%4").arg(QString::number(rectangle.x()), QString::number(rectangle.y()), QString::number(rectangle.width()), QString::number(rectangle.height()))
            : QStringLiteral("Invalid");
    }
    if (currentPropertyTypeName == QStringLiteral("QSizeF")) {
        const QSizeF size = value.toSizeF();
        return size.isValid()
            ? QStringLiteral("%1x%2").arg(QString::number(size.width()), QString::number(size.height()))
            : QStringLiteral("Invalid");
    }
    if (currentPropertyTypeName == QStringLiteral("QPointF")) {
        const QPointF point = value.toPointF();
        return QStringLiteral("%1,%2").arg(QString::number(point.x()), QString::number(point.y()));
    }
    if (currentPropertyTypeName == QStringLiteral("QUrl")) {
        return QStringLiteral("\"%1\"").arg(value.toUrl().toString());
    }
    if (currentPropertyTypeName == QStringLiteral("Qt::Orientation")) {
        return value.value<Qt::Orientation>() == Qt::Horizontal
            ? QStringLiteral("Qt::Horizontal")
            : QStringLiteral("Qt::Vertical");
    }

    if (QObject* objectValue = objectPointerValue(value)) {
        if (const QMetaObject* objectMetaObject = effectiveMetaObject(objectValue)) {
            return QString::fromLatin1(objectMetaObject->className());
        }
        return QStringLiteral("QObject");
    }
    if (currentPropertyTypeName.startsWith(QStringLiteral("QQmlListProperty"))) {
        return QStringLiteral("list");
    }

    return QString();
}
