// AudioLevelController は、macOS から取得した音声レベルを VU 表示へ渡すための
// 中間層です。AudioLevelMonitor が返す dBFS 値に入力レベル補正を加え、
// 表示可能な範囲へクリップして VuMeterPanel に反映します。画面取得や描画の詳細を
// 持たないため、音声処理の調整箇所をこのクラスに集中できます。

#pragma once

#include <QObject>

class AudioLevelMonitor;
class VuMeterPanel;

class AudioLevelController final : public QObject {
    Q_OBJECT

public:
    explicit AudioLevelController(VuMeterPanel *panel, QObject *parent = nullptr);

    void setInputLevelDb(int inputLevelDb);
    void start();
    void stop();
    void restart();
    void retranslate();

signals:
    void statusMessageChanged(const QString &message);

private:
    enum class Status {
        Starting,
        Monitoring,
        Restarting,
        Error,
    };

    double adjustedDbfs(double dbfs) const;
    void setStatus(Status status, const QString &message);
    QString translatedStatus(Status status) const;

    VuMeterPanel *m_panel = nullptr;
    AudioLevelMonitor *m_monitor = nullptr;
    int m_inputLevelDb = 0;
    Status m_status = Status::Starting;
    QString m_errorMessage;
};
