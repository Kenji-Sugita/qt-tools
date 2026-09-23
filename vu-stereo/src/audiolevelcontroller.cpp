// AudioLevelController の実装です。AudioLevelMonitor が取得した左右の dBFS 値へ
// 入力レベル補正を加え、VU メーターの扱う範囲に丸めて表示パネルへ渡します。
// ScreenCaptureKit など macOS 固有の処理は AudioLevelMonitor 側に閉じ込めます。

#include "audiolevelcontroller.h"

#include "audiolevelmonitor.h"
#include "vumeterpanel.h"

#include <algorithm>

namespace {

constexpr double kMinDbfs = -60.0;
constexpr double kMaxDbfs = 0.0;

double clamp(double value, double minValue, double maxValue)
{
    return std::max(minValue, std::min(value, maxValue));
}

} // namespace

AudioLevelController::AudioLevelController(VuMeterPanel *panel, QObject *parent)
    : QObject(parent),
      m_panel(panel),
      m_monitor(new AudioLevelMonitor(this))
{
    connect(m_monitor, &AudioLevelMonitor::levelsChanged, this, [this](double leftDbfs, double rightDbfs) {
        m_panel->setLevels(adjustedDbfs(leftDbfs), adjustedDbfs(rightDbfs));
        setStatus(Status::Monitoring, translatedStatus(Status::Monitoring));
    });
    connect(m_monitor, &AudioLevelMonitor::errorOccurred, this, [this](const QString &message) {
        m_errorMessage = message;
        setStatus(Status::Error, message);
    });
}

void AudioLevelController::setInputLevelDb(int inputLevelDb)
{
    m_inputLevelDb = inputLevelDb;
}

void AudioLevelController::start()
{
    m_monitor->start();
}

void AudioLevelController::stop()
{
    m_monitor->stop();
}

void AudioLevelController::restart()
{
    setStatus(Status::Restarting, translatedStatus(Status::Restarting));
    stop();
    start();
}

void AudioLevelController::retranslate()
{
    if (m_status == Status::Error) {
        setStatus(Status::Error, m_errorMessage);
        return;
    }

    setStatus(m_status, translatedStatus(m_status));
}

double AudioLevelController::adjustedDbfs(double dbfs) const
{
    return clamp(dbfs + m_inputLevelDb, kMinDbfs, kMaxDbfs);
}

void AudioLevelController::setStatus(Status status, const QString &message)
{
    m_status = status;
    if (status != Status::Monitoring) {
        m_panel->setStatusMessage(message);
    }
    emit statusMessageChanged(message);
}

QString AudioLevelController::translatedStatus(Status status) const
{
    switch (status) {
    case Status::Starting:
        return tr("Starting audio monitor");
    case Status::Monitoring:
        return tr("Audio monitor is running");
    case Status::Restarting:
        return tr("Restarting audio monitor");
    case Status::Error:
        break;
    }

    return QString();
}
