#include "objectpropertyeditorwidget.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>

#include <QtCore/QMetaEnum>
#include <QtCore/QMargins>
#include <QtCore/QLocale>
#include <QtCore/QMetaMethod>
#include <QtCore/QPoint>
#include <QtCore/QRect>
#include <QtCore/QSet>
#include <QtCore/QSignalBlocker>
#include <QtCore/QSize>
#include <QtCore/QTimer>
#include <QtGui/QColor>
#include <QtGui/QCursor>
#include <QtGui/QFont>
#include <QtGui/QFontDatabase>
#include <QtGui/QPalette>
#include <QtGui/QPainter>
#include <QtGui/QValidator>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QAbstractSlider>
#include <QtWidgets/QAbstractSpinBox>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSizePolicy>
#include <QtWidgets/QStyle>
#include <QtWidgets/QStyleOptionButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTreeWidgetItemIterator>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QtWidgets/QWidgetAction>

namespace {
constexpr int propertyNameColumn = 0;
constexpr int propertyValueColumn = 1;
constexpr int refreshIntervalMilliseconds = 120;

enum class PropertySubField {
    None = 0,
    FontFamily,
    FontPointSize,
    FontBold,
    FontItalic,
    FontUnderline,
    FontStrikeOut,
    FontKerning,
    FontWeight,
    FontAntialiasing,
    FontHintingPreference,
    SizeWidth,
    SizeHeight,
    RectX,
    RectY,
    RectWidth,
    RectHeight,
    MarginsLeft,
    MarginsTop,
    MarginsRight,
    MarginsBottom,
    SizePolicyHorizontalPolicy,
    SizePolicyVerticalPolicy,
    SizePolicyHorizontalStretch,
    SizePolicyVerticalStretch,
    LocaleLanguage,
    LocaleTerritory
};

QString fontSummaryText(const QFont &fontValue)
{
    const QString familyName = fontValue.family().isEmpty()
        ? QStringLiteral("<default>")
        : fontValue.family();
    return QStringLiteral("[%1, %2]")
        .arg(familyName, QString::number(fontValue.pointSize()));
}

QString marginsToText(const QMargins &marginsValue)
{
    return QStringLiteral("%1, %2, %3, %4")
        .arg(QString::number(marginsValue.left()),
             QString::number(marginsValue.top()),
             QString::number(marginsValue.right()),
             QString::number(marginsValue.bottom()));
}

QMargins marginsFromText(const QString &text, bool *isValid = nullptr)
{
    const QString normalizedText = text.trimmed();
    const QStringList rawParts = normalizedText.split(',', Qt::SkipEmptyParts);
    if (rawParts.size() != 4) {
        if (isValid != nullptr) {
            *isValid = false;
        }
        return {};
    }

    bool leftValid = false;
    bool topValid = false;
    bool rightValid = false;
    bool bottomValid = false;
    const int left = rawParts.at(0).trimmed().toInt(&leftValid);
    const int top = rawParts.at(1).trimmed().toInt(&topValid);
    const int right = rawParts.at(2).trimmed().toInt(&rightValid);
    const int bottom = rawParts.at(3).trimmed().toInt(&bottomValid);
    const bool parsedSuccessfully = leftValid && topValid && rightValid && bottomValid;
    if (isValid != nullptr) {
        *isValid = parsedSuccessfully;
    }

    return parsedSuccessfully ? QMargins(left, top, right, bottom) : QMargins();
}

class IntegerSpinBox : public QAbstractSpinBox
{
public:
    enum class IntegerKind {
        Signed,
        Unsigned
    };

    explicit IntegerSpinBox(IntegerKind integerKind, QWidget *parentWidget = nullptr)
        : QAbstractSpinBox(parentWidget)
        , integerKind(integerKind)
    {
        setFrame(false);
        setAccelerated(true);
        lineEdit()->setFrame(false);

        connect(lineEdit(), &QLineEdit::editingFinished, this, [this]() { applyEditorText(); });
        updateDisplayedText();
    }

    void setSignedRange(qlonglong minimumValue, qlonglong maximumValue)
    {
        integerKind = IntegerKind::Signed;
        signedMinimumValue = minimumValue;
        signedMaximumValue = std::max(minimumValue, maximumValue);
        signedCurrentValue = std::clamp(signedCurrentValue, signedMinimumValue, signedMaximumValue);
        updateDisplayedText();
    }

    void setUnsignedRange(qulonglong minimumValue, qulonglong maximumValue)
    {
        integerKind = IntegerKind::Unsigned;
        unsignedMinimumValue = minimumValue;
        unsignedMaximumValue = std::max(minimumValue, maximumValue);
        unsignedCurrentValue = std::clamp(unsignedCurrentValue, unsignedMinimumValue, unsignedMaximumValue);
        updateDisplayedText();
    }

    void setSignedValue(qlonglong value)
    {
        integerKind = IntegerKind::Signed;
        signedCurrentValue = std::clamp(value, signedMinimumValue, signedMaximumValue);
        updateDisplayedText();
    }

    void setUnsignedValue(qulonglong value)
    {
        integerKind = IntegerKind::Unsigned;
        unsignedCurrentValue = std::clamp(value, unsignedMinimumValue, unsignedMaximumValue);
        updateDisplayedText();
    }

    qlonglong signedValue() const
    {
        return signedCurrentValue;
    }

    qulonglong unsignedValue() const
    {
        return unsignedCurrentValue;
    }

    void commitEditorText()
    {
        applyEditorText();
    }

protected:
    StepEnabled stepEnabled() const override
    {
        if (integerKind == IntegerKind::Signed) {
            StepEnabled enabledSteps = StepNone;
            if (signedCurrentValue > signedMinimumValue) {
                enabledSteps |= StepDownEnabled;
            }
            if (signedCurrentValue < signedMaximumValue) {
                enabledSteps |= StepUpEnabled;
            }
            return enabledSteps;
        }

        StepEnabled enabledSteps = StepNone;
        if (unsignedCurrentValue > unsignedMinimumValue) {
            enabledSteps |= StepDownEnabled;
        }
        if (unsignedCurrentValue < unsignedMaximumValue) {
            enabledSteps |= StepUpEnabled;
        }
        return enabledSteps;
    }

    void stepBy(int steps) override
    {
        if (steps == 0) {
            return;
        }

        applyEditorText();

        if (integerKind == IntegerKind::Signed) {
            const int stepCount = std::abs(steps);
            for (int stepIndex = 0; stepIndex < stepCount; ++stepIndex) {
                if (steps > 0) {
                    if (signedCurrentValue >= signedMaximumValue) {
                        break;
                    }
                    ++signedCurrentValue;
                } else {
                    if (signedCurrentValue <= signedMinimumValue) {
                        break;
                    }
                    --signedCurrentValue;
                }
            }
        } else {
            const int stepCount = std::abs(steps);
            for (int stepIndex = 0; stepIndex < stepCount; ++stepIndex) {
                if (steps > 0) {
                    if (unsignedCurrentValue >= unsignedMaximumValue) {
                        break;
                    }
                    ++unsignedCurrentValue;
                } else {
                    if (unsignedCurrentValue <= unsignedMinimumValue) {
                        break;
                    }
                    --unsignedCurrentValue;
                }
            }
        }

        updateDisplayedText();
    }

    QValidator::State validate(QString &inputText, int &cursorPosition) const override
    {
        Q_UNUSED(cursorPosition)

        const QString trimmedText = inputText.trimmed();
        if (trimmedText.isEmpty()) {
            return QValidator::Intermediate;
        }

        if (integerKind == IntegerKind::Signed) {
            if (trimmedText == QStringLiteral("-") && signedMinimumValue < 0) {
                return QValidator::Intermediate;
            }

            bool isValid = false;
            const qlonglong parsedValue = trimmedText.toLongLong(&isValid);
            if (!isValid) {
                return QValidator::Invalid;
            }
            if (parsedValue < signedMinimumValue || parsedValue > signedMaximumValue) {
                return QValidator::Invalid;
            }
            return QValidator::Acceptable;
        }

        if (trimmedText.startsWith(QLatin1Char('-'))) {
            return QValidator::Invalid;
        }

        bool isValid = false;
        const qulonglong parsedValue = trimmedText.toULongLong(&isValid);
        if (!isValid) {
            return QValidator::Invalid;
        }
        if (parsedValue < unsignedMinimumValue || parsedValue > unsignedMaximumValue) {
            return QValidator::Invalid;
        }
        return QValidator::Acceptable;
    }

    void fixup(QString &inputText) const override
    {
        Q_UNUSED(inputText)
        inputText = currentText();
    }

    void focusOutEvent(QFocusEvent *focusEvent) override
    {
        applyEditorText();
        QAbstractSpinBox::focusOutEvent(focusEvent);
    }

private:
    QString currentText() const
    {
        return integerKind == IntegerKind::Signed
            ? QString::number(signedCurrentValue)
            : QString::number(unsignedCurrentValue);
    }

    void applyEditorText()
    {
        const QString trimmedText = lineEdit()->text().trimmed();
        if (trimmedText.isEmpty()) {
            updateDisplayedText();
            return;
        }

        if (integerKind == IntegerKind::Signed) {
            bool isValid = false;
            const qlonglong parsedValue = trimmedText.toLongLong(&isValid);
            if (isValid) {
                signedCurrentValue = std::clamp(parsedValue, signedMinimumValue, signedMaximumValue);
            }
        } else {
            bool isValid = false;
            const qulonglong parsedValue = trimmedText.toULongLong(&isValid);
            if (isValid) {
                unsignedCurrentValue = std::clamp(parsedValue, unsignedMinimumValue, unsignedMaximumValue);
            }
        }

        updateDisplayedText();
    }

    void updateDisplayedText()
    {
        QSignalBlocker blocker(lineEdit());
        lineEdit()->setText(currentText());
    }

    IntegerKind integerKind = IntegerKind::Signed;
    qlonglong signedMinimumValue = std::numeric_limits<qlonglong>::min();
    qlonglong signedMaximumValue = std::numeric_limits<qlonglong>::max();
    qlonglong signedCurrentValue = 0;
    qulonglong unsignedMinimumValue = 0;
    qulonglong unsignedMaximumValue = std::numeric_limits<qulonglong>::max();
    qulonglong unsignedCurrentValue = 0;
};

QString propertySubFieldKey(PropertySubField propertySubField)
{
    switch (propertySubField) {
    case PropertySubField::FontFamily:
        return QStringLiteral("family");
    case PropertySubField::FontPointSize:
        return QStringLiteral("pointSize");
    case PropertySubField::FontBold:
        return QStringLiteral("bold");
    case PropertySubField::FontItalic:
        return QStringLiteral("italic");
    case PropertySubField::FontUnderline:
        return QStringLiteral("underline");
    case PropertySubField::FontStrikeOut:
        return QStringLiteral("strikeOut");
    case PropertySubField::FontKerning:
        return QStringLiteral("kerning");
    case PropertySubField::FontWeight:
        return QStringLiteral("weight");
    case PropertySubField::FontAntialiasing:
        return QStringLiteral("antialiasing");
    case PropertySubField::FontHintingPreference:
        return QStringLiteral("hintingPreference");
    case PropertySubField::SizeWidth:
        return QStringLiteral("width");
    case PropertySubField::SizeHeight:
        return QStringLiteral("height");
    case PropertySubField::RectX:
        return QStringLiteral("x");
    case PropertySubField::RectY:
        return QStringLiteral("y");
    case PropertySubField::RectWidth:
        return QStringLiteral("width");
    case PropertySubField::RectHeight:
        return QStringLiteral("height");
    case PropertySubField::MarginsLeft:
        return QStringLiteral("left");
    case PropertySubField::MarginsTop:
        return QStringLiteral("top");
    case PropertySubField::MarginsRight:
        return QStringLiteral("right");
    case PropertySubField::MarginsBottom:
        return QStringLiteral("bottom");
    case PropertySubField::SizePolicyHorizontalPolicy:
        return QStringLiteral("horizontalPolicy");
    case PropertySubField::SizePolicyVerticalPolicy:
        return QStringLiteral("verticalPolicy");
    case PropertySubField::SizePolicyHorizontalStretch:
        return QStringLiteral("horizontalStretch");
    case PropertySubField::SizePolicyVerticalStretch:
        return QStringLiteral("verticalStretch");
    case PropertySubField::LocaleLanguage:
        return QStringLiteral("language");
    case PropertySubField::LocaleTerritory:
        return QStringLiteral("territory");
    case PropertySubField::None:
        break;
    }

    return {};
}

QString propertySubFieldLabel(PropertySubField propertySubField)
{
    switch (propertySubField) {
    case PropertySubField::FontFamily:
        return QStringLiteral("Family");
    case PropertySubField::FontPointSize:
        return QStringLiteral("Point Size");
    case PropertySubField::FontBold:
        return QStringLiteral("Bold");
    case PropertySubField::FontItalic:
        return QStringLiteral("Italic");
    case PropertySubField::FontUnderline:
        return QStringLiteral("Underline");
    case PropertySubField::FontStrikeOut:
        return QStringLiteral("Strike Out");
    case PropertySubField::FontKerning:
        return QStringLiteral("Kerning");
    case PropertySubField::FontWeight:
        return QStringLiteral("Weight");
    case PropertySubField::FontAntialiasing:
        return QStringLiteral("Antialiasing");
    case PropertySubField::FontHintingPreference:
        return QStringLiteral("HintingPreference");
    case PropertySubField::SizeWidth:
        return QStringLiteral("width");
    case PropertySubField::SizeHeight:
        return QStringLiteral("height");
    case PropertySubField::RectX:
        return QStringLiteral("X");
    case PropertySubField::RectY:
        return QStringLiteral("Y");
    case PropertySubField::RectWidth:
        return QStringLiteral("width");
    case PropertySubField::RectHeight:
        return QStringLiteral("height");
    case PropertySubField::MarginsLeft:
        return QStringLiteral("left");
    case PropertySubField::MarginsTop:
        return QStringLiteral("top");
    case PropertySubField::MarginsRight:
        return QStringLiteral("right");
    case PropertySubField::MarginsBottom:
        return QStringLiteral("bottom");
    case PropertySubField::SizePolicyHorizontalPolicy:
        return QStringLiteral("Horizontal Size Policy");
    case PropertySubField::SizePolicyVerticalPolicy:
        return QStringLiteral("Vertical Size Policy");
    case PropertySubField::SizePolicyHorizontalStretch:
        return QStringLiteral("Horizontal Stretch");
    case PropertySubField::SizePolicyVerticalStretch:
        return QStringLiteral("Vertical Stretch");
    case PropertySubField::LocaleLanguage:
        return QStringLiteral("Language");
    case PropertySubField::LocaleTerritory:
        return QStringLiteral("Territory");
    case PropertySubField::None:
        break;
    }

    return {};
}

QString fontWeightText(int fontWeight)
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

QList<int> editableFontWeights()
{
    return {
        QFont::Thin,
        QFont::ExtraLight,
        QFont::Light,
        QFont::Normal,
        QFont::Medium,
        QFont::DemiBold,
        QFont::Bold,
        QFont::ExtraBold,
        QFont::Black
    };
}

QString fontAntialiasingText(int antialiasingValue)
{
    switch (antialiasingValue) {
    case QFont::PreferDefault:
        return QStringLiteral("PreferDefault");
    case QFont::NoAntialias:
        return QStringLiteral("No Antialias");
    case QFont::PreferAntialias:
        return QStringLiteral("Prefer Antialias");
    case QFont::NoSubpixelAntialias:
        return QStringLiteral("NoSubpixelAntialias");
    }

    return QString::number(antialiasingValue);
}

QList<int> editableFontAntialiasingValues()
{
    return {
        QFont::PreferDefault,
        QFont::NoAntialias,
        QFont::PreferAntialias
    };
}

int fontAntialiasingValue(const QFont &fontValue)
{
    const int styleStrategy = static_cast<int>(fontValue.styleStrategy());
    if ((styleStrategy & QFont::NoAntialias) == QFont::NoAntialias) {
        return QFont::NoAntialias;
    }
    if ((styleStrategy & QFont::PreferAntialias) == QFont::PreferAntialias) {
        return QFont::PreferAntialias;
    }
    if ((styleStrategy & QFont::NoSubpixelAntialias) == QFont::NoSubpixelAntialias) {
        return QFont::NoSubpixelAntialias;
    }

    return QFont::PreferDefault;
}

QString hintingPreferenceText(QFont::HintingPreference hintingPreference)
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

QList<int> editableHintingPreferences()
{
    return {
        QFont::PreferDefaultHinting,
        QFont::PreferNoHinting,
        QFont::PreferVerticalHinting,
        QFont::PreferFullHinting
    };
}

QString sizePolicyPolicyText(QSizePolicy::Policy policy)
{
    switch (policy) {
    case QSizePolicy::Fixed:
        return QStringLiteral("Fixed");
    case QSizePolicy::Minimum:
        return QStringLiteral("Minimum");
    case QSizePolicy::Maximum:
        return QStringLiteral("Maximum");
    case QSizePolicy::Preferred:
        return QStringLiteral("Preferred");
    case QSizePolicy::Expanding:
        return QStringLiteral("Expanding");
    case QSizePolicy::MinimumExpanding:
        return QStringLiteral("MinimumExpanding");
    case QSizePolicy::Ignored:
        return QStringLiteral("Ignored");
    }

    return QStringLiteral("Policy(%1)").arg(static_cast<int>(policy));
}

QString sizePolicyToText(const QSizePolicy &sizePolicy)
{
    return QStringLiteral("H: %1 (%2), V: %3 (%4)")
        .arg(sizePolicyPolicyText(sizePolicy.horizontalPolicy()),
             QString::number(sizePolicy.horizontalStretch()),
             sizePolicyPolicyText(sizePolicy.verticalPolicy()),
             QString::number(sizePolicy.verticalStretch()));
}

QString localeEnumDisplayText(QLocale::Language language)
{
    const QString displayText = QLocale::languageToString(language);
    if (!displayText.isEmpty()) {
        return displayText;
    }

    return QStringLiteral("Language(%1)").arg(static_cast<int>(language));
}

QString localeEnumDisplayText(QLocale::Territory territory)
{
    const QString displayText = QLocale::territoryToString(territory);
    if (!displayText.isEmpty()) {
        return displayText;
    }

    return QStringLiteral("Territory(%1)").arg(static_cast<int>(territory));
}

QString localeToText(const QLocale &localeValue)
{
    return QStringLiteral("%1 / %2")
        .arg(localeEnumDisplayText(localeValue.language()),
             localeEnumDisplayText(localeValue.territory()));
}

QList<int> editableSizePolicyValues()
{
    return {
        QSizePolicy::Fixed,
        QSizePolicy::Minimum,
        QSizePolicy::Maximum,
        QSizePolicy::Preferred,
        QSizePolicy::Expanding,
        QSizePolicy::MinimumExpanding,
        QSizePolicy::Ignored
    };
}

void populateSizePolicyItems(QComboBox *comboBox)
{
    if (comboBox == nullptr) {
        return;
    }

    const QList<int> sizePolicyValues = editableSizePolicyValues();
    for (int sizePolicyValue : sizePolicyValues) {
        comboBox->addItem(sizePolicyPolicyText(static_cast<QSizePolicy::Policy>(sizePolicyValue)),
                          sizePolicyValue);
    }
}

void populateLocaleLanguageItems(QComboBox *comboBox)
{
    if (comboBox == nullptr) {
        return;
    }

    const QMetaEnum metaEnum = QMetaEnum::fromType<QLocale::Language>();
    QSet<int> seenValues;
    for (int enumIndex = 0; enumIndex < metaEnum.keyCount(); ++enumIndex) {
        const int enumValue = metaEnum.value(enumIndex);
        if (seenValues.contains(enumValue)) {
            continue;
        }

        const auto language = static_cast<QLocale::Language>(enumValue);
        comboBox->addItem(localeEnumDisplayText(language), enumValue);
        seenValues.insert(enumValue);
    }
}

void populateLocaleTerritoryItems(QComboBox *comboBox)
{
    if (comboBox == nullptr) {
        return;
    }

    const QMetaEnum metaEnum = QMetaEnum::fromType<QLocale::Territory>();
    QSet<int> seenValues;
    for (int enumIndex = 0; enumIndex < metaEnum.keyCount(); ++enumIndex) {
        const int enumValue = metaEnum.value(enumIndex);
        if (seenValues.contains(enumValue)) {
            continue;
        }

        const auto territory = static_cast<QLocale::Territory>(enumValue);
        comboBox->addItem(localeEnumDisplayText(territory), enumValue);
        seenValues.insert(enumValue);
    }
}

QVariant propertySubFieldValue(const QVariant &propertyValue, PropertySubField propertySubField)
{
    switch (propertySubField) {
    case PropertySubField::FontFamily:
        return propertyValue.value<QFont>().family();
    case PropertySubField::FontPointSize:
        return propertyValue.value<QFont>().pointSize();
    case PropertySubField::FontBold:
        return propertyValue.value<QFont>().bold();
    case PropertySubField::FontItalic:
        return propertyValue.value<QFont>().italic();
    case PropertySubField::FontUnderline:
        return propertyValue.value<QFont>().underline();
    case PropertySubField::FontStrikeOut:
        return propertyValue.value<QFont>().strikeOut();
    case PropertySubField::FontKerning:
        return propertyValue.value<QFont>().kerning();
    case PropertySubField::FontWeight:
        return propertyValue.value<QFont>().weight();
    case PropertySubField::FontAntialiasing:
        return fontAntialiasingValue(propertyValue.value<QFont>());
    case PropertySubField::FontHintingPreference:
        return static_cast<int>(propertyValue.value<QFont>().hintingPreference());
    case PropertySubField::SizeWidth:
        return propertyValue.toSize().width();
    case PropertySubField::SizeHeight:
        return propertyValue.toSize().height();
    case PropertySubField::RectX:
        return propertyValue.toRect().x();
    case PropertySubField::RectY:
        return propertyValue.toRect().y();
    case PropertySubField::RectWidth:
        return propertyValue.toRect().width();
    case PropertySubField::RectHeight:
        return propertyValue.toRect().height();
    case PropertySubField::MarginsLeft:
        return propertyValue.value<QMargins>().left();
    case PropertySubField::MarginsTop:
        return propertyValue.value<QMargins>().top();
    case PropertySubField::MarginsRight:
        return propertyValue.value<QMargins>().right();
    case PropertySubField::MarginsBottom:
        return propertyValue.value<QMargins>().bottom();
    case PropertySubField::SizePolicyHorizontalPolicy:
        return static_cast<int>(propertyValue.value<QSizePolicy>().horizontalPolicy());
    case PropertySubField::SizePolicyVerticalPolicy:
        return static_cast<int>(propertyValue.value<QSizePolicy>().verticalPolicy());
    case PropertySubField::SizePolicyHorizontalStretch:
        return propertyValue.value<QSizePolicy>().horizontalStretch();
    case PropertySubField::SizePolicyVerticalStretch:
        return propertyValue.value<QSizePolicy>().verticalStretch();
    case PropertySubField::LocaleLanguage:
        return static_cast<int>(propertyValue.toLocale().language());
    case PropertySubField::LocaleTerritory:
        return static_cast<int>(propertyValue.toLocale().territory());
    case PropertySubField::None:
        break;
    }

    return {};
}

ObjectPropertyEditorWidget::PropertyValueKind propertySubFieldKind(PropertySubField propertySubField)
{
    switch (propertySubField) {
    case PropertySubField::FontPointSize:
    case PropertySubField::SizeWidth:
    case PropertySubField::SizeHeight:
    case PropertySubField::RectX:
    case PropertySubField::RectY:
    case PropertySubField::RectWidth:
    case PropertySubField::RectHeight:
    case PropertySubField::MarginsLeft:
    case PropertySubField::MarginsTop:
    case PropertySubField::MarginsRight:
    case PropertySubField::MarginsBottom:
    case PropertySubField::SizePolicyHorizontalStretch:
    case PropertySubField::SizePolicyVerticalStretch:
        return ObjectPropertyEditorWidget::PropertyValueKind::Integer;
    case PropertySubField::FontBold:
    case PropertySubField::FontItalic:
    case PropertySubField::FontUnderline:
    case PropertySubField::FontStrikeOut:
    case PropertySubField::FontKerning:
        return ObjectPropertyEditorWidget::PropertyValueKind::Boolean;
    case PropertySubField::FontWeight:
    case PropertySubField::FontAntialiasing:
    case PropertySubField::FontHintingPreference:
    case PropertySubField::SizePolicyHorizontalPolicy:
    case PropertySubField::SizePolicyVerticalPolicy:
    case PropertySubField::LocaleLanguage:
    case PropertySubField::LocaleTerritory:
        return ObjectPropertyEditorWidget::PropertyValueKind::EnumValue;
    case PropertySubField::FontFamily:
        return ObjectPropertyEditorWidget::PropertyValueKind::String;
    case PropertySubField::None:
        break;
    }

    return ObjectPropertyEditorWidget::PropertyValueKind::Invalid;
}

void populatePropertySubFieldEnumData(PropertySubField propertySubField,
                                      QStringList *enumKeys,
                                      QList<int> *enumValues)
{
    if (enumKeys == nullptr || enumValues == nullptr) {
        return;
    }

    enumKeys->clear();
    enumValues->clear();

    if (propertySubField == PropertySubField::FontWeight) {
        const QList<int> weights = editableFontWeights();
        for (int weightValue : weights) {
            enumKeys->append(fontWeightText(weightValue));
            enumValues->append(weightValue);
        }
        return;
    }

    if (propertySubField == PropertySubField::FontAntialiasing) {
        const QList<int> antialiasingValues = editableFontAntialiasingValues();
        for (int antialiasingValue : antialiasingValues) {
            enumKeys->append(fontAntialiasingText(antialiasingValue));
            enumValues->append(antialiasingValue);
        }
        return;
    }

    if (propertySubField == PropertySubField::FontHintingPreference) {
        const QList<int> hintingPreferences = editableHintingPreferences();
        for (int hintingPreference : hintingPreferences) {
            enumKeys->append(hintingPreferenceText(static_cast<QFont::HintingPreference>(hintingPreference)));
            enumValues->append(hintingPreference);
        }
        return;
    }

    if (propertySubField == PropertySubField::SizePolicyHorizontalPolicy
        || propertySubField == PropertySubField::SizePolicyVerticalPolicy) {
        const QList<int> sizePolicyValues = editableSizePolicyValues();
        for (int sizePolicyValue : sizePolicyValues) {
            enumKeys->append(sizePolicyPolicyText(static_cast<QSizePolicy::Policy>(sizePolicyValue)));
            enumValues->append(sizePolicyValue);
        }
        return;
    }

    if (propertySubField == PropertySubField::LocaleLanguage) {
        const QMetaEnum metaEnum = QMetaEnum::fromType<QLocale::Language>();
        QSet<int> seenValues;
        for (int enumIndex = 0; enumIndex < metaEnum.keyCount(); ++enumIndex) {
            const int enumValue = metaEnum.value(enumIndex);
            if (seenValues.contains(enumValue)) {
                continue;
            }
            enumKeys->append(localeEnumDisplayText(static_cast<QLocale::Language>(enumValue)));
            enumValues->append(enumValue);
            seenValues.insert(enumValue);
        }
        return;
    }

    if (propertySubField == PropertySubField::LocaleTerritory) {
        const QMetaEnum metaEnum = QMetaEnum::fromType<QLocale::Territory>();
        QSet<int> seenValues;
        for (int enumIndex = 0; enumIndex < metaEnum.keyCount(); ++enumIndex) {
            const int enumValue = metaEnum.value(enumIndex);
            if (seenValues.contains(enumValue)) {
                continue;
            }
            enumKeys->append(localeEnumDisplayText(static_cast<QLocale::Territory>(enumValue)));
            enumValues->append(enumValue);
            seenValues.insert(enumValue);
        }
    }
}

bool propertyUsesChildItems(const QString &propertyName,
                            ObjectPropertyEditorWidget::PropertyValueKind propertyKind)
{
    if (propertyKind == ObjectPropertyEditorWidget::PropertyValueKind::Font) {
        return true;
    }
    if (propertyKind == ObjectPropertyEditorWidget::PropertyValueKind::Size) {
        return true;
    }
    if (propertyKind == ObjectPropertyEditorWidget::PropertyValueKind::Rect
        && propertyName == QStringLiteral("geometry")) {
        return true;
    }
    if (propertyKind == ObjectPropertyEditorWidget::PropertyValueKind::Margins
        && propertyName == QStringLiteral("contentsMargins")) {
        return true;
    }
    if (propertyKind == ObjectPropertyEditorWidget::PropertyValueKind::SizePolicyValue) {
        return true;
    }
    if (propertyKind == ObjectPropertyEditorWidget::PropertyValueKind::LocaleValue) {
        return true;
    }

    return false;
}

void addEnumItems(QComboBox *comboBox, const QStringList &enumKeys, const QList<QVariant> &enumValues)
{
    if (comboBox == nullptr) {
        return;
    }

    for (int valueIndex = 0; valueIndex < enumKeys.size(); ++valueIndex) {
        comboBox->addItem(enumKeys.at(valueIndex),
                          valueIndex < enumValues.size() ? enumValues.at(valueIndex)
                                                         : QVariant(enumKeys.at(valueIndex)));
    }
}

void connectSpinBoxCommitOnReturn(QAbstractSpinBox *spinBox,
                                  ObjectPropertyItemDelegate *delegateObject,
                                  ObjectPropertyEditorWidget *propertyEditorWidget,
                                  const QModelIndex &modelIndex)
{
    if (spinBox == nullptr || delegateObject == nullptr || propertyEditorWidget == nullptr) {
        return;
    }

    const QPersistentModelIndex persistentModelIndex(modelIndex);
    QPointer<QAbstractSpinBox> guardedSpinBox(spinBox);
    auto completed = std::make_shared<bool>(false);
    const auto commitAndClosePersistentEditor =
        [delegateObject, propertyEditorWidget, persistentModelIndex, guardedSpinBox, completed]() {
            if (*completed || guardedSpinBox == nullptr) {
                return;
            }

            *completed = true;
            emit delegateObject->commitData(guardedSpinBox);
            emit delegateObject->closeEditor(guardedSpinBox);
            QTimer::singleShot(0, propertyEditorWidget,
                               [propertyEditorWidget, persistentModelIndex]() {
                                   propertyEditorWidget->closePersistentEditorForIndex(
                                       persistentModelIndex);
                               });
        };

    QObject::connect(spinBox, &QAbstractSpinBox::editingFinished, delegateObject,
                     commitAndClosePersistentEditor);

    auto *editorLineEdit = spinBox->findChild<QLineEdit *>();
    if (editorLineEdit == nullptr) {
        return;
    }

    QObject::connect(editorLineEdit, &QLineEdit::returnPressed, delegateObject,
                     commitAndClosePersistentEditor);
}


double linearizedColorChannel(int channelValue)
{
    const double normalizedChannel = static_cast<double>(channelValue) / 255.0;
    if (normalizedChannel <= 0.04045) {
        return normalizedChannel / 12.92;
    }

    return std::pow((normalizedChannel + 0.055) / 1.055, 2.4);
}

double relativeLuminance(const QColor &color)
{
    const QColor rgbColor = color.toRgb();
    return 0.2126 * linearizedColorChannel(rgbColor.red())
           + 0.7152 * linearizedColorChannel(rgbColor.green())
           + 0.0722 * linearizedColorChannel(rgbColor.blue());
}

QColor readableTextColorForBackground(const QColor &backgroundColor)
{
    const double backgroundLuminance = relativeLuminance(backgroundColor);
    const double contrastRatioWithBlack = (backgroundLuminance + 0.05) / 0.05;
    const double contrastRatioWithWhite = 1.05 / (backgroundLuminance + 0.05);
    return contrastRatioWithBlack >= contrastRatioWithWhite ? QColor(Qt::black) : QColor(Qt::white);
}

QString cursorShapeToText(Qt::CursorShape cursorShape)
{
    switch (cursorShape) {
    case Qt::ArrowCursor:
        return QStringLiteral("ArrowCursor");
    case Qt::UpArrowCursor:
        return QStringLiteral("UpArrowCursor");
    case Qt::CrossCursor:
        return QStringLiteral("CrossCursor");
    case Qt::WaitCursor:
        return QStringLiteral("WaitCursor");
    case Qt::IBeamCursor:
        return QStringLiteral("IBeamCursor");
    case Qt::SizeVerCursor:
        return QStringLiteral("SizeVerCursor");
    case Qt::SizeHorCursor:
        return QStringLiteral("SizeHorCursor");
    case Qt::SizeBDiagCursor:
        return QStringLiteral("SizeBDiagCursor");
    case Qt::SizeFDiagCursor:
        return QStringLiteral("SizeFDiagCursor");
    case Qt::SizeAllCursor:
        return QStringLiteral("SizeAllCursor");
    case Qt::BlankCursor:
        return QStringLiteral("BlankCursor");
    case Qt::SplitVCursor:
        return QStringLiteral("SplitVCursor");
    case Qt::SplitHCursor:
        return QStringLiteral("SplitHCursor");
    case Qt::PointingHandCursor:
        return QStringLiteral("PointingHandCursor");
    case Qt::ForbiddenCursor:
        return QStringLiteral("ForbiddenCursor");
    case Qt::WhatsThisCursor:
        return QStringLiteral("WhatsThisCursor");
    case Qt::BusyCursor:
        return QStringLiteral("BusyCursor");
    case Qt::OpenHandCursor:
        return QStringLiteral("OpenHandCursor");
    case Qt::ClosedHandCursor:
        return QStringLiteral("ClosedHandCursor");
    case Qt::DragCopyCursor:
        return QStringLiteral("DragCopyCursor");
    case Qt::DragMoveCursor:
        return QStringLiteral("DragMoveCursor");
    case Qt::DragLinkCursor:
        return QStringLiteral("DragLinkCursor");
    case Qt::BitmapCursor:
        return QStringLiteral("BitmapCursor");
    case Qt::CustomCursor:
        return QStringLiteral("CustomCursor");
    }

    return QStringLiteral("CursorShape(%1)").arg(static_cast<int>(cursorShape));
}

void addCursorShapeItem(QComboBox *comboBox, Qt::CursorShape cursorShape)
{
    comboBox->addItem(cursorShapeToText(cursorShape), static_cast<int>(cursorShape));
}

void populateCursorShapeItems(QComboBox *comboBox)
{
    if (comboBox == nullptr) {
        return;
    }

    addCursorShapeItem(comboBox, Qt::ArrowCursor);
    addCursorShapeItem(comboBox, Qt::UpArrowCursor);
    addCursorShapeItem(comboBox, Qt::CrossCursor);
    addCursorShapeItem(comboBox, Qt::WaitCursor);
    addCursorShapeItem(comboBox, Qt::IBeamCursor);
    addCursorShapeItem(comboBox, Qt::SizeVerCursor);
    addCursorShapeItem(comboBox, Qt::SizeHorCursor);
    addCursorShapeItem(comboBox, Qt::SizeBDiagCursor);
    addCursorShapeItem(comboBox, Qt::SizeFDiagCursor);
    addCursorShapeItem(comboBox, Qt::SizeAllCursor);
    addCursorShapeItem(comboBox, Qt::BlankCursor);
    addCursorShapeItem(comboBox, Qt::SplitVCursor);
    addCursorShapeItem(comboBox, Qt::SplitHCursor);
    addCursorShapeItem(comboBox, Qt::PointingHandCursor);
    addCursorShapeItem(comboBox, Qt::ForbiddenCursor);
    addCursorShapeItem(comboBox, Qt::WhatsThisCursor);
    addCursorShapeItem(comboBox, Qt::BusyCursor);
    addCursorShapeItem(comboBox, Qt::OpenHandCursor);
    addCursorShapeItem(comboBox, Qt::ClosedHandCursor);
    addCursorShapeItem(comboBox, Qt::DragCopyCursor);
    addCursorShapeItem(comboBox, Qt::DragMoveCursor);
    addCursorShapeItem(comboBox, Qt::DragLinkCursor);
}

QString orientationToText(Qt::Orientation orientation)
{
    switch (orientation) {
    case Qt::Horizontal:
        return QStringLiteral("Horizontal");
    case Qt::Vertical:
        return QStringLiteral("Vertical");
    }

    return QStringLiteral("Orientation(%1)").arg(static_cast<int>(orientation));
}

void populateOrientationItems(QComboBox *comboBox)
{
    if (comboBox == nullptr) {
        return;
    }

    comboBox->addItem(QStringLiteral("Horizontal"), static_cast<int>(Qt::Horizontal));
    comboBox->addItem(QStringLiteral("Vertical"), static_cast<int>(Qt::Vertical));
}

QString pointToText(const QPoint &point)
{
    return QString::number(point.x()) + ", " + QString::number(point.y());
}

QString sizeToText(const QSize &size)
{
    return QString::number(size.width()) + " x " + QString::number(size.height());
}

QString rectToText(const QRect &rect)
{
    return QString::number(rect.x()) + ", " + QString::number(rect.y()) + ", "
        + QString::number(rect.width()) + ", " + QString::number(rect.height());
}

QString colorGroupToText(QPalette::ColorGroup colorGroup)
{
    switch (colorGroup) {
    case QPalette::Active:
        return QStringLiteral("Active");
    case QPalette::Inactive:
        return QStringLiteral("Inactive");
    case QPalette::Disabled:
        return QStringLiteral("Disabled");
    default:
        return QStringLiteral("Unknown");
    }
}

QString colorRoleToText(QPalette::ColorRole colorRole)
{
    switch (colorRole) {
    case QPalette::WindowText:
        return QStringLiteral("WindowText");
    case QPalette::Button:
        return QStringLiteral("Button");
    case QPalette::Light:
        return QStringLiteral("Light");
    case QPalette::Midlight:
        return QStringLiteral("Midlight");
    case QPalette::Dark:
        return QStringLiteral("Dark");
    case QPalette::Mid:
        return QStringLiteral("Mid");
    case QPalette::Text:
        return QStringLiteral("Text");
    case QPalette::BrightText:
        return QStringLiteral("BrightText");
    case QPalette::ButtonText:
        return QStringLiteral("ButtonText");
    case QPalette::Base:
        return QStringLiteral("Base");
    case QPalette::Window:
        return QStringLiteral("Window");
    case QPalette::Shadow:
        return QStringLiteral("Shadow");
    case QPalette::Highlight:
        return QStringLiteral("Highlight");
    case QPalette::HighlightedText:
        return QStringLiteral("HighlightedText");
    case QPalette::Link:
        return QStringLiteral("Link");
    case QPalette::LinkVisited:
        return QStringLiteral("LinkVisited");
    case QPalette::AlternateBase:
        return QStringLiteral("AlternateBase");
    case QPalette::NoRole:
        return QStringLiteral("NoRole");
    case QPalette::ToolTipBase:
        return QStringLiteral("ToolTipBase");
    case QPalette::ToolTipText:
        return QStringLiteral("ToolTipText");
    case QPalette::PlaceholderText:
        return QStringLiteral("PlaceholderText");
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    case QPalette::Accent:
        return QStringLiteral("Accent");
#endif
    default:
        return QStringLiteral("ColorRole(%1)").arg(static_cast<int>(colorRole));
    }
}

QList<QPalette::ColorGroup> editableColorGroups()
{
    return {QPalette::Active, QPalette::Inactive, QPalette::Disabled};
}

QList<QPalette::ColorRole> editableColorRoles()
{
    QList<QPalette::ColorRole> colorRoles {
        QPalette::WindowText,
        QPalette::Button,
        QPalette::Light,
        QPalette::Midlight,
        QPalette::Dark,
        QPalette::Mid,
        QPalette::Text,
        QPalette::BrightText,
        QPalette::ButtonText,
        QPalette::Base,
        QPalette::Window,
        QPalette::Shadow,
        QPalette::Highlight,
        QPalette::HighlightedText,
        QPalette::Link,
        QPalette::LinkVisited,
        QPalette::AlternateBase,
        QPalette::ToolTipBase,
        QPalette::ToolTipText,
        QPalette::PlaceholderText
    };
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    colorRoles.append(QPalette::Accent);
#endif
    return colorRoles;
}

QString paletteSummaryText(const QPalette &palette)
{
    const QColor windowColor = palette.color(QPalette::Active, QPalette::Window);
    const QColor textColor = palette.color(QPalette::Active, QPalette::WindowText);
    return QStringLiteral("Window=%1, WindowText=%2")
        .arg(windowColor.name(QColor::HexArgb),
             textColor.name(QColor::HexArgb));
}

QString paletteInheritedText()
{
    return QStringLiteral("Inherited from Parent");
}

QString paletteChangedText()
{
    return QStringLiteral("Change Palette");
}

class PaletteCellButton : public QPushButton
{
public:
    explicit PaletteCellButton(QWidget *parentWidget = nullptr)
        : QPushButton(parentWidget)
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        setMinimumHeight(fontMetrics().height() + 10);
        setFlat(true);
    }

    void setColor(const QColor &color)
    {
        currentColor = color;
        const QString colorName = color.name(QColor::HexArgb);
        displayTextColor = readableTextColorForBackground(color);
        setText(colorName);
        setStyleSheet(QString());
        setProperty("paletteColorName", colorName);
        updateGeometry();
        update();
    }

    QColor color() const
    {
        return currentColor;
    }

    QSize sizeHint() const override
    {
        const QFontMetrics fontMetricsForButton(font());
        const QRect textBounds = fontMetricsForButton.tightBoundingRect(text());
        return QSize(textBounds.width() + 20, fontMetricsForButton.height() + 10);
    }

protected:
    void paintEvent(QPaintEvent *paintEvent) override
    {
        Q_UNUSED(paintEvent);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, false);

        const QRect cellRectangle = rect().adjusted(0, 0, -1, -1);
        painter.fillRect(cellRectangle, currentColor);

        QColor borderColor = palette().color(QPalette::Mid);
        if (isDown()) {
            borderColor = palette().color(QPalette::Dark);
        }
        painter.setPen(borderColor);
        painter.drawRect(cellRectangle);

        const QRect availableTextRectangle = rect().adjusted(4, 0, -4, 0);
        const QFontMetrics fontMetricsForButton(font());
        const QRect tightTextBounds = fontMetricsForButton.tightBoundingRect(text());
        const int textBaselineY = availableTextRectangle.y()
                                  + (availableTextRectangle.height() - tightTextBounds.height()) / 2
                                  - tightTextBounds.top();
        const int textLeft = availableTextRectangle.x()
                             + (availableTextRectangle.width() - tightTextBounds.width()) / 2
                             - tightTextBounds.left();

        painter.setPen(displayTextColor);
        painter.drawText(textLeft, textBaselineY, text());
    }

private:
    QColor currentColor;
    QColor displayTextColor = Qt::black;
};

class PaletteEditorDialog : public QDialog
{
public:
    explicit PaletteEditorDialog(const QPalette &initialPalette, QWidget *parentWidget = nullptr)
        : QDialog(parentWidget)
        , editedPalette(initialPalette)
    {
        setWindowTitle(QStringLiteral("Edit Palette"));
        resize(640, 480);

        auto *mainLayout = new QVBoxLayout(this);

        auto *topRowLayout = new QHBoxLayout;
        buildFromButtonColorButton = new QPushButton(QStringLiteral("Build from button color"), this);
        topRowLayout->addWidget(buildFromButtonColorButton);
        topRowLayout->addStretch();
        mainLayout->addLayout(topRowLayout);

        auto *scrollArea = new QScrollArea(this);
        scrollArea->setWidgetResizable(false);
        scrollArea->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        paletteScrollArea = scrollArea;
        tableContainerWidget = new QWidget(scrollArea);
        colorGridLayout = new QGridLayout(tableContainerWidget);
        colorGridLayout->setContentsMargins(0, 0, 0, 0);
        colorGridLayout->setHorizontalSpacing(8);
        colorGridLayout->setVerticalSpacing(0);
        tableContainerWidget->setLayout(colorGridLayout);
        scrollArea->setWidget(tableContainerWidget);
        mainLayout->addWidget(scrollArea);

        auto *dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        connect(dialogButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(dialogButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        mainLayout->addWidget(dialogButtonBox);

        connect(buildFromButtonColorButton, &QAbstractButton::clicked, this,
                [this]() { buildPaletteFromButtonColor(); });

        rebuildColorGrid();
    }

    QPalette editedPaletteValue() const
    {
        return editedPalette;
    }

private:
    void clearColorGrid()
    {
        while (QLayoutItem *layoutItem = colorGridLayout->takeAt(0)) {
            if (QWidget *childWidget = layoutItem->widget()) {
                childWidget->deleteLater();
            }
            delete layoutItem;
        }
    }

    QList<QPalette::ColorGroup> visibleColorGroups() const
    {
        return editableColorGroups();
    }

    void rebuildColorGrid()
    {
        clearColorGrid();

        const QList<QPalette::ColorGroup> colorGroups = visibleColorGroups();
        colorGridLayout->addWidget(new QLabel(QStringLiteral("Role"), tableContainerWidget), 0, 0);
        for (int groupIndex = 0; groupIndex < colorGroups.size(); ++groupIndex) {
            auto *headerLabel = new QLabel(colorGroupToText(colorGroups.at(groupIndex)), tableContainerWidget);
            colorGridLayout->addWidget(headerLabel, 0, groupIndex + 1);
        }

        const QList<QPalette::ColorRole> colorRoles = editableColorRoles();
        for (int roleIndex = 0; roleIndex < colorRoles.size(); ++roleIndex) {
            const QPalette::ColorRole colorRole = colorRoles.at(roleIndex);
            colorGridLayout->addWidget(new QLabel(colorRoleToText(colorRole), tableContainerWidget),
                                       roleIndex + 1, 0);

            for (int groupIndex = 0; groupIndex < colorGroups.size(); ++groupIndex) {
                const QPalette::ColorGroup colorGroup = colorGroups.at(groupIndex);
                auto *colorButton = new PaletteCellButton(tableContainerWidget);
                colorButton->setColor(editedPalette.color(colorGroup, colorRole));
                connect(colorButton, &QAbstractButton::clicked, this,
                        [this, colorGroup, colorRole]() { chooseColor(colorGroup, colorRole); });
                colorGridLayout->addWidget(colorButton, roleIndex + 1, groupIndex + 1);
            }
        }

        for (int groupIndex = 0; groupIndex < colorGroups.size(); ++groupIndex) {
            colorGridLayout->setColumnStretch(groupIndex + 1, 1);
        }

        tableContainerWidget->adjustSize();
        updatePaletteDialogWidth();
    }

    void chooseColor(QPalette::ColorGroup colorGroup, QPalette::ColorRole colorRole)
    {
        const QColor currentColor = editedPalette.color(colorGroup, colorRole);
        const QColor selectedColor = QColorDialog::getColor(currentColor, this, QStringLiteral("Select Color"));
        if (!selectedColor.isValid()) {
            return;
        }

        editedPalette.setColor(colorGroup, colorRole, selectedColor);
        rebuildColorGrid();
    }

    void buildPaletteFromButtonColor()
    {
        const QColor baseButtonColor = QColorDialog::getColor(editedPalette.color(QPalette::Active, QPalette::Button),
                                                              this,
                                                              QStringLiteral("Select Button Color"));
        if (!baseButtonColor.isValid()) {
            return;
        }

        QPushButton helperButton;
        helperButton.setPalette(QApplication::palette(&helperButton));
        QPalette rebuiltPalette = helperButton.palette();
        for (QPalette::ColorGroup colorGroup : editableColorGroups()) {
            rebuiltPalette.setColor(colorGroup, QPalette::Button, baseButtonColor);
        }

        editedPalette = rebuiltPalette;
        rebuildColorGrid();
    }

    void updatePaletteDialogWidth()
    {
        if (paletteScrollArea == nullptr || tableContainerWidget == nullptr) {
            return;
        }

        const int scrollAreaFrameWidth = paletteScrollArea->frameWidth() * 2;
        const int dialogHorizontalMargins = layout()->contentsMargins().left()
                                            + layout()->contentsMargins().right();
        const int layoutSpacing = static_cast<QVBoxLayout *>(layout())->spacing() * 2;
        const int targetWidth = tableContainerWidget->sizeHint().width()
                                + scrollAreaFrameWidth
                                + dialogHorizontalMargins
                                + layoutSpacing;

        paletteScrollArea->setFixedWidth(tableContainerWidget->sizeHint().width() + scrollAreaFrameWidth);
        setMinimumWidth(targetWidth);
        resize(targetWidth, height());
    }

    QPalette editedPalette;
    QPushButton *buildFromButtonColorButton = nullptr;
    QScrollArea *paletteScrollArea = nullptr;
    QWidget *tableContainerWidget = nullptr;
    QGridLayout *colorGridLayout = nullptr;
};

class ColorEditorWidget : public QWidget
{
public:
    explicit ColorEditorWidget(QWidget *parentWidget = nullptr)
        : QWidget(parentWidget)
    {
        auto *horizontalLayout = new QHBoxLayout(this);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);

        changeColorButton = new PaletteCellButton(this);
        changeColorButton->setText(QStringLiteral("Select Color"));
        horizontalLayout->addWidget(changeColorButton);

        connect(changeColorButton, &QAbstractButton::clicked, this, [this]() { openColorDialog(); });
    }

    void setCurrentColor(const QColor &color)
    {
        currentColor = color;
        hasAcceptedColor = false;
        hasOpenedDialog = false;
        updateButtonPresentation();
    }

    QColor currentColorValue() const
    {
        return currentColor;
    }

    void openColorDialog()
    {
        if (hasOpenedDialog) {
            return;
        }

        hasOpenedDialog = true;
        const QColor selectedColor = QColorDialog::getColor(currentColor, this,
                                                            QStringLiteral("Select Color"),
                                                            QColorDialog::ShowAlphaChannel);
        if (selectedColor.isValid()) {
            currentColor = selectedColor;
            hasAcceptedColor = true;
            updateButtonPresentation();
        }

        if (finishedHandler) {
            finishedHandler(hasAcceptedColor);
        }
    }

    std::function<void(bool)> finishedHandler;

private:
    void updateButtonPresentation()
    {
        if (changeColorButton == nullptr) {
            return;
        }

        if (currentColor.isValid()) {
            changeColorButton->setColor(currentColor);
        } else {
            changeColorButton->setText(QStringLiteral("Select Color"));
        }
    }

    PaletteCellButton *changeColorButton = nullptr;
    QColor currentColor;
    bool hasAcceptedColor = false;
    bool hasOpenedDialog = false;
};

class PaletteEditorWidget : public QWidget
{
public:
    explicit PaletteEditorWidget(QWidget *parentWidget = nullptr)
        : QWidget(parentWidget)
    {
        auto *horizontalLayout = new QHBoxLayout(this);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);

        changePaletteButton = new QPushButton(QStringLiteral("Change Palette"), this);
        horizontalLayout->addWidget(changePaletteButton);

        connect(changePaletteButton, &QAbstractButton::clicked, this, [this]() { openPaletteDialog(); });
    }

    void setCurrentPalette(const QPalette &palette)
    {
        currentPalette = palette;
        hasAcceptedPalette = false;
        hasOpenedDialog = false;
        updateButtonText();
    }

    void setPaletteInherited(bool isInherited)
    {
        paletteIsInherited = isInherited;
        updateButtonText();
    }

    QPalette currentPaletteValue() const
    {
        return currentPalette;
    }

    void openPaletteDialog()
    {
        if (hasOpenedDialog) {
            return;
        }

        hasOpenedDialog = true;
        PaletteEditorDialog paletteEditorDialog(currentPalette, this);
        if (paletteEditorDialog.exec() == QDialog::Accepted) {
            currentPalette = paletteEditorDialog.editedPaletteValue();
            hasAcceptedPalette = true;
            paletteIsInherited = false;
            updateButtonText();
        }

        if (finishedHandler) {
            finishedHandler(hasAcceptedPalette);
        }
    }

    std::function<void(bool)> finishedHandler;

private:
    void updateButtonText()
    {
        if (changePaletteButton == nullptr) {
            return;
        }

        changePaletteButton->setText(paletteIsInherited ? paletteInheritedText()
                                                        : paletteChangedText());
    }

    QPushButton *changePaletteButton = nullptr;
    QPalette currentPalette;
    bool hasAcceptedPalette = false;
    bool hasOpenedDialog = false;
    bool paletteIsInherited = true;
};


bool isSingleBitFlagValue(int flagValue)
{
    return flagValue > 0 && (flagValue & (flagValue - 1)) == 0;
}

QString flagsToText(int flagValue, const QStringList &flagKeys, const QList<QVariant> &flagValues)
{
    if (flagValue == 0) {
        for (int valueIndex = 0; valueIndex < flagValues.size(); ++valueIndex) {
            if (flagValues.at(valueIndex).toInt() == 0) {
                return flagKeys.value(valueIndex);
            }
        }
        return QStringLiteral("0");
    }

    QStringList selectedKeys;
    for (int valueIndex = 0; valueIndex < flagValues.size(); ++valueIndex) {
        const int currentFlagValue = flagValues.at(valueIndex).toInt();
        if (!isSingleBitFlagValue(currentFlagValue)) {
            continue;
        }
        if ((flagValue & currentFlagValue) == currentFlagValue) {
            selectedKeys.append(flagKeys.value(valueIndex));
        }
    }

    if (!selectedKeys.isEmpty()) {
        return selectedKeys.join(QStringLiteral(" | "));
    }

    for (int valueIndex = 0; valueIndex < flagValues.size(); ++valueIndex) {
        if (flagValues.at(valueIndex).toInt() == flagValue) {
            return flagKeys.value(valueIndex);
        }
    }

    return QString::number(flagValue);
}

class FlagsEditorWidget : public QWidget
{
public:
    FlagsEditorWidget(const QStringList &flagKeys, const QList<QVariant> &flagValues, QWidget *parentWidget = nullptr)
        : QWidget(parentWidget)
        , allFlagKeys(flagKeys)
        , allFlagValues(flagValues)
    {
        auto *horizontalLayout = new QHBoxLayout(this);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);

        summaryButton = new QToolButton(this);
        summaryButton->setPopupMode(QToolButton::InstantPopup);
        summaryButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
        horizontalLayout->addWidget(summaryButton);

        popupMenu = new QMenu(summaryButton);
        summaryButton->setMenu(popupMenu);

        bool hasSingleBitFlags = false;
        for (const QVariant &flagValueVariant : flagValues) {
            if (isSingleBitFlagValue(flagValueVariant.toInt())) {
                hasSingleBitFlags = true;
                break;
            }
        }

        for (int valueIndex = 0; valueIndex < flagValues.size(); ++valueIndex) {
            const int currentFlagValue = flagValues.at(valueIndex).toInt();
            if (currentFlagValue == 0) {
                continue;
            }
            if (hasSingleBitFlags && !isSingleBitFlagValue(currentFlagValue)) {
                continue;
            }

            editableFlagValues.append(currentFlagValue);

            auto *checkBox = new QCheckBox(flagKeys.value(valueIndex), popupMenu);
            checkBoxes.append(checkBox);

            auto *checkBoxAction = new QWidgetAction(popupMenu);
            checkBoxAction->setDefaultWidget(checkBox);
            popupMenu->addAction(checkBoxAction);

            QObject::connect(checkBox, &QCheckBox::checkStateChanged, this,
                             [this](Qt::CheckState) { updateSummaryText(); });
        }

        updateSummaryText();
    }

    void setCurrentValue(int currentFlagValue)
    {
        for (int checkBoxIndex = 0; checkBoxIndex < checkBoxes.size(); ++checkBoxIndex) {
            const int candidateFlagValue = editableFlagValues.value(checkBoxIndex);
            const bool shouldBeChecked = candidateFlagValue != 0
                && (currentFlagValue & candidateFlagValue) == candidateFlagValue;
            checkBoxes.at(checkBoxIndex)->setChecked(shouldBeChecked);
        }
        updateSummaryText();
    }

    int currentValue() const
    {
        int combinedValue = 0;
        for (int checkBoxIndex = 0; checkBoxIndex < checkBoxes.size(); ++checkBoxIndex) {
            if (checkBoxes.at(checkBoxIndex)->isChecked()) {
                combinedValue |= editableFlagValues.value(checkBoxIndex);
            }
        }
        return combinedValue;
    }

    void openPopupMenu()
    {
        if (summaryButton != nullptr) {
            summaryButton->showMenu();
        }
    }

    QMenu *menu() const
    {
        return popupMenu;
    }

private:
    void updateSummaryText()
    {
        summaryButton->setText(flagsToText(currentValue(), allFlagKeys, allFlagValues));
    }

    QStringList allFlagKeys;
    QList<QVariant> allFlagValues;
    QList<int> editableFlagValues;
    QList<QCheckBox *> checkBoxes;
    QToolButton *summaryButton = nullptr;
    QMenu *popupMenu = nullptr;
};

QStringList splitCommaSeparatedValues(const QString &text)
{
    QStringList values;
    const QStringList rawValues = text.split(',', Qt::SkipEmptyParts);
    for (const QString &rawValue : rawValues) {
        values.append(rawValue.trimmed());
    }
    return values;
}

QPoint pointFromText(const QString &text, bool *isValid)
{
    const QStringList values = splitCommaSeparatedValues(text);
    if (values.size() != 2) {
        *isValid = false;
        return {};
    }

    bool isXValid = false;
    bool isYValid = false;
    const int xValue = values.at(0).toInt(&isXValid);
    const int yValue = values.at(1).toInt(&isYValid);
    *isValid = isXValid && isYValid;
    return QPoint(xValue, yValue);
}

QSize sizeFromText(const QString &text, bool *isValid)
{
    QString normalizedText = text;
    normalizedText.replace('x', ',');
    normalizedText.replace('X', ',');

    const QStringList values = splitCommaSeparatedValues(normalizedText);
    if (values.size() != 2) {
        *isValid = false;
        return {};
    }

    bool isWidthValid = false;
    bool isHeightValid = false;
    const int widthValue = values.at(0).toInt(&isWidthValid);
    const int heightValue = values.at(1).toInt(&isHeightValid);
    *isValid = isWidthValid && isHeightValid;
    return QSize(widthValue, heightValue);
}

QRect rectFromText(const QString &text, bool *isValid)
{
    const QStringList values = splitCommaSeparatedValues(text);
    if (values.size() != 4) {
        *isValid = false;
        return {};
    }

    bool isXValid = false;
    bool isYValid = false;
    bool isWidthValid = false;
    bool isHeightValid = false;

    const int xValue = values.at(0).toInt(&isXValid);
    const int yValue = values.at(1).toInt(&isYValid);
    const int widthValue = values.at(2).toInt(&isWidthValid);
    const int heightValue = values.at(3).toInt(&isHeightValid);

    *isValid = isXValid && isYValid && isWidthValid && isHeightValid;
    return QRect(xValue, yValue, widthValue, heightValue);
}
}

ObjectPropertyItemDelegate::ObjectPropertyItemDelegate(ObjectPropertyEditorWidget *propertyEditorWidget)
    : QStyledItemDelegate(propertyEditorWidget)
    , propertyEditorWidget(propertyEditorWidget)
{
}

QWidget *ObjectPropertyItemDelegate::createEditor(
    QWidget *parent, const QStyleOptionViewItem &styleOption, const QModelIndex &modelIndex) const
{
    Q_UNUSED(styleOption)

    if (!modelIndex.isValid() || modelIndex.column() != propertyValueColumn || propertyEditorWidget == nullptr) {
        return nullptr;
    }

    const bool isWritable = modelIndex.data(ObjectPropertyEditorWidget::WritableRole).toBool();
    if (!isWritable) {
        return nullptr;
    }

    auto propertyKind = static_cast<ObjectPropertyEditorWidget::PropertyValueKind>(
        modelIndex.data(ObjectPropertyEditorWidget::PropertyKindRole).toInt());
    const PropertySubField propertySubField = static_cast<PropertySubField>(
        modelIndex.data(ObjectPropertyEditorWidget::SubPropertyRole).toInt());

    if (propertySubField != PropertySubField::None) {
        propertyKind = propertySubFieldKind(propertySubField);

        if (propertySubField == PropertySubField::FontFamily) {
            auto *comboBox = new QComboBox(parent);
            comboBox->setEditable(false);
            comboBox->setInsertPolicy(QComboBox::NoInsert);

            const QStringList fontFamilies = QFontDatabase::families();
            for (const QString &fontFamily : fontFamilies) {
                comboBox->addItem(fontFamily, fontFamily);
            }

            propertyEditorWidget->registerEditorWidget(comboBox);

            auto *delegateObject = const_cast<ObjectPropertyItemDelegate *>(this);
            connect(comboBox, qOverload<int>(&QComboBox::activated), delegateObject,
                    [delegateObject, comboBox]() {
                        emit delegateObject->commitData(comboBox);
                        emit delegateObject->closeEditor(comboBox);
                    });
            QTimer::singleShot(0, comboBox, [comboBox]() { comboBox->showPopup(); });
            return comboBox;
        }

        if (propertySubField == PropertySubField::FontPointSize) {
            auto *spinBox = new QSpinBox(parent);
            spinBox->setFrame(false);
            spinBox->setMinimum(-1);
            spinBox->setMaximum(512);
            propertyEditorWidget->registerEditorWidget(spinBox);
            connectSpinBoxCommitOnReturn(spinBox, const_cast<ObjectPropertyItemDelegate *>(this),
                                         propertyEditorWidget, modelIndex);
            return spinBox;
        }

        if (propertySubField == PropertySubField::SizePolicyHorizontalStretch
            || propertySubField == PropertySubField::SizePolicyVerticalStretch) {
            auto *spinBox = new QSpinBox(parent);
            spinBox->setFrame(false);
            spinBox->setMinimum(0);
            spinBox->setMaximum(255);
            propertyEditorWidget->registerEditorWidget(spinBox);
            connectSpinBoxCommitOnReturn(spinBox, const_cast<ObjectPropertyItemDelegate *>(this),
                                         propertyEditorWidget, modelIndex);
            return spinBox;
        }

        if (propertySubField == PropertySubField::SizePolicyHorizontalPolicy
            || propertySubField == PropertySubField::SizePolicyVerticalPolicy
            || propertySubField == PropertySubField::LocaleLanguage
            || propertySubField == PropertySubField::LocaleTerritory) {
            auto *comboBox = new QComboBox(parent);
            comboBox->setEditable(false);

            if (propertySubField == PropertySubField::SizePolicyHorizontalPolicy
                || propertySubField == PropertySubField::SizePolicyVerticalPolicy) {
                populateSizePolicyItems(comboBox);
            } else if (propertySubField == PropertySubField::LocaleLanguage) {
                populateLocaleLanguageItems(comboBox);
            } else {
                populateLocaleTerritoryItems(comboBox);
            }

            propertyEditorWidget->registerEditorWidget(comboBox);
            return comboBox;
        }
    }

    switch (propertyKind) {
    case ObjectPropertyEditorWidget::PropertyValueKind::Boolean:
    case ObjectPropertyEditorWidget::PropertyValueKind::Font:
    case ObjectPropertyEditorWidget::PropertyValueKind::SizePolicyValue:
    case ObjectPropertyEditorWidget::PropertyValueKind::LocaleValue:
        return nullptr;
    case ObjectPropertyEditorWidget::PropertyValueKind::Integer: {
        auto *spinBox = new QSpinBox(parent);
        spinBox->setFrame(false);
        spinBox->setMinimum(std::numeric_limits<int>::min());
        spinBox->setMaximum(std::numeric_limits<int>::max());
        propertyEditorWidget->registerEditorWidget(spinBox);
        connectSpinBoxCommitOnReturn(spinBox, const_cast<ObjectPropertyItemDelegate *>(this),
                                     propertyEditorWidget, modelIndex);
        return spinBox;
    }
    case ObjectPropertyEditorWidget::PropertyValueKind::UnsignedInteger: {
        auto *spinBox = new IntegerSpinBox(IntegerSpinBox::IntegerKind::Unsigned, parent);
        spinBox->setFrame(false);
        spinBox->setUnsignedRange(0, std::numeric_limits<unsigned int>::max());
        propertyEditorWidget->registerEditorWidget(spinBox);
        connectSpinBoxCommitOnReturn(spinBox, const_cast<ObjectPropertyItemDelegate *>(this),
                                     propertyEditorWidget, modelIndex);
        return spinBox;
    }
    case ObjectPropertyEditorWidget::PropertyValueKind::LongLongInteger: {
        auto *spinBox = new IntegerSpinBox(IntegerSpinBox::IntegerKind::Signed, parent);
        spinBox->setFrame(false);
        spinBox->setSignedRange(std::numeric_limits<qlonglong>::min(), std::numeric_limits<qlonglong>::max());
        propertyEditorWidget->registerEditorWidget(spinBox);
        connectSpinBoxCommitOnReturn(spinBox, const_cast<ObjectPropertyItemDelegate *>(this),
                                     propertyEditorWidget, modelIndex);
        return spinBox;
    }
    case ObjectPropertyEditorWidget::PropertyValueKind::UnsignedLongLongInteger: {
        auto *spinBox = new IntegerSpinBox(IntegerSpinBox::IntegerKind::Unsigned, parent);
        spinBox->setFrame(false);
        spinBox->setUnsignedRange(0, std::numeric_limits<qulonglong>::max());
        propertyEditorWidget->registerEditorWidget(spinBox);
        connectSpinBoxCommitOnReturn(spinBox, const_cast<ObjectPropertyItemDelegate *>(this),
                                     propertyEditorWidget, modelIndex);
        return spinBox;
    }
    case ObjectPropertyEditorWidget::PropertyValueKind::DoubleValue: {
        auto *doubleSpinBox = new QDoubleSpinBox(parent);
        doubleSpinBox->setFrame(false);
        doubleSpinBox->setDecimals(6);
        doubleSpinBox->setMinimum(-1.0e12);
        doubleSpinBox->setMaximum(1.0e12);
        const QString propertyName = modelIndex.data(ObjectPropertyEditorWidget::SourcePropertyNameRole).toString();
        if (propertyName == QStringLiteral("windowOpacity")) {
            doubleSpinBox->setSingleStep(0.1);
        }
        propertyEditorWidget->registerEditorWidget(doubleSpinBox);
        connectSpinBoxCommitOnReturn(doubleSpinBox, const_cast<ObjectPropertyItemDelegate *>(this),
                                     propertyEditorWidget, modelIndex);
        return doubleSpinBox;
    }
    case ObjectPropertyEditorWidget::PropertyValueKind::EnumValue: {
        auto *comboBox = new QComboBox(parent);
        comboBox->setEditable(false);
        addEnumItems(comboBox,
                     modelIndex.data(ObjectPropertyEditorWidget::EnumKeysRole).toStringList(),
                     modelIndex.data(ObjectPropertyEditorWidget::EnumValuesRole).toList());
        propertyEditorWidget->registerEditorWidget(comboBox);
        return comboBox;
    }
    case ObjectPropertyEditorWidget::PropertyValueKind::FlagsValue: {
        auto *flagsEditorWidget = new FlagsEditorWidget(
            modelIndex.data(ObjectPropertyEditorWidget::EnumKeysRole).toStringList(),
            modelIndex.data(ObjectPropertyEditorWidget::EnumValuesRole).toList(), parent);
        propertyEditorWidget->registerEditorWidget(flagsEditorWidget);
        auto *delegateObject = const_cast<ObjectPropertyItemDelegate *>(this);
        connect(flagsEditorWidget->menu(), &QMenu::aboutToHide, delegateObject,
                [delegateObject, flagsEditorWidget]() {
                    emit delegateObject->commitData(flagsEditorWidget);
                    emit delegateObject->closeEditor(flagsEditorWidget);
                });
        QTimer::singleShot(0, flagsEditorWidget, [flagsEditorWidget]() { flagsEditorWidget->openPopupMenu(); });
        return flagsEditorWidget;
    }
    case ObjectPropertyEditorWidget::PropertyValueKind::Cursor: {
        auto *comboBox = new QComboBox(parent);
        populateCursorShapeItems(comboBox);
        propertyEditorWidget->registerEditorWidget(comboBox);
        return comboBox;
    }
    case ObjectPropertyEditorWidget::PropertyValueKind::Color: {
        auto *colorEditorWidget = new ColorEditorWidget(parent);
        propertyEditorWidget->registerEditorWidget(colorEditorWidget);
        auto *delegateObject = const_cast<ObjectPropertyItemDelegate *>(this);
        colorEditorWidget->finishedHandler = [delegateObject, colorEditorWidget](bool accepted) {
            if (accepted) {
                emit delegateObject->commitData(colorEditorWidget);
            }
            emit delegateObject->closeEditor(colorEditorWidget);
        };
        return colorEditorWidget;
    }
    case ObjectPropertyEditorWidget::PropertyValueKind::Palette: {
        auto *paletteEditorWidget = new PaletteEditorWidget(parent);
        propertyEditorWidget->registerEditorWidget(paletteEditorWidget);
        auto *delegateObject = const_cast<ObjectPropertyItemDelegate *>(this);
        paletteEditorWidget->finishedHandler = [delegateObject, paletteEditorWidget](bool accepted) {
            if (accepted) {
                emit delegateObject->commitData(paletteEditorWidget);
            }
            emit delegateObject->closeEditor(paletteEditorWidget);
        };
        return paletteEditorWidget;
    }
    case ObjectPropertyEditorWidget::PropertyValueKind::Orientation: {
        auto *comboBox = new QComboBox(parent);
        populateOrientationItems(comboBox);
        propertyEditorWidget->registerEditorWidget(comboBox);
        return comboBox;
    }
    default: {
        auto *lineEdit = new QLineEdit(parent);
        lineEdit->setFrame(false);
        propertyEditorWidget->registerEditorWidget(lineEdit);
        return lineEdit;
    }
    }
}

void ObjectPropertyItemDelegate::setEditorData(QWidget *editorWidget, const QModelIndex &modelIndex) const
{
    if (editorWidget == nullptr || propertyEditorWidget == nullptr) {
        return;
    }

    QTreeWidgetItem *treeItem = propertyEditorWidget->treeWidget()->itemFromIndex(modelIndex);
    if (treeItem == nullptr) {
        return;
    }

    const QVariant editableValue = propertyEditorWidget->editableValueForItem(treeItem);
    auto propertyKind = static_cast<ObjectPropertyEditorWidget::PropertyValueKind>(
        modelIndex.data(ObjectPropertyEditorWidget::PropertyKindRole).toInt());
    const PropertySubField propertySubField = static_cast<PropertySubField>(
        modelIndex.data(ObjectPropertyEditorWidget::SubPropertyRole).toInt());

    if (propertySubField != PropertySubField::None) {
        propertyKind = propertySubFieldKind(propertySubField);

        if (propertySubField == PropertySubField::FontFamily) {
            if (auto *comboBox = qobject_cast<QComboBox *>(editorWidget)) {
                const QString fontFamily = editableValue.toString();
                int currentIndex = comboBox->findData(fontFamily);
                if (currentIndex < 0) {
                    comboBox->insertItem(0, fontFamily, fontFamily);
                    currentIndex = 0;
                }
                comboBox->setCurrentIndex(currentIndex);
            }
            return;
        }
    }

    switch (propertyKind) {
    case ObjectPropertyEditorWidget::PropertyValueKind::Integer: {
        if (auto *spinBox = qobject_cast<QSpinBox *>(editorWidget)) {
            spinBox->setValue(editableValue.toInt());
        }
        break;
    }
    case ObjectPropertyEditorWidget::PropertyValueKind::UnsignedInteger: {
        if (auto *spinBox = dynamic_cast<IntegerSpinBox *>(editorWidget)) {
            spinBox->setUnsignedValue(editableValue.toUInt());
        }
        break;
    }
    case ObjectPropertyEditorWidget::PropertyValueKind::LongLongInteger: {
        if (auto *spinBox = dynamic_cast<IntegerSpinBox *>(editorWidget)) {
            spinBox->setSignedValue(editableValue.toLongLong());
        }
        break;
    }
    case ObjectPropertyEditorWidget::PropertyValueKind::UnsignedLongLongInteger: {
        if (auto *spinBox = dynamic_cast<IntegerSpinBox *>(editorWidget)) {
            spinBox->setUnsignedValue(editableValue.toULongLong());
        }
        break;
    }
    case ObjectPropertyEditorWidget::PropertyValueKind::DoubleValue: {
        if (auto *doubleSpinBox = qobject_cast<QDoubleSpinBox *>(editorWidget)) {
            doubleSpinBox->setValue(editableValue.toDouble());
        }
        break;
    }
    case ObjectPropertyEditorWidget::PropertyValueKind::EnumValue: {
        if (auto *comboBox = qobject_cast<QComboBox *>(editorWidget)) {
            int currentIndex = comboBox->findData(editableValue);
            if (currentIndex < 0) {
                currentIndex = comboBox->findText(editableValue.toString());
            }
            comboBox->setCurrentIndex(currentIndex >= 0 ? currentIndex : 0);
        }
        break;
    }
    case ObjectPropertyEditorWidget::PropertyValueKind::FlagsValue: {
        if (auto *flagsEditorWidget = dynamic_cast<FlagsEditorWidget *>(editorWidget)) {
            flagsEditorWidget->setCurrentValue(editableValue.toInt());
        }
        break;
    }
    case ObjectPropertyEditorWidget::PropertyValueKind::Cursor: {
        if (auto *comboBox = qobject_cast<QComboBox *>(editorWidget)) {
            const int currentIndex = comboBox->findData(editableValue.toInt());
            comboBox->setCurrentIndex(currentIndex >= 0 ? currentIndex : 0);
        }
        break;
    }
    case ObjectPropertyEditorWidget::PropertyValueKind::Color: {
        if (auto *colorEditorWidget = dynamic_cast<ColorEditorWidget *>(editorWidget)) {
            colorEditorWidget->setCurrentColor(editableValue.value<QColor>());
            QTimer::singleShot(0, colorEditorWidget, [colorEditorWidget]() {
                colorEditorWidget->openColorDialog();
            });
        }
        break;
    }
    case ObjectPropertyEditorWidget::PropertyValueKind::Palette: {
        if (auto *paletteEditorWidget = dynamic_cast<PaletteEditorWidget *>(editorWidget)) {
            const QString propertyName = treeItem->data(propertyValueColumn, ObjectPropertyEditorWidget::SourcePropertyNameRole).toString();
            paletteEditorWidget->setCurrentPalette(editableValue.value<QPalette>());
            paletteEditorWidget->setPaletteInherited(propertyEditorWidget->isPaletteInherited(propertyName));
            QTimer::singleShot(0, paletteEditorWidget, [paletteEditorWidget]() {
                paletteEditorWidget->openPaletteDialog();
            });
        }
        break;
    }
    case ObjectPropertyEditorWidget::PropertyValueKind::Orientation: {
        if (auto *comboBox = qobject_cast<QComboBox *>(editorWidget)) {
            const int currentIndex = comboBox->findData(editableValue.toInt());
            comboBox->setCurrentIndex(currentIndex >= 0 ? currentIndex : 0);
        }
        break;
    }
    default: {
        if (auto *lineEdit = qobject_cast<QLineEdit *>(editorWidget)) {
            lineEdit->setText(editableValue.toString());
        }
        break;
    }
    }
}

void ObjectPropertyItemDelegate::setModelData(
    QWidget *editorWidget, QAbstractItemModel *model, const QModelIndex &modelIndex) const
{
    if (editorWidget == nullptr || model == nullptr || propertyEditorWidget == nullptr) {
        return;
    }

    QTreeWidgetItem *treeItem = propertyEditorWidget->treeWidget()->itemFromIndex(modelIndex);
    if (treeItem == nullptr) {
        return;
    }

    const QVariant editedValue = propertyEditorWidget->valueFromEditor(editorWidget, treeItem);
    propertyEditorWidget->applyEditedValue(treeItem, editedValue);
    model->setData(modelIndex, treeItem->data(propertyValueColumn, Qt::EditRole), Qt::EditRole);
}

ObjectPropertyEditorWidget::ObjectPropertyEditorWidget(QWidget *parentWidget)
    : QWidget(parentWidget)
{
    auto *verticalLayout = new QVBoxLayout(this);
    verticalLayout->setContentsMargins(0, 0, 0, 0);

    propertyTreeWidget = new QTreeWidget(this);
    propertyTreeWidget->setColumnCount(2);
    propertyTreeWidget->setHeaderLabels({QStringLiteral("Property"), QStringLiteral("Value")});
    propertyTreeWidget->setAlternatingRowColors(true);
    propertyTreeWidget->setRootIsDecorated(true);
    propertyTreeWidget->setEditTriggers(QAbstractItemView::DoubleClicked
                                        | QAbstractItemView::EditKeyPressed
                                        | QAbstractItemView::SelectedClicked);
    propertyTreeWidget->header()->setStretchLastSection(false);
    propertyTreeWidget->header()->setSectionResizeMode(propertyNameColumn, QHeaderView::Interactive);
    propertyTreeWidget->header()->setSectionResizeMode(propertyValueColumn, QHeaderView::Interactive);

    itemDelegate = new ObjectPropertyItemDelegate(this);
    propertyTreeWidget->setItemDelegate(itemDelegate);

    connect(propertyTreeWidget, &QTreeWidget::itemChanged, this,
            [this](QTreeWidgetItem *treeItem, int columnIndex) { handleItemChanged(treeItem, columnIndex); });
    connect(propertyTreeWidget, &QTreeWidget::itemClicked, this,
            [this](QTreeWidgetItem *treeItem, int columnIndex) {
                if (treeItem == nullptr || columnIndex != propertyValueColumn) {
                    return;
                }

                const bool isWritable = treeItem->data(propertyValueColumn, WritableRole).toBool();
                const auto propertyKind = static_cast<PropertyValueKind>(
                    treeItem->data(propertyValueColumn, PropertyKindRole).toInt());
                const bool isEditableCell = (treeItem->flags() & Qt::ItemIsEditable) != 0;
                if (!isWritable || !isEditableCell || propertyKind == PropertyValueKind::Boolean) {
                    return;
                }

                if (persistentEditorItem != nullptr && persistentEditorItem != treeItem) {
                    propertyTreeWidget->closePersistentEditor(persistentEditorItem, propertyValueColumn);
                }
                persistentEditorItem = treeItem;
                propertyTreeWidget->setCurrentItem(treeItem, propertyValueColumn);
                propertyTreeWidget->openPersistentEditor(treeItem, propertyValueColumn);
            });

    refreshTimer = new QTimer(this);
    refreshTimer->setInterval(refreshIntervalMilliseconds);
    connect(refreshTimer, &QTimer::timeout, this, &ObjectPropertyEditorWidget::refreshAllProperties);
    refreshTimer->start();

    verticalLayout->addWidget(propertyTreeWidget);
}

void ObjectPropertyEditorWidget::showEvent(QShowEvent *showEvent)
{
    QWidget::showEvent(showEvent);

    if (hasAppliedInitialColumnWidths || propertyTreeWidget == nullptr) {
        return;
    }

    const int availableWidth = propertyTreeWidget->viewport()->width();
    if (availableWidth <= 0) {
        return;
    }

    const int propertyColumnWidth = availableWidth / 2;
    propertyTreeWidget->header()->resizeSection(propertyNameColumn, propertyColumnWidth);
    propertyTreeWidget->header()->resizeSection(propertyValueColumn,
                                                std::max(0, availableWidth - propertyColumnWidth));
    hasAppliedInitialColumnWidths = true;
}

void ObjectPropertyEditorWidget::setInspectedObject(QObject *targetObject)
{
    if (inspectedTargetObject == targetObject) {
        return;
    }

    clearObservedConnections();
    inspectedTargetObject = targetObject;

    if (inspectedTargetObject != nullptr) {
        destroyedConnection = connect(inspectedTargetObject, &QObject::destroyed, this, [this]() {
            clearObservedConnections();
            inspectedTargetObject = nullptr;
            rebuildPropertyTree();
        });
    }

    rebuildPropertyTree();
    connectNotifySignals();
    connectCommonWidgetSignals();
    attachPeriodicRefresh();
    refreshAllProperties();
}

QObject *ObjectPropertyEditorWidget::inspectedObject() const
{
    return inspectedTargetObject.data();
}

ObjectPropertyEditorWidget::PropertyValueKind
ObjectPropertyEditorWidget::propertyKindFromVariant(const QVariant &propertyValue) const
{
    if (!propertyValue.isValid()) {
        return PropertyValueKind::Invalid;
    }

    const QMetaType metaType = propertyValue.metaType();

    switch (metaType.id()) {
    case QMetaType::Bool:
        return PropertyValueKind::Boolean;
    case QMetaType::Char:
    case QMetaType::SChar:
    case QMetaType::Short:
    case QMetaType::Int:
        return PropertyValueKind::Integer;
    case QMetaType::UChar:
    case QMetaType::UShort:
    case QMetaType::UInt:
        return PropertyValueKind::UnsignedInteger;
    case QMetaType::Long:
        return sizeof(long) > sizeof(int)
            ? PropertyValueKind::LongLongInteger
            : PropertyValueKind::Integer;
    case QMetaType::ULong:
        return sizeof(unsigned long) > sizeof(unsigned int)
            ? PropertyValueKind::UnsignedLongLongInteger
            : PropertyValueKind::UnsignedInteger;
    case QMetaType::LongLong:
        return PropertyValueKind::LongLongInteger;
    case QMetaType::ULongLong:
        return PropertyValueKind::UnsignedLongLongInteger;
    default:
        break;
    }

    if (metaType == QMetaType::fromType<double>() || metaType == QMetaType::fromType<float>()) {
        return PropertyValueKind::DoubleValue;
    }
    if (metaType == QMetaType::fromType<QString>()) {
        return PropertyValueKind::String;
    }
    if (metaType == QMetaType::fromType<QStringList>()) {
        return PropertyValueKind::StringList;
    }
    if (metaType == QMetaType::fromType<QPoint>()) {
        return PropertyValueKind::Point;
    }
    if (metaType == QMetaType::fromType<QSize>()) {
        return PropertyValueKind::Size;
    }
    if (metaType == QMetaType::fromType<QRect>()) {
        return PropertyValueKind::Rect;
    }
    if (metaType == QMetaType::fromType<QMargins>()) {
        return PropertyValueKind::Margins;
    }
    if (metaType == QMetaType::fromType<QColor>()) {
        return PropertyValueKind::Color;
    }
    if (metaType == QMetaType::fromType<QPalette>()) {
        return PropertyValueKind::Palette;
    }
    if (metaType == QMetaType::fromType<QFont>()) {
        return PropertyValueKind::Font;
    }
    if (metaType == QMetaType::fromType<QByteArray>()) {
        return PropertyValueKind::ByteArray;
    }
    if (metaType == QMetaType::fromType<QCursor>()) {
        return PropertyValueKind::Cursor;
    }
    if (metaType == QMetaType::fromType<Qt::Orientation>()) {
        return PropertyValueKind::Orientation;
    }
    if (metaType == QMetaType::fromType<QSizePolicy>()) {
        return PropertyValueKind::SizePolicyValue;
    }
    if (metaType == QMetaType::fromType<QLocale>()) {
        return PropertyValueKind::LocaleValue;
    }
    if (propertyValue.canConvert<QString>()) {
        return PropertyValueKind::FallbackString;
    }

    return PropertyValueKind::Invalid;
}

QString ObjectPropertyEditorWidget::displayTextForValue(
    const QVariant &propertyValue, PropertyValueKind propertyKind) const
{
    if (!propertyValue.isValid()) {
        return QStringLiteral("<invalid>");
    }

    switch (propertyKind) {
    case PropertyValueKind::Boolean:
        return propertyValue.toBool() ? QStringLiteral("true") : QStringLiteral("false");
    case PropertyValueKind::String:
        return propertyValue.toString();
    case PropertyValueKind::StringList:
        return propertyValue.toStringList().join(QStringLiteral(", "));
    case PropertyValueKind::Point:
        return pointToText(propertyValue.toPoint());
    case PropertyValueKind::Size:
        return sizeToText(propertyValue.toSize());
    case PropertyValueKind::Rect:
        return rectToText(propertyValue.toRect());
    case PropertyValueKind::Margins:
        return marginsToText(propertyValue.value<QMargins>());
    case PropertyValueKind::Color:
        return propertyValue.value<QColor>().name(QColor::HexArgb);
    case PropertyValueKind::Palette:
        return paletteSummaryText(propertyValue.value<QPalette>());
    case PropertyValueKind::Font:
        return fontSummaryText(propertyValue.value<QFont>());
    case PropertyValueKind::ByteArray:
        return QString::fromUtf8(propertyValue.toByteArray().toHex(' '));
    case PropertyValueKind::Cursor:
        return cursorShapeToText(propertyValue.value<QCursor>().shape());
    case PropertyValueKind::Orientation:
        return orientationToText(propertyValue.value<Qt::Orientation>());
    case PropertyValueKind::SizePolicyValue:
        return sizePolicyToText(propertyValue.value<QSizePolicy>());
    case PropertyValueKind::LocaleValue:
        return localeToText(propertyValue.toLocale());
    default:
        return propertyValue.toString();
    }
}

QVariant ObjectPropertyEditorWidget::editableValueForItem(const QTreeWidgetItem *treeItem) const
{
    if (treeItem == nullptr) {
        return {};
    }

    const QString sourcePropertyName = sourcePropertyNameForItem(treeItem);
    const QVariant propertyValue = readPropertyValue(sourcePropertyName);
    const auto propertyKind = static_cast<PropertyValueKind>(
        treeItem->data(propertyValueColumn, PropertyKindRole).toInt());
    const PropertySubField propertySubField = static_cast<PropertySubField>(
        treeItem->data(propertyValueColumn, SubPropertyRole).toInt());

    if (propertySubField != PropertySubField::None) {
        return propertySubFieldValue(propertyValue, propertySubField);
    }

    if (propertyKind == PropertyValueKind::EnumValue) {
        return propertyValue.toInt();
    }

    if (propertyKind == PropertyValueKind::FlagsValue) {
        return propertyValue.toInt();
    }

    switch (propertyKind) {
    case PropertyValueKind::Integer:
    case PropertyValueKind::UnsignedInteger:
    case PropertyValueKind::LongLongInteger:
    case PropertyValueKind::UnsignedLongLongInteger:
    case PropertyValueKind::DoubleValue:
    case PropertyValueKind::Color:
    case PropertyValueKind::Palette:
    case PropertyValueKind::Font:
    case PropertyValueKind::SizePolicyValue:
    case PropertyValueKind::LocaleValue:
        return propertyValue;
    case PropertyValueKind::Cursor:
        return static_cast<int>(propertyValue.value<QCursor>().shape());
    case PropertyValueKind::Orientation:
        return static_cast<int>(propertyValue.value<Qt::Orientation>());
    default:
        return displayTextForValue(propertyValue, propertyKind);
    }
}

QVariant ObjectPropertyEditorWidget::valueFromEditor(QWidget *editorWidget, const QTreeWidgetItem *treeItem) const
{
    if (editorWidget == nullptr || treeItem == nullptr) {
        return {};
    }

    auto propertyKind = static_cast<PropertyValueKind>(
        treeItem->data(propertyValueColumn, PropertyKindRole).toInt());
    const PropertySubField propertySubField = static_cast<PropertySubField>(
        treeItem->data(propertyValueColumn, SubPropertyRole).toInt());

    if (propertySubField == PropertySubField::FontFamily) {
        if (auto *comboBox = qobject_cast<QComboBox *>(editorWidget)) {
            return comboBox->currentData().toString();
        }
    }
    if (propertySubField != PropertySubField::None) {
        propertyKind = propertySubFieldKind(propertySubField);
    }

    switch (propertyKind) {
    case PropertyValueKind::Integer: {
        if (auto *spinBox = qobject_cast<QSpinBox *>(editorWidget)) {
            return spinBox->value();
        }
        break;
    }
    case PropertyValueKind::UnsignedInteger: {
        if (auto *spinBox = dynamic_cast<IntegerSpinBox *>(editorWidget)) {
            spinBox->commitEditorText();
            return static_cast<unsigned int>(spinBox->unsignedValue());
        }
        break;
    }
    case PropertyValueKind::LongLongInteger: {
        if (auto *spinBox = dynamic_cast<IntegerSpinBox *>(editorWidget)) {
            spinBox->commitEditorText();
            return spinBox->signedValue();
        }
        break;
    }
    case PropertyValueKind::UnsignedLongLongInteger: {
        if (auto *spinBox = dynamic_cast<IntegerSpinBox *>(editorWidget)) {
            spinBox->commitEditorText();
            return spinBox->unsignedValue();
        }
        break;
    }
    case PropertyValueKind::DoubleValue: {
        if (auto *doubleSpinBox = qobject_cast<QDoubleSpinBox *>(editorWidget)) {
            return doubleSpinBox->value();
        }
        break;
    }
    case PropertyValueKind::EnumValue: {
        if (auto *comboBox = qobject_cast<QComboBox *>(editorWidget)) {
            return comboBox->currentData();
        }
        break;
    }
    case PropertyValueKind::FlagsValue: {
        if (auto *flagsEditorWidget = dynamic_cast<FlagsEditorWidget *>(editorWidget)) {
            return flagsEditorWidget->currentValue();
        }
        break;
    }
    case PropertyValueKind::Cursor: {
        if (auto *comboBox = qobject_cast<QComboBox *>(editorWidget)) {
            const Qt::CursorShape cursorShape =
                static_cast<Qt::CursorShape>(comboBox->currentData().toInt());
            return QVariant::fromValue(QCursor(cursorShape));
        }
        break;
    }
    case PropertyValueKind::Color: {
        if (auto *colorEditorWidget = dynamic_cast<ColorEditorWidget *>(editorWidget)) {
            return QVariant::fromValue(colorEditorWidget->currentColorValue());
        }
        break;
    }
    case PropertyValueKind::Palette: {
        if (auto *paletteEditorWidget = dynamic_cast<PaletteEditorWidget *>(editorWidget)) {
            return QVariant::fromValue(paletteEditorWidget->currentPaletteValue());
        }
        break;
    }
    case PropertyValueKind::Font:
    case PropertyValueKind::SizePolicyValue:
    case PropertyValueKind::LocaleValue:
        break;
    case PropertyValueKind::Orientation: {
        if (auto *comboBox = qobject_cast<QComboBox *>(editorWidget)) {
            return QVariant::fromValue(static_cast<Qt::Orientation>(comboBox->currentData().toInt()));
        }
        break;
    }
    default: {
        if (auto *lineEdit = qobject_cast<QLineEdit *>(editorWidget)) {
            const QString editorText = lineEdit->text();
            bool isValid = false;

            switch (propertyKind) {
            case PropertyValueKind::UnsignedInteger:
                return editorText.toUInt(&isValid);
            case PropertyValueKind::LongLongInteger:
                return editorText.toLongLong(&isValid);
            case PropertyValueKind::UnsignedLongLongInteger:
                return editorText.toULongLong(&isValid);
            case PropertyValueKind::Point:
                return pointFromText(editorText, &isValid);
            case PropertyValueKind::Size:
                return sizeFromText(editorText, &isValid);
            case PropertyValueKind::Rect:
                return rectFromText(editorText, &isValid);
            case PropertyValueKind::Margins:
                return QVariant::fromValue(marginsFromText(editorText, &isValid));
            case PropertyValueKind::ByteArray: {
                QString normalizedText = editorText;
                normalizedText.remove(' ');
                return QByteArray::fromHex(normalizedText.toUtf8());
            }
            case PropertyValueKind::StringList:
                return splitCommaSeparatedValues(editorText);
            default:
                isValid = true;
                return editorText;
            }
        }
        break;
    }
    }

    return {};
}

void ObjectPropertyEditorWidget::applyEditedValue(QTreeWidgetItem *treeItem, const QVariant &editedValue)
{
    if (treeItem == nullptr || inspectedTargetObject == nullptr) {
        return;
    }

    const QString propertyName = sourcePropertyNameForItem(treeItem);
    if (propertyName.isEmpty()) {
        return;
    }

    const auto propertyKind = static_cast<PropertyValueKind>(
        treeItem->data(propertyValueColumn, PropertyKindRole).toInt());
    const PropertySubField propertySubField = static_cast<PropertySubField>(
        treeItem->data(propertyValueColumn, SubPropertyRole).toInt());

    bool writeSucceeded = false;
    if (propertySubField != PropertySubField::None) {
        QVariant currentPropertyValue = readPropertyValue(propertyName);

        if (currentPropertyValue.metaType() == QMetaType::fromType<QFont>()) {
            QFont fontValue = currentPropertyValue.value<QFont>();
            switch (propertySubField) {
            case PropertySubField::FontFamily:
                fontValue.setFamily(editedValue.toString());
                break;
            case PropertySubField::FontPointSize:
                fontValue.setPointSize(editedValue.toInt());
                break;
            case PropertySubField::FontBold:
                fontValue.setBold(editedValue.toBool());
                break;
            case PropertySubField::FontItalic:
                fontValue.setItalic(editedValue.toBool());
                break;
            case PropertySubField::FontUnderline:
                fontValue.setUnderline(editedValue.toBool());
                break;
            case PropertySubField::FontStrikeOut:
                fontValue.setStrikeOut(editedValue.toBool());
                break;
            case PropertySubField::FontKerning:
                fontValue.setKerning(editedValue.toBool());
                break;
            case PropertySubField::FontWeight:
                fontValue.setWeight(static_cast<QFont::Weight>(editedValue.toInt()));
                break;
            case PropertySubField::FontAntialiasing: {
                int styleStrategy = static_cast<int>(fontValue.styleStrategy());
                styleStrategy &= ~(QFont::PreferDefault | QFont::PreferAntialias
                                   | QFont::NoAntialias | QFont::NoSubpixelAntialias);
                styleStrategy |= editedValue.toInt();
                fontValue.setStyleStrategy(static_cast<QFont::StyleStrategy>(styleStrategy));
                break;
            }
            case PropertySubField::FontHintingPreference:
                fontValue.setHintingPreference(
                    static_cast<QFont::HintingPreference>(editedValue.toInt()));
                break;
            default:
                break;
            }

            writeSucceeded = writePropertyValue(propertyName, QVariant::fromValue(fontValue));
        } else if (currentPropertyValue.metaType() == QMetaType::fromType<QSize>()) {
            QSize sizeValue = currentPropertyValue.toSize();
            if (propertySubField == PropertySubField::SizeWidth) {
                sizeValue.setWidth(editedValue.toInt());
            } else if (propertySubField == PropertySubField::SizeHeight) {
                sizeValue.setHeight(editedValue.toInt());
            }
            writeSucceeded = writePropertyValue(propertyName, QVariant::fromValue(sizeValue));
        } else if (currentPropertyValue.metaType() == QMetaType::fromType<QRect>()) {
            QRect rectValue = currentPropertyValue.toRect();
            if (propertySubField == PropertySubField::RectX) {
                rectValue.moveLeft(editedValue.toInt());
            } else if (propertySubField == PropertySubField::RectY) {
                rectValue.moveTop(editedValue.toInt());
            } else if (propertySubField == PropertySubField::RectWidth) {
                rectValue.setWidth(editedValue.toInt());
            } else if (propertySubField == PropertySubField::RectHeight) {
                rectValue.setHeight(editedValue.toInt());
            }
            writeSucceeded = writePropertyValue(propertyName, QVariant::fromValue(rectValue));
        } else if (currentPropertyValue.metaType() == QMetaType::fromType<QMargins>()) {
            QMargins marginsValue = currentPropertyValue.value<QMargins>();
            if (propertySubField == PropertySubField::MarginsLeft) {
                marginsValue.setLeft(editedValue.toInt());
            } else if (propertySubField == PropertySubField::MarginsTop) {
                marginsValue.setTop(editedValue.toInt());
            } else if (propertySubField == PropertySubField::MarginsRight) {
                marginsValue.setRight(editedValue.toInt());
            } else if (propertySubField == PropertySubField::MarginsBottom) {
                marginsValue.setBottom(editedValue.toInt());
            }
            writeSucceeded = writePropertyValue(propertyName, QVariant::fromValue(marginsValue));
        } else if (currentPropertyValue.metaType() == QMetaType::fromType<QSizePolicy>()) {
            QSizePolicy sizePolicyValue = currentPropertyValue.value<QSizePolicy>();
            if (propertySubField == PropertySubField::SizePolicyHorizontalPolicy) {
                sizePolicyValue.setHorizontalPolicy(
                    static_cast<QSizePolicy::Policy>(editedValue.toInt()));
            } else if (propertySubField == PropertySubField::SizePolicyVerticalPolicy) {
                sizePolicyValue.setVerticalPolicy(
                    static_cast<QSizePolicy::Policy>(editedValue.toInt()));
            } else if (propertySubField == PropertySubField::SizePolicyHorizontalStretch) {
                sizePolicyValue.setHorizontalStretch(static_cast<int>(std::clamp(editedValue.toInt(), 0, 255)));
            } else if (propertySubField == PropertySubField::SizePolicyVerticalStretch) {
                sizePolicyValue.setVerticalStretch(static_cast<int>(std::clamp(editedValue.toInt(), 0, 255)));
            }
            writeSucceeded = writePropertyValue(propertyName, QVariant::fromValue(sizePolicyValue));
        } else if (currentPropertyValue.metaType() == QMetaType::fromType<QLocale>()) {
            const QLocale currentLocale = currentPropertyValue.toLocale();
            QLocale updatedLocale = currentLocale;
            if (propertySubField == PropertySubField::LocaleLanguage) {
                updatedLocale = QLocale(static_cast<QLocale::Language>(editedValue.toInt()),
                                        currentLocale.territory());
            } else if (propertySubField == PropertySubField::LocaleTerritory) {
                updatedLocale = QLocale(currentLocale.language(),
                                        static_cast<QLocale::Territory>(editedValue.toInt()));
            }
            writeSucceeded = writePropertyValue(propertyName, QVariant::fromValue(updatedLocale));
        }
    } else {
        writeSucceeded = writePropertyValue(propertyName, editedValue);
    }

    if (writeSucceeded) {
        if (propertySubField != PropertySubField::None) {
            refreshPropertyValueByName(propertyName);
            const QString subPropertyPath = treeItem->data(propertyValueColumn, PropertyNameRole).toString();
            if (subPropertyPath != propertyName) {
                refreshPropertyValueByName(subPropertyPath);
            }
            for (QTreeWidgetItemIterator itemIterator(propertyTreeWidget); *itemIterator != nullptr; ++itemIterator) {
                QTreeWidgetItem *candidateItem = *itemIterator;
                if (sourcePropertyNameForItem(candidateItem) == propertyName) {
                    refreshPropertyValueByName(
                        candidateItem->data(propertyValueColumn, PropertyNameRole).toString());
                }
            }
        } else {
            refreshPropertyValueByName(propertyName);
        }
    } else {
        updateItemPresentation(treeItem, readPropertyValue(propertyName), propertyKind);
    }
}

void ObjectPropertyEditorWidget::registerEditorWidget(QWidget *editorWidget)
{
    if (editorWidget == nullptr) {
        return;
    }

    ++activeEditorWidgetCount;
    connect(editorWidget, &QObject::destroyed, this, [this]() {
        activeEditorWidgetCount = std::max(0, activeEditorWidgetCount - 1);
    });
}

void ObjectPropertyEditorWidget::closePersistentEditorForIndex(const QModelIndex &modelIndex)
{
    if (propertyTreeWidget == nullptr || !modelIndex.isValid()) {
        return;
    }

    if (QTreeWidgetItem *treeItem = propertyTreeWidget->itemFromIndex(modelIndex)) {
        propertyTreeWidget->closePersistentEditor(treeItem, propertyValueColumn);
        if (persistentEditorItem == treeItem) {
            persistentEditorItem = nullptr;
        }
    }
}

QTreeWidget *ObjectPropertyEditorWidget::treeWidget() const
{
    return propertyTreeWidget;
}

void ObjectPropertyEditorWidget::refreshAllProperties()
{
    if (inspectedTargetObject == nullptr || isUpdatingTree || activeEditorWidgetCount > 0) {
        return;
    }

    QTreeWidgetItemIterator itemIterator(propertyTreeWidget);
    while (*itemIterator != nullptr) {
        QTreeWidgetItem *treeItem = *itemIterator;
        ++itemIterator;

        const QString propertyName = treeItem->data(propertyValueColumn, PropertyNameRole).toString();
        if (!propertyName.isEmpty()) {
            refreshPropertyValueByName(propertyName);
        }
    }
}

void ObjectPropertyEditorWidget::rebuildPropertyTree()
{
    isUpdatingTree = true;

    const QSignalBlocker treeSignalBlocker(propertyTreeWidget);

    propertyTreeWidget->clear();
    groupItems.clear();
    propertyItemsByName.clear();
    persistentEditorItem = nullptr;

    if (inspectedTargetObject != nullptr) {
        addMetaObjectProperties();
        addDynamicProperties();
        propertyTreeWidget->expandAll();
    }

    isUpdatingTree = false;
}

void ObjectPropertyEditorWidget::addMetaObjectProperties()
{
    if (inspectedTargetObject == nullptr) {
        return;
    }

    QList<const QMetaObject *> metaObjects;
    for (const QMetaObject *metaObjectPointer = inspectedTargetObject->metaObject();
         metaObjectPointer != nullptr;
         metaObjectPointer = metaObjectPointer->superClass()) {
        metaObjects.prepend(metaObjectPointer);
    }

    QHash<QString, QPair<const QMetaObject *, int>> lastVisiblePropertyByName;
    for (const QMetaObject *metaObjectPointer : metaObjects) {
        for (int propertyIndex = metaObjectPointer->propertyOffset();
             propertyIndex < metaObjectPointer->propertyCount();
             ++propertyIndex) {
            const QMetaProperty metaProperty = metaObjectPointer->property(propertyIndex);
            const QString propertyName = QString::fromLatin1(metaProperty.name());
            const QVariant propertyValue = readPropertyValue(propertyName);

            PropertyValueKind propertyKind = PropertyValueKind::Invalid;
            if (metaProperty.isFlagType()) {
                propertyKind = PropertyValueKind::FlagsValue;
            } else if (metaProperty.isEnumType()) {
                propertyKind = PropertyValueKind::EnumValue;
            } else {
                propertyKind = propertyKindFromVariant(propertyValue);
            }

            if (shouldSkipMetaProperty(metaProperty, propertyValue, propertyKind)) {
                continue;
            }

            lastVisiblePropertyByName.insert(propertyName,
                                             qMakePair(metaObjectPointer, propertyIndex));
        }
    }

    for (const QMetaObject *metaObjectPointer : metaObjects) {
        for (int propertyIndex = metaObjectPointer->propertyOffset();
             propertyIndex < metaObjectPointer->propertyCount();
             ++propertyIndex) {
            const QMetaProperty metaProperty = metaObjectPointer->property(propertyIndex);
            const QString propertyName = QString::fromLatin1(metaProperty.name());
            const QVariant propertyValue = readPropertyValue(propertyName);

            PropertyValueKind propertyKind = PropertyValueKind::Invalid;
            if (metaProperty.isFlagType()) {
                propertyKind = PropertyValueKind::FlagsValue;
            } else if (metaProperty.isEnumType()) {
                propertyKind = PropertyValueKind::EnumValue;
            } else {
                propertyKind = propertyKindFromVariant(propertyValue);
            }

            if (shouldSkipMetaProperty(metaProperty, propertyValue, propertyKind)) {
                continue;
            }

            const auto lastVisiblePropertyIterator = lastVisiblePropertyByName.constFind(propertyName);
            if (lastVisiblePropertyIterator == lastVisiblePropertyByName.constEnd()) {
                continue;
            }
            if (lastVisiblePropertyIterator.value().first != metaObjectPointer ||
                lastVisiblePropertyIterator.value().second != propertyIndex) {
                continue;
            }

            const QString groupName = propertyGroupNameForMetaProperty(metaProperty);
            const bool isWritable = metaProperty.isWritable()
                && isPropertyKindEditable(propertyKind)
                && !isManagedByLayout(propertyName);

            QStringList enumKeys;
            QList<int> enumValues;
            if (metaProperty.isEnumType()) {
                const QMetaEnum metaEnum = metaProperty.enumerator();
                for (int keyIndex = 0; keyIndex < metaEnum.keyCount(); ++keyIndex) {
                    enumKeys.append(QString::fromLatin1(metaEnum.key(keyIndex)));
                    enumValues.append(metaEnum.value(keyIndex));
                }
            }

            addPropertyItem(groupName, propertyName, propertyValue, propertyKind, isWritable, false,
                            enumKeys, enumValues);
        }
    }
}

void ObjectPropertyEditorWidget::addDynamicProperties()
{
    if (inspectedTargetObject == nullptr) {
        return;
    }

    const QList<QByteArray> dynamicPropertyNames = inspectedTargetObject->dynamicPropertyNames();
    for (const QByteArray &propertyNameBytes : dynamicPropertyNames) {
        const QString propertyName = QString::fromUtf8(propertyNameBytes);
        if (propertyItemsByName.contains(propertyName)) {
            continue;
        }

        const QVariant propertyValue = readPropertyValue(propertyName);
        const PropertyValueKind propertyKind = propertyKindFromVariant(propertyValue);
        const bool isWritable = isPropertyKindEditable(propertyKind);
        addPropertyItem(QStringLiteral("Dynamic Properties"), propertyName, propertyValue,
                        propertyKind, isWritable, true);
    }
}

void ObjectPropertyEditorWidget::addPropertyItem(const QString &groupName, const QString &propertyName,
                                                 const QVariant &propertyValue,
                                                 PropertyValueKind propertyKind, bool isWritable,
                                                 bool isDynamicProperty, const QStringList &enumKeys,
                                                 const QList<int> &enumValues)
{
    QTreeWidgetItem *groupItem = ensureGroupItem(groupName);
    auto *propertyItem = new QTreeWidgetItem(groupItem);
    propertyItem->setText(propertyNameColumn, propertyName);

    propertyItem->setData(propertyValueColumn, PropertyNameRole, propertyName);
    propertyItem->setData(propertyValueColumn, SourcePropertyNameRole, propertyName);
    propertyItem->setData(propertyValueColumn, SubPropertyRole, static_cast<int>(PropertySubField::None));
    propertyItem->setData(propertyValueColumn, PropertyKindRole, static_cast<int>(propertyKind));
    propertyItem->setData(propertyValueColumn, DynamicPropertyRole, isDynamicProperty);
    propertyItem->setData(propertyValueColumn, WritableRole, isWritable);
    propertyItem->setData(propertyValueColumn, EnumKeysRole, enumKeys);

    QVariantList enumValueVariantList;
    for (int enumValue : enumValues) {
        enumValueVariantList.append(enumValue);
    }
    propertyItem->setData(propertyValueColumn, EnumValuesRole, enumValueVariantList);
    propertyItem->setData(propertyValueColumn, GroupRole, groupName);

    propertyItem->setData(propertyNameColumn, WritableRole, false);
    propertyItem->setData(propertyNameColumn, PropertyNameRole, propertyName);

    const bool hasChildItems = propertyUsesChildItems(propertyName, propertyKind);
    Qt::ItemFlags itemFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (!hasChildItems) {
        if (isWritable && propertyKind != PropertyValueKind::Boolean) {
            itemFlags |= Qt::ItemIsEditable;
        }
        if (isWritable && propertyKind == PropertyValueKind::Boolean) {
            itemFlags |= Qt::ItemIsUserCheckable;
        }
    }
    propertyItem->setFlags(itemFlags);

    propertyItemsByName.insert(propertyName, propertyItem);
    updateItemPresentation(propertyItem, propertyValue, propertyKind);

    if (propertyKind == PropertyValueKind::Font && propertyValue.metaType() == QMetaType::fromType<QFont>()) {
        addFontSubProperties(propertyItem, groupName, propertyName, propertyValue.value<QFont>(), isWritable);
    } else if (propertyKind == PropertyValueKind::Size
               && propertyValue.metaType() == QMetaType::fromType<QSize>()) {
        addSizeSubProperties(propertyItem, groupName, propertyName, isWritable);
    } else if (propertyKind == PropertyValueKind::Rect
               && propertyName == QStringLiteral("geometry")
               && propertyValue.metaType() == QMetaType::fromType<QRect>()) {
        addRectSubProperties(propertyItem, groupName, propertyName, isWritable);
    } else if (propertyKind == PropertyValueKind::Margins
               && propertyName == QStringLiteral("contentsMargins")
               && propertyValue.metaType() == QMetaType::fromType<QMargins>()) {
        addMarginsSubProperties(propertyItem, groupName, propertyName, isWritable);
    } else if (propertyKind == PropertyValueKind::SizePolicyValue
               && propertyValue.metaType() == QMetaType::fromType<QSizePolicy>()) {
        addSizePolicySubProperties(propertyItem, groupName, propertyName, isWritable);
    } else if (propertyKind == PropertyValueKind::LocaleValue
               && propertyValue.metaType() == QMetaType::fromType<QLocale>()) {
        addLocaleSubProperties(propertyItem, groupName, propertyName, isWritable);
    }
}

void ObjectPropertyEditorWidget::addSubPropertyItem(QTreeWidgetItem *parentPropertyItem,
                                                    const QString &groupName,
                                                    const QString &sourcePropertyName,
                                                    PropertyValueKind propertyKind,
                                                    int subPropertyCode,
                                                    bool isWritable,
                                                    const QStringList &enumKeys,
                                                    const QList<int> &enumValues)
{
    if (parentPropertyItem == nullptr) {
        return;
    }

    const auto propertySubField = static_cast<PropertySubField>(subPropertyCode);
    const QString subPropertyKey = propertySubFieldKey(propertySubField);
    const QString subPropertyPath = sourcePropertyName + QStringLiteral(".") + subPropertyKey;

    auto *subPropertyItem = new QTreeWidgetItem(parentPropertyItem);
    subPropertyItem->setText(propertyNameColumn, propertySubFieldLabel(propertySubField));
    subPropertyItem->setData(propertyValueColumn, PropertyNameRole, subPropertyPath);
    subPropertyItem->setData(propertyValueColumn, SourcePropertyNameRole, sourcePropertyName);
    subPropertyItem->setData(propertyValueColumn, SubPropertyRole, subPropertyCode);
    subPropertyItem->setData(propertyValueColumn, PropertyKindRole, static_cast<int>(propertyKind));
    subPropertyItem->setData(propertyValueColumn, DynamicPropertyRole, false);
    subPropertyItem->setData(propertyValueColumn, WritableRole, isWritable);
    subPropertyItem->setData(propertyValueColumn, EnumKeysRole, enumKeys);

    QVariantList enumValueVariantList;
    for (int enumValue : enumValues) {
        enumValueVariantList.append(enumValue);
    }
    subPropertyItem->setData(propertyValueColumn, EnumValuesRole, enumValueVariantList);
    subPropertyItem->setData(propertyValueColumn, GroupRole, groupName);

    Qt::ItemFlags itemFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (isWritable && propertyKind != PropertyValueKind::Boolean) {
        itemFlags |= Qt::ItemIsEditable;
    }
    if (isWritable && propertyKind == PropertyValueKind::Boolean) {
        itemFlags |= Qt::ItemIsUserCheckable;
    }
    subPropertyItem->setFlags(itemFlags);

    propertyItemsByName.insert(subPropertyPath, subPropertyItem);
    updateItemPresentation(subPropertyItem, readPropertyValue(sourcePropertyName), propertyKind);
}

void ObjectPropertyEditorWidget::addFontSubProperties(QTreeWidgetItem *fontPropertyItem,
                                                     const QString &groupName,
                                                     const QString &propertyName,
                                                     const QFont &fontValue,
                                                     bool isWritable)
{
    Q_UNUSED(fontValue)

    if (fontPropertyItem == nullptr) {
        return;
    }

    const QList<PropertySubField> fontSubFields {
        PropertySubField::FontFamily,
        PropertySubField::FontPointSize,
        PropertySubField::FontBold,
        PropertySubField::FontItalic,
        PropertySubField::FontUnderline,
        PropertySubField::FontStrikeOut,
        PropertySubField::FontKerning,
        PropertySubField::FontWeight,
        PropertySubField::FontAntialiasing,
        PropertySubField::FontHintingPreference
    };

    for (PropertySubField fontSubField : fontSubFields) {
        QStringList enumKeys;
        QList<int> enumValues;
        populatePropertySubFieldEnumData(fontSubField, &enumKeys, &enumValues);
        addSubPropertyItem(fontPropertyItem, groupName, propertyName, propertySubFieldKind(fontSubField),
                           static_cast<int>(fontSubField), isWritable, enumKeys, enumValues);
    }
}

void ObjectPropertyEditorWidget::addSizeSubProperties(QTreeWidgetItem *sizePropertyItem,
                                                      const QString &groupName,
                                                      const QString &propertyName,
                                                      bool isWritable)
{
    if (sizePropertyItem == nullptr) {
        return;
    }

    const QList<PropertySubField> sizeSubFields {
        PropertySubField::SizeWidth,
        PropertySubField::SizeHeight
    };

    for (PropertySubField sizeSubField : sizeSubFields) {
        addSubPropertyItem(sizePropertyItem, groupName, propertyName, propertySubFieldKind(sizeSubField),
                           static_cast<int>(sizeSubField), isWritable);
    }
}

void ObjectPropertyEditorWidget::addRectSubProperties(QTreeWidgetItem *rectPropertyItem,
                                                      const QString &groupName,
                                                      const QString &propertyName,
                                                      bool isWritable)
{
    if (rectPropertyItem == nullptr) {
        return;
    }

    const QList<PropertySubField> rectSubFields {
        PropertySubField::RectX,
        PropertySubField::RectY,
        PropertySubField::RectWidth,
        PropertySubField::RectHeight
    };

    for (PropertySubField rectSubField : rectSubFields) {
        addSubPropertyItem(rectPropertyItem, groupName, propertyName, propertySubFieldKind(rectSubField),
                           static_cast<int>(rectSubField), isWritable);
    }
}

void ObjectPropertyEditorWidget::addMarginsSubProperties(QTreeWidgetItem *marginsPropertyItem,
                                                         const QString &groupName,
                                                         const QString &propertyName,
                                                         bool isWritable)
{
    if (marginsPropertyItem == nullptr) {
        return;
    }

    const QList<PropertySubField> marginsSubFields {
        PropertySubField::MarginsLeft,
        PropertySubField::MarginsTop,
        PropertySubField::MarginsRight,
        PropertySubField::MarginsBottom
    };

    for (PropertySubField marginsSubField : marginsSubFields) {
        addSubPropertyItem(marginsPropertyItem, groupName, propertyName,
                           propertySubFieldKind(marginsSubField),
                           static_cast<int>(marginsSubField), isWritable);
    }
}

void ObjectPropertyEditorWidget::addSizePolicySubProperties(QTreeWidgetItem *sizePolicyPropertyItem,
                                                            const QString &groupName,
                                                            const QString &propertyName,
                                                            bool isWritable)
{
    if (sizePolicyPropertyItem == nullptr) {
        return;
    }

    const QList<PropertySubField> sizePolicySubFields {
        PropertySubField::SizePolicyHorizontalPolicy,
        PropertySubField::SizePolicyVerticalPolicy,
        PropertySubField::SizePolicyHorizontalStretch,
        PropertySubField::SizePolicyVerticalStretch
    };

    for (PropertySubField sizePolicySubField : sizePolicySubFields) {
        QStringList enumKeys;
        QList<int> enumValues;
        populatePropertySubFieldEnumData(sizePolicySubField, &enumKeys, &enumValues);
        addSubPropertyItem(sizePolicyPropertyItem, groupName, propertyName,
                           propertySubFieldKind(sizePolicySubField),
                           static_cast<int>(sizePolicySubField), isWritable, enumKeys, enumValues);
    }
}

void ObjectPropertyEditorWidget::addLocaleSubProperties(QTreeWidgetItem *localePropertyItem,
                                                        const QString &groupName,
                                                        const QString &propertyName,
                                                        bool isWritable)
{
    if (localePropertyItem == nullptr) {
        return;
    }

    const QList<PropertySubField> localeSubFields {
        PropertySubField::LocaleLanguage,
        PropertySubField::LocaleTerritory
    };

    for (PropertySubField localeSubField : localeSubFields) {
        QStringList enumKeys;
        QList<int> enumValues;
        populatePropertySubFieldEnumData(localeSubField, &enumKeys, &enumValues);
        addSubPropertyItem(localePropertyItem, groupName, propertyName,
                           propertySubFieldKind(localeSubField),
                           static_cast<int>(localeSubField), isWritable, enumKeys, enumValues);
    }
}

QTreeWidgetItem *ObjectPropertyEditorWidget::ensureGroupItem(const QString &groupName)
{
    const auto existingIterator = groupItems.constFind(groupName);
    if (existingIterator != groupItems.constEnd()) {
        return existingIterator.value();
    }

    auto *groupItem = new QTreeWidgetItem(propertyTreeWidget);
    groupItem->setText(propertyNameColumn, groupName);
    groupItem->setFirstColumnSpanned(true);
    groupItem->setFlags(Qt::ItemIsEnabled);
    groupItems.insert(groupName, groupItem);
    return groupItem;
}

void ObjectPropertyEditorWidget::updateItemPresentation(
    QTreeWidgetItem *treeItem, const QVariant &propertyValue, PropertyValueKind propertyKind)
{
    if (treeItem == nullptr) {
        return;
    }

    isUpdatingTree = true;

    const PropertySubField propertySubField = static_cast<PropertySubField>(
        treeItem->data(propertyValueColumn, SubPropertyRole).toInt());
    if (propertySubField != PropertySubField::None) {
        const QVariant subPropertyValue = propertySubFieldValue(propertyValue, propertySubField);
        const PropertyValueKind subPropertyKind = propertySubFieldKind(propertySubField);

        if (subPropertyKind == PropertyValueKind::Boolean) {
            const bool booleanValue = subPropertyValue.toBool();
            treeItem->setCheckState(propertyValueColumn, booleanValue ? Qt::Checked : Qt::Unchecked);
            treeItem->setText(propertyValueColumn, booleanValue ? QStringLiteral("true") : QStringLiteral("false"));
            treeItem->setData(propertyValueColumn, Qt::EditRole, booleanValue);
        } else if (subPropertyKind == PropertyValueKind::EnumValue) {
            const QStringList enumKeys = treeItem->data(propertyValueColumn, EnumKeysRole).toStringList();
            const QList<QVariant> enumValues = treeItem->data(propertyValueColumn, EnumValuesRole).toList();
            const int currentValue = subPropertyValue.toInt();

            int displayIndex = -1;
            for (int valueIndex = 0; valueIndex < enumValues.size(); ++valueIndex) {
                if (enumValues.at(valueIndex).toInt() == currentValue) {
                    displayIndex = valueIndex;
                    break;
                }
            }

            const QString enumText = (displayIndex >= 0 && displayIndex < enumKeys.size())
                ? enumKeys.at(displayIndex)
                : QString::number(currentValue);
            treeItem->setText(propertyValueColumn, enumText);
            treeItem->setData(propertyValueColumn, Qt::EditRole, enumText);
        } else {
            const QString displayText = displayTextForValue(subPropertyValue, subPropertyKind);
            treeItem->setText(propertyValueColumn, displayText);
            treeItem->setData(propertyValueColumn, Qt::EditRole, subPropertyValue);
        }

        isUpdatingTree = false;
        return;
    }

    if (propertyKind == PropertyValueKind::Font
        && propertyValue.metaType() == QMetaType::fromType<QFont>()) {
        const QString displayText = fontSummaryText(propertyValue.value<QFont>());
        treeItem->setText(propertyValueColumn, displayText);
        treeItem->setData(propertyValueColumn, Qt::EditRole, displayText);
        isUpdatingTree = false;
        return;
    }

    if (propertyKind == PropertyValueKind::Boolean) {
        const bool booleanValue = propertyValue.toBool();
        treeItem->setCheckState(propertyValueColumn, booleanValue ? Qt::Checked : Qt::Unchecked);
        treeItem->setText(propertyValueColumn, booleanValue ? QStringLiteral("true") : QStringLiteral("false"));
        treeItem->setData(propertyValueColumn, Qt::EditRole, booleanValue);
    } else if (propertyKind == PropertyValueKind::EnumValue) {
        const QStringList enumKeys = treeItem->data(propertyValueColumn, EnumKeysRole).toStringList();
        const QList<QVariant> enumValues = treeItem->data(propertyValueColumn, EnumValuesRole).toList();
        const int currentValue = propertyValue.toInt();

        int displayIndex = -1;
        for (int valueIndex = 0; valueIndex < enumValues.size(); ++valueIndex) {
            if (enumValues.at(valueIndex).toInt() == currentValue) {
                displayIndex = valueIndex;
                break;
            }
        }

        const QString enumText = (displayIndex >= 0 && displayIndex < enumKeys.size())
            ? enumKeys.at(displayIndex)
            : QString::number(currentValue);
        treeItem->setText(propertyValueColumn, enumText);
        treeItem->setData(propertyValueColumn, Qt::EditRole, enumText);
    } else if (propertyKind == PropertyValueKind::FlagsValue) {
        const QStringList flagKeys = treeItem->data(propertyValueColumn, EnumKeysRole).toStringList();
        const QList<QVariant> flagValues = treeItem->data(propertyValueColumn, EnumValuesRole).toList();
        const int currentValue = propertyValue.toInt();
        const QString flagText = flagsToText(currentValue, flagKeys, flagValues);
        treeItem->setText(propertyValueColumn, flagText);
        treeItem->setData(propertyValueColumn, Qt::EditRole, flagText);
    } else {
        const QString propertyName = sourcePropertyNameForItem(treeItem);
        const QString displayText = (propertyKind == PropertyValueKind::Palette)
            ? paletteDisplayText(propertyName, propertyValue)
            : displayTextForValue(propertyValue, propertyKind);
        treeItem->setText(propertyValueColumn, displayText);
        treeItem->setData(propertyValueColumn, Qt::EditRole, displayText);
    }

    isUpdatingTree = false;
}

bool ObjectPropertyEditorWidget::isPaletteInherited(const QString &propertyName) const
{
    if (propertyName != QStringLiteral("palette")) {
        return false;
    }

    const auto *widget = qobject_cast<QWidget *>(inspectedTargetObject.data());
    if (widget == nullptr) {
        return false;
    }

    return !widget->testAttribute(Qt::WA_SetPalette);
}

QString ObjectPropertyEditorWidget::paletteDisplayText(const QString &propertyName,
                                                       const QVariant &propertyValue) const
{
    if (propertyName == QStringLiteral("palette")
        && propertyValue.metaType() == QMetaType::fromType<QPalette>()) {
        return isPaletteInherited(propertyName) ? paletteInheritedText() : paletteChangedText();
    }

    return displayTextForValue(propertyValue, propertyKindFromVariant(propertyValue));
}

QString ObjectPropertyEditorWidget::propertyGroupNameForMetaProperty(const QMetaProperty &metaProperty) const
{
    if (metaProperty.enclosingMetaObject() != nullptr) {
        return QString::fromLatin1(metaProperty.enclosingMetaObject()->className());
    }
    return QStringLiteral("Properties");
}

bool ObjectPropertyEditorWidget::shouldForceShowMetaProperty(const QMetaProperty &metaProperty) const
{
    if (inspectedTargetObject == nullptr) {
        return false;
    }

    const QString propertyName = QString::fromLatin1(metaProperty.name());
    if (propertyName != QStringLiteral("modal")) {
        return false;
    }

    return qobject_cast<QDialog *>(inspectedTargetObject.data()) != nullptr;
}

bool ObjectPropertyEditorWidget::isPropertyKindEditable(PropertyValueKind propertyKind) const
{
    switch (propertyKind) {
    case PropertyValueKind::Boolean:
    case PropertyValueKind::Integer:
    case PropertyValueKind::UnsignedInteger:
    case PropertyValueKind::LongLongInteger:
    case PropertyValueKind::UnsignedLongLongInteger:
    case PropertyValueKind::DoubleValue:
    case PropertyValueKind::String:
    case PropertyValueKind::StringList:
    case PropertyValueKind::EnumValue:
    case PropertyValueKind::FlagsValue:
    case PropertyValueKind::Point:
    case PropertyValueKind::Size:
    case PropertyValueKind::Rect:
    case PropertyValueKind::Margins:
    case PropertyValueKind::Color:
    case PropertyValueKind::Palette:
    case PropertyValueKind::Font:
    case PropertyValueKind::ByteArray:
    case PropertyValueKind::Cursor:
    case PropertyValueKind::Orientation:
    case PropertyValueKind::SizePolicyValue:
    case PropertyValueKind::LocaleValue:
        return true;
    case PropertyValueKind::Invalid:
    case PropertyValueKind::FallbackString:
        return false;
    }

    return false;
}

bool ObjectPropertyEditorWidget::isManagedByLayout(const QString &propertyName) const
{
    if (inspectedTargetObject == nullptr || propertyName != QStringLiteral("geometry")) {
        return false;
    }

    auto *widget = qobject_cast<QWidget *>(inspectedTargetObject.data());
    if (widget == nullptr) {
        return false;
    }

    QWidget *parentWidget = widget->parentWidget();
    if (parentWidget == nullptr) {
        return false;
    }

    QLayout *parentLayout = parentWidget->layout();
    if (parentLayout == nullptr) {
        return false;
    }

    return parentLayout->indexOf(widget) >= 0;
}

bool ObjectPropertyEditorWidget::shouldSkipMetaProperty(const QMetaProperty &metaProperty,
                                                        const QVariant &propertyValue,
                                                        PropertyValueKind propertyKind) const
{
    Q_UNUSED(propertyValue)

    if (inspectedTargetObject == nullptr) {
        return false;
    }

    if (!metaProperty.isDesignable() && !shouldForceShowMetaProperty(metaProperty)) {
        return true;
    }

    if (!metaProperty.isWritable() && !shouldForceShowMetaProperty(metaProperty)) {
        return true;
    }

    auto *dockWidget = qobject_cast<QDockWidget *>(inspectedTargetObject.data());
    if (dockWidget != nullptr) {
        const QString propertyName = QString::fromLatin1(metaProperty.name());
        if (propertyName == QStringLiteral("dockLocation")
            && qobject_cast<QMainWindow *>(dockWidget->parentWidget()) == nullptr) {
            return true;
        }
    }

    Q_UNUSED(propertyKind)
    return false;
}

QString ObjectPropertyEditorWidget::sourcePropertyNameForItem(const QTreeWidgetItem *treeItem) const
{
    if (treeItem == nullptr) {
        return {};
    }

    const QString sourcePropertyName = treeItem->data(propertyValueColumn, SourcePropertyNameRole).toString();
    if (!sourcePropertyName.isEmpty()) {
        return sourcePropertyName;
    }

    return treeItem->data(propertyValueColumn, PropertyNameRole).toString();
}

QVariant ObjectPropertyEditorWidget::readPropertyValue(const QString &propertyName) const
{
    if (inspectedTargetObject == nullptr) {
        return {};
    }
    return inspectedTargetObject->property(propertyName.toUtf8().constData());
}

bool ObjectPropertyEditorWidget::writePropertyValue(const QString &propertyName, const QVariant &propertyValue)
{
    if (inspectedTargetObject == nullptr) {
        return false;
    }
    return inspectedTargetObject->setProperty(propertyName.toUtf8().constData(), propertyValue);
}

void ObjectPropertyEditorWidget::handleItemChanged(QTreeWidgetItem *treeItem, int columnIndex)
{
    if (isUpdatingTree || treeItem == nullptr || columnIndex != propertyValueColumn) {
        return;
    }

    const auto propertyKind = static_cast<PropertyValueKind>(
        treeItem->data(propertyValueColumn, PropertyKindRole).toInt());
    if (propertyKind == PropertyValueKind::Boolean) {
        applyEditedValue(treeItem, treeItem->checkState(propertyValueColumn) == Qt::Checked);
    }
}

void ObjectPropertyEditorWidget::refreshPropertyValueByName(const QString &propertyName)
{
    const auto itemIterator = propertyItemsByName.constFind(propertyName);
    if (itemIterator == propertyItemsByName.constEnd()) {
        return;
    }

    QTreeWidgetItem *treeItem = itemIterator.value();
    const QString sourcePropertyName = sourcePropertyNameForItem(treeItem);
    if (sourcePropertyName.isEmpty()) {
        return;
    }

    if (auto *dockWidget = qobject_cast<QDockWidget *>(inspectedTargetObject.data())) {
        if (sourcePropertyName == QStringLiteral("dockLocation")
            && qobject_cast<QMainWindow *>(dockWidget->parentWidget()) == nullptr) {
            return;
        }
    }

    const auto propertyKind = static_cast<PropertyValueKind>(
        treeItem->data(propertyValueColumn, PropertyKindRole).toInt());
    updateItemPresentation(treeItem, readPropertyValue(sourcePropertyName), propertyKind);
}

void ObjectPropertyEditorWidget::clearObservedConnections()
{
    for (const QMetaObject::Connection &connection : std::as_const(observedConnections)) {
        QObject::disconnect(connection);
    }
    observedConnections.clear();

    if (destroyedConnection) {
        QObject::disconnect(destroyedConnection);
        destroyedConnection = {};
    }
}

void ObjectPropertyEditorWidget::connectNotifySignals()
{
    if (inspectedTargetObject == nullptr) {
        return;
    }

    const int refreshSlotIndex = metaObject()->indexOfSlot("refreshAllProperties()");
    const QMetaMethod refreshMethod = metaObject()->method(refreshSlotIndex);

    QSet<int> connectedSignalIndexes;
    const QMetaObject *metaObjectPointer = inspectedTargetObject->metaObject();
    while (metaObjectPointer != nullptr) {
        for (int propertyIndex = metaObjectPointer->propertyOffset();
             propertyIndex < metaObjectPointer->propertyCount(); ++propertyIndex) {
            const QMetaProperty metaProperty = metaObjectPointer->property(propertyIndex);
            if (!metaProperty.hasNotifySignal()) {
                continue;
            }

            const int signalIndex = metaProperty.notifySignalIndex();
            if (signalIndex < 0 || connectedSignalIndexes.contains(signalIndex)) {
                continue;
            }

            const QMetaMethod notifySignal = metaProperty.notifySignal();
            const QMetaObject::Connection connection = QObject::connect(
                inspectedTargetObject, notifySignal, this, refreshMethod);
            if (connection) {
                observedConnections.append(connection);
                connectedSignalIndexes.insert(signalIndex);
            }
        }
        metaObjectPointer = metaObjectPointer->superClass();
    }
}

void ObjectPropertyEditorWidget::connectCommonWidgetSignals()
{
    if (inspectedTargetObject == nullptr) {
        return;
    }

    if (auto *abstractSlider = qobject_cast<QAbstractSlider *>(inspectedTargetObject)) {
        observedConnections.append(connect(abstractSlider, &QAbstractSlider::valueChanged, this,
                                           [this](int) { refreshPropertyValueByName(QStringLiteral("value")); }));
        observedConnections.append(connect(abstractSlider, &QAbstractSlider::rangeChanged, this,
                                           [this](int, int) {
                                               refreshPropertyValueByName(QStringLiteral("minimum"));
                                               refreshPropertyValueByName(QStringLiteral("maximum"));
                                               refreshPropertyValueByName(QStringLiteral("value"));
                                           }));
        observedConnections.append(connect(abstractSlider, &QAbstractSlider::sliderMoved, this,
                                           [this](int) { refreshPropertyValueByName(QStringLiteral("sliderPosition")); }));
    }

    if (auto *comboBox = qobject_cast<QComboBox *>(inspectedTargetObject)) {
        observedConnections.append(connect(comboBox,
                                           qOverload<int>(&QComboBox::currentIndexChanged), this,
                                           [this](int) {
                                               refreshPropertyValueByName(QStringLiteral("currentIndex"));
                                               refreshPropertyValueByName(QStringLiteral("currentText"));
                                           }));
        observedConnections.append(connect(comboBox, &QComboBox::currentTextChanged, this,
                                           [this](const QString &) {
                                               refreshPropertyValueByName(QStringLiteral("currentText"));
                                           }));
        observedConnections.append(connect(comboBox, &QComboBox::editTextChanged, this,
                                           [this](const QString &) {
                                               refreshPropertyValueByName(QStringLiteral("currentText"));
                                           }));
    }

    if (auto *lineEdit = qobject_cast<QLineEdit *>(inspectedTargetObject)) {
        observedConnections.append(connect(lineEdit, &QLineEdit::textChanged, this,
                                           [this](const QString &) {
                                               refreshPropertyValueByName(QStringLiteral("text"));
                                           }));
    }

    if (auto *abstractButton = qobject_cast<QAbstractButton *>(inspectedTargetObject)) {
        observedConnections.append(connect(abstractButton, &QAbstractButton::toggled, this,
                                           [this](bool) {
                                               refreshPropertyValueByName(QStringLiteral("checked"));
                                           }));
        observedConnections.append(connect(abstractButton, &QObject::objectNameChanged, this,
                                           [this](const QString &) {
                                               refreshPropertyValueByName(QStringLiteral("objectName"));
                                           }));
    }

    if (auto *widget = qobject_cast<QWidget *>(inspectedTargetObject)) {
        observedConnections.append(connect(widget, &QWidget::windowTitleChanged, this,
                                           [this](const QString &) {
                                               refreshPropertyValueByName(QStringLiteral("windowTitle"));
                                           }));
        observedConnections.append(connect(widget, &QObject::objectNameChanged, this,
                                           [this](const QString &) {
                                               refreshPropertyValueByName(QStringLiteral("objectName"));
                                           }));
    }
}

void ObjectPropertyEditorWidget::attachPeriodicRefresh()
{
    if (refreshTimer != nullptr && !refreshTimer->isActive()) {
        refreshTimer->start();
    }
}
