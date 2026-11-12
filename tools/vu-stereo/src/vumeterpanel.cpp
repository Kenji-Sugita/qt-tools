// VuMeterPanel の実装です。左右の VuMeterWidget を同じ台座の上へ配置し、
// 現在のレベル表示テキストもここで更新します。画面全体のウィンドウ制御や
// 音声取得処理は持たず、VU メーター表示のまとまりだけを担当します。

#include "vumeterpanel.h"

#include "meterbasewidget.h"
#include "vumeterwidget.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

VuMeterPanel::VuMeterPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    auto *meterBase = new MeterBaseWidget(this);
    auto *baseLayout = new QVBoxLayout(meterBase);
    auto *meterLayout = new QHBoxLayout;
    auto *leftLayout = new QVBoxLayout;
    auto *rightLayout = new QVBoxLayout;

    m_leftMeter = new VuMeterWidget(this);
    m_rightMeter = new VuMeterWidget(this);
    m_leftLabel = new QLabel(this);
    m_rightLabel = new QLabel(this);
    m_statusLabel = new QLabel(this);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);
    m_leftLabel->setAlignment(Qt::AlignCenter);
    m_rightLabel->setAlignment(Qt::AlignCenter);
    m_leftLabel->setStyleSheet(QStringLiteral("color: #e8e2c8; font-weight: 700;"));
    m_rightLabel->setStyleSheet(QStringLiteral("color: #e8e2c8; font-weight: 700;"));
    leftLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(4);
    rightLayout->setSpacing(4);
    leftLayout->addWidget(m_leftLabel);
    leftLayout->addWidget(m_leftMeter);
    rightLayout->addWidget(m_rightLabel);
    rightLayout->addWidget(m_rightMeter);
    meterLayout->setContentsMargins(18, 18, 18, 18);
    meterLayout->setSpacing(12);
    meterLayout->addLayout(leftLayout);
    meterLayout->addLayout(rightLayout);
    m_statusLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    m_statusLabel->setStyleSheet(QStringLiteral("color: #e8e2c8; font-size: 11px;"));
    baseLayout->setContentsMargins(0, 0, 0, 6);
    baseLayout->setSpacing(3);
    baseLayout->addLayout(meterLayout);
    baseLayout->addWidget(m_statusLabel);

    layout->addWidget(meterBase);
    showStartingMessage();
    retranslateUi();
}

void VuMeterPanel::setLevels(double leftDbfs, double rightDbfs)
{
    m_lastLeftDbfs = leftDbfs;
    m_lastRightDbfs = rightDbfs;
    m_hasLevels = true;
    m_leftMeter->setValue(leftDbfs);
    m_rightMeter->setValue(rightDbfs);
    m_statusLabel->setText(tr("L %1 dBFS   R %2 dBFS")
                               .arg(leftDbfs, 0, 'f', 1)
                               .arg(rightDbfs, 0, 'f', 1));
}

void VuMeterPanel::setStatusMessage(const QString &message)
{
    m_statusLabel->setText(message);
}

void VuMeterPanel::showStartingMessage()
{
    m_hasLevels = false;
    setStatusMessage(tr("Starting audio monitor"));
}

void VuMeterPanel::applySettings(const MeterSettings &settings)
{
    m_leftMeter->setNeedleWidth(settings.needleWidth);
    m_rightMeter->setNeedleWidth(settings.needleWidth);
    m_leftMeter->setPivotVisible(settings.pivotVisible);
    m_rightMeter->setPivotVisible(settings.pivotVisible);
    m_leftMeter->setNeedlePivotLowered(settings.needlePivotLowered);
    m_rightMeter->setNeedlePivotLowered(settings.needlePivotLowered);
    m_leftMeter->setPeakHoldEnabled(settings.peakHoldEnabled);
    m_rightMeter->setPeakHoldEnabled(settings.peakHoldEnabled);
    m_leftMeter->setScaleStyle(settings.scaleStyle);
    m_rightMeter->setScaleStyle(settings.scaleStyle);
}

void VuMeterPanel::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }

    QWidget::changeEvent(event);
}

void VuMeterPanel::retranslateUi()
{
    m_leftLabel->setText(tr("L"));
    m_rightLabel->setText(tr("R"));

    if (m_hasLevels) {
        setLevels(m_lastLeftDbfs, m_lastRightDbfs);
    }
}
