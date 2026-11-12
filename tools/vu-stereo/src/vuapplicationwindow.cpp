// VuApplicationWindow の実装です。フレームレスな透明ウィンドウを作り、
// VU 表示パネル、音声レベル制御、アプリケーションメニュー、設定ダイアログを接続します。
// ここではアプリ全体の配線だけを行い、表示部品や音声処理の細部は専用クラスに任せます。

#include "vuapplicationwindow.h"

#include "audiolevelcontroller.h"
#include "settingsdialog.h"
#include "translationmanager.h"
#include "vumeterpanel.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QEvent>
#include <QGuiApplication>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMoveEvent>
#include <QPoint>
#include <QSettings>
#include <QVariant>
#include <QVBoxLayout>

#include <algorithm>

VuApplicationWindow::VuApplicationWindow(TranslationManager *translationManager, QWidget *parent)
    : DragWindow(parent),
      m_translationManager(translationManager)
{
    qApp->installEventFilter(this);
    setWindowIcon(qApp->windowIcon());
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    buildUi();
    buildMenu();

    m_audioController = new AudioLevelController(m_meterPanel, this);
    connect(m_audioController, &AudioLevelController::statusMessageChanged, this, &QWidget::setToolTip);
    connect(m_translationManager, &TranslationManager::languageChanged, this, &VuApplicationWindow::retranslateUi);
    applySettings(loadSettings());
    restoreWindowPosition();
    m_audioController->start();
    retranslateUi();
}

void VuApplicationWindow::buildUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    m_meterPanel = new VuMeterPanel(this);
    layout->addWidget(m_meterPanel);
}

void VuApplicationWindow::buildMenu()
{
    auto *menuBar = new QMenuBar(this);
    menuBar->setNativeMenuBar(true);
    m_applicationMenu = menuBar->addMenu(QString());
    m_settingsAction = new QAction(this);
    m_settingsAction->setMenuRole(QAction::PreferencesRole);
    m_settingsAction->setShortcut(QKeySequence::Preferences);
    m_applicationMenu->addAction(m_settingsAction);
    m_audioInputInfoAction = new QAction(this);
    m_applicationMenu->addAction(m_audioInputInfoAction);
    m_restartAudioAction = new QAction(this);
    m_applicationMenu->addAction(m_restartAudioAction);

    connect(m_settingsAction, &QAction::triggered, this, &VuApplicationWindow::showSettingsDialog);
    connect(m_audioInputInfoAction, &QAction::triggered, this, &VuApplicationWindow::showAudioInputInformation);
    connect(m_restartAudioAction, &QAction::triggered, this, [this] {
        m_audioController->restart();
    });
}

void VuApplicationWindow::showSettingsDialog()
{
    SettingsDialog dialog(m_settings, this);
    connect(&dialog, &SettingsDialog::settingsChanged, this, &VuApplicationWindow::applySettings);
    dialog.exec();
}

void VuApplicationWindow::showAudioInputInformation()
{
    QMessageBox::information(
        this,
        tr("Audio Input and Privacy"),
        tr("VuStereo uses macOS ScreenCaptureKit to monitor system audio levels.\n\n"
           "macOS requires Screen Recording permission for this audio monitor. After granting permission, quit and restart VuStereo.\n\n"
           "VuStereo does not record, save, or send audio. The captured audio samples are used only to calculate left and right dBFS levels for the meters."));
}

void VuApplicationWindow::applySettings(const MeterSettings &settings)
{
    const bool alwaysOnTopChanged = m_settings.alwaysOnTop != settings.alwaysOnTop;
    m_settings = settings;
    m_translationManager->setLanguage(m_settings.language);
    if (alwaysOnTopChanged) {
        const QPoint currentPosition = pos();
        const bool wasVisible = isVisible();
        setWindowFlag(Qt::WindowStaysOnTopHint, m_settings.alwaysOnTop);
        if (wasVisible) {
            show();
            move(currentPosition);
        }
    }
    m_meterPanel->applySettings(m_settings);
    if (m_audioController) {
        m_audioController->setInputLevelDb(m_settings.inputLevelDb);
    }
    saveSettings();
}

MeterSettings VuApplicationWindow::loadSettings() const
{
    QSettings settings;
    MeterSettings meterSettings;

    meterSettings.needleWidth = std::clamp(settings.value(QStringLiteral("meter/needleWidth"), meterSettings.needleWidth).toInt(), 1, 30);
    meterSettings.inputLevelDb = std::clamp(settings.value(QStringLiteral("audio/inputLevelDb"), meterSettings.inputLevelDb).toInt(), -24, 24);
    meterSettings.pivotVisible = settings.value(QStringLiteral("meter/pivotVisible"), meterSettings.pivotVisible).toBool();
    meterSettings.needlePivotLowered = settings.value(QStringLiteral("meter/needlePivotLowered"), meterSettings.needlePivotLowered).toBool();
    meterSettings.peakHoldEnabled = settings.value(QStringLiteral("meter/peakHoldEnabled"), meterSettings.peakHoldEnabled).toBool();
    meterSettings.alwaysOnTop = settings.value(QStringLiteral("window/alwaysOnTop"), meterSettings.alwaysOnTop).toBool();
    const int scaleStyle = settings.value(QStringLiteral("meter/scaleStyle"), static_cast<int>(meterSettings.scaleStyle)).toInt();
    if (scaleStyle >= static_cast<int>(MeterScaleStyle::Arc) && scaleStyle <= static_cast<int>(MeterScaleStyle::Linear)) {
        meterSettings.scaleStyle = static_cast<MeterScaleStyle>(scaleStyle);
    }

    const int language = settings.value(QStringLiteral("ui/language"), static_cast<int>(meterSettings.language)).toInt();
    if (language >= static_cast<int>(AppLanguage::System) && language <= static_cast<int>(AppLanguage::Japanese)) {
        meterSettings.language = static_cast<AppLanguage>(language);
    }

    return meterSettings;
}

void VuApplicationWindow::saveSettings() const
{
    QSettings settings;
    settings.setValue(QStringLiteral("meter/needleWidth"), m_settings.needleWidth);
    settings.setValue(QStringLiteral("audio/inputLevelDb"), m_settings.inputLevelDb);
    settings.setValue(QStringLiteral("meter/pivotVisible"), m_settings.pivotVisible);
    settings.setValue(QStringLiteral("meter/needlePivotLowered"), m_settings.needlePivotLowered);
    settings.setValue(QStringLiteral("meter/peakHoldEnabled"), m_settings.peakHoldEnabled);
    settings.setValue(QStringLiteral("window/alwaysOnTop"), m_settings.alwaysOnTop);
    settings.setValue(QStringLiteral("meter/scaleStyle"), static_cast<int>(m_settings.scaleStyle));
    settings.setValue(QStringLiteral("ui/language"), static_cast<int>(m_settings.language));
}

void VuApplicationWindow::restoreWindowPosition()
{
    QSettings settings;
    const QVariant savedPosition = settings.value(QStringLiteral("window/position"));
    if (!savedPosition.isValid()) {
        return;
    }

    const QPoint position = savedPosition.toPoint();
    m_restoringWindowPosition = true;
    move(position);
    m_restoringWindowPosition = false;
}

void VuApplicationWindow::saveWindowPosition() const
{
    if (m_restoringWindowPosition) {
        return;
    }

    QSettings settings;
    settings.setValue(QStringLiteral("window/position"), pos());
}

void VuApplicationWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }

    DragWindow::changeEvent(event);
}

void VuApplicationWindow::closeEvent(QCloseEvent *event)
{
    saveWindowPosition();
    DragWindow::closeEvent(event);
}

void VuApplicationWindow::moveEvent(QMoveEvent *event)
{
    saveWindowPosition();
    DragWindow::moveEvent(event);
}

void VuApplicationWindow::retranslateUi()
{
    setWindowTitle(m_translationManager->applicationName());
    QGuiApplication::setApplicationDisplayName(m_translationManager->applicationName());

    if (m_applicationMenu) {
        m_applicationMenu->setTitle(QGuiApplication::applicationDisplayName());
    }
    if (m_settingsAction) {
        m_settingsAction->setText(tr("Settings..."));
    }
    if (m_audioInputInfoAction) {
        m_audioInputInfoAction->setText(tr("Audio Input and Privacy..."));
    }
    if (m_restartAudioAction) {
        m_restartAudioAction->setText(tr("Restart audio monitor"));
    }
    if (m_audioController) {
        m_audioController->retranslate();
    }
}
