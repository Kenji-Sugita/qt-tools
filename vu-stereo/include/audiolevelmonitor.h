// AudioLevelMonitor は、macOS の ScreenCaptureKit を使ってシステム音声の
// 左右レベルを取得するための Qt 向けラッパーです。Objective-C++ 側の実装から
// Qt の signal として dBFS 値やエラーを通知し、UI 側が macOS 固有 API に
// 直接依存しないようにします。

#pragma once

#include <QObject>

class AudioLevelMonitor : public QObject {
    Q_OBJECT

public:
    explicit AudioLevelMonitor(QObject *parent = nullptr);
    ~AudioLevelMonitor() override;

    bool start();
    void stop();

    void emitNativeLevels(double leftDbfs, double rightDbfs);
    void emitNativeError(const QString &message);

signals:
    void levelsChanged(double leftDbfs, double rightDbfs);
    void errorOccurred(const QString &message);

private:
    class Private;
    Private *d = nullptr;
};
