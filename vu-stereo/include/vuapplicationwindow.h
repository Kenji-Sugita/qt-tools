// VuApplicationWindow は、アプリケーションの最上位ウィンドウです。透明な
// フレームレスウィンドウ、アプリケーションメニュー、設定ダイアログ、表示パネル、
// 音声レベル制御を組み立てます。個々の VU メーター描画や音声取得の低レベル処理は
// 専用クラスに任せ、このクラスは画面全体の配線だけを担当します。

#pragma once

#include "dragwindow.h"
#include "metersettings.h"

class AudioLevelController;
class QAction;
class QCloseEvent;
class QMenu;
class QMoveEvent;
class TranslationManager;
class VuMeterPanel;

class VuApplicationWindow final : public DragWindow {
    Q_OBJECT

public:
    explicit VuApplicationWindow(TranslationManager *translationManager, QWidget *parent = nullptr);

protected:
    void changeEvent(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void moveEvent(QMoveEvent *event) override;

private:
    void buildUi();
    void buildMenu();
    void showSettingsDialog();
    void showAudioInputInformation();
    void applySettings(const MeterSettings &settings);
    MeterSettings loadSettings() const;
    void saveSettings() const;
    void restoreWindowPosition();
    void saveWindowPosition() const;
    void retranslateUi();

    VuMeterPanel *m_meterPanel = nullptr;
    AudioLevelController *m_audioController = nullptr;
    TranslationManager *m_translationManager = nullptr;
    QMenu *m_applicationMenu = nullptr;
    QAction *m_settingsAction = nullptr;
    QAction *m_audioInputInfoAction = nullptr;
    QAction *m_restartAudioAction = nullptr;
    MeterSettings m_settings;
    bool m_restoringWindowPosition = false;
};
