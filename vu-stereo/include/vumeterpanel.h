// VuMeterPanel は、左右 2 個の VU メーターを 1 つの表示領域としてまとめる
// ウィジェットです。台座、L/R ラベル、下部の dBFS 表示をこのクラスに集約し、
// 外側のウィンドウからは setLevels() と applySettings() だけで操作できるようにします。
// 個々の針の描画は VuMeterWidget に任せます。

#pragma once

#include "metersettings.h"

#include <QWidget>

class QLabel;
class VuMeterWidget;

class VuMeterPanel final : public QWidget {
    Q_OBJECT

public:
    explicit VuMeterPanel(QWidget *parent = nullptr);

    void setLevels(double leftDbfs, double rightDbfs);
    void setStatusMessage(const QString &message);
    void showStartingMessage();
    void applySettings(const MeterSettings &settings);

protected:
    void changeEvent(QEvent *event) override;

private:
    void retranslateUi();

    VuMeterWidget *m_leftMeter = nullptr;
    VuMeterWidget *m_rightMeter = nullptr;
    QLabel *m_statusLabel = nullptr;
    QLabel *m_leftLabel = nullptr;
    QLabel *m_rightLabel = nullptr;
    double m_lastLeftDbfs = -60.0;
    double m_lastRightDbfs = -60.0;
    bool m_hasLevels = false;
};
