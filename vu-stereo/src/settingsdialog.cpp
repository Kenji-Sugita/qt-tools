// SettingsDialog の実装です。ここでは Qt の各入力部品と MeterSettings の同期だけを
// 行います。変更は即時に settingsChanged() として通知し、実際にメーターへ適用するか
// どうかは呼び出し側に任せます。

#include "settingsdialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QSignalBlocker>
#include <QSlider>
#include <QSpinBox>
#include <QVariant>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(const MeterSettings &settings, QWidget *parent)
    : QDialog(parent),
      m_settings(settings)
{
    buildUi();
    retranslateUi();
}

MeterSettings SettingsDialog::settings() const
{
    return m_settings;
}

void SettingsDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }

    QDialog::changeEvent(event);
}

void SettingsDialog::buildUi()
{
    auto *dialogLayout = new QVBoxLayout(this);
    auto *inputLevelLayout = new QHBoxLayout;
    auto *inputLevelSlider = new QSlider(Qt::Horizontal, this);
    auto *needleWidthLayout = new QHBoxLayout;
    auto *needleWidthSlider = new QSlider(Qt::Horizontal, this);
    auto *needleWidthSpinBox = new QSpinBox(this);
    auto *scaleStyleLayout = new QHBoxLayout;
    auto *languageLayout = new QHBoxLayout;

    m_inputLevelLabel = new QLabel(this);
    m_needleWidthLabel = new QLabel(this);
    m_scaleStyleLabel = new QLabel(this);
    m_languageLabel = new QLabel(this);
    m_inputLevelSpinBox = new QSpinBox(this);
    m_pivotCheckBox = new QCheckBox(this);
    m_needlePivotLoweredCheckBox = new QCheckBox(this);
    m_peakHoldCheckBox = new QCheckBox(this);
    m_alwaysOnTopCheckBox = new QCheckBox(this);
    m_scaleStyleComboBox = new QComboBox(this);
    m_languageComboBox = new QComboBox(this);
    m_buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);

    inputLevelSlider->setRange(-24, 24);
    inputLevelSlider->setValue(m_settings.inputLevelDb);
    m_inputLevelSpinBox->setRange(-24, 24);
    m_inputLevelSpinBox->setValue(m_settings.inputLevelDb);
    needleWidthSlider->setRange(1, 30);
    needleWidthSlider->setValue(m_settings.needleWidth);
    needleWidthSpinBox->setRange(1, 30);
    needleWidthSpinBox->setValue(m_settings.needleWidth);
    m_pivotCheckBox->setChecked(m_settings.pivotVisible);
    m_needlePivotLoweredCheckBox->setChecked(m_settings.needlePivotLowered);
    m_peakHoldCheckBox->setChecked(m_settings.peakHoldEnabled);
    m_alwaysOnTopCheckBox->setChecked(m_settings.alwaysOnTop);

    inputLevelLayout->addWidget(m_inputLevelLabel);
    inputLevelLayout->addWidget(inputLevelSlider, 1);
    inputLevelLayout->addWidget(m_inputLevelSpinBox);
    needleWidthLayout->addWidget(m_needleWidthLabel);
    needleWidthLayout->addWidget(needleWidthSlider, 1);
    needleWidthLayout->addWidget(needleWidthSpinBox);
    scaleStyleLayout->addWidget(m_scaleStyleLabel);
    scaleStyleLayout->addWidget(m_scaleStyleComboBox, 1);
    languageLayout->addWidget(m_languageLabel);
    languageLayout->addWidget(m_languageComboBox, 1);
    dialogLayout->addLayout(inputLevelLayout);
    dialogLayout->addLayout(needleWidthLayout);
    dialogLayout->addLayout(scaleStyleLayout);
    dialogLayout->addLayout(languageLayout);
    dialogLayout->addWidget(m_pivotCheckBox);
    dialogLayout->addWidget(m_needlePivotLoweredCheckBox);
    dialogLayout->addWidget(m_peakHoldCheckBox);
    dialogLayout->addWidget(m_alwaysOnTopCheckBox);
    dialogLayout->addWidget(m_buttons);

    connect(inputLevelSlider, &QSlider::valueChanged, m_inputLevelSpinBox, &QSpinBox::setValue);
    connect(m_inputLevelSpinBox, &QSpinBox::valueChanged, inputLevelSlider, &QSlider::setValue);
    connect(inputLevelSlider, &QSlider::valueChanged, this, [this](int value) {
        m_settings.inputLevelDb = value;
        emit settingsChanged(m_settings);
    });
    connect(needleWidthSlider, &QSlider::valueChanged, needleWidthSpinBox, &QSpinBox::setValue);
    connect(needleWidthSpinBox, &QSpinBox::valueChanged, needleWidthSlider, &QSlider::setValue);
    connect(needleWidthSlider, &QSlider::valueChanged, this, [this](int value) {
        m_settings.needleWidth = value;
        emit settingsChanged(m_settings);
    });
    connect(m_pivotCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        m_settings.pivotVisible = checked;
        emit settingsChanged(m_settings);
    });
    connect(m_needlePivotLoweredCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        m_settings.needlePivotLowered = checked;
        emit settingsChanged(m_settings);
    });
    connect(m_peakHoldCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        m_settings.peakHoldEnabled = checked;
        emit settingsChanged(m_settings);
    });
    connect(m_alwaysOnTopCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        m_settings.alwaysOnTop = checked;
        emit settingsChanged(m_settings);
    });
    connect(m_scaleStyleComboBox, &QComboBox::currentIndexChanged, this, [this](int index) {
        const QVariant scaleStyleData = m_scaleStyleComboBox->itemData(index);
        if (!scaleStyleData.isValid()) {
            return;
        }

        m_settings.scaleStyle = static_cast<MeterScaleStyle>(scaleStyleData.toInt());
        emit settingsChanged(m_settings);
    });
    connect(m_languageComboBox, &QComboBox::currentIndexChanged, this, [this](int index) {
        const QVariant languageData = m_languageComboBox->itemData(index);
        if (!languageData.isValid()) {
            return;
        }

        m_settings.language = static_cast<AppLanguage>(languageData.toInt());
        emit settingsChanged(m_settings);
    });
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::accept);
}

void SettingsDialog::retranslateUi()
{
    setWindowTitle(tr("Settings"));
    m_inputLevelLabel->setText(tr("Input level"));
    m_needleWidthLabel->setText(tr("Needle width"));
    m_scaleStyleLabel->setText(tr("Scale"));
    m_languageLabel->setText(tr("Language"));
    m_inputLevelSpinBox->setSuffix(tr(" dB"));
    m_pivotCheckBox->setText(tr("Show black pivot"));
    m_needlePivotLoweredCheckBox->setText(tr("Lower needle pivot"));
    m_peakHoldCheckBox->setText(tr("Show peak hold"));
    m_alwaysOnTopCheckBox->setText(tr("Always on top"));

    {
        const QSignalBlocker blocker(m_scaleStyleComboBox);
        m_scaleStyleComboBox->clear();
        m_scaleStyleComboBox->addItem(tr("Arc"), static_cast<int>(MeterScaleStyle::Arc));
        m_scaleStyleComboBox->addItem(tr("Linear"), static_cast<int>(MeterScaleStyle::Linear));

        const int index = m_scaleStyleComboBox->findData(static_cast<int>(m_settings.scaleStyle));
        if (index >= 0) {
            m_scaleStyleComboBox->setCurrentIndex(index);
        }
    }

    const QSignalBlocker blocker(m_languageComboBox);
    m_languageComboBox->clear();
    m_languageComboBox->addItem(tr("System"), static_cast<int>(AppLanguage::System));
    m_languageComboBox->addItem(tr("English"), static_cast<int>(AppLanguage::English));
    m_languageComboBox->addItem(tr("Japanese"), static_cast<int>(AppLanguage::Japanese));

    const int index = m_languageComboBox->findData(static_cast<int>(m_settings.language));
    if (index >= 0) {
        m_languageComboBox->setCurrentIndex(index);
    }
}
