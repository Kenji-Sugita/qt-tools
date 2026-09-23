#pragma once

#include <QtCore/qglobal.h>

#include <QtCore/QByteArray>
#include <QtCore/QHash>
#include <QtCore/QMetaProperty>
#include <QtCore/QPointer>
#include <QtCore/QVariant>
#include <QtCore/QVector>
#include <QtGui/QFont>
#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QWidget>

class QTimer;
class QTreeWidgetItem;

#if defined(PROPERTY_EDITOR_LIBRARY)
#  define PROPERTY_EDITOR_EXPORT Q_DECL_EXPORT
#else
#  define PROPERTY_EDITOR_EXPORT Q_DECL_IMPORT
#endif

class ObjectPropertyEditorWidget;

class PROPERTY_EDITOR_EXPORT ObjectPropertyItemDelegate : public QStyledItemDelegate
{
public:
    explicit ObjectPropertyItemDelegate(ObjectPropertyEditorWidget *propertyEditorWidget);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &styleOption,
                          const QModelIndex &modelIndex) const override;
    void setEditorData(QWidget *editorWidget, const QModelIndex &modelIndex) const override;
    void setModelData(QWidget *editorWidget, QAbstractItemModel *model,
                      const QModelIndex &modelIndex) const override;

private:
    ObjectPropertyEditorWidget *propertyEditorWidget = nullptr;
};

class PROPERTY_EDITOR_EXPORT ObjectPropertyEditorWidget : public QWidget
{
    Q_OBJECT

public:
    enum class PropertyValueKind {
        Invalid,
        Boolean,
        Integer,
        UnsignedInteger,
        LongLongInteger,
        UnsignedLongLongInteger,
        DoubleValue,
        String,
        StringList,
        EnumValue,
        FlagsValue,
        Point,
        Size,
        Rect,
        Margins,
        Color,
        Palette,
        Font,
        ByteArray,
        Cursor,
        Orientation,
        SizePolicyValue,
        LocaleValue,
        FallbackString
    };

    enum ItemDataRole {
        PropertyNameRole = Qt::UserRole + 1,
        PropertyKindRole,
        DynamicPropertyRole,
        WritableRole,
        EnumKeysRole,
        EnumValuesRole,
        GroupRole,
        SourcePropertyNameRole,
        SubPropertyRole
    };

    explicit ObjectPropertyEditorWidget(QWidget *parentWidget = nullptr);

    void setInspectedObject(QObject *targetObject);
    QObject *inspectedObject() const;

    PropertyValueKind propertyKindFromVariant(const QVariant &propertyValue) const;
    QString displayTextForValue(const QVariant &propertyValue, PropertyValueKind propertyKind) const;
    QString paletteDisplayText(const QString &propertyName, const QVariant &propertyValue) const;
    bool isPaletteInherited(const QString &propertyName) const;
    QVariant editableValueForItem(const QTreeWidgetItem *treeItem) const;
    QVariant valueFromEditor(QWidget *editorWidget, const QTreeWidgetItem *treeItem) const;
    void applyEditedValue(QTreeWidgetItem *treeItem, const QVariant &editedValue);
    void registerEditorWidget(QWidget *editorWidget);
    void closePersistentEditorForIndex(const QModelIndex &modelIndex);
    QTreeWidget *treeWidget() const;

private slots:
    void refreshAllProperties();

private:
    void showEvent(QShowEvent *showEvent) override;
    void rebuildPropertyTree();
    void addMetaObjectProperties();
    void addDynamicProperties();
    void addPropertyItem(const QString &groupName,
                         const QString &propertyName,
                         const QVariant &propertyValue,
                         PropertyValueKind propertyKind,
                         bool isWritable,
                         bool isDynamicProperty,
                         const QStringList &enumKeys = {},
                         const QList<int> &enumValues = {});
    void addSubPropertyItem(QTreeWidgetItem *parentPropertyItem,
                            const QString &groupName,
                            const QString &sourcePropertyName,
                            PropertyValueKind propertyKind,
                            int subPropertyCode,
                            bool isWritable,
                            const QStringList &enumKeys = {},
                            const QList<int> &enumValues = {});
    void addFontSubProperties(QTreeWidgetItem *fontPropertyItem,
                              const QString &groupName,
                              const QString &propertyName,
                              const QFont &fontValue,
                              bool isWritable);
    void addSizeSubProperties(QTreeWidgetItem *sizePropertyItem,
                              const QString &groupName,
                              const QString &propertyName,
                              bool isWritable);
    void addRectSubProperties(QTreeWidgetItem *rectPropertyItem,
                              const QString &groupName,
                              const QString &propertyName,
                              bool isWritable);
    void addMarginsSubProperties(QTreeWidgetItem *marginsPropertyItem,
                                 const QString &groupName,
                                 const QString &propertyName,
                                 bool isWritable);
    void addSizePolicySubProperties(QTreeWidgetItem *sizePolicyPropertyItem,
                                    const QString &groupName,
                                    const QString &propertyName,
                                    bool isWritable);
    void addLocaleSubProperties(QTreeWidgetItem *localePropertyItem,
                                const QString &groupName,
                                const QString &propertyName,
                                bool isWritable);
    QTreeWidgetItem *ensureGroupItem(const QString &groupName);
    void updateItemPresentation(QTreeWidgetItem *treeItem, const QVariant &propertyValue,
                                PropertyValueKind propertyKind);
    QString propertyGroupNameForMetaProperty(const QMetaProperty &metaProperty) const;
    bool shouldForceShowMetaProperty(const QMetaProperty &metaProperty) const;
    bool isPropertyKindEditable(PropertyValueKind propertyKind) const;
    bool isManagedByLayout(const QString &propertyName) const;
    bool shouldSkipMetaProperty(const QMetaProperty &metaProperty,
                                const QVariant &propertyValue,
                                PropertyValueKind propertyKind) const;
    QVariant readPropertyValue(const QString &propertyName) const;
    QString sourcePropertyNameForItem(const QTreeWidgetItem *treeItem) const;
    bool writePropertyValue(const QString &propertyName, const QVariant &propertyValue);
    void handleItemChanged(QTreeWidgetItem *treeItem, int columnIndex);
    void refreshPropertyValueByName(const QString &propertyName);
    void clearObservedConnections();
    void connectNotifySignals();
    void connectCommonWidgetSignals();
    void attachPeriodicRefresh();

    QPointer<QObject> inspectedTargetObject;
    QTreeWidget *propertyTreeWidget = nullptr;
    ObjectPropertyItemDelegate *itemDelegate = nullptr;
    QHash<QString, QTreeWidgetItem *> groupItems;
    QHash<QString, QTreeWidgetItem *> propertyItemsByName;
    QVector<QMetaObject::Connection> observedConnections;
    QMetaObject::Connection destroyedConnection;
    bool isUpdatingTree = false;
    int activeEditorWidgetCount = 0;
    QTreeWidgetItem *persistentEditorItem = nullptr;
    QTimer *refreshTimer = nullptr;
    bool hasAppliedInitialColumnWidths = false;
};
