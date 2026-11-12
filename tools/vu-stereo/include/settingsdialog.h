// SettingsDialog は、ユーザーが変更できる VU メーター設定を編集するための
// ダイアログです。針の太さ、入力レベル補正、支点の表示、ピークホールド表示、
// 常に前面表示を MeterSettings として保持し、操作のたびに settingsChanged() で外側へ通知します。
// 設定値の適用先は持たず、UI と設定値の編集だけを担当します。

#pragma once

#include "metersettings.h"

#include <QDialog>

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QLabel;
class QSpinBox;

class SettingsDialog final : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(const MeterSettings &settings, QWidget *parent = nullptr);

    MeterSettings settings() const;

protected:
    void changeEvent(QEvent *event) override;

signals:
    void settingsChanged(const MeterSettings &settings);

private:
    void buildUi();
    void retranslateUi();

    MeterSettings m_settings;
    QLabel *m_inputLevelLabel = nullptr;
    QLabel *m_needleWidthLabel = nullptr;
    QLabel *m_scaleStyleLabel = nullptr;
    QLabel *m_languageLabel = nullptr;
    QSpinBox *m_inputLevelSpinBox = nullptr;
    QCheckBox *m_pivotCheckBox = nullptr;
    QCheckBox *m_needlePivotLoweredCheckBox = nullptr;
    QCheckBox *m_peakHoldCheckBox = nullptr;
    QCheckBox *m_alwaysOnTopCheckBox = nullptr;
    QComboBox *m_scaleStyleComboBox = nullptr;
    QComboBox *m_languageComboBox = nullptr;
    QDialogButtonBox *m_buttons = nullptr;
};
