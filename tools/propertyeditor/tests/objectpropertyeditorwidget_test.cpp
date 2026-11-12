#include "objectpropertyeditorwidget.h"

#include <functional>
#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtGui/QColor>
#include <QtGui/QCursor>
#include <QtGui/QFont>
#include <QtGui/QPalette>
#include <QtTest/QTest>
#include <QtWidgets/QAbstractSpinBox>
#include <QtWidgets/QStyleOptionViewItem>
#include <QtWidgets/QTreeWidgetItemIterator>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>

namespace {
constexpr int propertyNameColumn = 0;
constexpr int propertyValueColumn = 1;

QTreeWidgetItem *findPropertyItem(ObjectPropertyEditorWidget *propertyEditorWidget,
                                  const QString &propertyName)
{
    if (propertyEditorWidget == nullptr) {
        return nullptr;
    }

    QTreeWidgetItemIterator itemIterator(propertyEditorWidget->treeWidget());
    while (*itemIterator != nullptr) {
        QTreeWidgetItem *treeItem = *itemIterator;
        ++itemIterator;

        if (treeItem->data(propertyValueColumn, ObjectPropertyEditorWidget::PropertyNameRole).toString()
            == propertyName) {
            return treeItem;
        }
    }

    return nullptr;
}

QModelIndex propertyValueIndex(ObjectPropertyEditorWidget *propertyEditorWidget,
                               const QString &propertyName)
{
    QTreeWidgetItem *treeItem = findPropertyItem(propertyEditorWidget, propertyName);
    if (treeItem == nullptr) {
        return {};
    }

    return propertyEditorWidget->treeWidget()->indexFromItem(treeItem, propertyValueColumn);
}

ObjectPropertyItemDelegate *itemDelegate(ObjectPropertyEditorWidget *propertyEditorWidget)
{
    return static_cast<ObjectPropertyItemDelegate *>(propertyEditorWidget->treeWidget()->itemDelegate());
}

QString testFontWeightText(int fontWeight)
{
    switch (fontWeight) {
    case QFont::Thin:
        return QStringLiteral("Thin");
    case QFont::ExtraLight:
        return QStringLiteral("ExtraLight");
    case QFont::Light:
        return QStringLiteral("Light");
    case QFont::Normal:
        return QStringLiteral("Normal");
    case QFont::Medium:
        return QStringLiteral("Medium");
    case QFont::DemiBold:
        return QStringLiteral("DemiBold");
    case QFont::Bold:
        return QStringLiteral("Bold");
    case QFont::ExtraBold:
        return QStringLiteral("ExtraBold");
    case QFont::Black:
        return QStringLiteral("Black");
    }

    return QString::number(fontWeight);
}

QString testAntialiasingText(const QFont &fontValue)
{
    const int styleStrategy = static_cast<int>(fontValue.styleStrategy());
    if ((styleStrategy & QFont::NoAntialias) == QFont::NoAntialias) {
        return QStringLiteral("No Antialias");
    }
    if ((styleStrategy & QFont::PreferAntialias) == QFont::PreferAntialias) {
        return QStringLiteral("Prefer Antialias");
    }
    return QStringLiteral("Prefer Default Antialias");
}

QString testHintingPreferenceText(QFont::HintingPreference hintingPreference)
{
    switch (hintingPreference) {
    case QFont::PreferDefaultHinting:
        return QStringLiteral("PreferDefaultHinting");
    case QFont::PreferNoHinting:
        return QStringLiteral("PreferNoHinting");
    case QFont::PreferVerticalHinting:
        return QStringLiteral("PreferVerticalHinting");
    case QFont::PreferFullHinting:
        return QStringLiteral("PreferFullHinting");
    }

    return QString::number(static_cast<int>(hintingPreference));
}

}

class PropertyEditorTestObject : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int integerValue READ integerValue WRITE setIntegerValue NOTIFY integerValueChanged)
    Q_PROPERTY(uint unsignedIntegerValue READ unsignedIntegerValue WRITE setUnsignedIntegerValue NOTIFY unsignedIntegerValueChanged)
    Q_PROPERTY(qlonglong longLongValue READ longLongValue WRITE setLongLongValue NOTIFY longLongValueChanged)
    Q_PROPERTY(qulonglong unsignedLongLongValue READ unsignedLongLongValue WRITE setUnsignedLongLongValue NOTIFY unsignedLongLongValueChanged)
    Q_PROPERTY(double doubleValue READ doubleValue WRITE setDoubleValue NOTIFY doubleValueChanged)
    Q_PROPERTY(QString textValue READ textValue WRITE setTextValue NOTIFY textValueChanged)
    Q_PROPERTY(bool booleanValue READ booleanValue WRITE setBooleanValue NOTIFY booleanValueChanged)
    Q_PROPERTY(QStringList stringListValue READ stringListValue WRITE setStringListValue NOTIFY stringListValueChanged)
    Q_PROPERTY(QPoint pointValue READ pointValue WRITE setPointValue NOTIFY pointValueChanged)
    Q_PROPERTY(QSize sizeValue READ sizeValue WRITE setSizeValue NOTIFY sizeValueChanged)
    Q_PROPERTY(QRect rectValue READ rectValue WRITE setRectValue NOTIFY rectValueChanged)
    Q_PROPERTY(QColor colorValue READ colorValue WRITE setColorValue NOTIFY colorValueChanged)
    Q_PROPERTY(QByteArray byteArrayValue READ byteArrayValue WRITE setByteArrayValue NOTIFY byteArrayValueChanged)
    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(Options options READ options WRITE setOptions NOTIFY optionsChanged)
    Q_PROPERTY(QString hiddenTextValue READ hiddenTextValue WRITE setHiddenTextValue NOTIFY hiddenTextValueChanged DESIGNABLE false)
    Q_PROPERTY(QString readOnlyText READ readOnlyText NOTIFY readOnlyTextChanged)

public:
    enum Mode {
        Alpha = 0,
        Beta = 1,
        Gamma = 2
    };
    Q_ENUM(Mode)

    enum Option {
        OptionAlpha = 0x1,
        OptionBeta = 0x2,
        OptionGamma = 0x4
    };
    Q_ENUM(Option)
    Q_DECLARE_FLAGS(Options, Option)
    Q_FLAG(Options)

    explicit PropertyEditorTestObject(QWidget *parentWidget = nullptr)
        : QWidget(parentWidget)
    {
    }

    int integerValue() const
    {
        return integerValueStorage;
    }

    void setIntegerValue(int value)
    {
        if (integerValueStorage == value) {
            return;
        }
        integerValueStorage = value;
        emit integerValueChanged(integerValueStorage);
    }

    unsigned int unsignedIntegerValue() const
    {
        return unsignedIntegerValueStorage;
    }

    void setUnsignedIntegerValue(unsigned int value)
    {
        if (unsignedIntegerValueStorage == value) {
            return;
        }
        unsignedIntegerValueStorage = value;
        emit unsignedIntegerValueChanged(unsignedIntegerValueStorage);
    }

    qlonglong longLongValue() const
    {
        return longLongValueStorage;
    }

    void setLongLongValue(qlonglong value)
    {
        if (longLongValueStorage == value) {
            return;
        }
        longLongValueStorage = value;
        emit longLongValueChanged(longLongValueStorage);
    }

    qulonglong unsignedLongLongValue() const
    {
        return unsignedLongLongValueStorage;
    }

    void setUnsignedLongLongValue(qulonglong value)
    {
        if (unsignedLongLongValueStorage == value) {
            return;
        }
        unsignedLongLongValueStorage = value;
        emit unsignedLongLongValueChanged(unsignedLongLongValueStorage);
    }

    double doubleValue() const
    {
        return doubleValueStorage;
    }

    void setDoubleValue(double value)
    {
        if (qFuzzyCompare(doubleValueStorage, value)) {
            return;
        }
        doubleValueStorage = value;
        emit doubleValueChanged(doubleValueStorage);
    }

    QString textValue() const
    {
        return textValueStorage;
    }

    void setTextValue(const QString &value)
    {
        if (textValueStorage == value) {
            return;
        }
        textValueStorage = value;
        emit textValueChanged(textValueStorage);
    }

    bool booleanValue() const
    {
        return booleanValueStorage;
    }

    void setBooleanValue(bool value)
    {
        if (booleanValueStorage == value) {
            return;
        }
        booleanValueStorage = value;
        emit booleanValueChanged(booleanValueStorage);
    }

    QStringList stringListValue() const
    {
        return stringListValueStorage;
    }

    void setStringListValue(const QStringList &value)
    {
        if (stringListValueStorage == value) {
            return;
        }
        stringListValueStorage = value;
        emit stringListValueChanged(stringListValueStorage);
    }

    QPoint pointValue() const
    {
        return pointValueStorage;
    }

    void setPointValue(const QPoint &value)
    {
        if (pointValueStorage == value) {
            return;
        }
        pointValueStorage = value;
        emit pointValueChanged(pointValueStorage);
    }

    QSize sizeValue() const
    {
        return sizeValueStorage;
    }

    void setSizeValue(const QSize &value)
    {
        if (sizeValueStorage == value) {
            return;
        }
        sizeValueStorage = value;
        emit sizeValueChanged(sizeValueStorage);
    }

    QRect rectValue() const
    {
        return rectValueStorage;
    }

    void setRectValue(const QRect &value)
    {
        if (rectValueStorage == value) {
            return;
        }
        rectValueStorage = value;
        emit rectValueChanged(rectValueStorage);
    }

    QColor colorValue() const
    {
        return colorValueStorage;
    }

    void setColorValue(const QColor &value)
    {
        if (colorValueStorage == value) {
            return;
        }
        colorValueStorage = value;
        emit colorValueChanged(colorValueStorage);
    }

    QByteArray byteArrayValue() const
    {
        return byteArrayValueStorage;
    }

    void setByteArrayValue(const QByteArray &value)
    {
        if (byteArrayValueStorage == value) {
            return;
        }
        byteArrayValueStorage = value;
        emit byteArrayValueChanged(byteArrayValueStorage);
    }

    Mode mode() const
    {
        return modeStorage;
    }

    void setMode(Mode value)
    {
        if (modeStorage == value) {
            return;
        }
        modeStorage = value;
        emit modeChanged(modeStorage);
    }

    Options options() const
    {
        return optionsStorage;
    }

    void setOptions(Options value)
    {
        if (optionsStorage == value) {
            return;
        }
        optionsStorage = value;
        emit optionsChanged(optionsStorage);
    }

    QString hiddenTextValue() const
    {
        return hiddenTextValueStorage;
    }

    QString readOnlyText() const
    {
        return readOnlyTextStorage;
    }

    void setHiddenTextValue(const QString &value)
    {
        if (hiddenTextValueStorage == value) {
            return;
        }
        hiddenTextValueStorage = value;
        emit hiddenTextValueChanged(hiddenTextValueStorage);
    }

signals:
    void integerValueChanged(int value);
    void unsignedIntegerValueChanged(unsigned int value);
    void longLongValueChanged(qlonglong value);
    void unsignedLongLongValueChanged(qulonglong value);
    void doubleValueChanged(double value);
    void textValueChanged(const QString &value);
    void booleanValueChanged(bool value);
    void stringListValueChanged(const QStringList &value);
    void pointValueChanged(const QPoint &value);
    void sizeValueChanged(const QSize &value);
    void rectValueChanged(const QRect &value);
    void colorValueChanged(const QColor &value);
    void byteArrayValueChanged(const QByteArray &value);
    void modeChanged(Mode value);
    void optionsChanged(Options value);
    void hiddenTextValueChanged(const QString &value);
    void readOnlyTextChanged(const QString &value);

private:
    int integerValueStorage = 5;
    unsigned int unsignedIntegerValueStorage = 4000000000U;
    qlonglong longLongValueStorage = -5000000000LL;
    qulonglong unsignedLongLongValueStorage = 10000000000ULL;
    double doubleValueStorage = 1.5;
    QString textValueStorage = QStringLiteral("hello");
    bool booleanValueStorage = false;
    QStringList stringListValueStorage {QStringLiteral("red"), QStringLiteral("green")};
    QPoint pointValueStorage {3, 4};
    QSize sizeValueStorage {40, 50};
    QRect rectValueStorage {1, 2, 30, 60};
    QColor colorValueStorage {Qt::red};
    QByteArray byteArrayValueStorage {QByteArray::fromHex("0a0b0c")};
    Mode modeStorage = Beta;
    Options optionsStorage = Options(OptionAlpha | OptionGamma);
    QString hiddenTextValueStorage = QStringLiteral("hidden");
    QString readOnlyTextStorage = QStringLiteral("read only");
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PropertyEditorTestObject::Options)

class ObjectPropertyEditorWidgetTest : public QObject
{
    Q_OBJECT

private slots:
    void propertyTreeIncludesMetaAndDynamicProperties();
    void dialogModalPropertyMatchesQtDesignerBehavior();
    void booleanPropertyIsWrittenFromCheckState();
    void integerPropertyUsesSpinBoxEditor();
    void extendedIntegerPropertiesUseSpinBoxEditors();
    void enumPropertyUsesComboBoxEditorForSliderOrientation();
    void flagsPropertyUsesCheckBoxEditorForMultipleSelection();
    void cursorPropertyUsesComboBoxEditor();
    void fontPropertyExpandsLikeQtDesigner();
    void fontSubPropertiesWriteBackToFontProperty();
    void fontFamilyEditorIsNotTextEditable();
    void fontEnumSubPropertiesShowComboBoxText();
    void fontFamilyEditorKeepsCurrentValueWhenFontIsNotListed();
    void colorPropertyUsesColorDialogEditor();
    void colorValueFromEditorReturnsSelectedColor();
    void palettePropertyUsesPaletteDialogEditor();
    void palettePropertyShowsInheritedOrChangedText();
    void externalSliderValueChangeRefreshesTree();
    void lineEditParsingHandlesPointSizeRectStringListAndByteArray();
    void displayTextFormattingCoversRepresentativeTypes();
    void readOnlyPropertyIsHiddenLikeQtDesigner();
    void sizePropertyUsesDisclosureChildrenAndWritesBack();
    void layoutContentsMarginsUsesDisclosureChildrenAndWritesBack();
    void sizePolicyUsesDisclosureChildrenAndWritesBack();
    void localePropertyUsesDisclosureChildrenAndWritesBack();
    void windowOpacityEditorUsesSingleStepPointOne();
    void geometryUsesDisclosureChildrenAndIsNotEditableWhenManagedByLayout();
};

void ObjectPropertyEditorWidgetTest::propertyTreeIncludesMetaAndDynamicProperties()
{
    ObjectPropertyEditorWidget propertyEditorWidget;
    PropertyEditorTestObject targetObject;
    targetObject.setProperty("dynamicText", QStringLiteral("dynamic value"));

    propertyEditorWidget.setInspectedObject(&targetObject);

    QTreeWidgetItem *integerItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("integerValue"));
    QVERIFY(integerItem != nullptr);
    QCOMPARE(integerItem->text(propertyValueColumn), QStringLiteral("5"));

    QTreeWidgetItem *dynamicItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("dynamicText"));
    QVERIFY(dynamicItem != nullptr);
    QCOMPARE(dynamicItem->data(propertyValueColumn, ObjectPropertyEditorWidget::DynamicPropertyRole).toBool(), true);
    QCOMPARE(dynamicItem->data(propertyValueColumn, ObjectPropertyEditorWidget::GroupRole).toString(),
             QStringLiteral("Dynamic Properties"));
    QCOMPARE(dynamicItem->text(propertyValueColumn), QStringLiteral("dynamic value"));

    QTreeWidgetItem *hiddenItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("hiddenTextValue"));
    QCOMPARE(hiddenItem, nullptr);
}

void ObjectPropertyEditorWidgetTest::dialogModalPropertyMatchesQtDesignerBehavior()
{
    ObjectPropertyEditorWidget propertyEditorWidget;
    QDialog targetDialog;
    targetDialog.setModal(false);

    propertyEditorWidget.setInspectedObject(&targetDialog);

    QTreeWidgetItem *modalItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("modal"));
    QVERIFY(modalItem != nullptr);
    QCOMPARE(modalItem->data(propertyValueColumn, ObjectPropertyEditorWidget::GroupRole).toString(),
             QStringLiteral("QDialog"));
    QVERIFY(modalItem->flags().testFlag(Qt::ItemIsUserCheckable));
    QCOMPARE(targetDialog.isModal(), false);

    modalItem->setCheckState(propertyValueColumn, Qt::Checked);
    QCoreApplication::processEvents();

    QCOMPARE(targetDialog.isModal(), true);
    QCOMPARE(modalItem->text(propertyValueColumn), QStringLiteral("true"));
}

void ObjectPropertyEditorWidgetTest::booleanPropertyIsWrittenFromCheckState()
{
    ObjectPropertyEditorWidget propertyEditorWidget;
    PropertyEditorTestObject targetObject;
    propertyEditorWidget.setInspectedObject(&targetObject);

    QTreeWidgetItem *booleanItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("booleanValue"));
    QVERIFY(booleanItem != nullptr);
    QVERIFY(booleanItem->flags().testFlag(Qt::ItemIsUserCheckable));
    QCOMPARE(targetObject.booleanValue(), false);

    booleanItem->setCheckState(propertyValueColumn, Qt::Checked);
    QCoreApplication::processEvents();

    QCOMPARE(targetObject.booleanValue(), true);
    QCOMPARE(booleanItem->text(propertyValueColumn), QStringLiteral("true"));
}

void ObjectPropertyEditorWidgetTest::integerPropertyUsesSpinBoxEditor()
{
    ObjectPropertyEditorWidget propertyEditorWidget;
    PropertyEditorTestObject targetObject;
    propertyEditorWidget.setInspectedObject(&targetObject);

    const QModelIndex modelIndex = propertyValueIndex(&propertyEditorWidget, QStringLiteral("integerValue"));
    QVERIFY(modelIndex.isValid());

    QWidget *editorWidget = itemDelegate(&propertyEditorWidget)->createEditor(
        propertyEditorWidget.treeWidget(), QStyleOptionViewItem(), modelIndex);
    QVERIFY(editorWidget != nullptr);

    auto *spinBox = qobject_cast<QSpinBox *>(editorWidget);
    QVERIFY(spinBox != nullptr);

    itemDelegate(&propertyEditorWidget)->setEditorData(editorWidget, modelIndex);
    QCOMPARE(spinBox->value(), 5);

    spinBox->setValue(42);
    itemDelegate(&propertyEditorWidget)->setModelData(editorWidget,
                                                      propertyEditorWidget.treeWidget()->model(),
                                                      modelIndex);

    QCOMPARE(targetObject.integerValue(), 42);

    QTreeWidgetItem *integerItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("integerValue"));
    QVERIFY(integerItem != nullptr);
    QCOMPARE(integerItem->text(propertyValueColumn), QString::number(42));

    delete editorWidget;
}

void ObjectPropertyEditorWidgetTest::extendedIntegerPropertiesUseSpinBoxEditors()
{
    ObjectPropertyEditorWidget propertyEditorWidget;
    PropertyEditorTestObject targetObject;
    propertyEditorWidget.setInspectedObject(&targetObject);

    const struct TestCase {
        QString propertyName;
        QString expectedInitialText;
        QString editedText;
        std::function<void()> verifyTargetValue;
    } testCases[] = {
        {
            QStringLiteral("unsignedIntegerValue"),
            QStringLiteral("4000000000"),
            QStringLiteral("4000000001"),
            [&targetObject]() {
                QCOMPARE(targetObject.unsignedIntegerValue(), 4000000001U);
            }
        },
        {
            QStringLiteral("longLongValue"),
            QStringLiteral("-5000000000"),
            QStringLiteral("-5000000001"),
            [&targetObject]() {
                QCOMPARE(targetObject.longLongValue(), -5000000001LL);
            }
        },
        {
            QStringLiteral("unsignedLongLongValue"),
            QStringLiteral("10000000000"),
            QStringLiteral("10000000001"),
            [&targetObject]() {
                QCOMPARE(targetObject.unsignedLongLongValue(), 10000000001ULL);
            }
        }
    };

    for (const TestCase &testCase : testCases) {
        const QModelIndex modelIndex = propertyValueIndex(&propertyEditorWidget, testCase.propertyName);
        QVERIFY(modelIndex.isValid());

        QWidget *editorWidget = itemDelegate(&propertyEditorWidget)->createEditor(
            propertyEditorWidget.treeWidget(), QStyleOptionViewItem(), modelIndex);
        QVERIFY(editorWidget != nullptr);

        auto *spinBox = qobject_cast<QAbstractSpinBox *>(editorWidget);
        QVERIFY(spinBox != nullptr);
        QVERIFY(qobject_cast<QSpinBox *>(editorWidget) == nullptr);

        itemDelegate(&propertyEditorWidget)->setEditorData(editorWidget, modelIndex);

        auto *editorLineEdit = editorWidget->findChild<QLineEdit *>();
        QVERIFY(editorLineEdit != nullptr);
        QCOMPARE(editorLineEdit->text(), testCase.expectedInitialText);

        editorLineEdit->setText(testCase.editedText);
        itemDelegate(&propertyEditorWidget)->setModelData(editorWidget,
                                                          propertyEditorWidget.treeWidget()->model(),
                                                          modelIndex);

        testCase.verifyTargetValue();

        QTreeWidgetItem *propertyItem = findPropertyItem(&propertyEditorWidget, testCase.propertyName);
        QVERIFY(propertyItem != nullptr);
        QCOMPARE(propertyItem->text(propertyValueColumn), testCase.editedText);

        delete editorWidget;
    }
}

void ObjectPropertyEditorWidgetTest::enumPropertyUsesComboBoxEditorForSliderOrientation()
{
    ObjectPropertyEditorWidget propertyEditorWidget;
    QSlider targetSlider(Qt::Horizontal);
    propertyEditorWidget.setInspectedObject(&targetSlider);

    QTreeWidgetItem *orientationItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("orientation"));
    QVERIFY(orientationItem != nullptr);
    QCOMPARE(orientationItem->data(propertyValueColumn, ObjectPropertyEditorWidget::PropertyKindRole).toInt(),
             static_cast<int>(ObjectPropertyEditorWidget::PropertyValueKind::EnumValue));

    const QModelIndex modelIndex = propertyValueIndex(&propertyEditorWidget, QStringLiteral("orientation"));
    QVERIFY(modelIndex.isValid());

    QWidget *editorWidget = itemDelegate(&propertyEditorWidget)->createEditor(
        propertyEditorWidget.treeWidget(), QStyleOptionViewItem(), modelIndex);
    QVERIFY(editorWidget != nullptr);

    auto *comboBox = qobject_cast<QComboBox *>(editorWidget);
    QVERIFY(comboBox != nullptr);

    itemDelegate(&propertyEditorWidget)->setEditorData(editorWidget, modelIndex);
    QCOMPARE(comboBox->currentText(), QStringLiteral("Horizontal"));
    QVERIFY(comboBox->findText(QStringLiteral("Horizontal")) >= 0);
    QVERIFY(comboBox->findText(QStringLiteral("Vertical")) >= 0);

    comboBox->setCurrentIndex(comboBox->findText(QStringLiteral("Vertical")));
    itemDelegate(&propertyEditorWidget)->setModelData(editorWidget,
                                                      propertyEditorWidget.treeWidget()->model(),
                                                      modelIndex);

    QCOMPARE(targetSlider.orientation(), Qt::Vertical);
    QCOMPARE(orientationItem->text(propertyValueColumn), QStringLiteral("Vertical"));

    delete editorWidget;
}

void ObjectPropertyEditorWidgetTest::flagsPropertyUsesCheckBoxEditorForMultipleSelection()
{
    ObjectPropertyEditorWidget propertyEditorWidget;
    PropertyEditorTestObject targetObject;
    propertyEditorWidget.setInspectedObject(&targetObject);

    QTreeWidgetItem *optionsItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("options"));
    QVERIFY(optionsItem != nullptr);
    QCOMPARE(optionsItem->data(propertyValueColumn, ObjectPropertyEditorWidget::PropertyKindRole).toInt(),
             static_cast<int>(ObjectPropertyEditorWidget::PropertyValueKind::FlagsValue));
    QCOMPARE(optionsItem->text(propertyValueColumn), QStringLiteral("OptionAlpha | OptionGamma"));

    const QModelIndex modelIndex = propertyValueIndex(&propertyEditorWidget, QStringLiteral("options"));
    QVERIFY(modelIndex.isValid());

    QWidget *editorWidget = itemDelegate(&propertyEditorWidget)->createEditor(
        propertyEditorWidget.treeWidget(), QStyleOptionViewItem(), modelIndex);
    QVERIFY(editorWidget != nullptr);

    const QList<QCheckBox *> checkBoxes = editorWidget->findChildren<QCheckBox *>();
    QCOMPARE(checkBoxes.size(), 3);

    itemDelegate(&propertyEditorWidget)->setEditorData(editorWidget, modelIndex);

    QCheckBox *optionAlphaCheckBox = nullptr;
    QCheckBox *optionBetaCheckBox = nullptr;
    QCheckBox *optionGammaCheckBox = nullptr;
    for (QCheckBox *checkBox : checkBoxes) {
        if (checkBox->text() == QStringLiteral("OptionAlpha")) {
            optionAlphaCheckBox = checkBox;
        } else if (checkBox->text() == QStringLiteral("OptionBeta")) {
            optionBetaCheckBox = checkBox;
        } else if (checkBox->text() == QStringLiteral("OptionGamma")) {
            optionGammaCheckBox = checkBox;
        }
    }

    QVERIFY(optionAlphaCheckBox != nullptr);
    QVERIFY(optionBetaCheckBox != nullptr);
    QVERIFY(optionGammaCheckBox != nullptr);
    QCOMPARE(optionAlphaCheckBox->isChecked(), true);
    QCOMPARE(optionBetaCheckBox->isChecked(), false);
    QCOMPARE(optionGammaCheckBox->isChecked(), true);

    optionAlphaCheckBox->setChecked(false);
    optionBetaCheckBox->setChecked(true);
    optionGammaCheckBox->setChecked(true);
    itemDelegate(&propertyEditorWidget)->setModelData(editorWidget,
                                                      propertyEditorWidget.treeWidget()->model(),
                                                      modelIndex);

    QCOMPARE(targetObject.options(), PropertyEditorTestObject::Options(
                                         PropertyEditorTestObject::OptionBeta
                                         | PropertyEditorTestObject::OptionGamma));
    QCOMPARE(optionsItem->text(propertyValueColumn), QStringLiteral("OptionBeta | OptionGamma"));

    delete editorWidget;
}

void ObjectPropertyEditorWidgetTest::cursorPropertyUsesComboBoxEditor()
{
    ObjectPropertyEditorWidget propertyEditorWidget;
    PropertyEditorTestObject targetObject;
    targetObject.setCursor(Qt::ArrowCursor);
    propertyEditorWidget.setInspectedObject(&targetObject);

    QTreeWidgetItem *cursorItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("cursor"));
    QVERIFY(cursorItem != nullptr);
    QCOMPARE(cursorItem->data(propertyValueColumn, ObjectPropertyEditorWidget::PropertyKindRole).toInt(),
             static_cast<int>(ObjectPropertyEditorWidget::PropertyValueKind::Cursor));

    const QModelIndex modelIndex = propertyValueIndex(&propertyEditorWidget, QStringLiteral("cursor"));
    QVERIFY(modelIndex.isValid());

    QWidget *editorWidget = itemDelegate(&propertyEditorWidget)->createEditor(
        propertyEditorWidget.treeWidget(), QStyleOptionViewItem(), modelIndex);
    QVERIFY(editorWidget != nullptr);

    auto *comboBox = qobject_cast<QComboBox *>(editorWidget);
    QVERIFY(comboBox != nullptr);

    itemDelegate(&propertyEditorWidget)->setEditorData(editorWidget, modelIndex);
    QCOMPARE(comboBox->currentData().toInt(), static_cast<int>(Qt::ArrowCursor));
    QVERIFY(comboBox->findData(static_cast<int>(Qt::WaitCursor)) >= 0);

    comboBox->setCurrentIndex(comboBox->findData(static_cast<int>(Qt::WaitCursor)));
    itemDelegate(&propertyEditorWidget)->setModelData(editorWidget,
                                                      propertyEditorWidget.treeWidget()->model(),
                                                      modelIndex);

    QCOMPARE(targetObject.cursor().shape(), Qt::WaitCursor);
    QCOMPARE(cursorItem->text(propertyValueColumn), QStringLiteral("WaitCursor"));

    delete editorWidget;
}

void ObjectPropertyEditorWidgetTest::fontPropertyExpandsLikeQtDesigner()
{
    ObjectPropertyEditorWidget propertyEditorWidget;
    PropertyEditorTestObject targetObject;
    propertyEditorWidget.setInspectedObject(&targetObject);

    QTreeWidgetItem *fontItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("font"));
    QVERIFY(fontItem != nullptr);
    QCOMPARE(fontItem->data(propertyValueColumn, ObjectPropertyEditorWidget::PropertyKindRole).toInt(),
             static_cast<int>(ObjectPropertyEditorWidget::PropertyValueKind::Font));
    QVERIFY(fontItem->childCount() >= 10);
    QCOMPARE(fontItem->text(propertyNameColumn), QStringLiteral("font"));
    QCOMPARE(fontItem->child(0)->text(propertyNameColumn), QStringLiteral("ファミリー"));

    QTreeWidgetItem *familyItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("font.family"));
    QTreeWidgetItem *pointSizeItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("font.pointSize"));
    QTreeWidgetItem *weightItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("font.weight"));
    QTreeWidgetItem *antialiasingItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("font.antialiasing"));
    QTreeWidgetItem *hintingItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("font.hintingPreference"));

    QVERIFY(familyItem != nullptr);
    QVERIFY(pointSizeItem != nullptr);
    QVERIFY(weightItem != nullptr);
    QVERIFY(antialiasingItem != nullptr);
    QVERIFY(hintingItem != nullptr);

    QCOMPARE(weightItem->text(propertyValueColumn), testFontWeightText(targetObject.font().weight()));
    QCOMPARE(antialiasingItem->text(propertyValueColumn), testAntialiasingText(targetObject.font()));
    QCOMPARE(hintingItem->text(propertyValueColumn),
             testHintingPreferenceText(targetObject.font().hintingPreference()));
    QVERIFY(fontItem->text(propertyValueColumn).startsWith(QStringLiteral("[")));
}

void ObjectPropertyEditorWidgetTest::fontSubPropertiesWriteBackToFontProperty()
{
    ObjectPropertyEditorWidget propertyEditorWidget;
    PropertyEditorTestObject targetObject;
    propertyEditorWidget.setInspectedObject(&targetObject);

    const QModelIndex familyIndex = propertyValueIndex(&propertyEditorWidget, QStringLiteral("font.family"));
    QVERIFY(familyIndex.isValid());
    QWidget *familyEditorWidget = itemDelegate(&propertyEditorWidget)->createEditor(
        propertyEditorWidget.treeWidget(), QStyleOptionViewItem(), familyIndex);
    QVERIFY(familyEditorWidget != nullptr);

    auto *familyComboBox = qobject_cast<QComboBox *>(familyEditorWidget);
    QVERIFY(familyComboBox != nullptr);
    itemDelegate(&propertyEditorWidget)->setEditorData(familyEditorWidget, familyIndex);

    int courierIndex = familyComboBox->findData(QStringLiteral("Courier New"));
    if (courierIndex < 0) {
        familyComboBox->addItem(QStringLiteral("Courier New"), QStringLiteral("Courier New"));
        courierIndex = familyComboBox->findData(QStringLiteral("Courier New"));
    }
    QVERIFY(courierIndex >= 0);
    familyComboBox->setCurrentIndex(courierIndex);

    itemDelegate(&propertyEditorWidget)->setModelData(familyEditorWidget,
                                                      propertyEditorWidget.treeWidget()->model(),
                                                      familyIndex);
    QCOMPARE(targetObject.font().family(), QStringLiteral("Courier New"));
    delete familyEditorWidget;

    const QModelIndex pointSizeIndex = propertyValueIndex(&propertyEditorWidget, QStringLiteral("font.pointSize"));
    QVERIFY(pointSizeIndex.isValid());
    QWidget *pointSizeEditorWidget = itemDelegate(&propertyEditorWidget)->createEditor(
        propertyEditorWidget.treeWidget(), QStyleOptionViewItem(), pointSizeIndex);
    QVERIFY(pointSizeEditorWidget != nullptr);

    auto *spinBox = qobject_cast<QSpinBox *>(pointSizeEditorWidget);
    QVERIFY(spinBox != nullptr);
    itemDelegate(&propertyEditorWidget)->setEditorData(pointSizeEditorWidget, pointSizeIndex);
    spinBox->setValue(19);
    itemDelegate(&propertyEditorWidget)->setModelData(pointSizeEditorWidget,
                                                      propertyEditorWidget.treeWidget()->model(),
                                                      pointSizeIndex);
    QCOMPARE(targetObject.font().pointSize(), 19);
    delete pointSizeEditorWidget;

    QTreeWidgetItem *boldItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("font.bold"));
    QVERIFY(boldItem != nullptr);
    boldItem->setCheckState(propertyValueColumn, Qt::Checked);
    QCoreApplication::processEvents();
    QCOMPARE(targetObject.font().bold(), true);

    QTreeWidgetItem *weightItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("font.weight"));
    QVERIFY(weightItem != nullptr);
    QCOMPARE(weightItem->text(propertyValueColumn), QStringLiteral("Bold"));
}

void ObjectPropertyEditorWidgetTest::fontFamilyEditorIsNotTextEditable()
{
    ObjectPropertyEditorWidget propertyEditorWidget;
    PropertyEditorTestObject targetObject;
    propertyEditorWidget.setInspectedObject(&targetObject);

    const QModelIndex familyIndex = propertyValueIndex(&propertyEditorWidget, QStringLiteral("font.family"));
    QVERIFY(familyIndex.isValid());

    QWidget *familyEditorWidget = itemDelegate(&propertyEditorWidget)->createEditor(
        propertyEditorWidget.treeWidget(), QStyleOptionViewItem(), familyIndex);
    QVERIFY(familyEditorWidget != nullptr);

    auto *familyComboBox = qobject_cast<QComboBox *>(familyEditorWidget);
    QVERIFY(familyComboBox != nullptr);
    QCOMPARE(familyComboBox->isEditable(), false);

    delete familyEditorWidget;
}

void ObjectPropertyEditorWidgetTest::fontFamilyEditorKeepsCurrentValueWhenFontIsNotListed()
{
    ObjectPropertyEditorWidget propertyEditorWidget;
    PropertyEditorTestObject targetObject;
    const QString missingFontFamily = QStringLiteral("__MissingFontForEditorTest__");
    targetObject.setFont(QFont(missingFontFamily, 13));
    propertyEditorWidget.setInspectedObject(&targetObject);

    const QModelIndex familyIndex = propertyValueIndex(&propertyEditorWidget, QStringLiteral("font.family"));
    QVERIFY(familyIndex.isValid());

    QWidget *familyEditorWidget = itemDelegate(&propertyEditorWidget)->createEditor(
        propertyEditorWidget.treeWidget(), QStyleOptionViewItem(), familyIndex);
    QVERIFY(familyEditorWidget != nullptr);

    auto *familyComboBox = qobject_cast<QComboBox *>(familyEditorWidget);
    QVERIFY(familyComboBox != nullptr);

    itemDelegate(&propertyEditorWidget)->setEditorData(familyEditorWidget, familyIndex);
    QCOMPARE(familyComboBox->currentData().toString(), missingFontFamily);
    QCOMPARE(familyComboBox->currentText(), missingFontFamily);

    itemDelegate(&propertyEditorWidget)->setModelData(familyEditorWidget,
                                                      propertyEditorWidget.treeWidget()->model(),
                                                      familyIndex);
    QCOMPARE(targetObject.font().family(), missingFontFamily);

    delete familyEditorWidget;
}

void ObjectPropertyEditorWidgetTest::fontEnumSubPropertiesShowComboBoxText()
{
    ObjectPropertyEditorWidget propertyEditorWidget;
    PropertyEditorTestObject targetObject;
    propertyEditorWidget.setInspectedObject(&targetObject);

    struct FontEnumTestCase {
        QString propertyName;
        int selectedValue;
        QString expectedText;
    };

    const QList<FontEnumTestCase> testCases {
        {QStringLiteral("font.weight"), QFont::DemiBold, QStringLiteral("DemiBold")},
        {QStringLiteral("font.antialiasing"), QFont::PreferAntialias, QStringLiteral("アンチエイリアス優先")},
        {QStringLiteral("font.hintingPreference"), QFont::PreferFullHinting, QStringLiteral("PreferFullHinting")}
    };

    for (const FontEnumTestCase &testCase : testCases) {
        const QModelIndex modelIndex = propertyValueIndex(&propertyEditorWidget, testCase.propertyName);
        QVERIFY(modelIndex.isValid());

        QWidget *editorWidget = itemDelegate(&propertyEditorWidget)->createEditor(
            propertyEditorWidget.treeWidget(), QStyleOptionViewItem(), modelIndex);
        QVERIFY(editorWidget != nullptr);

        auto *comboBox = qobject_cast<QComboBox *>(editorWidget);
        QVERIFY(comboBox != nullptr);

        itemDelegate(&propertyEditorWidget)->setEditorData(editorWidget, modelIndex);
        const int selectedIndex = comboBox->findData(testCase.selectedValue);
        QVERIFY(selectedIndex >= 0);
        comboBox->setCurrentIndex(selectedIndex);

        itemDelegate(&propertyEditorWidget)->setModelData(editorWidget,
                                                          propertyEditorWidget.treeWidget()->model(),
                                                          modelIndex);

        QTreeWidgetItem *treeItem = findPropertyItem(&propertyEditorWidget, testCase.propertyName);
        QVERIFY(treeItem != nullptr);
        QCOMPARE(treeItem->text(propertyValueColumn), testCase.expectedText);
        QCOMPARE(modelIndex.data(Qt::DisplayRole).toString(), testCase.expectedText);

        delete editorWidget;
    }
}

void ObjectPropertyEditorWidgetTest::colorPropertyUsesColorDialogEditor()
{
    ObjectPropertyEditorWidget propertyEditorWidget;
    PropertyEditorTestObject targetObject;
    propertyEditorWidget.setInspectedObject(&targetObject);

    QTreeWidgetItem *colorItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("colorValue"));
    QVERIFY(colorItem != nullptr);
    QCOMPARE(colorItem->data(propertyValueColumn, ObjectPropertyEditorWidget::PropertyKindRole).toInt(),
             static_cast<int>(ObjectPropertyEditorWidget::PropertyValueKind::Color));

    const QModelIndex modelIndex = propertyValueIndex(&propertyEditorWidget, QStringLiteral("colorValue"));
    QVERIFY(modelIndex.isValid());

    QWidget *editorWidget = itemDelegate(&propertyEditorWidget)->createEditor(
        propertyEditorWidget.treeWidget(), QStyleOptionViewItem(), modelIndex);
    QVERIFY(editorWidget != nullptr);
    QVERIFY(qobject_cast<QLineEdit *>(editorWidget) == nullptr);

    const QList<QPushButton *> pushButtons = editorWidget->findChildren<QPushButton *>();
    QVERIFY(!pushButtons.isEmpty());

    bool hasColorButton = false;
    for (QPushButton *pushButton : pushButtons) {
        if (!pushButton->text().isEmpty()) {
            hasColorButton = true;
            break;
        }
    }

    QCOMPARE(hasColorButton, true);

    delete editorWidget;
}

void ObjectPropertyEditorWidgetTest::colorValueFromEditorReturnsSelectedColor()
{
    ObjectPropertyEditorWidget propertyEditorWidget;
    PropertyEditorTestObject targetObject;
    propertyEditorWidget.setInspectedObject(&targetObject);

    QTreeWidgetItem *colorItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("colorValue"));
    QVERIFY(colorItem != nullptr);

    const QVariant editableValue = propertyEditorWidget.editableValueForItem(colorItem);
    QVERIFY(editableValue.metaType() == QMetaType::fromType<QColor>());
    QCOMPARE(editableValue.value<QColor>(), targetObject.colorValue());
}

void ObjectPropertyEditorWidgetTest::palettePropertyUsesPaletteDialogEditor()
{
    ObjectPropertyEditorWidget propertyEditorWidget;
    PropertyEditorTestObject targetObject;
    propertyEditorWidget.setInspectedObject(&targetObject);

    QTreeWidgetItem *paletteItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("palette"));
    QVERIFY(paletteItem != nullptr);
    QCOMPARE(paletteItem->data(propertyValueColumn, ObjectPropertyEditorWidget::PropertyKindRole).toInt(),
             static_cast<int>(ObjectPropertyEditorWidget::PropertyValueKind::Palette));

    const QModelIndex modelIndex = propertyValueIndex(&propertyEditorWidget, QStringLiteral("palette"));
    QVERIFY(modelIndex.isValid());

    QWidget *editorWidget = itemDelegate(&propertyEditorWidget)->createEditor(
        propertyEditorWidget.treeWidget(), QStyleOptionViewItem(), modelIndex);
    QVERIFY(editorWidget != nullptr);
    QVERIFY(qobject_cast<QLineEdit *>(editorWidget) == nullptr);

    const QList<QPushButton *> pushButtons = editorWidget->findChildren<QPushButton *>();
    QVERIFY(!pushButtons.isEmpty());

    bool hasChangePaletteButton = false;
    for (QPushButton *pushButton : pushButtons) {
        if (pushButton->text() == QStringLiteral("Change Palette")) {
            hasChangePaletteButton = true;
            break;
        }
    }

    QCOMPARE(hasChangePaletteButton, true);

    delete editorWidget;
}

void ObjectPropertyEditorWidgetTest::palettePropertyShowsInheritedOrChangedText()
{
    ObjectPropertyEditorWidget propertyEditorWidget;
    PropertyEditorTestObject targetObject;
    propertyEditorWidget.setInspectedObject(&targetObject);

    QTreeWidgetItem *paletteItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("palette"));
    QVERIFY(paletteItem != nullptr);
    QCOMPARE(paletteItem->text(propertyValueColumn), QStringLiteral("親から継承"));

    QPalette changedPalette = targetObject.palette();
    changedPalette.setColor(QPalette::Active, QPalette::Window, QColor(Qt::red));
    targetObject.setPalette(changedPalette);

    QTRY_COMPARE(paletteItem->text(propertyValueColumn), QStringLiteral("バレットを変更"));
}

void ObjectPropertyEditorWidgetTest::externalSliderValueChangeRefreshesTree()
{
    ObjectPropertyEditorWidget propertyEditorWidget;
    QSlider targetSlider(Qt::Horizontal);
    targetSlider.setRange(0, 100);
    targetSlider.setValue(10);
    propertyEditorWidget.setInspectedObject(&targetSlider);

    QTreeWidgetItem *valueItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("value"));
    QVERIFY(valueItem != nullptr);
    QCOMPARE(valueItem->text(propertyValueColumn), QStringLiteral("10"));

    targetSlider.setValue(37);
    QTRY_COMPARE(valueItem->text(propertyValueColumn), QStringLiteral("37"));
}

void ObjectPropertyEditorWidgetTest::lineEditParsingHandlesPointSizeRectStringListAndByteArray()
{
    ObjectPropertyEditorWidget propertyEditorWidget;
    PropertyEditorTestObject targetObject;
    propertyEditorWidget.setInspectedObject(&targetObject);

    QLineEdit pointEditor;
    pointEditor.setText(QStringLiteral("12, 34"));
    QCOMPARE(propertyEditorWidget.valueFromEditor(&pointEditor,
                                                  findPropertyItem(&propertyEditorWidget,
                                                                   QStringLiteral("pointValue"))).toPoint(),
             QPoint(12, 34));

    QLineEdit sizeEditor;
    sizeEditor.setText(QStringLiteral("80 x 90"));
    QCOMPARE(propertyEditorWidget.valueFromEditor(&sizeEditor,
                                                  findPropertyItem(&propertyEditorWidget,
                                                                   QStringLiteral("sizeValue"))).toSize(),
             QSize(80, 90));

    QLineEdit rectEditor;
    rectEditor.setText(QStringLiteral("1, 2, 3, 4"));
    QCOMPARE(propertyEditorWidget.valueFromEditor(&rectEditor,
                                                  findPropertyItem(&propertyEditorWidget,
                                                                   QStringLiteral("rectValue"))).toRect(),
             QRect(1, 2, 3, 4));

    QLineEdit stringListEditor;
    stringListEditor.setText(QStringLiteral("alpha, beta, gamma"));
    QCOMPARE(propertyEditorWidget.valueFromEditor(&stringListEditor,
                                                  findPropertyItem(&propertyEditorWidget,
                                                                   QStringLiteral("stringListValue"))).toStringList(),
             QStringList({QStringLiteral("alpha"), QStringLiteral("beta"), QStringLiteral("gamma")}));

    QLineEdit byteArrayEditor;
    byteArrayEditor.setText(QStringLiteral("de ad be ef"));
    QCOMPARE(propertyEditorWidget.valueFromEditor(&byteArrayEditor,
                                                  findPropertyItem(&propertyEditorWidget,
                                                                   QStringLiteral("byteArrayValue"))).toByteArray(),
             QByteArray::fromHex("deadbeef"));
}

void ObjectPropertyEditorWidgetTest::displayTextFormattingCoversRepresentativeTypes()
{
    ObjectPropertyEditorWidget propertyEditorWidget;

    QCOMPARE(propertyEditorWidget.displayTextForValue(QVariant(true),
                                                      ObjectPropertyEditorWidget::PropertyValueKind::Boolean),
             QStringLiteral("true"));
    QCOMPARE(propertyEditorWidget.displayTextForValue(QVariant::fromValue(QPoint(7, 8)),
                                                      ObjectPropertyEditorWidget::PropertyValueKind::Point),
             QStringLiteral("7, 8"));
    QCOMPARE(propertyEditorWidget.displayTextForValue(QVariant::fromValue(QSize(11, 22)),
                                                      ObjectPropertyEditorWidget::PropertyValueKind::Size),
             QStringLiteral("11 x 22"));
    QCOMPARE(propertyEditorWidget.displayTextForValue(QVariant::fromValue(QRect(1, 2, 3, 4)),
                                                      ObjectPropertyEditorWidget::PropertyValueKind::Rect),
             QStringLiteral("1, 2, 3, 4"));
    QCOMPARE(propertyEditorWidget.displayTextForValue(QVariant::fromValue(QColor(0x11, 0x22, 0x33, 0x44)),
                                                      ObjectPropertyEditorWidget::PropertyValueKind::Color),
             QStringLiteral("#44112233"));
    QCOMPARE(propertyEditorWidget.displayTextForValue(QVariant::fromValue(QByteArray::fromHex("0a0b")),
                                                      ObjectPropertyEditorWidget::PropertyValueKind::ByteArray),
             QStringLiteral("0a 0b"));
    QCOMPARE(propertyEditorWidget.displayTextForValue(QVariant::fromValue(QCursor(Qt::CrossCursor)),
                                                      ObjectPropertyEditorWidget::PropertyValueKind::Cursor),
             QStringLiteral("CrossCursor"));
    QCOMPARE(propertyEditorWidget.displayTextForValue(QVariant::fromValue(Qt::Vertical),
                                                      ObjectPropertyEditorWidget::PropertyValueKind::Orientation),
             QStringLiteral("Vertical"));
}


void ObjectPropertyEditorWidgetTest::readOnlyPropertyIsHiddenLikeQtDesigner()
{
    ObjectPropertyEditorWidget propertyEditorWidget;
    PropertyEditorTestObject targetObject;

    propertyEditorWidget.setInspectedObject(&targetObject);

    QTreeWidgetItem *readOnlyItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("readOnlyText"));
    QCOMPARE(readOnlyItem, nullptr);
}

void ObjectPropertyEditorWidgetTest::sizePropertyUsesDisclosureChildrenAndWritesBack()
{
    ObjectPropertyEditorWidget propertyEditorWidget;
    PropertyEditorTestObject targetObject;
    propertyEditorWidget.setInspectedObject(&targetObject);

    QTreeWidgetItem *sizeItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("sizeValue"));
    QVERIFY(sizeItem != nullptr);
    QVERIFY(!sizeItem->flags().testFlag(Qt::ItemIsEditable));
    QCOMPARE(sizeItem->childCount(), 2);

    QTreeWidgetItem *widthItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("sizeValue.width"));
    QTreeWidgetItem *heightItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("sizeValue.height"));
    QVERIFY(widthItem != nullptr);
    QVERIFY(heightItem != nullptr);

    const QModelIndex widthIndex = propertyValueIndex(&propertyEditorWidget, QStringLiteral("sizeValue.width"));
    QVERIFY(widthIndex.isValid());
    QWidget *widthEditorWidget = itemDelegate(&propertyEditorWidget)->createEditor(
        propertyEditorWidget.treeWidget(), QStyleOptionViewItem(), widthIndex);
    QVERIFY(widthEditorWidget != nullptr);

    auto *widthSpinBox = qobject_cast<QSpinBox *>(widthEditorWidget);
    QVERIFY(widthSpinBox != nullptr);
    itemDelegate(&propertyEditorWidget)->setEditorData(widthEditorWidget, widthIndex);
    widthSpinBox->setValue(91);
    itemDelegate(&propertyEditorWidget)->setModelData(widthEditorWidget,
                                                      propertyEditorWidget.treeWidget()->model(),
                                                      widthIndex);
    QCOMPARE(targetObject.sizeValue(), QSize(91, 50));
    delete widthEditorWidget;

    const QModelIndex heightIndex = propertyValueIndex(&propertyEditorWidget, QStringLiteral("sizeValue.height"));
    QVERIFY(heightIndex.isValid());
    QWidget *heightEditorWidget = itemDelegate(&propertyEditorWidget)->createEditor(
        propertyEditorWidget.treeWidget(), QStyleOptionViewItem(), heightIndex);
    QVERIFY(heightEditorWidget != nullptr);

    auto *heightSpinBox = qobject_cast<QSpinBox *>(heightEditorWidget);
    QVERIFY(heightSpinBox != nullptr);
    itemDelegate(&propertyEditorWidget)->setEditorData(heightEditorWidget, heightIndex);
    heightSpinBox->setValue(123);
    itemDelegate(&propertyEditorWidget)->setModelData(heightEditorWidget,
                                                      propertyEditorWidget.treeWidget()->model(),
                                                      heightIndex);
    QCOMPARE(targetObject.sizeValue(), QSize(91, 123));
    QCOMPARE(sizeItem->text(propertyValueColumn), QStringLiteral("91 x 123"));
    delete heightEditorWidget;
}

void ObjectPropertyEditorWidgetTest::layoutContentsMarginsUsesDisclosureChildrenAndWritesBack()
{
    QWidget targetWidget;
    auto *layout = new QHBoxLayout(&targetWidget);
    layout->setContentsMargins(1, 2, 3, 4);
    layout->addWidget(new QPushButton(QStringLiteral("Child"), &targetWidget));

    ObjectPropertyEditorWidget propertyEditorWidget;
    propertyEditorWidget.setInspectedObject(layout);

    QTreeWidgetItem *marginsItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("contentsMargins"));
    QVERIFY(marginsItem != nullptr);
    QVERIFY(!marginsItem->flags().testFlag(Qt::ItemIsEditable));
    QCOMPARE(marginsItem->childCount(), 4);
    QCOMPARE(marginsItem->text(propertyValueColumn), QStringLiteral("1, 2, 3, 4"));

    QVERIFY(findPropertyItem(&propertyEditorWidget, QStringLiteral("contentsMargins.left")) != nullptr);
    QVERIFY(findPropertyItem(&propertyEditorWidget, QStringLiteral("contentsMargins.top")) != nullptr);
    QVERIFY(findPropertyItem(&propertyEditorWidget, QStringLiteral("contentsMargins.right")) != nullptr);
    QVERIFY(findPropertyItem(&propertyEditorWidget, QStringLiteral("contentsMargins.bottom")) != nullptr);

    const QModelIndex leftIndex =
        propertyValueIndex(&propertyEditorWidget, QStringLiteral("contentsMargins.left"));
    QVERIFY(leftIndex.isValid());
    QWidget *leftEditorWidget = itemDelegate(&propertyEditorWidget)->createEditor(
        propertyEditorWidget.treeWidget(), QStyleOptionViewItem(), leftIndex);
    QVERIFY(leftEditorWidget != nullptr);
    auto *leftSpinBox = qobject_cast<QSpinBox *>(leftEditorWidget);
    QVERIFY(leftSpinBox != nullptr);
    itemDelegate(&propertyEditorWidget)->setEditorData(leftEditorWidget, leftIndex);
    leftSpinBox->setValue(11);
    itemDelegate(&propertyEditorWidget)->setModelData(leftEditorWidget,
                                                      propertyEditorWidget.treeWidget()->model(),
                                                      leftIndex);
    delete leftEditorWidget;

    const QModelIndex bottomIndex =
        propertyValueIndex(&propertyEditorWidget, QStringLiteral("contentsMargins.bottom"));
    QVERIFY(bottomIndex.isValid());
    QWidget *bottomEditorWidget = itemDelegate(&propertyEditorWidget)->createEditor(
        propertyEditorWidget.treeWidget(), QStyleOptionViewItem(), bottomIndex);
    QVERIFY(bottomEditorWidget != nullptr);
    auto *bottomSpinBox = qobject_cast<QSpinBox *>(bottomEditorWidget);
    QVERIFY(bottomSpinBox != nullptr);
    itemDelegate(&propertyEditorWidget)->setEditorData(bottomEditorWidget, bottomIndex);
    bottomSpinBox->setValue(44);
    itemDelegate(&propertyEditorWidget)->setModelData(bottomEditorWidget,
                                                      propertyEditorWidget.treeWidget()->model(),
                                                      bottomIndex);
    delete bottomEditorWidget;

    QCOMPARE(layout->contentsMargins(), QMargins(11, 2, 3, 44));
    QCOMPARE(marginsItem->text(propertyValueColumn), QStringLiteral("11, 2, 3, 44"));
}

void ObjectPropertyEditorWidgetTest::sizePolicyUsesDisclosureChildrenAndWritesBack()
{
    ObjectPropertyEditorWidget propertyEditorWidget;
    QPushButton targetButton(QStringLiteral("Button"));
    propertyEditorWidget.setInspectedObject(&targetButton);

    QTreeWidgetItem *sizePolicyItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("sizePolicy"));
    QVERIFY(sizePolicyItem != nullptr);
    QVERIFY(!sizePolicyItem->flags().testFlag(Qt::ItemIsEditable));
    QCOMPARE(sizePolicyItem->childCount(), 4);

    QVERIFY(findPropertyItem(&propertyEditorWidget, QStringLiteral("sizePolicy.horizontalPolicy")) != nullptr);
    QVERIFY(findPropertyItem(&propertyEditorWidget, QStringLiteral("sizePolicy.verticalPolicy")) != nullptr);
    QVERIFY(findPropertyItem(&propertyEditorWidget, QStringLiteral("sizePolicy.horizontalStretch")) != nullptr);
    QVERIFY(findPropertyItem(&propertyEditorWidget, QStringLiteral("sizePolicy.verticalStretch")) != nullptr);

    const QModelIndex horizontalPolicyIndex = propertyValueIndex(&propertyEditorWidget, QStringLiteral("sizePolicy.horizontalPolicy"));
    QVERIFY(horizontalPolicyIndex.isValid());
    QWidget *horizontalPolicyEditorWidget = itemDelegate(&propertyEditorWidget)->createEditor(
        propertyEditorWidget.treeWidget(), QStyleOptionViewItem(), horizontalPolicyIndex);
    QVERIFY(horizontalPolicyEditorWidget != nullptr);
    auto *horizontalPolicyComboBox = qobject_cast<QComboBox *>(horizontalPolicyEditorWidget);
    QVERIFY(horizontalPolicyComboBox != nullptr);
    itemDelegate(&propertyEditorWidget)->setEditorData(horizontalPolicyEditorWidget, horizontalPolicyIndex);
    horizontalPolicyComboBox->setCurrentIndex(horizontalPolicyComboBox->findData(static_cast<int>(QSizePolicy::Expanding)));
    itemDelegate(&propertyEditorWidget)->setModelData(horizontalPolicyEditorWidget,
                                                      propertyEditorWidget.treeWidget()->model(),
                                                      horizontalPolicyIndex);
    delete horizontalPolicyEditorWidget;

    const QModelIndex verticalPolicyIndex = propertyValueIndex(&propertyEditorWidget, QStringLiteral("sizePolicy.verticalPolicy"));
    QVERIFY(verticalPolicyIndex.isValid());
    QWidget *verticalPolicyEditorWidget = itemDelegate(&propertyEditorWidget)->createEditor(
        propertyEditorWidget.treeWidget(), QStyleOptionViewItem(), verticalPolicyIndex);
    QVERIFY(verticalPolicyEditorWidget != nullptr);
    auto *verticalPolicyComboBox = qobject_cast<QComboBox *>(verticalPolicyEditorWidget);
    QVERIFY(verticalPolicyComboBox != nullptr);
    itemDelegate(&propertyEditorWidget)->setEditorData(verticalPolicyEditorWidget, verticalPolicyIndex);
    verticalPolicyComboBox->setCurrentIndex(verticalPolicyComboBox->findData(static_cast<int>(QSizePolicy::MinimumExpanding)));
    itemDelegate(&propertyEditorWidget)->setModelData(verticalPolicyEditorWidget,
                                                      propertyEditorWidget.treeWidget()->model(),
                                                      verticalPolicyIndex);
    delete verticalPolicyEditorWidget;

    const QModelIndex horizontalStretchIndex = propertyValueIndex(&propertyEditorWidget, QStringLiteral("sizePolicy.horizontalStretch"));
    QVERIFY(horizontalStretchIndex.isValid());
    QWidget *horizontalStretchEditorWidget = itemDelegate(&propertyEditorWidget)->createEditor(
        propertyEditorWidget.treeWidget(), QStyleOptionViewItem(), horizontalStretchIndex);
    QVERIFY(horizontalStretchEditorWidget != nullptr);
    auto *horizontalStretchSpinBox = qobject_cast<QSpinBox *>(horizontalStretchEditorWidget);
    QVERIFY(horizontalStretchSpinBox != nullptr);
    itemDelegate(&propertyEditorWidget)->setEditorData(horizontalStretchEditorWidget, horizontalStretchIndex);
    horizontalStretchSpinBox->setValue(5);
    itemDelegate(&propertyEditorWidget)->setModelData(horizontalStretchEditorWidget,
                                                      propertyEditorWidget.treeWidget()->model(),
                                                      horizontalStretchIndex);
    delete horizontalStretchEditorWidget;

    const QModelIndex verticalStretchIndex = propertyValueIndex(&propertyEditorWidget, QStringLiteral("sizePolicy.verticalStretch"));
    QVERIFY(verticalStretchIndex.isValid());
    QWidget *verticalStretchEditorWidget = itemDelegate(&propertyEditorWidget)->createEditor(
        propertyEditorWidget.treeWidget(), QStyleOptionViewItem(), verticalStretchIndex);
    QVERIFY(verticalStretchEditorWidget != nullptr);
    auto *verticalStretchSpinBox = qobject_cast<QSpinBox *>(verticalStretchEditorWidget);
    QVERIFY(verticalStretchSpinBox != nullptr);
    itemDelegate(&propertyEditorWidget)->setEditorData(verticalStretchEditorWidget, verticalStretchIndex);
    verticalStretchSpinBox->setValue(7);
    itemDelegate(&propertyEditorWidget)->setModelData(verticalStretchEditorWidget,
                                                      propertyEditorWidget.treeWidget()->model(),
                                                      verticalStretchIndex);
    delete verticalStretchEditorWidget;

    QCOMPARE(targetButton.sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);
    QCOMPARE(targetButton.sizePolicy().verticalPolicy(), QSizePolicy::MinimumExpanding);
    QCOMPARE(targetButton.sizePolicy().horizontalStretch(), 5);
    QCOMPARE(targetButton.sizePolicy().verticalStretch(), 7);
}

void ObjectPropertyEditorWidgetTest::localePropertyUsesDisclosureChildrenAndWritesBack()
{
    ObjectPropertyEditorWidget propertyEditorWidget;
    QWidget targetWidget;
    propertyEditorWidget.setInspectedObject(&targetWidget);

    QTreeWidgetItem *localeItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("locale"));
    QVERIFY(localeItem != nullptr);
    QVERIFY(!localeItem->flags().testFlag(Qt::ItemIsEditable));
    QCOMPARE(localeItem->childCount(), 2);

    QVERIFY(findPropertyItem(&propertyEditorWidget, QStringLiteral("locale.language")) != nullptr);
    QVERIFY(findPropertyItem(&propertyEditorWidget, QStringLiteral("locale.territory")) != nullptr);

    const QModelIndex languageIndex = propertyValueIndex(&propertyEditorWidget, QStringLiteral("locale.language"));
    QVERIFY(languageIndex.isValid());
    QWidget *languageEditorWidget = itemDelegate(&propertyEditorWidget)->createEditor(
        propertyEditorWidget.treeWidget(), QStyleOptionViewItem(), languageIndex);
    QVERIFY(languageEditorWidget != nullptr);
    auto *languageComboBox = qobject_cast<QComboBox *>(languageEditorWidget);
    QVERIFY(languageComboBox != nullptr);
    itemDelegate(&propertyEditorWidget)->setEditorData(languageEditorWidget, languageIndex);
    languageComboBox->setCurrentIndex(languageComboBox->findData(static_cast<int>(QLocale::Japanese)));
    itemDelegate(&propertyEditorWidget)->setModelData(languageEditorWidget,
                                                      propertyEditorWidget.treeWidget()->model(),
                                                      languageIndex);
    delete languageEditorWidget;

    const QModelIndex territoryIndex = propertyValueIndex(&propertyEditorWidget, QStringLiteral("locale.territory"));
    QVERIFY(territoryIndex.isValid());
    QWidget *territoryEditorWidget = itemDelegate(&propertyEditorWidget)->createEditor(
        propertyEditorWidget.treeWidget(), QStyleOptionViewItem(), territoryIndex);
    QVERIFY(territoryEditorWidget != nullptr);
    auto *territoryComboBox = qobject_cast<QComboBox *>(territoryEditorWidget);
    QVERIFY(territoryComboBox != nullptr);
    itemDelegate(&propertyEditorWidget)->setEditorData(territoryEditorWidget, territoryIndex);
    territoryComboBox->setCurrentIndex(territoryComboBox->findData(static_cast<int>(QLocale::Japan)));
    itemDelegate(&propertyEditorWidget)->setModelData(territoryEditorWidget,
                                                      propertyEditorWidget.treeWidget()->model(),
                                                      territoryIndex);
    delete territoryEditorWidget;

    QCOMPARE(targetWidget.locale().language(), QLocale::Japanese);
    QCOMPARE(targetWidget.locale().territory(), QLocale::Japan);
}

void ObjectPropertyEditorWidgetTest::windowOpacityEditorUsesSingleStepPointOne()
{
    ObjectPropertyEditorWidget propertyEditorWidget;
    QWidget targetWidget;
    targetWidget.setWindowOpacity(0.5);
    propertyEditorWidget.setInspectedObject(&targetWidget);

    const QModelIndex windowOpacityIndex = propertyValueIndex(&propertyEditorWidget, QStringLiteral("windowOpacity"));
    QVERIFY(windowOpacityIndex.isValid());

    QWidget *editorWidget = itemDelegate(&propertyEditorWidget)->createEditor(
        propertyEditorWidget.treeWidget(), QStyleOptionViewItem(), windowOpacityIndex);
    QVERIFY(editorWidget != nullptr);

    auto *doubleSpinBox = qobject_cast<QDoubleSpinBox *>(editorWidget);
    QVERIFY(doubleSpinBox != nullptr);
    QCOMPARE(doubleSpinBox->singleStep(), 0.1);

    delete editorWidget;
}

void ObjectPropertyEditorWidgetTest::geometryUsesDisclosureChildrenAndIsNotEditableWhenManagedByLayout()
{
    {
        QWidget targetWidget;
        targetWidget.setGeometry(1, 2, 30, 40);

        ObjectPropertyEditorWidget propertyEditorWidget;
        propertyEditorWidget.setInspectedObject(&targetWidget);

        QTreeWidgetItem *geometryItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("geometry"));
        QVERIFY(geometryItem != nullptr);
        QVERIFY(!geometryItem->flags().testFlag(Qt::ItemIsEditable));
        QCOMPARE(geometryItem->childCount(), 4);

        QVERIFY(findPropertyItem(&propertyEditorWidget, QStringLiteral("geometry.x")) != nullptr);
        QVERIFY(findPropertyItem(&propertyEditorWidget, QStringLiteral("geometry.y")) != nullptr);
        QVERIFY(findPropertyItem(&propertyEditorWidget, QStringLiteral("geometry.width")) != nullptr);
        QVERIFY(findPropertyItem(&propertyEditorWidget, QStringLiteral("geometry.height")) != nullptr);

        const QModelIndex xIndex = propertyValueIndex(&propertyEditorWidget, QStringLiteral("geometry.x"));
        QVERIFY(xIndex.isValid());
        QWidget *xEditorWidget = itemDelegate(&propertyEditorWidget)->createEditor(
            propertyEditorWidget.treeWidget(), QStyleOptionViewItem(), xIndex);
        QVERIFY(xEditorWidget != nullptr);
        auto *xSpinBox = qobject_cast<QSpinBox *>(xEditorWidget);
        QVERIFY(xSpinBox != nullptr);
        itemDelegate(&propertyEditorWidget)->setEditorData(xEditorWidget, xIndex);
        xSpinBox->setValue(15);
        itemDelegate(&propertyEditorWidget)->setModelData(xEditorWidget,
                                                          propertyEditorWidget.treeWidget()->model(),
                                                          xIndex);
        delete xEditorWidget;

        const QModelIndex widthIndex = propertyValueIndex(&propertyEditorWidget, QStringLiteral("geometry.width"));
        QVERIFY(widthIndex.isValid());
        QWidget *widthEditorWidget = itemDelegate(&propertyEditorWidget)->createEditor(
            propertyEditorWidget.treeWidget(), QStyleOptionViewItem(), widthIndex);
        QVERIFY(widthEditorWidget != nullptr);
        auto *widthSpinBox = qobject_cast<QSpinBox *>(widthEditorWidget);
        QVERIFY(widthSpinBox != nullptr);
        itemDelegate(&propertyEditorWidget)->setEditorData(widthEditorWidget, widthIndex);
        widthSpinBox->setValue(80);
        itemDelegate(&propertyEditorWidget)->setModelData(widthEditorWidget,
                                                          propertyEditorWidget.treeWidget()->model(),
                                                          widthIndex);
        delete widthEditorWidget;

        QCOMPARE(targetWidget.geometry(), QRect(15, 2, 80, 40));
    }

    QWidget parentWidget;
    auto *layout = new QVBoxLayout(&parentWidget);
    auto *childButton = new QPushButton(QStringLiteral("Child"), &parentWidget);
    layout->addWidget(childButton);

    ObjectPropertyEditorWidget propertyEditorWidget;
    propertyEditorWidget.setInspectedObject(childButton);

    QTreeWidgetItem *geometryItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("geometry"));
    QVERIFY(geometryItem != nullptr);
    QVERIFY(!geometryItem->flags().testFlag(Qt::ItemIsEditable));
    QVERIFY(!geometryItem->flags().testFlag(Qt::ItemIsUserCheckable));

    QTreeWidgetItem *geometryXItem = findPropertyItem(&propertyEditorWidget, QStringLiteral("geometry.x"));
    QVERIFY(geometryXItem != nullptr);
    QVERIFY(!geometryXItem->flags().testFlag(Qt::ItemIsEditable));
    QVERIFY(!geometryXItem->flags().testFlag(Qt::ItemIsUserCheckable));
}

QTEST_MAIN(ObjectPropertyEditorWidgetTest)
#include "objectpropertyeditorwidget_test.moc"
